#include "../wheels/VehicleWheels.hpp"
#include "RailVehicleServer.hpp"
#include "RailVehicleStepper.hpp"
#include <godot_cpp/classes/window.hpp>
#include "../core/VehicleComponent.hpp"

#include "../core/RailVehicle3D.hpp"
#include "../core/GameLog.hpp"
#include "../core/VehicleController.hpp"

#include <godot_cpp/classes/curve3d.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    /* Reports physics inconsistencies with push_error (see _check_movement, _check_velocity_jumps) */
    static const char *DIAGNOSTICS_SETTING = "maszyna/physics/diagnostics";

    RailVehicleServer::RailVehicleServer() {
        ProjectSettings *settings = ProjectSettings::get_singleton();
        diagnostics = settings->get_setting(DIAGNOSTICS_SETTING, false);
        catch_up_limit = settings->get_setting(CATCH_UP_LIMIT_SETTING, DEFAULT_CATCH_UP_LIMIT);
    }

    RailVehicleServer::~RailVehicleServer() {
        _set_stepping(false);
    }

    void RailVehicleServer::_bind_methods() {
        ClassDB::bind_method(D_METHOD("vehicle_create"), &RailVehicleServer::vehicle_create);
        ClassDB::bind_method(D_METHOD("vehicle_free", "vehicle"), &RailVehicleServer::vehicle_free);
        ClassDB::bind_method(D_METHOD("vehicle_exists", "vehicle"), &RailVehicleServer::vehicle_exists);
        ClassDB::bind_method(
                D_METHOD("vehicle_attach_controller", "vehicle", "controller_id"),
                &RailVehicleServer::vehicle_attach_controller);
        ClassDB::bind_method(
                D_METHOD("vehicle_set_name", "vehicle", "name"), &RailVehicleServer::vehicle_set_name);
        ClassDB::bind_method(D_METHOD("vehicle_get_name", "vehicle"), &RailVehicleServer::vehicle_get_name);
        ClassDB::bind_method(
                D_METHOD("vehicle_get_rid_by_name", "name"), &RailVehicleServer::vehicle_get_rid_by_name);
        ClassDB::bind_method(
                D_METHOD("vehicle_get_coupled", "vehicle", "end", "element"), &RailVehicleServer::vehicle_get_coupled);

        ClassDB::bind_method(
                D_METHOD("vehicle_set_track", "vehicle", "track", "track_offset", "track_direction"),
                &RailVehicleServer::vehicle_set_track);
        ClassDB::bind_method(D_METHOD("vehicle_move", "vehicle", "distance"), &RailVehicleServer::vehicle_move);
        ClassDB::bind_method(
                D_METHOD("vehicle_process_movement", "vehicle", "delta"), &RailVehicleServer::vehicle_process_movement);
        ClassDB::bind_method(D_METHOD("step", "delta"), &RailVehicleServer::step);
        ClassDB::bind_method(D_METHOD("step_frame", "delta"), &RailVehicleServer::step_frame);
        ClassDB::bind_method(D_METHOD("vehicle_get_velocity", "vehicle"), &RailVehicleServer::vehicle_get_velocity);
        ClassDB::bind_method(D_METHOD("vehicle_get_speed", "vehicle"), &RailVehicleServer::vehicle_get_speed);
        ClassDB::bind_method(
                D_METHOD("vehicle_component_get", "vehicle", "type"),
                &RailVehicleServer::vehicle_component_get);
        ClassDB::bind_method(
                D_METHOD("generic_vehicle_component_find", "vehicle", "tag"),
                &RailVehicleServer::generic_vehicle_component_find);
        ClassDB::bind_method(D_METHOD("vehicle_dump_state", "vehicle"), &RailVehicleServer::vehicle_dump_state);
        ClassDB::bind_method(
                D_METHOD("vehicle_dump_config", "vehicle"), &RailVehicleServer::vehicle_dump_config);
        ClassDB::bind_method(
                D_METHOD("vehicle_get_transform", "vehicle"), &RailVehicleServer::vehicle_get_transform);
        ClassDB::bind_method(
                D_METHOD("vehicle_get_transform_at_distance", "vehicle", "distance"),
                &RailVehicleServer::vehicle_get_transform_at_distance);
        ClassDB::bind_method(
                D_METHOD("vehicle_get_track_position", "vehicle"), &RailVehicleServer::vehicle_get_track_position);
        ClassDB::bind_method(
                D_METHOD("vehicle_get_curve", "vehicle", "bogie_pivot_spacing"),
                &RailVehicleServer::vehicle_get_curve);
        ClassDB::bind_method(
                D_METHOD("vehicle_attach_rail_vehicle", "vehicle", "rail_vehicle_id"),
                &RailVehicleServer::vehicle_attach_rail_vehicle);
        ClassDB::bind_method(
                D_METHOD("set_stepping_enabled", "enabled"), &RailVehicleServer::set_stepping_enabled);
        ClassDB::bind_method(D_METHOD("is_stepping_enabled"), &RailVehicleServer::is_stepping_enabled);
    }

    void RailVehicleServer::vehicle_attach_rail_vehicle(const RID &p_vehicle, const uint64_t p_rail_vehicle_id) {
        VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        if (placement == nullptr) {
            return;
        }
        placement->rail_vehicle_id = p_rail_vehicle_id;
    }

    uint64_t RailVehicleServer::vehicle_get_rail_vehicle(const RID &p_vehicle) const {
        const VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        return placement == nullptr ? 0 : placement->rail_vehicle_id;
    }

    void RailVehicleServer::set_stepping_enabled(const bool p_enabled) {
        stepping_enabled = p_enabled;
        _set_stepping(p_enabled && !vehicles.is_empty());
    }

    bool RailVehicleServer::is_stepping_enabled() const {
        return stepping_enabled;
    }

    /// The step runs only while the world holds a vehicle, the same shape as E3DRenderingServer's
    /// smoke tick.
    void RailVehicleServer::_set_stepping(const bool p_stepping) {
        if (stepping == p_stepping) {
            return;
        }
        SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
        if (tree == nullptr) {
            return;
        }
        stepping = p_stepping;
        // a simulation that is starting owes nothing: whatever passed while it was stopped is not
        // time it failed to integrate
        owed_seconds = 0.0;
        if (p_stepping) {
            RailVehicleStepper *stepper = memnew(RailVehicleStepper);
            stepper_id = stepper->get_instance_id();
            // internal, so a node nobody declared does not turn up in the root's children and
            // surprise whatever walks the tree
            tree->get_root()->add_child(stepper, false, Node::INTERNAL_MODE_FRONT);
            return;
        }
        if (Node *stepper = Object::cast_to<Node>(ObjectDB::get_instance(ObjectID(stepper_id))); stepper != nullptr) {
            stepper->queue_free();
        }
        stepper_id = 0;
    }

    VehicleController *RailVehicleServer::_get_controller(const VehiclePlacement &p_placement) const {
        if (p_placement.controller_id.is_null()) {
            return nullptr;
        }
        return Object::cast_to<VehicleController>(ObjectDB::get_instance(p_placement.controller_id));
    }

    RID RailVehicleServer::vehicle_create() {
        ++next_vehicle_id;
        const RID vehicle_rid = UtilityFunctions::rid_from_int64(next_vehicle_id);
        VehiclePlacement placement;
        vehicles.insert(vehicle_rid, placement);
        _set_stepping(stepping_enabled);
        return vehicle_rid;
    }

    void RailVehicleServer::vehicle_free(const RID &p_vehicle) {
        VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        if (placement == nullptr) {
            return;
        }
        diagnostics_velocity.erase(placement->controller_id);
        if (!placement->name.is_empty()) {
            vehicles_by_name.erase(placement->name);
        }
        vehicles.erase(p_vehicle);
        if (vehicles.is_empty()) {
            _set_stepping(false);
        }
    }

    bool RailVehicleServer::vehicle_exists(const RID &p_vehicle) const {
        return vehicles.has(p_vehicle);
    }

    void RailVehicleServer::vehicle_set_name(const RID &p_vehicle, const String &p_name) {
        VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        if (placement == nullptr) {
            return;
        }
        if (!placement->name.is_empty()) {
            vehicles_by_name.erase(placement->name);
        }
        placement->name = p_name;
        if (p_name.is_empty()) {
            return;
        }
        /* Two vehicles of one name is a scenery's mistake, and the second one silently taking the
         * name away from the first is how it stays invisible - an event or a console command then
         * reaches a vehicle nobody meant. */
        if (const RID *taken = vehicles_by_name.getptr(p_name); taken != nullptr && *taken != p_vehicle) {
            UtilityFunctions::push_warning(
                    vformat("Bad scenario: two vehicles named \"%s\" - the later one takes the name", p_name));
        }
        vehicles_by_name[p_name] = p_vehicle;
    }

    String RailVehicleServer::vehicle_get_name(const RID &p_vehicle) const {
        const VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        return placement != nullptr ? placement->name : String();
    }

    RID RailVehicleServer::vehicle_get_rid_by_name(const String &p_name) const {
        const RID *found = vehicles_by_name.getptr(p_name);
        return found != nullptr ? *found : RID();
    }

    TypedArray<RID> RailVehicleServer::vehicle_get_coupled(
            const RID &p_vehicle, const int p_end, const VehicleController::CouplingElement p_element) const {
        TypedArray<RID> result;
        const VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        VehicleController *first = placement != nullptr ? _get_controller(*placement) : nullptr;
        if (first == nullptr) {
            return result;
        }
        // out through p_end to the last vehicle; entering a neighbour by one end, the walk leaves it
        // by the other, whichever way round it stands
        int end = p_end;
        while (first->is_coupled_by(end, p_element)) {
            const int entered = first->get_coupled_end(end);
            first = first->get_coupled_controller(end);
            end = 1 - entered;
        }
        // and back, from that end, through every vehicle joined the same way
        VehicleController *vehicle = first;
        end = 1 - end;
        while (vehicle != nullptr) {
            result.push_back(vehicle->get_rid());
            if (!vehicle->is_coupled_by(end, p_element)) {
                break;
            }
            const int entered = vehicle->get_coupled_end(end);
            vehicle = vehicle->get_coupled_controller(end);
            end = 1 - entered;
        }
        return result;
    }

    void RailVehicleServer::vehicle_attach_controller(const RID &p_vehicle, const uint64_t p_controller_id) {
        VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        if (placement == nullptr) {
            return;
        }
        placement->controller_id = ObjectID(p_controller_id);
        if (VehicleController *controller = _get_controller(*placement); controller != nullptr) {
            controller->set_vehicle_rid(p_vehicle);
            controller->emit_position_changed_if_needed();
        }
    }

    void RailVehicleServer::vehicle_set_track(
            const RID &p_vehicle, const RID &p_track, const double p_track_offset,
            const TrackManager::Direction p_track_direction) {
        VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        if (placement == nullptr) {
            return;
        }
        TrackManager *tracks = TrackManager::get_instance();
        ERR_FAIL_NULL(tracks);
        placement->track = p_track;
        placement->track_direction = p_track_direction;
        placement->moved = true;
        placement->body_transform_valid = false;
        placement->track_is_switch = tracks->track_is_switch(p_track);
        placement->switch_track = placement->track_is_switch
                ? static_cast<TrackManager::SwitchTrack>(tracks->switch_get_active_track(p_track))
                : TrackManager::TRACK_COMMON;
        placement->track_offset =
                CLAMP(p_track_offset, 0.0, tracks->track_get_length(p_track, placement->switch_track));
        const double remaining_offset = p_track_offset - placement->track_offset;
        const double direction_sign = p_track_direction == TrackManager::DIRECTION_NORMAL ? -1.0 : 1.0;
        _move_placement(*placement, remaining_offset * direction_sign, false);
    }

    void RailVehicleServer::vehicle_move(const RID &p_vehicle, const double p_distance) {
        VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        TrackManager *tracks = TrackManager::get_instance();
        if (placement == nullptr || tracks == nullptr || !tracks->track_exists(placement->track)) {
            return;
        }
        _move_placement(*placement, p_distance, true);
        if (VehicleController *controller = _get_controller(*placement); controller != nullptr) {
            controller->emit_position_changed_if_needed();
        }
    }

    void RailVehicleServer::_move_placement(
            VehiclePlacement &p_placement, const double p_distance, const bool p_force_switch_state) {
        if (Math::is_zero_approx(p_distance)) {
            return;
        }
        TrackManager *tracks = TrackManager::get_instance();
        ERR_FAIL_NULL(tracks);
        p_placement.moved = true;

        RID current_track = p_placement.track;
        TrackManager::SwitchTrack current_switch_track = p_placement.switch_track;
        bool current_is_switch = p_placement.track_is_switch;
        // Length of the occupied branch, kept across the loop: it used to be asked twice for the
        // same track on every step.
        double current_length = tracks->track_get_length(current_track, current_switch_track);
        double current_track_offset = CLAMP(p_placement.track_offset, 0.0, current_length);
        TrackManager::Direction current_track_direction = p_placement.track_direction;
        double remaining = Math::abs(p_distance);
        const double request_sign = p_distance < 0.0 ? -1.0 : 1.0;
        Vector3 start_point;
        if (diagnostics && p_force_switch_state) {
            const Ref<Curve3D> curve = tracks->track_get_domain_curve(current_track, current_switch_track);
            if (curve.is_valid()) {
                start_point = curve->sample_baked(static_cast<real_t>(current_track_offset), false);
            }
        }
        // Convert movement relative to the vehicle front into curve offset movement. Positive sign
        // moves toward the branch end, negative toward the branch start.
        double movement_sign =
                (current_track_direction == TrackManager::DIRECTION_NORMAL ? -1.0 : 1.0) * request_sign;

        while (remaining > MOVEMENT_EPSILON) {
            // track_get_length() returns 0 for a track that is gone, so this covers track_exists()
            if (current_length <= 0.0) {
                break;
            }

            // The endpoint this movement heads toward on the occupied branch.
            const double distance_to_endpoint =
                    movement_sign > 0.0 ? current_length - current_track_offset : current_track_offset;
            int endpoint_index = 0;
            if (current_is_switch) {
                endpoint_index = movement_sign > 0.0
                        ? tracks->switch_get_branch_end_endpoint(current_track, current_switch_track)
                        : tracks->switch_get_branch_start_endpoint(current_track, current_switch_track);
                // Reversing through a switch blade on a non-active branch keeps the branch the
                // vehicle already occupies and forces the switch back.
                if (tracks->switch_get_active_track(current_track) != current_switch_track) {
                    const double requested_distance = MIN(remaining, distance_to_endpoint);
                    const double next_offset_on_track = current_track_offset + (movement_sign * requested_distance);
                    const double blade_boundary_offset =
                            tracks->switch_get_blade_boundary_offset(current_track, current_switch_track);
                    if (p_force_switch_state && current_track_offset > blade_boundary_offset &&
                        next_offset_on_track <= blade_boundary_offset) {
                        tracks->switch_set_active_track(current_track, current_switch_track);
                    }
                }
            } else {
                endpoint_index = movement_sign > 0.0 ? TrackManager::CURVE1_P2 : TrackManager::CURVE1_P1;
            }

            // Hot path: an ordinary step stays within the current branch and never asks topology
            // for the next track.
            if (remaining <= distance_to_endpoint) {
                current_track_offset += movement_sign * remaining;
                remaining = 0.0;
                break;
            }

            current_track_offset = movement_sign > 0.0 ? current_length : 0.0;
            remaining -= distance_to_endpoint;

            // Large init/debug jumps cross endpoints by following the single unambiguous
            // connection. An ambiguous node stops the movement at the endpoint.
            RID next_track;
            int next_endpoint = 0;
            if (!_motion_connection(current_track, endpoint_index, p_force_switch_state, next_track, next_endpoint)) {
                break;
            }

            current_track = next_track;
            current_is_switch = tracks->track_is_switch(current_track);
            // The connection endpoint is where the vehicle enters the next track; on a switch it
            // also says which branch it now occupies.
            if (current_is_switch) {
                current_switch_track = static_cast<TrackManager::SwitchTrack>(
                        tracks->switch_get_endpoint_branch(current_track, next_endpoint));
                const bool entered_at_end =
                        next_endpoint == tracks->switch_get_branch_end_endpoint(current_track, current_switch_track);
                movement_sign = entered_at_end ? -1.0 : 1.0;
            } else {
                current_switch_track = TrackManager::TRACK_COMMON;
                movement_sign = next_endpoint == TrackManager::CURVE1_P2 ? -1.0 : 1.0;
            }
            // Entering at the branch start means the offset grows; entering at the branch end
            // means it decreases from the branch length.
            current_length = tracks->track_get_length(current_track, current_switch_track);
            current_track_offset = movement_sign > 0.0 ? 0.0 : current_length;
            current_track_direction = movement_sign * request_sign < 0.0 ? TrackManager::DIRECTION_NORMAL
                                                                        : TrackManager::DIRECTION_REVERSED;
        }

        p_placement.track = current_track;
        p_placement.track_is_switch = current_is_switch;
        p_placement.track_offset = current_track_offset;
        p_placement.track_direction = current_track_direction;
        p_placement.switch_track = current_switch_track;
        p_placement.body_transform_valid = false;
        if (diagnostics && p_force_switch_state) {
            _check_movement(p_placement, start_point, Math::abs(p_distance) - remaining);
        }
    }

    /* Diagnostics: the vehicle must move in the world by the distance it moved along the track (a
     * step is far shorter than any curve radius, so the chord equals the arc) - a mismatch shifts
     * the simulated location and kicks the coupled vehicles. */
    void RailVehicleServer::_check_movement(
            const VehiclePlacement &p_placement, const Vector3 &p_start, const double p_moved) const {
        TrackManager *tracks = TrackManager::get_instance();
        ERR_FAIL_NULL(tracks);
        const Ref<Curve3D> curve = tracks->track_get_domain_curve(p_placement.track, p_placement.switch_track);
        if (curve.is_null()) {
            return;
        }
        const Vector3 end_point = curve->sample_baked(static_cast<real_t>(p_placement.track_offset), false);
        const double world_moved = p_start.distance_to(end_point);
        if (Math::abs(world_moved - p_moved) > DIAGNOSTICS_MOVE_TOLERANCE) {
            UtilityFunctions::push_error(vformat(
                    "RailVehicleServer: moved %.4f m in the world instead of %.4f m on track %s (offset %.3f)",
                    world_moved, p_moved, tracks->track_get_name(p_placement.track), p_placement.track_offset));
        }
    }

    /* Where the vehicle's body is. A vehicle on bogies is carried by them, so its body is the
     * chord between the two pivots and its attitude the mean of theirs - which is how the
     * original reads the running shape too (DynObj.cpp:2950-2970). The track sampled under the
     * vehicle's centre is the same thing only on straight track; on a curve it differs, and on a
     * switch it differs most. One of them has to be the answer, and it is this one. */
    Transform3D RailVehicleServer::vehicle_get_transform(const RID &p_vehicle) {
        VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        const TrackManager *tracks = TrackManager::get_instance();
        if (placement == nullptr || tracks == nullptr || !tracks->track_exists(placement->track)) {
            return Transform3D();
        }
        if (placement->body_transform_valid) {
            return placement->body_transform;
        }
        placement->body_transform = _compose_body_transform(*placement, p_vehicle);
        placement->body_transform_valid = true;
        return placement->body_transform;
    }

    Transform3D RailVehicleServer::_compose_body_transform(VehiclePlacement &p_placement, const RID &p_vehicle) {
        const VehicleWheels *wheels =
                Object::cast_to<VehicleWheels>(vehicle_component_get(p_vehicle, VehicleComponentType::COMPONENT_WHEELS));
        const double spacing = wheels != nullptr ? wheels->get_bogie_pivot_spacing() : 0.0;
        if (spacing <= 0.0) {
            // no bogies to be carried by: the track under the vehicle's own centre is all there is
            return _placement_transform(p_placement);
        }
        /* The offset is rear-relative, the same sign VehicleWheels::get_bogie_transform() uses -
         * getting it wrong flips the vehicle the moment it moves. */
        const Transform3D front = _placement_transform(_sample_placement(p_placement, -0.5 * spacing));
        const Transform3D rear = _placement_transform(_sample_placement(p_placement, 0.5 * spacing));
        Vector3 body_forward = front.origin - rear.origin;
        if (body_forward.is_zero_approx()) {
            return _placement_transform(p_placement);
        }
        body_forward.normalize();
        const Vector3 average_up = (front.basis.get_column(1) + rear.basis.get_column(1)).normalized();
        const Vector3 z_axis = -body_forward;
        const Vector3 x_axis = average_up.cross(z_axis).normalized();
        const Vector3 y_axis = z_axis.cross(x_axis).normalized();
        return Transform3D(Basis(x_axis, y_axis, z_axis).orthonormalized(), (front.origin + rear.origin) * 0.5);
    }

    Transform3D RailVehicleServer::vehicle_get_transform_at_distance(
            const RID &p_vehicle, const double p_distance) {
        const VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        const TrackManager *tracks = TrackManager::get_instance();
        if (placement == nullptr || tracks == nullptr || !tracks->track_exists(placement->track)) {
            return Transform3D();
        }
        return _placement_transform(_sample_placement(*placement, p_distance));
    }

    RailVehicleServer::VehiclePlacement RailVehicleServer::_sample_placement(
            const VehiclePlacement &p_placement, const double p_distance) {
        VehiclePlacement sampled = p_placement;
        sampled.controller_id = ObjectID();
        _move_placement(sampled, p_distance, false);
        return sampled;
    }

    Transform3D RailVehicleServer::_placement_transform(const VehiclePlacement &p_placement) const {
        TrackManager *tracks = TrackManager::get_instance();
        ERR_FAIL_NULL_V(tracks, Transform3D());
        const Ref<Resource> curve_data = tracks->track_get_curve(p_placement.track, p_placement.switch_track);
        const Ref<Curve3D> curve = tracks->track_get_domain_curve(p_placement.track, p_placement.switch_track);
        if (curve_data.is_null() || curve.is_null()) {
            return Transform3D();
        }

        const double length = curve->get_baked_length();
        const double safe_offset = CLAMP(p_placement.track_offset, 0.0, length);
        // linear on purpose: the cubic interpolation has no neighbour point at the curve ends, so
        // the position advanced there only about 60% of the offset - every vehicle lost
        // centimetres at each track joint, which kicked the consist through its couplers
        const Vector3 origin = curve->sample_baked(static_cast<real_t>(safe_offset), false);
        const double sample_distance = MIN(HEADING_SAMPLE_DISTANCE, length);
        double previous_offset = CLAMP(safe_offset - sample_distance, 0.0, length);
        double next_offset = CLAMP(safe_offset + sample_distance, 0.0, length);
        if (Math::is_equal_approx(previous_offset, next_offset)) {
            previous_offset = 0.0;
            next_offset = length;
        }
        Vector3 forward = curve->sample_baked(static_cast<real_t>(next_offset), false) -
                curve->sample_baked(static_cast<real_t>(previous_offset), false);
        if (forward.length_squared() <= 0.000001) {
            forward = Vector3(0.0, 0.0, -1.0);
        } else {
            forward = forward.normalized();
        }

        Vector3 reference_up(0.0, 1.0, 0.0);
        if (Math::abs(forward.dot(reference_up)) > 0.999) {
            reference_up = Vector3(1.0, 0.0, 0.0);
        }

        const Vector3 z_axis = -forward;
        const Vector3 x_axis = reference_up.cross(z_axis).normalized();
        const Vector3 y_axis = z_axis.cross(x_axis).normalized();
        const double roll1 = curve_data->get("roll1");
        const double roll2 = curve_data->get("roll2");
        const double roll =
                length <= 0.0 ? roll1 : Math::lerp(roll1, roll2, CLAMP(safe_offset / length, 0.0, 1.0));
        Transform3D track_transform(
                Basis(x_axis, y_axis, z_axis)
                        .orthonormalized()
                        .rotated(forward, static_cast<real_t>(Math::deg_to_rad(roll)))
                        .orthonormalized(),
                origin);
        if (p_placement.track_direction == TrackManager::DIRECTION_REVERSED) {
            track_transform.basis =
                    track_transform.basis.rotated(track_transform.basis.get_column(1).normalized(), Math_PI)
                            .orthonormalized();
        }
        track_transform.origin.y += static_cast<real_t>(tracks->get_rail_height());
        return track_transform;
    }

    double RailVehicleServer::_placement_roll(const VehiclePlacement &p_placement) const {
        TrackManager *tracks = TrackManager::get_instance();
        ERR_FAIL_NULL_V(tracks, 0.0);
        const Ref<Resource> curve_data = tracks->track_get_curve(p_placement.track, p_placement.switch_track);
        const double length = tracks->track_get_length(p_placement.track, p_placement.switch_track);
        if (curve_data.is_null() || length <= 0.0) {
            return 0.0;
        }
        const double roll1 = curve_data->get("roll1");
        const double roll2 = curve_data->get("roll2");
        return Math::lerp(roll1, roll2, CLAMP(p_placement.track_offset / length, 0.0, 1.0));
    }

    Dictionary RailVehicleServer::vehicle_get_track_position(const RID &p_vehicle) const {
        Dictionary result;
        const VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        const TrackManager *tracks = TrackManager::get_instance();
        if (placement == nullptr || tracks == nullptr || !tracks->track_exists(placement->track)) {
            result["track_rid"] = RID();
            result["along"] = 0.0;
            return result;
        }
        result["track_rid"] = placement->track;
        // moving forward decreases the offset on a track run in its normal direction
        result["along"] = placement->track_direction == TrackManager::DIRECTION_NORMAL ? -placement->track_offset
                                                                                      : placement->track_offset;
        return result;
    }

    Dictionary RailVehicleServer::vehicle_get_curve(const RID &p_vehicle, const double p_bogie_pivot_spacing) {
        Dictionary result;
        const VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        const TrackManager *tracks = TrackManager::get_instance();
        if (placement == nullptr || tracks == nullptr || !tracks->track_exists(placement->track)) {
            result["radius"] = 0.0;
            result["cant"] = 0.0;
            return result;
        }
        const VehiclePlacement front = _sample_placement(*placement, 0.5 * p_bogie_pivot_spacing);
        const VehiclePlacement rear = _sample_placement(*placement, -0.5 * p_bogie_pivot_spacing);
        const Vector3 front_forward = -_placement_transform(front).basis.get_column(2);
        const Vector3 rear_forward = -_placement_transform(rear).basis.get_column(2);
        double yaw_difference = Math::atan2(front_forward.x, front_forward.z) -
                Math::atan2(rear_forward.x, rear_forward.z);
        yaw_difference = Math::wrapf(yaw_difference, -Math_PI, Math_PI);
        double radius = 0.0;
        if (!Math::is_zero_approx(Math::sin(yaw_difference * 0.5))) {
            radius = -0.5 * p_bogie_pivot_spacing / Math::sin(yaw_difference * 0.5);
        }
        if (Math::abs(radius) > CURVE_RADIUS_LIMIT) {
            radius = 0.0;
        }
        result["radius"] = radius;
        result["cant"] = Math::deg_to_rad(0.5 * (_placement_roll(front) + _placement_roll(rear)));
        return result;
    }

    /* The single track a movement may continue onto, or false when the node is ambiguous. Entering
     * a switch from a branch side physically selects that branch, so it is forced - but only after
     * the connection has been proven unique, so an ambiguous node cannot change switch state as a
     * side effect. */
    bool RailVehicleServer::_motion_connection(
            const RID &p_track, const int p_endpoint_index, const bool p_force_switch_state, RID &p_track_out,
            int &p_endpoint_out) {
        TrackManager *tracks = TrackManager::get_instance();
        if (tracks == nullptr || !tracks->track_exists(p_track)) {
            return false;
        }
        const TypedArray<TrackEndpointRef> connections =
                tracks->track_get_endpoint_connections(p_track, p_endpoint_index);

        bool has_unique = false;
        RID unique_track;
        int unique_endpoint = 0;
        int unique_forced_switch_track = TrackManager::TRACK_COMMON;
        bool has_unique_forced_switch_track = false;

        for (int index = 0; index < connections.size(); ++index) {
            const Ref<TrackEndpointRef> raw = connections[index];
            RID candidate_track = raw->get_track_rid();
            int candidate_endpoint = raw->get_endpoint_index();
            int candidate_forced_switch_track = TrackManager::TRACK_COMMON;
            bool has_candidate_forced_switch_track = false;

            if (tracks->track_is_switch(candidate_track)) {
                const PackedInt32Array common_endpoints = tracks->switch_get_common_endpoints(candidate_track);
                if (common_endpoints.has(raw->get_endpoint_index())) {
                    // Entering from the common point follows whichever branch is active; remap the
                    // shared endpoint to the active branch endpoint at the same physical point.
                    const int active_track = tracks->switch_get_active_track(candidate_track);
                    const int branch_start = tracks->switch_get_branch_start_endpoint(candidate_track, active_track);
                    candidate_endpoint = common_endpoints.has(branch_start)
                            ? branch_start
                            : tracks->switch_get_branch_end_endpoint(candidate_track, active_track);
                } else {
                    candidate_forced_switch_track =
                            tracks->switch_get_endpoint_branch(candidate_track, raw->get_endpoint_index());
                    has_candidate_forced_switch_track = true;
                }
            }

            // Discard endpoints that cannot be used for motion on the selected route. Branch-side
            // switch entry is allowed because it will force that branch.
            bool is_motion_accessible = false;
            if (tracks->track_is_switch(candidate_track)) {
                const int active_track = tracks->switch_get_active_track(candidate_track);
                is_motion_accessible =
                        candidate_endpoint == tracks->switch_get_branch_start_endpoint(candidate_track, active_track) ||
                        candidate_endpoint == tracks->switch_get_branch_end_endpoint(candidate_track, active_track);
            } else {
                is_motion_accessible =
                        candidate_endpoint == TrackManager::CURVE1_P1 || candidate_endpoint == TrackManager::CURVE1_P2;
            }
            if (!is_motion_accessible && !has_candidate_forced_switch_track) {
                continue;
            }

            if (!has_unique) {
                has_unique = true;
                unique_track = candidate_track;
                unique_endpoint = candidate_endpoint;
                unique_forced_switch_track = candidate_forced_switch_track;
                has_unique_forced_switch_track = has_candidate_forced_switch_track;
                continue;
            }

            // More than one different usable target means the node is ambiguous. Identical
            // candidates can happen at switch common points and still count as one route.
            const bool is_same_candidate = unique_track == candidate_track &&
                    unique_endpoint == candidate_endpoint &&
                    has_unique_forced_switch_track == has_candidate_forced_switch_track &&
                    (!has_unique_forced_switch_track || unique_forced_switch_track == candidate_forced_switch_track);
            if (!is_same_candidate) {
                return false;
            }
        }

        if (!has_unique) {
            return false;
        }
        if (has_unique_forced_switch_track && p_force_switch_state &&
            tracks->switch_get_active_track(unique_track) != unique_forced_switch_track) {
            tracks->switch_set_active_track(unique_track, unique_forced_switch_track);
        }
        p_track_out = unique_track;
        p_endpoint_out = unique_endpoint;
        return true;
    }

    /* The whole step of every registered vehicle, in the phase order of the original's
     * vehicle_table::update() (DynObj.cpp:8686-8724): locations and neighbours once, then the
     * forces of all before the movement of all in each sub-iteration.
     *
     * It runs on the rendered frame, not on Godot's fixed tick, exactly like the original - on the
     * fixed tick the same step ran several times per frame to catch up and the vehicles juddered.
     * Because `process_frame` fires *after* every node's `_process`, the vehicles are handed their
     * new placement at the end of this (apply_track_placement) rather than pulling it themselves,
     * which would leave them a frame behind. */
    double RailVehicleServer::vehicle_get_velocity(const RID &p_vehicle) const {
        const VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        if (placement == nullptr) {
            return 0.0;
        }
        const VehicleController *controller = _get_controller(*placement);
        return controller != nullptr ? controller->get_velocity() : 0.0;
    }

    double RailVehicleServer::vehicle_get_speed(const RID &p_vehicle) const {
        const VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        if (placement == nullptr) {
            return 0.0;
        }
        const VehicleController *controller = _get_controller(*placement);
        return controller != nullptr ? controller->get_speed() : 0.0;
    }

    /* Everything this vehicle publishes, by name, in one Dictionary. Expensive on purpose: it is
     * what a console, a diagnostic dump or a cab full of widgets wants. Built once per physics
     * step and handed out unchanged until the next one, because nothing but a step can change it;
     * a reader after one value still takes the component that owns it and reads its property. */
    VehicleComponent *RailVehicleServer::vehicle_component_get(
            const RID &p_vehicle, const VehicleComponentType::Type p_type) const {
        const VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        if (placement == nullptr) {
            return nullptr;
        }
        const VehicleController *controller = _get_controller(*placement);
        return controller != nullptr ? controller->get_component(p_type) : nullptr;
    }

    TypedArray<VehicleComponent> RailVehicleServer::generic_vehicle_component_find(
            const RID &p_vehicle, const StringName &p_tag) const {
        const VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        if (placement == nullptr) {
            return TypedArray<VehicleComponent>();
        }
        const VehicleController *controller = _get_controller(*placement);
        return controller != nullptr ? controller->find_generic_components(p_tag)
                                     : TypedArray<VehicleComponent>();
    }

    Dictionary RailVehicleServer::vehicle_dump_state(const RID &p_vehicle) {
        VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        if (placement == nullptr) {
            return Dictionary();
        }
        VehicleController *controller = _get_controller(*placement);
        /* A command runs between two reads of the same step and changes what the vehicle says, so
         * the step alone does not decide whether the dump still describes it. A widget reads the
         * state the moment it sends a command; keyed on the step alone it read the values from
         * before the command and only caught up one command later. */
        const uint64_t command_serial = controller != nullptr ? controller->get_command_serial() : 0;
        if (placement->state_dump_step == step_serial && placement->state_dump_command_serial == command_serial) {
            return placement->state_dump;
        }
        placement->state_dump = controller != nullptr ? controller->get_state() : Dictionary();
        placement->state_dump_step = step_serial;
        placement->state_dump_command_serial = command_serial;
        return placement->state_dump;
    }

    /* The configuration this vehicle was built with, by name. Diagnostic, like the state dump -
     * a reader after one value takes the component that owns it. */
    Dictionary RailVehicleServer::vehicle_dump_config(const RID &p_vehicle) const {
        const VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        if (placement == nullptr) {
            return Dictionary();
        }
        const VehicleController *controller = _get_controller(*placement);
        return controller != nullptr ? controller->get_config() : Dictionary();
    }

    void RailVehicleServer::vehicle_process_movement(const RID &p_vehicle, const double p_delta) {
        VehiclePlacement *placement = vehicles.getptr(p_vehicle);
        if (placement == nullptr) {
            return;
        }
        const TrackManager *tracks = TrackManager::get_instance();
        if (placement->track.is_valid() && (tracks == nullptr || !tracks->track_exists(placement->track))) {
            return;
        }
        VehicleController *controller = _get_controller(*placement);
        if (controller == nullptr) {
            return;
        }
        // the controller's distance is front-relative (it mirrors the mover's V); this server's
        // track-offset math is rear-relative - negate at the boundary
        const double distance = -controller->process_movement(p_delta);
        if (Math::is_zero_approx(distance)) {
            return;
        }
        _move_placement(*placement, distance, true);
    }

    /* Simulation time is never dropped: the scenario's events and, later, multiplayer are driven
     * by it, so a simulation that quietly ran slower than the clock would drift out of both.
     *
     * A frame therefore hands over its whole delta, and what cannot be integrated now is owed and
     * paid off by the frames that follow. What "cannot be integrated now" means is one thing:
     * the sub-step must stay at or below PHYSICS_STEP, because that is what the coupler springs
     * were tuned for - integrate a stiff spring with a step several times larger and the consist
     * kicks. MAX_PHYSICS_ITERATIONS sub-steps of PHYSICS_STEP is the most a frame can honestly
     * take, so that product is the budget.
     *
     * Every frame still integrates at least its own delta, so nothing is quantised and the motion
     * stays as smooth as the frame rate - only the backlog is spread.
     *
     * Past CATCH_UP_LIMIT_SETTING the machine is not stalling, it is too slow to simulate in real
     * time, and spreading the debt would only add work to frames that are already late. There the
     * debt is taken in one step: the step is too large for the couplers and the consist visibly
     * jumps, which is the deliberate choice - a jump that can be seen beats a clock that silently
     * lies. It is logged, so it is not mistaken for a physics bug. */
    void RailVehicleServer::step_frame(const double p_delta) {
        if (!stepping_enabled || Engine::get_singleton()->is_editor_hint() || p_delta <= 0.0) {
            return;
        }
        owed_seconds += p_delta;

        double budget = MIN(owed_seconds, MAX_PHYSICS_ITERATIONS * PHYSICS_STEP);
        if (owed_seconds > catch_up_limit) {
            if (GameLog *game_log = GameLog::get_instance(); game_log != nullptr) {
                game_log->warning(vformat(
                        "RailVehicleServer: %.2f s of simulation owed, over the %.2f s catch-up "
                        "limit - taking it in one step, so the vehicles jump",
                        owed_seconds, catch_up_limit));
            }
            budget = owed_seconds;
        }
        owed_seconds -= budget;
        step(budget);
    }

    void RailVehicleServer::step(const double p_delta) {
        if (p_delta <= 0.0) {
            return;
        }
        // every dump handed out before this step describes the world as it was
        ++step_serial;

        stepped_vehicles.clear();
        stepped_controllers.clear();
        track_vehicles.clear();
        for (KeyValue<RID, VehiclePlacement> &item: vehicles) {
            VehicleController *controller = _get_controller(item.value);
            if (controller == nullptr) {
                continue;
            }
            stepped_vehicles.push_back(item.key);
            stepped_controllers.push_back(controller);
        }
        if (stepped_vehicles.is_empty()) {
            return;
        }

        for (int index = 0; index < stepped_vehicles.size(); ++index) {
            VehiclePlacement *placement = vehicles.getptr(stepped_vehicles[index]);
            VehicleController *controller = Object::cast_to<VehicleController>(stepped_controllers[index]);
            // a vehicle that has not moved keeps its location - sampling the track is not needed
            if (placement->moved) {
                controller->update_location();
                controller->emit_position_changed_if_needed();
            }
            placement->moved = false;
            track_vehicles[placement->track].push_back(stepped_vehicles[index]);
        }
        // once per update, like the original (DynObj.cpp:8691-8699)
        for (const RID &vehicle_rid: stepped_vehicles) {
            _update_neighbours(vehicle_rid, *vehicles.getptr(vehicle_rid));
        }

        const int iterations =
                CLAMP(static_cast<int>(Math::ceil(p_delta / PHYSICS_STEP)), 1, MAX_PHYSICS_ITERATIONS);
        const double sub_step = p_delta / iterations;
        for (int iteration = 0; iteration < iterations; ++iteration) {
            // the original computes the forces of every vehicle before moving any of them, so
            // coupled vehicles see a consistent state (DynObj.cpp:8199-8205)
            for (int index = 0; index < stepped_vehicles.size(); ++index) {
                Object::cast_to<VehicleController>(stepped_controllers[index])->compute_forces(sub_step);
            }
            const bool full_movement = iteration == iterations - 1;
            for (int index = 0; index < stepped_vehicles.size(); ++index) {
                VehicleController *controller = Object::cast_to<VehicleController>(stepped_controllers[index]);
                if (!controller->is_physics_active()) {
                    continue;
                }
                // the cheap integration in every sub-iteration but the last (DynObj.cpp:4086)
                if (full_movement) {
                    controller->compute_movement(sub_step);
                } else {
                    controller->compute_fast_movement(sub_step);
                }
                vehicle_process_movement(stepped_vehicles[index], sub_step);
            }
        }

        for (int index = 0; index < stepped_vehicles.size(); ++index) {
            VehicleController *controller = Object::cast_to<VehicleController>(stepped_controllers[index]);
            if (controller->is_physics_active()) {
                controller->update_state();
            }
            controller->process_components(p_delta);
        }
        if (diagnostics) {
            _check_velocity_jumps(p_delta);
        }

        // the vehicles are given the placement this step produced, so nothing renders a frame late
        for (const RID &vehicle_rid: stepped_vehicles) {
            const VehiclePlacement *placement = vehicles.getptr(vehicle_rid);
            if (placement->rail_vehicle_id == 0) {
                continue;
            }
            if (RailVehicle3D *rail_vehicle = Object::cast_to<RailVehicle3D>(
                        ObjectDB::get_instance(ObjectID(placement->rail_vehicle_id)));
                rail_vehicle != nullptr) {
                rail_vehicle->apply_track_placement();
            }
        }
    }

    /* Original engine: TDynamicObject::update_neighbours() (DynObj.cpp:7135) - a coupled end keeps
     * its coupled vehicle, a free end looks for the nearest vehicle on the route. */
    void RailVehicleServer::_clear_neighbour(
            VehicleController *p_controller, VehiclePlacement &p_placement, const int p_end) {
        if (p_placement.neighbour_cleared[p_end]) {
            return;
        }
        p_placement.neighbour_cleared[p_end] = true;
        p_controller->update_neighbour(p_end, nullptr, -1, 0.0);
    }

    void RailVehicleServer::_update_neighbours(const RID &p_vehicle, VehiclePlacement &p_placement) {
        VehicleController *controller = _get_controller(p_placement);
        const TrackManager *tracks = TrackManager::get_instance();
        if (controller == nullptr || tracks == nullptr) {
            return;
        }
        const double velocity = controller->get_velocity();
        const double scan_range = MAX(SCAN_RANGE_MINIMUM, Math::abs(velocity)) + SCAN_RANGE_MARGIN;
        // the track does not change between the two ends, so it is asked about once
        const bool on_track = tracks->track_exists(p_placement.track);
        for (int end = 0; end < 2; ++end) {
            // a coupled end is not cleared by this call: the original recomputes its coupler
            // distance on every update (DynObj.cpp:7144-7154) and CouplerForce() starts from it on
            // every step (Mover.cpp:4781) - skip it and the couplers stretch with no force
            if (controller->is_coupled(end)) {
                p_placement.neighbour_cleared[end] = false;
                controller->update_neighbour(end, nullptr, -1, 0.0);
                continue;
            }
            if (!on_track) {
                _clear_neighbour(controller, p_placement, end);
                continue;
            }
            RID found;
            int found_end = 0;
            double found_distance = 0.0;
            if (!_find_vehicle(p_vehicle, p_placement, end, scan_range, found, found_end, found_distance)) {
                _clear_neighbour(controller, p_placement, end);
                continue;
            }
            p_placement.neighbour_cleared[end] = false;
            const VehiclePlacement *other = vehicles.getptr(found);
            controller->update_neighbour(end, _get_controller(*other), found_end, found_distance);
        }
    }

    /* Original engine: TDynamicObject::find_vehicle() (DynObj.cpp:7183) - scans the route from the
     * vehicle centre towards the given end (0 front, 1 rear). */
    bool RailVehicleServer::_find_vehicle(
            const RID &p_vehicle, const VehiclePlacement &p_placement, const int p_end, const double p_scan_range,
            RID &p_found_out, int &p_found_end_out, double &p_found_distance_out) {
        TrackManager *tracks = TrackManager::get_instance();
        if (tracks == nullptr) {
            return false;
        }
        // server distances are rear-relative, see the step's own note on this
        const double request_sign = p_end == 0 ? -1.0 : 1.0;
        VehiclePlacement cursor = p_placement;
        cursor.controller_id = ObjectID();
        cursor.rail_vehicle_id = 0;
        double scanned = 0.0;
        double min_along = 0.0;

        while (scanned < p_scan_range) {
            // same conversion to the curve offset direction as in _move_placement()
            const double movement_sign =
                    (cursor.track_direction == TrackManager::DIRECTION_NORMAL ? -1.0 : 1.0) * request_sign;
            RID found_rid;
            double found_along = INFINITY;
            if (const Vector<RID> *on_track = track_vehicles.getptr(cursor.track); on_track != nullptr) {
                for (const RID &other_rid: *on_track) {
                    const VehiclePlacement *other = vehicles.getptr(other_rid);
                    if (other_rid == p_vehicle || other == nullptr || other->switch_track != cursor.switch_track) {
                        continue;
                    }
                    const double along = (other->track_offset - cursor.track_offset) * movement_sign;
                    if (along > min_along && along < found_along) {
                        found_rid = other_rid;
                        found_along = along;
                    }
                }
            }
            if (found_rid.is_valid()) {
                const VehiclePlacement *found = vehicles.getptr(found_rid);
                const double found_front_sign =
                        found->track_direction == TrackManager::DIRECTION_NORMAL ? 1.0 : -1.0;
                p_found_out = found_rid;
                p_found_end_out = Math::is_equal_approx(found_front_sign, -movement_sign) ? 0 : 1;
                p_found_distance_out = scanned + found_along;
                return true;
            }

            const double length = tracks->track_get_length(cursor.track, cursor.switch_track);
            const double distance_to_endpoint =
                    movement_sign > 0.0 ? length - cursor.track_offset : cursor.track_offset;
            const RID previous_track = cursor.track;
            _move_placement(cursor, request_sign * (distance_to_endpoint + SCAN_ENDPOINT_EPSILON), false);
            if (cursor.track == previous_track) {
                return false;
            }
            scanned += distance_to_endpoint + SCAN_ENDPOINT_EPSILON;
            // a vehicle standing right at the entry point is still ahead
            min_along = -SCAN_ENDPOINT_EPSILON;
        }
        return false;
    }

    /* Diagnostics: a velocity jump within one frame is a kick - with consistent track movement it
     * comes from the forces, typically a coupler reacting to an inconsistent vehicle position. */
    void RailVehicleServer::_check_velocity_jumps(const double p_delta) {
        for (int index = 0; index < stepped_vehicles.size(); ++index) {
            VehicleController *controller = Object::cast_to<VehicleController>(stepped_controllers[index]);
            const uint64_t id = controller->get_instance_id();
            const double velocity = controller->get_velocity();
            const double *previous = diagnostics_velocity.getptr(id);
            const double acceleration = (velocity - (previous != nullptr ? *previous : velocity)) / p_delta;
            diagnostics_velocity[id] = velocity;
            if (Math::abs(acceleration) > DIAGNOSTICS_MAX_ACCELERATION) {
                UtilityFunctions::push_error(vformat(
                        "RailVehicleServer: %s kicked, dV/dt=%.2f m/s^2 at V=%.2f m/s", controller->get_train_id(),
                        acceleration, velocity));
            }
        }
    }
} // namespace godot
