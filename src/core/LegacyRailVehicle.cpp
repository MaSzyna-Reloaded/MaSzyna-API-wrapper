#include "../core/GameLog.hpp"
#include "../core/LegacyRailVehicle.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    const char *LegacyRailVehicle::MOVER_CONFIG_CHANGED_SIGNAL = "mover_config_changed";
    const char *LegacyRailVehicle::MOVER_INITIALIZED_SIGNAL = "mover_initialized";
    const char *LegacyRailVehicle::CONFIG_CHANGED = "config_changed";

    LegacyRailVehicle::~LegacyRailVehicle() {
        _notification_before_mover_cleanup();
        if (TrackManager *track_manager = TrackManager::get_instance(); track_manager != nullptr) {
            track_manager->remove_vehicle(this);
        }
        state.clear();
        config.clear();
        internal_state.clear();
        _external_move_accumulator = 0.0;
        _movement_delta = 0.0;
        if (mover != nullptr) {
            delete mover;
            mover = nullptr;
        }
    }

    void LegacyRailVehicle::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_state"), &LegacyRailVehicle::get_state);
        ClassDB::bind_method(D_METHOD("get_config"), &LegacyRailVehicle::get_config);
        ClassDB::bind_method(D_METHOD("update_mover"), &LegacyRailVehicle::update_mover);
        ClassDB::bind_method(D_METHOD("update_state"), &LegacyRailVehicle::update_state);
        ClassDB::bind_method(D_METHOD("update_config"), &LegacyRailVehicle::update_config);
        ClassDB::bind_method(D_METHOD("set_mover_location", "position"), &LegacyRailVehicle::set_mover_location);
        ClassDB::bind_method(D_METHOD("get_mover_location"), &LegacyRailVehicle::get_mover_location);
        ClassDB::bind_method(D_METHOD("assign_track_rid", "track_rid", "track_id"), &LegacyRailVehicle::assign_track_rid);
        ClassDB::bind_method(D_METHOD("get_track_rid"), &LegacyRailVehicle::get_track_rid);
        ClassDB::bind_method(D_METHOD("assign_track", "track_id"), &LegacyRailVehicle::assign_track);
        ClassDB::bind_method(D_METHOD("get_track_id"), &LegacyRailVehicle::get_track_id);
        ClassDB::bind_method(D_METHOD("set_track_offset", "offset"), &LegacyRailVehicle::set_track_offset);
        ClassDB::bind_method(D_METHOD("get_track_offset"), &LegacyRailVehicle::get_track_offset);

        ADD_PROPERTY(
                PropertyInfo(
                        Variant::DICTIONARY, "state", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_READ_ONLY | PROPERTY_USAGE_DEFAULT),
                "", "get_state");
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::DICTIONARY, "config", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_READ_ONLY | PROPERTY_USAGE_DEFAULT),
                "", "get_config");

        BIND_PROPERTY(
                Variant::STRING, "type_name", "type_name", &LegacyRailVehicle::set_type_name,
                &LegacyRailVehicle::get_type_name, "type_name");
        BIND_PROPERTY(
                Variant::STRING, "axle_arrangement", "axle_arrangement", &LegacyRailVehicle::set_axle_arrangement,
                &LegacyRailVehicle::get_axle_arrangement, "axle_arrangement");
        BIND_PROPERTY(
                Variant::FLOAT, "drag_coefficient", "drag_coefficient", &LegacyRailVehicle::set_drag_coefficient,
                &LegacyRailVehicle::get_drag_coefficient, "drag_coefficient");
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "track_id"), "assign_track", "get_track_id");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "track_offset"), "set_track_offset", "get_track_offset");

        ADD_SIGNAL(MethodInfo(MOVER_CONFIG_CHANGED_SIGNAL));
        ADD_SIGNAL(MethodInfo(MOVER_INITIALIZED_SIGNAL));
        ADD_SIGNAL(MethodInfo(CONFIG_CHANGED));
    }

    int LegacyRailVehicle::to_mover_end(const Side side) {
        return side == Side::FRONT ? end::front : end::rear;
    }

    bool LegacyRailVehicle::can_host_commands() const {
        return false;
    }

    String LegacyRailVehicle::get_command_target_id() const {
        return "";
    }

    TMoverParameters *LegacyRailVehicle::get_mover() const {
        return mover;
    }

    void LegacyRailVehicle::set_mover_location(const Vector3 &position) {
        if (mover == nullptr) {
            return;
        }

        mover->Loc = {-position.x, position.z, position.y};
    }

    Vector3 LegacyRailVehicle::get_mover_location() const {
        if (mover == nullptr) {
            return Vector3();
        }

        return Vector3(-mover->Loc.X, mover->Loc.Z, mover->Loc.Y);
    }

    void LegacyRailVehicle::initialize_mover() {
        const auto _type_name = std::string(type_name.utf8().ptr());
        const auto name = std::string(String(get_name()).utf8().ptr());
        mover = std::make_unique<TMoverParameters>(initial_velocity, _type_name, name, cabin_number).release();

        _dirty = true;
        _dirty_prop = true;
        _update_mover_config_if_dirty();

        mover->CheckLocomotiveParameters(true, 0);

        _dirty = true;
        _dirty_prop = true;
        _update_mover_config_if_dirty();

        mover->CabActive = 1;
        mover->CabMaster = true;
        mover->CabOccupied = 1;
        mover->AutomaticCabActivation = true;
        mover->CabActivisationAuto();
        mover->CabActivisation();
        mover->switch_physics(true);

        if (front != nullptr) {
            sync_mover_coupling(front, Side::FRONT, front->back == this ? Side::BACK : Side::FRONT, true);
        }
        if (back != nullptr) {
            sync_mover_coupling(back, Side::BACK, back->front == this ? Side::FRONT : Side::BACK, true);
        }

        emit_signal(MOVER_INITIALIZED_SIGNAL);
    }

    void LegacyRailVehicle::_notification(int p_what) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        switch (p_what) {
            case NOTIFICATION_ENTER_TREE:
                set_process(true);
                break;
            case NOTIFICATION_READY:
                initialize_mover();
                update_state();
                _notification_after_mover_ready();
                break;
            case NOTIFICATION_EXIT_TREE:
                set_process(false);
                if (TrackManager *track_manager = TrackManager::get_instance(); track_manager != nullptr) {
                    track_manager->remove_vehicle(this);
                }
                break;
            default:
                break;
        }
    }

    void LegacyRailVehicle::_process(const double delta) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        _update_mover_config_if_dirty();
        _process_mover(delta);
    }

    void LegacyRailVehicle::_notification_after_mover_ready() {}

    void LegacyRailVehicle::_notification_before_mover_cleanup() {}

    void LegacyRailVehicle::_update_mover_config_if_dirty() {
        if (_dirty) {
            emit_signal(MOVER_CONFIG_CHANGED_SIGNAL);

            _dirty = false;
            _dirty_prop = true;
        }

        if (_dirty_prop) {
            update_mover();
            _dirty_prop = false;
        }
    }

    void LegacyRailVehicle::_process_mover(const double delta) {
        mover->dMoveLen = _external_move_accumulator;

        TrackManager *track_manager = TrackManager::get_instance();
        if (track_manager == nullptr || current_track_rid == RID()) {
            _movement_delta = 0.0;
            _external_move_accumulator = 0.0;
            _handle_mover_update();
            return;
        }

        const Ref<VirtualTrack> track = track_manager->get_track(current_track_rid);
        if (track.is_null()) {
            _movement_delta = 0.0;
            _external_move_accumulator = 0.0;
            _handle_mover_update();
            return;
        }

        const Vector3 world_position = track->get_world_position(current_track_offset);
        set_mover_location(world_position);
        mover->RunningShape = track->get_shape();
        mover->RunningTrack = track->get_track_param();
        _sync_mover_neighbours();
        mover->ComputeTotalForce(delta);
        _movement_delta = mover->ComputeMovement(
                delta, delta, mover->RunningShape, mover->RunningTrack, mover->RunningTraction, mover->Loc, mover->Rot);
        _external_move_accumulator = 0.0;
        debug_tick_counter += 1;
        if ((debug_tick_counter % 20) == 0 &&
            (front != nullptr || back != nullptr || mover->Couplers[end::front].Connected != nullptr ||
             mover->Couplers[end::rear].Connected != nullptr)) {}
        _handle_mover_update();
    }

    void LegacyRailVehicle::_do_update_internal_mover(TMoverParameters *mover) const {
        mover->Mass = get_mass();
        mover->Vmax = get_max_velocity();
        mover->Dim.L = get_length();
        mover->Dim.W = get_width();
        mover->Dim.H = get_height();
        mover->Cx = get_drag_coefficient();

        mover->ComputeMass();
        mover->NPoweredAxles = Maszyna::s2NPW(axle_arrangement.ascii().get_data());
        mover->NAxles = mover->NPoweredAxles + Maszyna::s2NNW(axle_arrangement.ascii().get_data());
    }

    void LegacyRailVehicle::_do_fetch_config_from_mover(const TMoverParameters *mover, Dictionary &config) const {
        config["axles_powered_count"] = mover->NPoweredAxles;
        config["axles_count"] = mover->NAxles;
        config["length"] = mover->Dim.L;
        config["width"] = mover->Dim.W;
        config["height"] = mover->Dim.H;
        config["drag_coefficient"] = mover->Cx;
    }

    void LegacyRailVehicle::update_mover() {
        if (TMoverParameters *current_mover = get_mover(); current_mover != nullptr) {
            _do_update_internal_mover(current_mover);
            Dictionary new_config;
            _do_fetch_config_from_mover(current_mover, new_config);
            update_config(new_config);
            current_mover->CheckLocomotiveParameters(true, 0);
        } else {
            UtilityFunctions::push_warning("LegacyRailVehicle::update_mover() failed: internal mover not initialized");
        }
    }

    Dictionary LegacyRailVehicle::get_mover_state() {
        if (TMoverParameters *current_mover = get_mover(); current_mover != nullptr) {
            _do_fetch_state_from_mover(current_mover, internal_state);
        } else {
            UtilityFunctions::push_warning(
                    "LegacyRailVehicle::get_mover_state() failed: internal mover not initialized");
        }
        return internal_state;
    }

    void LegacyRailVehicle::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        internal_state["mass_total"] = mover->TotalMass;
        internal_state["velocity"] = mover->V;
        internal_state["speed"] = mover->Vel;
        internal_state["total_distance"] = mover->DistCounter;
        internal_state["direction"] = mover->DirActive;
        internal_state["damage_flag"] = mover->DamageFlag;
        internal_state["movement_delta"] = _movement_delta;
        internal_state["track_rid"] = current_track_rid;
        internal_state["track_id"] = current_track_id;
        internal_state["track_offset"] = current_track_offset;
        internal_state["front_neighbour_present"] = mover->Neighbours[end::front].vehicle != nullptr;
        internal_state["front_neighbour_end"] = mover->Neighbours[end::front].vehicle_end;
        internal_state["front_neighbour_distance"] = mover->Neighbours[end::front].distance;
        internal_state["rear_neighbour_present"] = mover->Neighbours[end::rear].vehicle != nullptr;
        internal_state["rear_neighbour_end"] = mover->Neighbours[end::rear].vehicle_end;
        internal_state["rear_neighbour_distance"] = mover->Neighbours[end::rear].distance;
    }

    Dictionary LegacyRailVehicle::get_config() const {
        return config;
    }

    void LegacyRailVehicle::update_config(const Dictionary &p_config) {
        config.merge(p_config, true);
        emit_signal(CONFIG_CHANGED);
    }

    void LegacyRailVehicle::update_state() {
        _handle_mover_update();
    }

    void LegacyRailVehicle::_handle_mover_update() {
        state.merge(get_mover_state(), true);
    }

    void LegacyRailVehicle::_sync_mover_neighbours() {
        if (mover == nullptr) {
            return;
        }

        for (int end_index = end::front; end_index <= end::rear; ++end_index) {
            auto &neighbour = mover->Neighbours[end_index];
            auto &coupler = mover->Couplers[end_index];

            if (coupler.Connected == nullptr) {
                neighbour = neighbour_data();

                TrackManager *track_manager = TrackManager::get_instance();
                if (track_manager == nullptr || current_track_rid == RID()) {
                    continue;
                }

                double track_distance = 0.0;
                int other_end = -1;
                LegacyRailVehicle *other_vehicle = end_index == end::front
                        ? track_manager->get_nearest_vehicle_ahead(this, &track_distance, &other_end)
                        : track_manager->get_nearest_vehicle_behind(this, &track_distance, &other_end);
                if (other_vehicle == nullptr) {
                    continue;
                }

                auto *other_legacy = dynamic_cast<LegacyRailVehicle *>(other_vehicle);
                if (other_legacy == nullptr || other_legacy->get_mover() == nullptr) {
                    continue;
                }

                neighbour.vehicle = other_legacy->get_mover();
                neighbour.vehicle_end = other_end;
                neighbour.distance = static_cast<float>(track_distance - 0.5 * (mover->Dim.L + neighbour.vehicle->Dim.L));

                if (neighbour.vehicle_end >= end::front && neighbour.vehicle_end <= end::rear) {
                    auto &other_coupler = neighbour.vehicle->Couplers[neighbour.vehicle_end];
                    neighbour.distance -= static_cast<float>(coupler.adapter_length + other_coupler.adapter_length);
                }
                continue;
            }

            neighbour.vehicle = coupler.Connected;
            neighbour.vehicle_end = coupler.ConnectedNr;
            neighbour.distance = static_cast<float>(TMoverParameters::CouplerDist(mover, coupler.Connected));

            if (neighbour.vehicle_end >= end::front && neighbour.vehicle_end <= end::rear) {
                auto &other_coupler = coupler.Connected->Couplers[neighbour.vehicle_end];
                neighbour.distance -= static_cast<float>(coupler.adapter_length + other_coupler.adapter_length);
            }
        }
    }

    Dictionary LegacyRailVehicle::get_state() {
        return state;
    }

    Dictionary &LegacyRailVehicle::_get_state_internal() {
        return state;
    }

    void LegacyRailVehicle::sync_mover_coupling(
            RailVehicle *other_vehicle, const Side self_side, const Side other_side, const bool attach) {
        auto *other_legacy = dynamic_cast<LegacyRailVehicle *>(other_vehicle);

        if (!other_legacy) {
            GameLog::get_instance()->error("Cannot sync mover coupling. Other vehicle is not a LegacyRailVehicle.");
            return;
        }
        const int self_end = to_mover_end(self_side);
        const int other_end = to_mover_end(other_side);
        auto &self_coupler = mover->Couplers[self_end];
        auto &other_coupler = other_legacy->get_mover()->Couplers[other_end];

        if (attach) {
            int coupling_type = self_coupler.AutomaticCouplingFlag & other_coupler.AutomaticCouplingFlag;
            if (self_coupler.control_type != other_coupler.control_type) {
                coupling_type &= ~coupling::control;
            }
            if (coupling_type == coupling::faux) {
                coupling_type = self_coupler.AllowedFlag & other_coupler.AllowedFlag;
            }

            const bool attached =
                    mover->Attach(self_end, other_end, other_legacy->get_mover(), coupling_type, true, false);
        } else if (self_coupler.Connected != nullptr) {
            mover->Dettach(self_end);
        }
    }

    void LegacyRailVehicle::assign_track_rid(const RID &track_rid, const String &track_id) {
        current_track_rid = track_rid;
        current_track_id = track_id;

        if (TrackManager *track_manager = TrackManager::get_instance(); track_manager != nullptr) {
            if (track_rid == RID()) {
                track_manager->remove_vehicle(this);
            } else {
                track_manager->register_vehicle(this, track_rid, current_track_id, current_track_offset);
            }
        }
    }

    RID LegacyRailVehicle::get_track_rid() const {
        return current_track_rid;
    }

    void LegacyRailVehicle::assign_track(const String &track_id) {
        if (track_id.is_empty()) {
            assign_track_rid(RID(), "");
            return;
        }

        if (TrackManager *track_manager = TrackManager::get_instance(); track_manager != nullptr) {
            const Ref<VirtualTrack> track = track_manager->get_track_by_name(track_id);
            assign_track_rid(track.is_null() ? RID() : track->get_rid(), track_id);
            return;
        }

        current_track_rid = RID();
        current_track_id = track_id;
    }

    String LegacyRailVehicle::get_track_id() const {
        return current_track_id;
    }

    void LegacyRailVehicle::set_track_offset(const double offset) {
        current_track_offset = offset;

        if (TrackManager *track_manager = TrackManager::get_instance(); track_manager != nullptr && current_track_rid != RID()) {
            track_manager->register_vehicle(this, current_track_rid, current_track_id, current_track_offset);
        }
    }

    double LegacyRailVehicle::get_track_offset() const {
        return current_track_offset;
    }

    void LegacyRailVehicle::_on_coupled(RailVehicle *other_vehicle, const Side self_side, const Side other_side) {
        RailVehicle::_on_coupled(other_vehicle, self_side, other_side);
        sync_mover_coupling(other_vehicle, self_side, other_side, true);
    }

    void LegacyRailVehicle::_on_uncoupled(RailVehicle *other_vehicle, const Side self_side, const Side other_side) {
        RailVehicle::_on_uncoupled(other_vehicle, self_side, other_side);
        sync_mover_coupling(other_vehicle, self_side, other_side, false);
    }
} // namespace godot
