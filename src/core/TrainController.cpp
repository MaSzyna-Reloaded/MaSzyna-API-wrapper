#include "../core/GameLog.hpp"
#include "../core/TrainController.hpp"
#include "TrackManager.hpp"
#include "TrainPart.hpp"
#include "TrainSet.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <cmath>
#include <memory>

namespace godot {
    const char *TrainController::MOVER_CONFIG_CHANGED_SIGNAL = "mover_config_changed";
    const char *TrainController::MOVER_INITIALIZED_SIGNAL = "mover_initialized";
    const char *TrainController::CONFIG_CHANGED = "config_changed";
    const char *TrainController::POWER_CHANGED_SIGNAL = "power_changed";
    const char *TrainController::COMMAND_RECEIVED = "command_received";
    const char *TrainController::RADIO_TOGGLED = "radio_toggled";
    const char *TrainController::RADIO_CHANNEL_CHANGED = "radio_channel_changed";
    const char *TrainController::DERAILED_SIGNAL = "derailed";

    TrainController::TrainController() {
        trainset.instantiate();
        trainset->_init(this);
    }

    TrainController::~TrainController() {
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

    void TrainController::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_state"), &TrainController::get_state);
        ClassDB::bind_method(D_METHOD("get_config"), &TrainController::get_config);
        ClassDB::bind_method(D_METHOD("update_mover"), &TrainController::update_mover);
        ClassDB::bind_method(D_METHOD("update_state"), &TrainController::update_state);
        ClassDB::bind_method(D_METHOD("update_config"), &TrainController::update_config);
        ClassDB::bind_method(D_METHOD("set_mover_location", "position"), &TrainController::set_mover_location);
        ClassDB::bind_method(D_METHOD("get_mover_location"), &TrainController::get_mover_location);
        ClassDB::bind_method(D_METHOD("assign_track_rid", "track_rid", "track_id"), &TrainController::assign_track_rid);
        ClassDB::bind_method(D_METHOD("get_track_rid"), &TrainController::get_track_rid);
        ClassDB::bind_method(D_METHOD("assign_track", "track_id"), &TrainController::assign_track);
        ClassDB::bind_method(D_METHOD("get_track_id"), &TrainController::get_track_id);
        ClassDB::bind_method(D_METHOD("set_track_offset", "offset"), &TrainController::set_track_offset);
        ClassDB::bind_method(D_METHOD("get_track_offset"), &TrainController::get_track_offset);

        ClassDB::bind_method(D_METHOD("couple", "other_vehicle", "self_side", "other_side"), &TrainController::couple);
        ClassDB::bind_method(D_METHOD("couple_front", "other_vehicle", "other_side"), &TrainController::couple_front);
        ClassDB::bind_method(D_METHOD("couple_back", "other_vehicle", "other_side"), &TrainController::couple_back);
        ClassDB::bind_method(D_METHOD("get_front_vehicle"), &TrainController::get_front_vehicle);
        ClassDB::bind_method(D_METHOD("get_back_vehicle"), &TrainController::get_back_vehicle);
        ClassDB::bind_method(D_METHOD("get_rail_vehicle_modules"), &TrainController::get_rail_vehicle_modules);
        ClassDB::bind_method(D_METHOD("decouple", "relative_index"), &TrainController::decouple);
        ClassDB::bind_method(D_METHOD("uncouple_front"), &TrainController::uncouple_front);
        ClassDB::bind_method(D_METHOD("uncouple_back"), &TrainController::uncouple_back);
        ClassDB::bind_method(D_METHOD("get_trainset"), &TrainController::get_trainset);
        ClassDB::bind_method(D_METHOD("_to_string"), &TrainController::_to_string);
        ClassDB::bind_method(D_METHOD("set_mass", "value"), &TrainController::set_mass);
        ClassDB::bind_method(D_METHOD("get_mass"), &TrainController::get_mass);
        ClassDB::bind_method(D_METHOD("set_max_velocity", "value"), &TrainController::set_max_velocity);
        ClassDB::bind_method(D_METHOD("get_max_velocity"), &TrainController::get_max_velocity);
        ClassDB::bind_method(D_METHOD("set_length", "value"), &TrainController::set_length);
        ClassDB::bind_method(D_METHOD("get_length"), &TrainController::get_length);
        ClassDB::bind_method(D_METHOD("set_width", "value"), &TrainController::set_width);
        ClassDB::bind_method(D_METHOD("get_width"), &TrainController::get_width);
        ClassDB::bind_method(D_METHOD("set_height", "value"), &TrainController::set_height);
        ClassDB::bind_method(D_METHOD("get_height"), &TrainController::get_height);

        ClassDB::bind_method(D_METHOD("get_supported_commands"), &TrainController::get_supported_commands);
        ClassDB::bind_method(D_METHOD("battery", "enabled"), &TrainController::battery);
        ClassDB::bind_method(D_METHOD("main_controller_increase", "step"), &TrainController::main_controller_increase, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("main_controller_decrease", "step"), &TrainController::main_controller_decrease, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("direction_increase"), &TrainController::direction_increase);
        ClassDB::bind_method(D_METHOD("direction_decrease"), &TrainController::direction_decrease);
        ClassDB::bind_method(D_METHOD("radio", "enabled"), &TrainController::radio);
        ClassDB::bind_method(D_METHOD("radio_channel_set", "channel"), &TrainController::radio_channel_set);
        ClassDB::bind_method(D_METHOD("radio_channel_increase", "step"), &TrainController::radio_channel_increase, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("radio_channel_decrease", "step"), &TrainController::radio_channel_decrease, DEFVAL(1));

        ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "state", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_READ_ONLY | PROPERTY_USAGE_DEFAULT), "", "get_state");
        ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "config", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_READ_ONLY | PROPERTY_USAGE_DEFAULT), "", "get_config");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mass"), "set_mass", "get_mass");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_velocity"), "set_max_velocity", "get_max_velocity");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "length"), "set_length", "get_length");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width"), "set_width", "get_width");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height"), "set_height", "get_height");

        BIND_PROPERTY(Variant::STRING, "type_name", "type_name", &TrainController::set_type_name, &TrainController::get_type_name, "type_name");
        BIND_PROPERTY(Variant::STRING, "axle_arrangement", "axle_arrangement", &TrainController::set_axle_arrangement, &TrainController::get_axle_arrangement, "axle_arrangement");
        BIND_PROPERTY(Variant::FLOAT, "drag_coefficient", "drag_coefficient", &TrainController::set_drag_coefficient, &TrainController::get_drag_coefficient, "drag_coefficient");
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "track_id"), "assign_track", "get_track_id");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "track_offset"), "set_track_offset", "get_track_offset");

        BIND_PROPERTY(Variant::FLOAT, "power", "power", &TrainController::set_power, &TrainController::get_power, "power");
        BIND_PROPERTY_W_HINT(Variant::FLOAT, "battery_voltage", "battery_voltage", &TrainController::set_battery_voltage, &TrainController::get_battery_voltage, "battery_voltage", PROPERTY_HINT_RANGE, "0,500,1");
        BIND_PROPERTY(Variant::INT, "radio_channel_min", "radio_channel/min", &TrainController::set_radio_channel_min, &TrainController::get_radio_channel_min, "radio_channel_min");
        BIND_PROPERTY(Variant::INT, "radio_channel_max", "radio_channel/max", &TrainController::set_radio_channel_max, &TrainController::get_radio_channel_max, "radio_channel_max");

        ADD_SIGNAL(MethodInfo(MOVER_CONFIG_CHANGED_SIGNAL));
        ADD_SIGNAL(MethodInfo(MOVER_INITIALIZED_SIGNAL));
        ADD_SIGNAL(MethodInfo(CONFIG_CHANGED));
        ADD_SIGNAL(MethodInfo(POWER_CHANGED_SIGNAL, PropertyInfo(Variant::BOOL, "is_powered")));
        ADD_SIGNAL(MethodInfo(RADIO_TOGGLED, PropertyInfo(Variant::BOOL, "is_enabled")));
        ADD_SIGNAL(MethodInfo(RADIO_CHANNEL_CHANGED, PropertyInfo(Variant::INT, "channel")));
        ADD_SIGNAL(MethodInfo(COMMAND_RECEIVED, PropertyInfo(Variant::STRING, "command"), PropertyInfo(Variant::NIL, "p1"), PropertyInfo(Variant::NIL, "p2")));
        ADD_SIGNAL(MethodInfo(DERAILED_SIGNAL, PropertyInfo(Variant::INT, "damage_flag"), PropertyInfo(Variant::INT, "derail_reason")));

        BIND_ENUM_CONSTANT(FRONT);
        BIND_ENUM_CONSTANT(BACK);
        BIND_ENUM_CONSTANT(POWER_SOURCE_NOT_DEFINED);
        BIND_ENUM_CONSTANT(POWER_SOURCE_INTERNAL);
        BIND_ENUM_CONSTANT(POWER_SOURCE_TRANSDUCER);
        BIND_ENUM_CONSTANT(POWER_SOURCE_GENERATOR);
        BIND_ENUM_CONSTANT(POWER_SOURCE_ACCUMULATOR);
        BIND_ENUM_CONSTANT(POWER_SOURCE_CURRENTCOLLECTOR);
        BIND_ENUM_CONSTANT(POWER_SOURCE_POWERCABLE);
        BIND_ENUM_CONSTANT(POWER_SOURCE_HEATER);
        BIND_ENUM_CONSTANT(POWER_SOURCE_MAIN);
        BIND_ENUM_CONSTANT(POWER_TYPE_NONE);
        BIND_ENUM_CONSTANT(POWER_TYPE_BIO);
        BIND_ENUM_CONSTANT(POWER_TYPE_MECH);
        BIND_ENUM_CONSTANT(POWER_TYPE_ELECTRIC);
        BIND_ENUM_CONSTANT(POWER_TYPE_STEAM);
    }

    String TrainController::_to_string() const {
        return String("<TrainController({0})>").format(Array::make(get_name()));
    }

    int TrainController::to_mover_end(const Side side) {
        return side == Side::FRONT ? end::front : end::rear;
    }

    TMoverParameters *TrainController::get_mover() const {
        return mover;
    }

    void TrainController::set_mover_location(const Vector3 &position) {
        if (mover == nullptr) {
            return;
        }

        mover->Loc = {-position.x, position.z, position.y};
    }

    Vector3 TrainController::get_mover_location() const {
        if (mover == nullptr) {
            return Vector3();
        }

        return Vector3(-mover->Loc.X, mover->Loc.Z, mover->Loc.Y);
    }

    void TrainController::initialize_mover() {
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

    void TrainController::_notification(int p_what) {
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

    void TrainController::_process(const double delta) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        _update_mover_config_if_dirty();
        if (mover != nullptr) {
            _process_mover(delta);
        }
        refresh_runtime_signals();
    }

    void TrainController::_notification_after_mover_ready() {
        refresh_runtime_signals();
    }

    void TrainController::_notification_before_mover_cleanup() {}

    void TrainController::_update_mover_config_if_dirty() {
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

    bool TrainController::_resolve_track_state(
            const double requested_offset, RID &resolved_track_rid, String &resolved_track_id, double &resolved_offset,
            Vector3 *resolved_position, Vector3 *resolved_axis, Maszyna::TTrackShape *resolved_shape) const {
        TrackManager *track_manager = TrackManager::get_instance();
        if (track_manager == nullptr || current_track_rid == RID()) {
            return false;
        }

        return track_manager->resolve_track_state(
                current_track_rid, requested_offset, resolved_track_rid, resolved_track_id, resolved_offset,
                resolved_position, resolved_axis, resolved_shape);
    }

    bool TrainController::_apply_resolved_track_state(
            const RID &resolved_track_rid, const String &resolved_track_id, const double resolved_offset) {
        if (resolved_track_rid == RID()) {
            return false;
        }

        const bool track_changed = resolved_track_rid != current_track_rid || resolved_track_id != current_track_id;
        current_track_offset = resolved_offset;

        if (track_changed) {
            assign_track_rid(resolved_track_rid, resolved_track_id);
        } else if (TrackManager *track_manager = TrackManager::get_instance(); track_manager != nullptr) {
            track_manager->register_vehicle(this, current_track_rid, current_track_id, current_track_offset);
        }

        return true;
    }

    void TrainController::_process_mover(const double delta) {
        if (mover == nullptr) {
            return;
        }

        mover->dMoveLen = _external_move_accumulator;

        TrackManager *track_manager = TrackManager::get_instance();
        if (track_manager == nullptr || current_track_rid == RID()) {
            _movement_delta = 0.0;
            _external_move_accumulator = 0.0;
            _handle_mover_update();
            return;
        }

        RID resolved_track_rid;
        String resolved_track_id;
        double resolved_track_offset = 0.0;
        Vector3 world_position;
        Maszyna::TTrackShape running_shape;
        if (!_resolve_track_state(
                    current_track_offset, resolved_track_rid, resolved_track_id, resolved_track_offset, &world_position,
                    nullptr, &running_shape)) {
            _movement_delta = 0.0;
            _external_move_accumulator = 0.0;
            _handle_mover_update();
            return;
        }

        if (!_apply_resolved_track_state(resolved_track_rid, resolved_track_id, resolved_track_offset)) {
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

        set_mover_location(world_position);
        mover->RunningShape = running_shape;
        mover->RunningTrack = track->get_track_param();
        _sync_mover_neighbours();
        mover->ComputeTotalForce(delta);
        _movement_delta = mover->ComputeMovement(
                delta, delta, mover->RunningShape, mover->RunningTrack, mover->RunningTraction, mover->Loc, mover->Rot);
        _external_move_accumulator = 0.0;
        RID next_track_rid;
        String next_track_id;
        double next_track_offset = 0.0;
        Vector3 next_world_position;
        if (_resolve_track_state(
                    current_track_offset + _movement_delta, next_track_rid, next_track_id, next_track_offset,
                    &next_world_position)) {
            _apply_resolved_track_state(next_track_rid, next_track_id, next_track_offset);
            set_mover_location(next_world_position);
        }
        debug_tick_counter += 1;
        _handle_mover_update();
    }

    void TrainController::_do_update_internal_mover(TMoverParameters *mover) const {
        mover->Mass = get_mass();
        mover->Vmax = get_max_velocity();
        mover->Dim.L = get_length();
        mover->Dim.W = get_width();
        mover->Dim.H = get_height();
        mover->Cx = get_drag_coefficient();
        mover->Power = power;
        mover->BatteryVoltage = battery_voltage;
        mover->NominalBatteryVoltage = static_cast<float>(battery_voltage);

        mover->ComputeMass();
        mover->NPoweredAxles = Maszyna::s2NPW(axle_arrangement.ascii().get_data());
        mover->NAxles = mover->NPoweredAxles + Maszyna::s2NNW(axle_arrangement.ascii().get_data());
    }

    void TrainController::_do_fetch_config_from_mover(const TMoverParameters *mover, Dictionary &config) const {
        config["axles_powered_count"] = mover->NPoweredAxles;
        config["axles_count"] = mover->NAxles;
        config["length"] = mover->Dim.L;
        config["width"] = mover->Dim.W;
        config["height"] = mover->Dim.H;
        config["drag_coefficient"] = mover->Cx;
    }

    void TrainController::update_mover() {
        if (TMoverParameters *current_mover = get_mover(); current_mover != nullptr) {
            _do_update_internal_mover(current_mover);
            Dictionary new_config;
            _do_fetch_config_from_mover(current_mover, new_config);
            update_config(new_config);
            current_mover->CheckLocomotiveParameters(true, 0);
        } else {
            UtilityFunctions::push_warning("TrainController::update_mover() failed: internal mover not initialized");
        }
    }

    Dictionary TrainController::get_mover_state() {
        if (TMoverParameters *current_mover = get_mover(); current_mover != nullptr) {
            _do_fetch_state_from_mover(current_mover, internal_state);
        } else {
            UtilityFunctions::push_warning("TrainController::get_mover_state() failed: internal mover not initialized");
        }
        return internal_state;
    }

    void TrainController::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        const auto &front_coupler = mover->Couplers[end::front];
        const auto &rear_coupler = mover->Couplers[end::rear];
        const auto compute_relative_speed_kmh = [](TMoverParameters *self, neighbour_data &neighbour) -> double {
            if (neighbour.vehicle == nullptr) {
                return 0.0;
            }

            const auto selfvx = std::cos(self->Rot.Rz) * self->V;
            const auto selfvy = std::sin(self->Rot.Rz) * self->V;
            const auto othervx = std::cos(neighbour.vehicle->Rot.Rz) * neighbour.vehicle->V;
            const auto othervy = std::sin(neighbour.vehicle->Rot.Rz) * neighbour.vehicle->V;
            return std::hypot(selfvx - othervx, selfvy - othervy) * 3.6;
        };

        state["mass_total"] = mover->TotalMass;
        state["velocity"] = mover->V;
        state["speed"] = mover->Vel;
        state["total_distance"] = mover->DistCounter;
        state["direction"] = mover->DirActive;
        state["damage_flag"] = mover->DamageFlag;
        state["is_derailed"] = (mover->DamageFlag & dtrain_out) != 0;
        state["derail_reason"] = mover->DerailReason;
        state["crash_damage_enabled"] = true;
        state["movement_delta"] = _movement_delta;
        state["track_rid"] = current_track_rid;
        state["track_id"] = current_track_id;
        state["track_offset"] = current_track_offset;
        state["front_coupler_connected"] = front_coupler.Connected != nullptr;
        state["front_coupler_connected_number"] = front_coupler.ConnectedNr;
        state["front_coupler_flag"] = front_coupler.CouplingFlag;
        state["front_coupler_automatic_flag"] = front_coupler.AutomaticCouplingFlag;
        state["front_coupler_allowed_flag"] = front_coupler.AllowedFlag;
        state["front_coupler_distance"] = front_coupler.Dist;
        state["rear_coupler_connected"] = rear_coupler.Connected != nullptr;
        state["rear_coupler_connected_number"] = rear_coupler.ConnectedNr;
        state["rear_coupler_flag"] = rear_coupler.CouplingFlag;
        state["rear_coupler_automatic_flag"] = rear_coupler.AutomaticCouplingFlag;
        state["rear_coupler_allowed_flag"] = rear_coupler.AllowedFlag;
        state["rear_coupler_distance"] = rear_coupler.Dist;
        state["front_neighbour_present"] = mover->Neighbours[end::front].vehicle != nullptr;
        state["front_neighbour_end"] = mover->Neighbours[end::front].vehicle_end;
        state["front_neighbour_distance"] = mover->Neighbours[end::front].distance;
        state["front_neighbour_damage_flag"] = mover->Neighbours[end::front].vehicle != nullptr
                ? mover->Neighbours[end::front].vehicle->DamageFlag
                : 0;
        state["front_relative_speed_kmh"] = compute_relative_speed_kmh(mover, mover->Neighbours[end::front]);
        state["front_derail_threshold_self_kmh"] = mover->Neighbours[end::front].vehicle != nullptr &&
                        mover->Neighbours[end::front].vehicle->TotalMass > 0.0
                ? 15.0 * (mover->TotalMass / mover->Neighbours[end::front].vehicle->TotalMass)
                : 0.0;
        state["front_derail_threshold_other_kmh"] = mover->Neighbours[end::front].vehicle != nullptr &&
                        mover->TotalMass > 0.0
                ? 15.0 * (mover->Neighbours[end::front].vehicle->TotalMass / mover->TotalMass)
                : 0.0;
        state["rear_neighbour_present"] = mover->Neighbours[end::rear].vehicle != nullptr;
        state["rear_neighbour_end"] = mover->Neighbours[end::rear].vehicle_end;
        state["rear_neighbour_distance"] = mover->Neighbours[end::rear].distance;
        state["rear_neighbour_damage_flag"] = mover->Neighbours[end::rear].vehicle != nullptr
                ? mover->Neighbours[end::rear].vehicle->DamageFlag
                : 0;
        state["rear_relative_speed_kmh"] = compute_relative_speed_kmh(mover, mover->Neighbours[end::rear]);
        state["rear_derail_threshold_self_kmh"] = mover->Neighbours[end::rear].vehicle != nullptr &&
                        mover->Neighbours[end::rear].vehicle->TotalMass > 0.0
                ? 15.0 * (mover->TotalMass / mover->Neighbours[end::rear].vehicle->TotalMass)
                : 0.0;
        state["rear_derail_threshold_other_kmh"] = mover->Neighbours[end::rear].vehicle != nullptr &&
                        mover->TotalMass > 0.0
                ? 15.0 * (mover->Neighbours[end::rear].vehicle->TotalMass / mover->TotalMass)
                : 0.0;

        state["cabin"] = mover->CabActive;
        state["cabin_controleable"] = mover->IsCabMaster();
        state["cabin_occupied"] = mover->CabOccupied;
        state["battery_enabled"] = mover->Battery;
        state["battery_voltage"] = mover->BatteryVoltage;
        state["power24_voltage"] = mover->Power24vVoltage;
        state["power24_available"] = mover->Power24vIsAvailable;
        state["power110_available"] = mover->Power110vIsAvailable;
        state["current0"] = mover->ShowCurrent(0);
        state["current1"] = mover->ShowCurrent(1);
        state["current2"] = mover->ShowCurrent(2);
        state["relay_novolt"] = mover->NoVoltRelay;
        state["relay_overvoltage"] = mover->OvervoltageRelay;
        state["relay_ground"] = mover->GroundRelay;
        state["controller_second_position"] = mover->ScndCtrlPos;
        state["controller_main_position"] = mover->MainCtrlPos;
        state["radio_enabled"] = mover->Radio;
        state["radio_powered"] = mover->Radio && (mover->Power24vIsAvailable || mover->Power110vIsAvailable);
        state["radio_channel"] = radio_channel;
    }

    Dictionary TrainController::get_config() const {
        return config;
    }

    void TrainController::update_config(const Dictionary &p_config) {
        config.merge(p_config, true);
        emit_signal(CONFIG_CHANGED);
    }

    void TrainController::update_state() {
        _handle_mover_update();
    }

    void TrainController::_handle_mover_update() {
        const bool was_derailed = static_cast<bool>(state.get("is_derailed", false));
        state.merge(get_mover_state(), true);
        const bool is_derailed = static_cast<bool>(state.get("is_derailed", false));
        if (!was_derailed && is_derailed) {
            emit_signal(
                    DERAILED_SIGNAL,
                    static_cast<int>(state.get("damage_flag", 0)),
                    static_cast<int>(state.get("derail_reason", 0)));
        }
    }

    void TrainController::_sync_mover_neighbours() {
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
                TrainController *other_vehicle = end_index == end::front
                        ? track_manager->get_nearest_vehicle_ahead(this, &track_distance, &other_end)
                        : track_manager->get_nearest_vehicle_behind(this, &track_distance, &other_end);
                if (other_vehicle == nullptr || other_vehicle->get_mover() == nullptr) {
                    continue;
                }

                neighbour.vehicle = other_vehicle->get_mover();
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

    Dictionary TrainController::get_state() {
        return state;
    }

    Dictionary &TrainController::_get_state_internal() {
        return state;
    }

    void TrainController::sync_mover_coupling(
            TrainController *other_vehicle, const Side self_side, const Side other_side, const bool attach) {
        if (other_vehicle == nullptr || mover == nullptr || other_vehicle->get_mover() == nullptr) {
            GameLog::get_instance()->error("Cannot sync mover coupling. Missing vehicle mover.");
            return;
        }

        const int self_end = to_mover_end(self_side);
        const int other_end = to_mover_end(other_side);
        auto &self_coupler = mover->Couplers[self_end];
        auto &other_coupler = other_vehicle->get_mover()->Couplers[other_end];

        if (attach) {
            int coupling_type = self_coupler.AutomaticCouplingFlag & other_coupler.AutomaticCouplingFlag;
            if (self_coupler.control_type != other_coupler.control_type) {
                coupling_type &= ~coupling::control;
            }
            if (coupling_type == coupling::faux) {
                coupling_type = self_coupler.AllowedFlag & other_coupler.AllowedFlag;
            }

            mover->Attach(self_end, other_end, other_vehicle->get_mover(), coupling_type, true, false);
        } else if (self_coupler.Connected != nullptr) {
            mover->Dettach(self_end);
        }
    }

    void TrainController::assign_track_rid(const RID &track_rid, const String &track_id) {
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

    RID TrainController::get_track_rid() const {
        return current_track_rid;
    }

    void TrainController::assign_track(const String &track_id) {
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

    String TrainController::get_track_id() const {
        return current_track_id;
    }

    void TrainController::set_track_offset(const double offset) {
        current_track_offset = offset;

        if (TrackManager *track_manager = TrackManager::get_instance(); track_manager != nullptr && current_track_rid != RID()) {
            track_manager->register_vehicle(this, current_track_rid, current_track_id, current_track_offset);
        }
    }

    double TrainController::get_track_offset() const {
        return current_track_offset;
    }

    void TrainController::_on_coupled(TrainController *other_vehicle, const Side self_side, const Side other_side) {
        sync_mover_coupling(other_vehicle, self_side, other_side, true);
    }

    void TrainController::_on_uncoupled(TrainController *other_vehicle, const Side self_side, const Side other_side) {
        sync_mover_coupling(other_vehicle, self_side, other_side, false);
    }

    void TrainController::couple(TrainController *other_vehicle, const Side self_side, const Side other_side) {
        if (other_vehicle == nullptr) {
            UtilityFunctions::push_error("Cannot couple null vehicle.");
            return;
        }

        if (self_side == Side::FRONT && other_side == Side::BACK) {
            if (front != nullptr || other_vehicle->back != nullptr) {
                UtilityFunctions::push_error("One of the cars is already coupled.");
                return;
            }
            front = other_vehicle;
            other_vehicle->back = this;
            _on_coupled(other_vehicle, self_side, other_side);
            other_vehicle->_on_coupled(this, other_side, self_side);
        } else if (self_side == Side::BACK && other_side == Side::FRONT) {
            if (back != nullptr || other_vehicle->front != nullptr) {
                UtilityFunctions::push_error("One of the cars is already coupled.");
                return;
            }
            back = other_vehicle;
            other_vehicle->front = this;
            _on_coupled(other_vehicle, self_side, other_side);
            other_vehicle->_on_coupled(this, other_side, self_side);
        } else {
            UtilityFunctions::push_error("Invalid coupling sides specified.");
        }
    }

    void TrainController::couple_front(TrainController *other_vehicle, const Side other_side) {
        couple(other_vehicle, Side::FRONT, other_side);
    }

    void TrainController::couple_back(TrainController *other_vehicle, const Side other_side) {
        couple(other_vehicle, Side::BACK, other_side);
    }

    TrainController *TrainController::get_front_vehicle() const {
        return front;
    }

    TrainController *TrainController::get_back_vehicle() const {
        return back;
    }

    Array TrainController::get_rail_vehicle_modules() const {
        Array modules;
        const int child_count = get_child_count();
        for (int i = 0; i < child_count; ++i) {
            if (auto *module = Object::cast_to<TrainPart>(get_child(i)); module != nullptr) {
                modules.append(module);
            }
        }
        return modules;
    }

    Ref<TrainSet> TrainController::get_trainset() const {
        return trainset;
    }

    TrainController *TrainController::decouple(const int relative_index) {
        if (relative_index == 0) {
            UtilityFunctions::push_error("TrainController::decouple() requires a non-zero relative index.");
            return nullptr;
        }

        TrainController *target = this;
        const bool decouple_front_side = relative_index > 0;
        const int steps = std::abs(relative_index) - 1;

        for (int index = 0; index < steps; ++index) {
            target = decouple_front_side ? target->front : target->back;
            if (target == nullptr) {
                UtilityFunctions::push_error("TrainController::decouple() target vehicle is out of consist bounds.");
                return nullptr;
            }
        }

        TrainController *result = decouple_front_side ? target->uncouple_front() : target->uncouple_back();
        return result;
    }

    TrainController *TrainController::uncouple_front() {
        if (front == nullptr) {
            UtilityFunctions::push_error("No car coupled at the front.");
            return nullptr;
        }
        TrainController *uncoupled = front;
        front->back = nullptr;
        front = nullptr;
        _on_uncoupled(uncoupled, Side::FRONT, Side::BACK);
        uncoupled->_on_uncoupled(this, Side::BACK, Side::FRONT);
        return uncoupled;
    }

    TrainController *TrainController::uncouple_back() {
        if (back == nullptr) {
            UtilityFunctions::push_error("No car coupled at the back.");
            return nullptr;
        }
        TrainController *uncoupled = back;
        back->front = nullptr;
        back = nullptr;
        _on_uncoupled(uncoupled, Side::BACK, Side::FRONT);
        uncoupled->_on_uncoupled(this, Side::FRONT, Side::BACK);
        return uncoupled;
    }

    void TrainController::refresh_runtime_signals() {
        Dictionary current_state = get_state();
        const bool new_is_powered = (current_state.get("power24_available", false) || current_state.get("power110_available", false));
        if (prev_is_powered != new_is_powered) {
            prev_is_powered = new_is_powered;
            emit_signal(POWER_CHANGED_SIGNAL, prev_is_powered);
        }

        const bool new_radio_enabled = current_state.get("radio_enabled", false) && new_is_powered;
        if (prev_radio_enabled != new_radio_enabled) {
            prev_radio_enabled = new_radio_enabled;
            emit_signal(RADIO_TOGGLED, new_radio_enabled);
        }

        const int new_radio_channel = current_state.get("radio_channel", 0);
        if (prev_radio_channel != new_radio_channel) {
            prev_radio_channel = new_radio_channel;
            emit_signal(RADIO_CHANNEL_CHANGED, new_radio_channel);
        }
    }

    TypedArray<TrainCommand> TrainController::get_supported_commands() {
        TypedArray<TrainCommand> commands;

        commands.append(make_train_command("battery", Callable(this, "battery")));
        commands.append(make_train_command("main_controller_increase", Callable(this, "main_controller_increase")));
        commands.append(make_train_command("main_controller_decrease", Callable(this, "main_controller_decrease")));
        commands.append(make_train_command("direction_increase", Callable(this, "direction_increase")));
        commands.append(make_train_command("direction_decrease", Callable(this, "direction_decrease")));
        commands.append(make_train_command("decouple", Callable(this, "decouple")));
        commands.append(make_train_command("radio", Callable(this, "radio")));
        commands.append(make_train_command("radio_channel_set", Callable(this, "radio_channel_set")));
        commands.append(make_train_command("radio_channel_increase", Callable(this, "radio_channel_increase")));
        commands.append(make_train_command("radio_channel_decrease", Callable(this, "radio_channel_decrease")));

        const Array modules = get_rail_vehicle_modules();
        for (const Variant &module : modules) {
            if (TrainPart *train_part = Object::cast_to<TrainPart>(module); train_part != nullptr) {
                commands.append_array(train_part->get_supported_commands());
            }
        }

        return commands;
    }

    void TrainController::emit_command_received_signal(const String &command, const Variant &p1, const Variant &p2) {
        emit_signal(COMMAND_RECEIVED, command, p1, p2);
    }

    void TrainController::battery(const bool p_enabled) const {
        if (mover != nullptr) {
            mover->BatterySwitch(p_enabled);
        }
    }

    void TrainController::main_controller_increase(const int p_step) const {
        if (mover != nullptr) {
            mover->IncMainCtrl(p_step > 0 ? p_step : 1);
        }
    }

    void TrainController::main_controller_decrease(const int p_step) const {
        if (mover != nullptr) {
            mover->DecMainCtrl(p_step > 0 ? p_step : 1);
        }
    }

    void TrainController::direction_increase() const {
        if (mover != nullptr) {
            mover->DirectionForward();
        }
    }

    void TrainController::direction_decrease() const {
        if (mover != nullptr) {
            mover->DirectionBackward();
        }
    }

    void TrainController::radio(const bool p_enabled) {
        if (mover != nullptr) {
            mover->Radio = p_enabled;
        }
        refresh_runtime_signals();
    }

    void TrainController::radio_channel_set(const int p_channel) {
        radio_channel = Math::clamp(p_channel, radio_channel_min, radio_channel_max);
        refresh_runtime_signals();
    }

    void TrainController::radio_channel_increase(const int p_step) {
        radio_channel = Math::clamp(radio_channel + (p_step > 0 ? p_step : 1), radio_channel_min, radio_channel_max);
        refresh_runtime_signals();
    }

    void TrainController::radio_channel_decrease(const int p_step) {
        radio_channel = Math::clamp(radio_channel - (p_step != 0 ? p_step : 1), radio_channel_min, radio_channel_max);
        refresh_runtime_signals();
    }
} // namespace godot
