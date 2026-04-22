#include "../core/GameLog.hpp"
#include "../core/LegacyRailVehicle.hpp"
#include "../core/LegacyRailVehicleModule.hpp"
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
        _d_move_len = 0.0;
        _applied_move_len = 0.0;
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
        DEBUG("LegacyRailVehicle::initialize_mover begin vehicle=%s", get_name());
        const auto _type_name = std::string(type_name.utf8().ptr());
        const auto name = std::string(String(get_name()).utf8().ptr());
        mover = std::make_unique<TMoverParameters>(initial_velocity, _type_name, name, cabin_number).release();

        emit_signal(MOVER_CONFIG_CHANGED_SIGNAL);
        update_mover();
        mover->CheckLocomotiveParameters(true, 0);
        emit_signal(MOVER_CONFIG_CHANGED_SIGNAL);
        update_mover();

        mover->CabActive = 1;
        mover->CabMaster = true;
        mover->CabOccupied = 1;
        mover->AutomaticCabActivation = true;
        mover->CabActivisationAuto();
        mover->CabActivisation();
        mover->switch_physics(true);

        if (RailVehicle *front = get_front_vehicle(); front != nullptr) {
            sync_mover_coupling(
                    front, Side::FRONT, front->get_back_vehicle() == this ? Side::BACK : Side::FRONT, true);
        }
        if (RailVehicle *back = get_back_vehicle(); back != nullptr) {
            sync_mover_coupling(
                    back, Side::BACK, back->get_front_vehicle() == this ? Side::FRONT : Side::BACK, true);
        }

        DEBUG("LegacyRailVehicle::initialize_mover done vehicle=%s", get_name());
        emit_signal(MOVER_INITIALIZED_SIGNAL);
    }

    void LegacyRailVehicle::_initialize() {
        if (mover == nullptr) {
            initialize_mover();
            update_state();
            _notification_after_mover_initialized();
        }
    }

    void LegacyRailVehicle::_finalize() {
        if (TrackManager *track_manager = TrackManager::get_instance(); track_manager != nullptr) {
            track_manager->remove_vehicle(this);
        }
    }

    void LegacyRailVehicle::_update(const double delta) {
        _process_mover(delta);
    }

    void LegacyRailVehicle::_notification_after_mover_initialized() {}

    void LegacyRailVehicle::_notification_before_mover_cleanup() {}

    void LegacyRailVehicle::_process_mover(const double delta) {
        // Keep the wrapper-side movement flow analogous to DynObj:
        // dMoveLen is injected into the mover before ComputeMovement(), then the
        // full applied distance is dMoveLen + ComputeMovement(...).
        mover->dMoveLen = _d_move_len;

        TrackManager *track_manager = TrackManager::get_instance();
        if (track_manager == nullptr || current_track_rid == RID()) {
            _applied_move_len = 0.0;
            _d_move_len = 0.0;
            _handle_mover_update();
            return;
        }

        const Ref<VirtualTrack> track = track_manager->get_track(current_track_rid);
        if (track.is_null()) {
            _applied_move_len = 0.0;
            _d_move_len = 0.0;
            _handle_mover_update();
            return;
        }

        const Vector3 world_position = track->get_world_position(current_track_offset);
        set_mover_location(world_position);
        mover->RunningShape = track->get_shape();
        mover->RunningTrack = track->get_track_param();
        _sync_mover_neighbours();
        mover->ComputeTotalForce(delta);
        const double computed_move_len = mover->ComputeMovement(
                delta, delta, mover->RunningShape, mover->RunningTrack, mover->RunningTraction, mover->Loc, mover->Rot);
        // DynObj applies the full resolved distance, not only the raw ComputeMovement() result.
        // The wrapper must do the same so track_offset stays analogous to vehicle Move(dDOMoveLen).
        _applied_move_len = mover->dMoveLen + computed_move_len;
        set_track_offset(current_track_offset + _applied_move_len);
        set_mover_location(track->get_world_position(current_track_offset));
        _d_move_len = 0.0;
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
        internal_state["movement_delta"] = _applied_move_len;
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
        if (mover == nullptr) {
            GameLog::get_instance()->error("Cannot sync mover coupling. Self mover is not initialized.");
            return;
        }
        if (other_legacy->get_mover() == nullptr) {
            GameLog::get_instance()->error("Cannot sync mover coupling. Other mover is not initialized.");
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

            DEBUG(
                    "LegacyRailVehicle::sync_mover_coupling attach self=%s self_end=%s other=%s other_end=%s "
                    "self_auto=%s other_auto=%s self_allowed=%s other_allowed=%s self_control=%s other_control=%s "
                    "coupling_type=%s",
                    get_name(), self_end, other_legacy->get_name(), other_end,
                    self_coupler.AutomaticCouplingFlag, other_coupler.AutomaticCouplingFlag,
                    self_coupler.AllowedFlag, other_coupler.AllowedFlag,
                    self_coupler.control_type.c_str(), other_coupler.control_type.c_str(), coupling_type);
            const bool attached =
                    mover->Attach(self_end, other_end, other_legacy->get_mover(), coupling_type, true, false);
            DEBUG(
                    "LegacyRailVehicle::sync_mover_coupling attach_result self=%s self_end=%s other=%s other_end=%s "
                    "attached=%s self_connected=%s self_connected_nr=%s other_connected=%s other_connected_nr=%s",
                    get_name(), self_end, other_legacy->get_name(), other_end, attached,
                    static_cast<const void *>(self_coupler.Connected), self_coupler.ConnectedNr,
                    static_cast<const void *>(other_coupler.Connected), other_coupler.ConnectedNr);
        } else if (self_coupler.Connected != nullptr) {
            DEBUG(
                    "LegacyRailVehicle::sync_mover_coupling dettach self=%s self_end=%s other=%s other_end=%s "
                    "self_connected=%s self_connected_nr=%s other_connected=%s other_connected_nr=%s",
                    get_name(), self_end, other_legacy->get_name(), other_end,
                    static_cast<const void *>(self_coupler.Connected), self_coupler.ConnectedNr,
                    static_cast<const void *>(other_coupler.Connected), other_coupler.ConnectedNr);
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
