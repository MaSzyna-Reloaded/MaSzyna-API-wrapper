#include "../core/VehicleController.hpp"
#include "../core/VehicleComponent.hpp"
#include "../core/TrainSystem.hpp"
#include "../engines/VehicleEngine.hpp"
#include "../physics/MaszynaMoverPhysicsServer.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    const char *VehicleController::mover_config_changed_signal = "mover_config_changed";
    const char *VehicleController::mover_initialized_signal = "mover_initialized";
    const char *VehicleController::power_changed_signal = "power_changed";
    const char *VehicleController::command_received = "command_received";
    const char *VehicleController::radio_toggled = "radio_toggled";
    const char *VehicleController::radio_channel_changed = "radio_channel_changed";
    const char *VehicleController::roof_light_changed = "roof_light_changed";
    const char *VehicleController::cabin_occupied_changed = "cabin_occupied_changed";
    const char *VehicleController::config_changed = "config_changed";
    const char *VehicleController::position_changed_signal = "position_changed";
    const char *VehicleController::consist_changed_signal = "consist_changed";
    const char *VehicleController::coupler_attached_signal = "coupler_attached";
    const char *VehicleController::coupler_detached_signal = "coupler_detached";

    void VehicleController::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_state"), &VehicleController::get_state);
        ClassDB::bind_method(D_METHOD("get_config"), &VehicleController::get_config);
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

        ClassDB::bind_method(
                D_METHOD("send_command", "command", "p1", "p2"), &VehicleController::send_command, DEFVAL(Variant()),
                DEFVAL(Variant()));

        ClassDB::bind_method(
                D_METHOD("broadcast_command", "command", "p1", "p2"), &VehicleController::broadcast_command,
                DEFVAL(Variant()), DEFVAL(Variant()));


        ClassDB::bind_method(D_METHOD("register_command", "command", "callable"), &VehicleController::register_command);
        ClassDB::bind_method(
                D_METHOD("unregister_command", "command", "callable"), &VehicleController::unregister_command);
        ClassDB::bind_method(D_METHOD("battery", "enabled"), &VehicleController::battery);
        ClassDB::bind_method(D_METHOD("cab_activation", "enabled"), &VehicleController::cab_activation);
        ClassDB::bind_method(D_METHOD("cab_activation_auto"), &VehicleController::cab_activation_auto);
        ClassDB::bind_method(D_METHOD("cab_change", "direction"), &VehicleController::cab_change);
        ClassDB::bind_method(
                D_METHOD("main_controller_increase", "step"), &VehicleController::main_controller_increase, DEFVAL(1));
        ClassDB::bind_method(
                D_METHOD("main_controller_decrease", "step"), &VehicleController::main_controller_decrease, DEFVAL(1));
        ClassDB::bind_method(
                D_METHOD("second_controller_increase", "step"), &VehicleController::second_controller_increase,
                DEFVAL(1));
        ClassDB::bind_method(
                D_METHOD("second_controller_decrease", "step"), &VehicleController::second_controller_decrease,
                DEFVAL(1));
        ClassDB::bind_method(D_METHOD("direction_increase"), &VehicleController::direction_increase);
        ClassDB::bind_method(D_METHOD("direction_decrease"), &VehicleController::direction_decrease);
        ClassDB::bind_method(D_METHOD("radio", "enabled"), &VehicleController::radio);
        ClassDB::bind_method(D_METHOD("radio_channel_set", "channel"), &VehicleController::radio_channel_set);
        ClassDB::bind_method(
                D_METHOD("radio_channel_increase", "step"), &VehicleController::radio_channel_increase, DEFVAL(1));
        ClassDB::bind_method(
                D_METHOD("radio_channel_decrease", "step"), &VehicleController::radio_channel_decrease, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("apply_config"), &VehicleController::apply_config);
        ClassDB::bind_method(D_METHOD("update_state"), &VehicleController::update_state);
        ClassDB::bind_method(D_METHOD("get_velocity"), &VehicleController::get_velocity);
        ClassDB::bind_method(D_METHOD("update_config"), &VehicleController::update_config);
        ClassDB::bind_method(D_METHOD("process_movement", "delta"), &VehicleController::process_movement);
        ClassDB::bind_method(D_METHOD("update_location"), &VehicleController::update_location);
        ClassDB::bind_method(
                D_METHOD("update_neighbour", "end", "other", "other_end", "track_distance"),
                &VehicleController::update_neighbour);
        ClassDB::bind_method(D_METHOD("compute_forces", "delta"), &VehicleController::compute_forces);
        ClassDB::bind_method(D_METHOD("compute_movement", "delta"), &VehicleController::compute_movement);
        ClassDB::bind_method(D_METHOD("compute_fast_movement", "delta"), &VehicleController::compute_fast_movement);
        ClassDB::bind_method(D_METHOD("is_physics_active"), &VehicleController::is_physics_active);
        ClassDB::bind_method(
                D_METHOD("couple", "other", "end", "other_end", "coupling_type"), &VehicleController::couple);
        ClassDB::bind_method(D_METHOD("uncouple", "end"), &VehicleController::uncouple);
        ClassDB::bind_method(D_METHOD("is_coupled", "end"), &VehicleController::is_coupled);
        ClassDB::bind_method(D_METHOD("get_coupled_controller", "end"), &VehicleController::get_coupled_controller);
        ClassDB::bind_method(D_METHOD("get_coupled_end", "end"), &VehicleController::get_coupled_end);
        ClassDB::bind_method(D_METHOD("coupler_connect", "where"), &VehicleController::coupler_connect);
        ClassDB::bind_method(D_METHOD("coupler_disconnect", "where"), &VehicleController::coupler_disconnect);
        ClassDB::bind_method(D_METHOD("get_world_transform"), &VehicleController::get_world_transform);
        ClassDB::bind_method(D_METHOD("get_world_position"), &VehicleController::get_world_position);
        ClassDB::bind_method(
                D_METHOD("change_track", "track_name", "track_offset", "track_direction"),
                &VehicleController::change_track);
        ClassDB::bind_method(D_METHOD("get_rid"), &VehicleController::get_rid);
        ClassDB::bind_method(
                D_METHOD("_emit_position_changed_if_needed"), &VehicleController::_emit_position_changed_if_needed);

        BIND_PROPERTY(VehicleController, Variant::STRING, train_id);
        BIND_PROPERTY(VehicleController, Variant::STRING, type_name);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, mass);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, power);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, max_velocity);
        BIND_PROPERTY(VehicleController, Variant::INT, radio_channel_min, "radio_channel");
        BIND_PROPERTY(VehicleController, Variant::INT, radio_channel_max, "radio_channel");
        /* FIXME: move to TrainPower section? */
        BIND_PROPERTY_W_HINT(VehicleController, Variant::FLOAT, battery_voltage, PROPERTY_HINT_RANGE, "0,500,1");
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, category, PROPERTY_HINT_ENUM,
                enum_hint(
                        {{"Train", CATEGORY_TRAIN},
                         {"Road", CATEGORY_ROAD},
                         {"Ship", CATEGORY_SHIP},
                         {"Airplane", CATEGORY_AIRPLANE}}));
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, train_type, PROPERTY_HINT_ENUM,
                enum_hint(
                        {{"Default", TRAIN_TYPE_DEFAULT},
                         {"EZT", TRAIN_TYPE_EZT},
                         {"ET41", TRAIN_TYPE_ET41},
                         {"ET42", TRAIN_TYPE_ET42},
                         {"PseudoDiesel", TRAIN_TYPE_PSEUDODIESEL},
                         {"ET22", TRAIN_TYPE_ET22},
                         {"SN61", TRAIN_TYPE_SN61},
                         {"EP05", TRAIN_TYPE_EP05},
                         {"ET40", TRAIN_TYPE_ET40},
                         {"T181", TRAIN_TYPE_181},
                         {"DMU", TRAIN_TYPE_DMU}}));
        BIND_PROPERTY(VehicleController, Variant::FLOAT, reduced_mass);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, sand_capacity);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, heating_power);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, light_power);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_length, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_height, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_width, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_drag_coefficient, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_floor_height, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, initial_velocity);
        BIND_PROPERTY(VehicleController, Variant::INT, cabin_number);
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, cntrl_battery_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, cntrl_ground_relay_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, cntrl_compartment_lights_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(VehicleController, Variant::BOOL, cntrl_automatic_cab_activation, "cntrl");
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, cntrl_inactive_cab_flag, "cntrl", PROPERTY_HINT_FLAGS,
                "Emergency Brake,Toggle Mirrors,Raise Second Pantograph,End Of Train Lights,Grant Both Side Permits,"
                "Apply Spring Brake,Release Spring Brake,Reset Direction");

        ADD_SIGNAL(MethodInfo(mover_config_changed_signal));
        ADD_SIGNAL(MethodInfo(mover_initialized_signal));
        ADD_SIGNAL(MethodInfo(power_changed_signal, PropertyInfo(Variant::BOOL, "is_powered")));
        ADD_SIGNAL(MethodInfo(radio_toggled, PropertyInfo(Variant::BOOL, "is_enabled")));
        ADD_SIGNAL(MethodInfo(radio_channel_changed, PropertyInfo(Variant::INT, "channel")));
        ADD_SIGNAL(MethodInfo(roof_light_changed, PropertyInfo(Variant::BOOL, "is_enabled")));
        ADD_SIGNAL(MethodInfo(cabin_occupied_changed, PropertyInfo(Variant::INT, "cabin_occupied")));
        ADD_SIGNAL(MethodInfo(config_changed));
        ADD_SIGNAL(MethodInfo(position_changed_signal, PropertyInfo(Variant::VECTOR3, "position")));
        ADD_SIGNAL(MethodInfo(consist_changed_signal));
        const String coupling_element_hint = enum_hint({{"Coupler", COUPLING_ELEMENT_COUPLER},
                           {"BrakeHose", COUPLING_ELEMENT_BRAKEHOSE},
                           {"MainHose", COUPLING_ELEMENT_MAINHOSE},
                           {"Control", COUPLING_ELEMENT_CONTROL},
                           {"Gangway", COUPLING_ELEMENT_GANGWAY},
                           {"Heating", COUPLING_ELEMENT_HEATING}});
        ADD_SIGNAL(MethodInfo(
                coupler_attached_signal,
                PropertyInfo(Variant::INT, "element", PROPERTY_HINT_ENUM, coupling_element_hint)));
        ADD_SIGNAL(MethodInfo(
                coupler_detached_signal,
                PropertyInfo(Variant::INT, "element", PROPERTY_HINT_ENUM, coupling_element_hint)));
        ADD_SIGNAL(MethodInfo(
                command_received, PropertyInfo(Variant::STRING, "command"), PropertyInfo(Variant::NIL, "p1"),
                PropertyInfo(Variant::NIL, "p2")));

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

        BIND_ENUM_CONSTANT(COUPLING_ELEMENT_COUPLER);
        BIND_ENUM_CONSTANT(COUPLING_ELEMENT_BRAKEHOSE);
        BIND_ENUM_CONSTANT(COUPLING_ELEMENT_MAINHOSE);
        BIND_ENUM_CONSTANT(COUPLING_ELEMENT_CONTROL);
        BIND_ENUM_CONSTANT(COUPLING_ELEMENT_GANGWAY);
        BIND_ENUM_CONSTANT(COUPLING_ELEMENT_HEATING);
        BIND_ENUM_CONSTANT(CATEGORY_TRAIN);
        BIND_ENUM_CONSTANT(CATEGORY_ROAD);
        BIND_ENUM_CONSTANT(CATEGORY_SHIP);
        BIND_ENUM_CONSTANT(CATEGORY_AIRPLANE);

        BIND_ENUM_CONSTANT(TRAIN_TYPE_DEFAULT);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_EZT);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_ET41);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_ET42);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_PSEUDODIESEL);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_ET22);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_SN61);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_EP05);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_ET40);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_181);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_DMU);

        BIND_ENUM_CONSTANT(START_MODE_DISABLED);
        BIND_ENUM_CONSTANT(START_MODE_MANUAL);
        BIND_ENUM_CONSTANT(START_MODE_AUTOMATIC);
        BIND_ENUM_CONSTANT(START_MODE_MANUAL_WITH_AUTO_FALLBACK);
        BIND_ENUM_CONSTANT(START_MODE_CONVERTER);
        BIND_ENUM_CONSTANT(START_MODE_BATTERY);
        BIND_ENUM_CONSTANT(START_MODE_DIRECTION);
    }

    std::unordered_map<const TMoverParameters *, VehicleController *> VehicleController::controllers_by_mover;

    TMoverParameters *VehicleController::get_mover() const {
        return mover;
    }

    // the end of the coupled vehicle facing this one (TCoupling::ConnectedNr), -1 when not coupled
    int VehicleController::get_coupled_end(const int p_end) const {
        if (mover == nullptr || mover->Couplers[p_end].Connected == nullptr) {
            return -1;
        }
        return mover->Couplers[p_end].ConnectedNr;
    }

    VehicleController *VehicleController::get_coupled_controller(const int p_end) const {
        if (mover == nullptr || mover->Couplers[p_end].Connected == nullptr) {
            return nullptr;
        }
        const auto it = controllers_by_mover.find(mover->Couplers[p_end].Connected);
        return it == controllers_by_mover.end() ? nullptr : it->second;
    }

    void VehicleController::initialize_mover_state() {
        const bool driver_active = initial_velocity != 0.0;

        mover->MainCtrlPos = mover->MainCtrlNoPowerPos();
        mover->LocalBrakePosA = 0.0;
        mover->BrakeCtrlPos =
                static_cast<int>(std::floor(mover->Handle->GetPos(driver_active && cabin_number != 0 ? bh_RP : bh_NP)));
        mover->BrakeLevelSet(mover->BrakeCtrlPos);
    }

    void VehicleController::initialize_mover() {
        const auto initial_vel = this->initial_velocity;
        const auto mover_type_name = std::string(type_name.utf8().ptr());
        const auto name = std::string(this->get_name().left(this->get_name().length()).utf8().ptr());
        MaszynaMoverPhysicsServer *physics = MaszynaMoverPhysicsServer::get_instance();
        ERR_FAIL_NULL(physics);
        physics_rid = physics->vehicle_create(
                type_name, String(name.c_str()), initial_vel, this->cabin_number);
        mover = physics->vehicle_get_mover(physics_rid);
        ERR_FAIL_NULL(mover);
        controllers_by_mover[mover] = this;

        dirty = true;
        dirty_prop = true;
        _update_mover_config_if_dirty();

        /* FIXME: CheckLocomotiveParameters should be called after (re)initialization */
        mover->CheckLocomotiveParameters(initial_velocity != 0.0, 0); // FIXME: brakujace parametery

        /* CheckLocomotiveParameters() will reset some parameters, so the changes
         * must be applied second time */

        dirty = true;
        dirty_prop = true;
        _update_mover_config_if_dirty();
        initialize_mover_state();

        // Original engine: Load() (Mover.cpp:11692) calls ComputeConstans() once, after every
        // physical parameter (TotalMass, Dim, Cx, BearingType, NPoweredAxles, TrackW - all
        // already applied above by the two _update_mover_config_if_dirty() passes) is settled -
        // it derives FrictConst1/FrictConst2s/FrictConst2d, the per-vehicle rolling/air-drag
        // resistance coefficients FrictionForce() (called every tick from ComputeTotalForce())
        // actually uses. Never called anywhere else in the original either (a single call at
        // load time is correct - the original itself never updates curve-dependent resistance
        // terms after that point). Without this, every one of this wrapper's vehicles ran with
        // zero rolling/air resistance: free acceleration to unrealistic speeds and near-zero
        // coasting deceleration, since FrictConst1/2s/2d all silently stayed at their
        // compiled-zero defaults.
        mover->ComputeConstans();

        // Original engine: the scenery's driver type picks the cab (DynObj.cpp:1812-1825).
        // FIXME: a vehicle without a driver stays in cab 0 there; here it still starts in cab 1.
        if (mover->CabOccupied == 0) {
            mover->CabOccupied = 1;
        }
        // only a driven vehicle gets its cab activated by the driver (Driver.cpp:2126); an unmanned
        // one stays inactive, so ComputeTotalForce() can switch its physics off
        if (cabin_number != 0) {
            mover->CabActivisation();
        }

        /* switch_physics() raczej trzeba zostawic */
        mover->switch_physics(true);

        DEBUG("[MaSzyna::TMoverParameters] Mover initialized successfully");
        emit_signal(mover_initialized_signal);
    }

    void VehicleController::register_command(const String &p_command, const Callable &p_callable) {
        TrainSystem::get_instance()->register_command(train_id, p_command, p_callable);
    }

    void VehicleController::unregister_command(const String &p_command, const Callable &p_callable) {
        TrainSystem::get_instance()->unregister_command(train_id, p_command, p_callable);
    }

    void VehicleController::_notification(const int p_what) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        if (p_what == NOTIFICATION_PREDELETE && mover != nullptr) {
            controllers_by_mover.erase(mover);
            // the backend owns the Mover, so freeing the handle is what destroys it
            if (MaszynaMoverPhysicsServer *physics = MaszynaMoverPhysicsServer::get_instance();
                physics != nullptr) {
                physics->vehicle_free(physics_rid);
            }
            physics_rid = RID();
            mover = nullptr;
        }
        switch (p_what) {
            case NOTIFICATION_ENTER_TREE:
                if (Object *rail_vehicle_physics_server = _get_rail_vehicle_physics_server();
                    rail_vehicle_physics_server != nullptr) {
                    rid = rail_vehicle_physics_server->call("controller_create", this);
                }
                TrainSystem::get_instance()->register_train(train_id, this);
                register_command("battery", Callable(this, "battery"));
                register_command("cab_change", Callable(this, "cab_change"));
                register_command("cab_activation", Callable(this, "cab_activation"));
                register_command("cab_activation_auto", Callable(this, "cab_activation_auto"));
                register_command("main_controller_increase", Callable(this, "main_controller_increase"));
                register_command("main_controller_decrease", Callable(this, "main_controller_decrease"));
                register_command("second_controller_increase", Callable(this, "second_controller_increase"));
                register_command("second_controller_decrease", Callable(this, "second_controller_decrease"));
                register_command("direction_increase", Callable(this, "direction_increase"));
                register_command("direction_decrease", Callable(this, "direction_decrease"));
                register_command("radio", Callable(this, "radio"));
                register_command("radio_channel_set", Callable(this, "radio_channel_set"));
                register_command("radio_channel_increase", Callable(this, "radio_channel_increase"));
                register_command("radio_channel_decrease", Callable(this, "radio_channel_decrease"));
                register_command("coupler_connect", Callable(this, "coupler_connect"));
                register_command("coupler_disconnect", Callable(this, "coupler_disconnect"));
                break;
            case NOTIFICATION_EXIT_TREE:
                unregister_command("battery", Callable(this, "battery"));
                unregister_command("cab_change", Callable(this, "cab_change"));
                unregister_command("cab_activation", Callable(this, "cab_activation"));
                unregister_command("cab_activation_auto", Callable(this, "cab_activation_auto"));
                unregister_command("main_controller_increase", Callable(this, "main_controller_increase"));
                unregister_command("main_controller_decrease", Callable(this, "main_controller_decrease"));
                unregister_command("second_controller_increase", Callable(this, "second_controller_increase"));
                unregister_command("second_controller_decrease", Callable(this, "second_controller_decrease"));
                unregister_command("direction_increase", Callable(this, "direction_increase"));
                unregister_command("direction_decrease", Callable(this, "direction_decrease"));
                unregister_command("radio", Callable(this, "radio"));
                unregister_command("radio_channel_set", Callable(this, "radio_channel_set"));
                unregister_command("radio_channel_increase", Callable(this, "radio_channel_increase"));
                unregister_command("radio_channel_decrease", Callable(this, "radio_channel_decrease"));
                unregister_command("coupler_connect", Callable(this, "coupler_connect"));
                unregister_command("coupler_disconnect", Callable(this, "coupler_disconnect"));
                TrainSystem::get_instance()->unregister_train(train_id);
                if (Object *rail_vehicle_physics_server = _get_rail_vehicle_physics_server();
                    rail_vehicle_physics_server != nullptr && rid.is_valid()) {
                    rail_vehicle_physics_server->call("controller_free", rid);
                }
                rid = RID();
                break;
            case NOTIFICATION_READY:
                initialize_mover();
                update_state();
                DEBUG("VehicleController::_ready() signals connected to train parts");

                emit_signal(power_changed_signal, prev_is_powered);
                emit_signal(radio_channel_changed, prev_radio_channel);
                emit_signal(roof_light_changed, prev_roof_light_enabled);
                break;
            default:;
        }
    }

    void VehicleController::_update_mover_config_if_dirty() {
        if (dirty) {
            /* update all train parts
             */
            emit_signal(mover_config_changed_signal);

            dirty = false;
            dirty_prop = true; // sforsowanie odswiezenia stanu lokalnych propsow
        }

        if (dirty_prop) {
            apply_config();
            dirty_prop = false;
        }
    }

    void VehicleController::_process_mover(const double p_delta) {
        compute_forces(p_delta);
        compute_movement(p_delta);
        _handle_mover_update();
    }

    // Original engine: TDynamicObject::Move sets Loc = {-x, z, y} (DynObj.cpp:2334); dMoveLen collects the
    // movement of one simulation frame and is reset after it (ResetdMoveLen, DynObj.cpp:3473)
    bool VehicleController::is_physics_active() const {
        const MaszynaMoverPhysicsServer *physics = MaszynaMoverPhysicsServer::get_instance();
        return physics != nullptr && physics->vehicle_is_active(physics_rid);
    }

    void VehicleController::update_location() {
        MaszynaMoverPhysicsServer *physics = MaszynaMoverPhysicsServer::get_instance();
        if (physics == nullptr) {
            return;
        }
        physics->vehicle_set_location(physics_rid, get_world_position());
    }

    // Original engine: TDynamicObject::update_neighbours() (DynObj.cpp:7135); the track scan itself
    // (find_vehicle) is done by RailVehiclePhysicsServer, which passes the center to center track distance
    void VehicleController::update_neighbour(
            const int p_end, VehicleController *p_other, const int p_other_end, const double p_track_distance) {
        if (mover == nullptr) {
            return;
        }
        neighbour_data &neighbour = mover->Neighbours[p_end];
        const TCoupling &coupler = mover->Couplers[p_end];

        if (coupler.Connected != nullptr) {
            // physical connection with another vehicle locks down collision source on this end
            neighbour.vehicle = coupler.Connected;
            neighbour.vehicle_end = coupler.ConnectedNr;
            neighbour.distance = static_cast<float>(
                    TMoverParameters::CouplerDist(mover, coupler.Connected) - coupler.adapter_length -
                    coupler.Connected->Couplers[coupler.ConnectedNr].adapter_length);
            return;
        }

        neighbour = neighbour_data();
        if (p_other == nullptr || p_other->mover == nullptr) {
            return;
        }
        TMoverParameters *other_mover = p_other->mover;
        const TCoupling &other_coupler = other_mover->Couplers[p_other_end];
        neighbour.vehicle = other_mover;
        neighbour.vehicle_end = p_other_end;
        neighbour.distance = static_cast<float>(p_track_distance - 0.5 * (mover->Dim.L + other_mover->Dim.L));
        if (neighbour.distance < (other_mover->CategoryFlag == 2 ? 50 : 100)) {
            // at short distances (re)calculate range between couplers directly
            neighbour.distance = static_cast<float>(
                    TMoverParameters::CouplerDist(mover, other_mover) - coupler.adapter_length -
                    other_coupler.adapter_length);
        }
    }

    void VehicleController::compute_forces(const double p_delta) {
        // the components' authored config is applied before the backend integrates anything;
        // this becomes the server's own `configure` phase once the components move there
        _update_mover_config_if_dirty();
        MaszynaMoverPhysicsServer *physics = MaszynaMoverPhysicsServer::get_instance();
        if (physics == nullptr) {
            return;
        }
        physics->vehicle_compute_forces(physics_rid, p_delta);
    }

    void VehicleController::compute_movement(const double p_delta) {
        MaszynaMoverPhysicsServer *physics = MaszynaMoverPhysicsServer::get_instance();
        if (physics == nullptr) {
            return;
        }
        physics->vehicle_compute_movement(
                physics_rid, p_delta, MaszynaMoverPhysicsServer::MOVEMENT_FULL);
        // the Hasler recorder is vehicle state, not integration - it stays here until the state
        // registry takes it over
        _update_tachometer(p_delta);
    }

    /// The cheap movement of the intermediate physics iterations: the original runs UpdateForce +
    /// FastUpdate for every sub-iteration and the full Update() only once per frame
    /// (DynObj.cpp:8195-8210), where FastUpdate calls Mover::FastComputeMovement()
    /// (DynObj.cpp:4086) instead of the full ComputeMovement().
    void VehicleController::compute_fast_movement(const double p_delta) {
        if (MaszynaMoverPhysicsServer *physics = MaszynaMoverPhysicsServer::get_instance();
            physics != nullptr) {
            physics->vehicle_compute_movement(
                    physics_rid, p_delta, MaszynaMoverPhysicsServer::MOVEMENT_FAST);
        }
    }

    // Original engine: TDynamicObject::AttachNext() couples with Enforce, without sound (DynObj.cpp:2590)
    void VehicleController::couple(
            VehicleController *p_other, const int p_end, const int p_other_end, const int p_coupling_type) {
        if (mover == nullptr || p_other == nullptr || p_other->mover == nullptr) {
            UtilityFunctions::push_error("Cannot couple vehicles without initialized movers.");
            return;
        }
        int coupling_type = p_coupling_type;
        // a coupler allowing only permanent coupling keeps it permanent (simulationstateserializer.cpp:990)
        if (coupling_type != coupling::faux && (mover->Couplers[p_end].AllowedFlag & coupling::permanent) != 0) {
            coupling_type |= coupling::permanent;
        }
        mover->Attach(p_end, p_other_end, p_other->mover, coupling_type, true, false);
        // the original re-inspects the consist on a coupling change (CheckVehicles(), Driver.cpp:2622)
        emit_signal(consist_changed_signal);
        p_other->emit_signal(consist_changed_signal);
    }

    void VehicleController::uncouple(const int p_end) {
        if (mover == nullptr || mover->Couplers[p_end].Connected == nullptr) {
            return;
        }
        mover->Dettach(p_end);
        emit_signal(consist_changed_signal);
    }

    bool VehicleController::is_coupled(const int p_end) const {
        return mover != nullptr && mover->Couplers[p_end].Connected != nullptr;
    }

    // p_where is a coupler end (0 front, 1 rear) or a world position - then the vehicle end nearest to
    // it is used, like the walk mode commands of the original (ABuScanNearestObject, Train.cpp:6213)
    int VehicleController::_resolve_coupler_end(const Variant &p_where) const {
        if (p_where.get_type() != Variant::VECTOR3) {
            return CLAMP(static_cast<int>(p_where), 0, 1);
        }
        const Transform3D transform = get_world_transform();
        // vehicles face -Z; the front coupler (end 0) is half the length ahead of the center
        const Vector3 front = transform.origin - transform.basis.get_column(2).normalized() * (0.5 * mover->Dim.L);
        const Vector3 rear = transform.origin + transform.basis.get_column(2).normalized() * (0.5 * mover->Dim.L);
        const Vector3 position = p_where;
        return position.distance_squared_to(front) <= position.distance_squared_to(rear) ? 0 : 1;
    }

    // Original engine: TDynamicObject::couple() (DynObj.cpp:1509) - one more coupling type per call,
    // with the vehicle detected at that end
    void VehicleController::coupler_connect(const Variant &p_where) {
        if (mover == nullptr) {
            return;
        }
        const int side = _resolve_coupler_end(p_where);
        const neighbour_data &neighbour = mover->Neighbours[side];
        if (neighbour.vehicle == nullptr) {
            return;
        }
        const TCoupling &coupler = mover->Couplers[side];
        const TCoupling &other_coupler = neighbour.vehicle->Couplers[neighbour.vehicle_end];
        const int allowed = coupler.AllowedFlag & other_coupler.AllowedFlag;

        if (coupler.CouplingFlag == coupling::faux && (allowed & coupling::coupler) == coupling::coupler &&
            mover->Attach(side, neighbour.vehicle_end, neighbour.vehicle, coupling::coupler)) {
            return;
        }
        for (const int flag:
             {coupling::brakehose, coupling::mainhose, coupling::control, coupling::gangway, coupling::heating}) {
            if ((coupler.CouplingFlag & flag) == flag || (allowed & flag) != flag) {
                continue;
            }
            if (flag == coupling::control && coupler.control_type != other_coupler.control_type) {
                continue;
            }
            if (mover->Attach(side, neighbour.vehicle_end, neighbour.vehicle, coupler.CouplingFlag | flag)) {
                return;
            }
        }
    }

    // Original engine: TDynamicObject::uncouple() (DynObj.cpp:1614)
    void VehicleController::coupler_disconnect(const Variant &p_where) {
        if (mover == nullptr) {
            return;
        }
        const int side = _resolve_coupler_end(p_where);
        if (mover->DettachStatus(side) >= 0 || (mover->Couplers[side].CouplingFlag & coupling::permanent) != 0) {
            return;
        }
        mover->Dettach(side);
    }

    // Original engine: TTrain::Update() Hasler block (Train.cpp:6917-6940) and its tachoclock
    // sound gate (Train.cpp:8323-8335).
    void VehicleController::_update_tachometer(const double p_delta) {
        const double max_tacho = 3.0;
        tacho_velocity = std::min(std::abs(11.31 * mover->WheelDiameter * mover->nrot), mover->Vmax * 1.05);

        // the needle jumps once per simulation second, with a small random error
        const double previous_second = std::floor(tacho_time);
        tacho_time += p_delta;
        if (std::floor(tacho_time) != previous_second) {
            tacho_velocity_jump = tacho_velocity > 1.0
                                          ? tacho_velocity + (2.0 - UtilityFunctions::randf_range(0.0, 3.0) +
                                                              UtilityFunctions::randf_range(0.0, 3.0)) *
                                                                     0.5
                                          : 0.0;
        }

        // ticking starts ~1 s after moving off and fades out slowly after stopping
        if (tacho_velocity > 1.0) {
            tacho_count = std::min(max_tacho, tacho_count + p_delta * 3.0);
        } else if (tacho_count > 0.0) {
            tacho_count = std::max(0.0, tacho_count - p_delta * 0.66);
        }
        if (tacho_count >= 3.0) {
            tacho_clock_active = true;
        } else if (tacho_count < 1.0) {
            tacho_clock_active = false;
        }
    }

    void VehicleController::update_state() {
        _handle_mover_update();
    }

    /// Only marks the state for a rebuild - whoever reads it gets it fresh (see get_state()). The
    /// signals below have to be decided every step though, so they read the mover directly rather
    /// than through a dictionary that may not be built at all.
    void VehicleController::_handle_mover_update() {
        state_dirty = true;
        TMoverParameters *mover_ptr = get_mover();
        if (mover_ptr == nullptr) {
            return;
        }
        _consume_coupler_sounds(mover_ptr);

        const bool new_is_powered = mover_ptr->Power24vIsAvailable || mover_ptr->Power110vIsAvailable;
        if (prev_is_powered != new_is_powered) {
            prev_is_powered = new_is_powered; // FIXME: I don't like this
            emit_signal(power_changed_signal, prev_is_powered);
        }

        if (const bool new_radio_enabled = mover_ptr->Radio && new_is_powered;
            prev_radio_enabled != new_radio_enabled) {
            prev_radio_enabled = new_radio_enabled; // FIXME: I don't like this
            emit_signal(radio_toggled, new_radio_enabled);
        }

        if (const int new_radio_channel = radio_channel; prev_radio_channel != new_radio_channel) {
            prev_radio_channel = new_radio_channel; // FIXME: I don't like this
            emit_signal(radio_channel_changed, new_radio_channel);
        }

        if (const bool new_roof_light_enabled = state.get("roof_light_enabled", false);
            prev_roof_light_enabled != new_roof_light_enabled) {
            prev_roof_light_enabled = new_roof_light_enabled; // FIXME: I don't like this
            emit_signal(roof_light_changed, new_roof_light_enabled);
        }

        if (const int new_cabin_occupied = mover_ptr->CabOccupied; prev_cabin_occupied != new_cabin_occupied) {
            prev_cabin_occupied = new_cabin_occupied;
            emit_signal(cabin_occupied_changed, new_cabin_occupied);
        }
    }

    void VehicleController::_process(const double p_delta) {
        /* nie daj borze w edytorze */
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        // controllers registered in RailVehiclePhysicsServer are stepped by its global tick
        if (rid.is_valid()) {
            return;
        }
        _process_mover(p_delta);
    }

    Object *VehicleController::_get_rail_vehicle_physics_server() const {
        return Engine::get_singleton()->get_singleton("RailVehiclePhysicsServer");
    }

    double VehicleController::process_movement(const double p_delta) {
        return mover != nullptr ? mover->V * p_delta : 0.0;
    }

    void VehicleController::_emit_position_changed_if_needed() {
        const Vector3 position = get_world_position();
        if (position.distance_to(last_emitted_position) < 1.0) {
            return;
        }
        last_emitted_position = position;
        emit_signal(position_changed_signal, position);
    }

    void VehicleController::_do_update_internal_mover(TMoverParameters *p_mover) const {
        p_mover->Mass = mass;
        p_mover->Power = power;
        p_mover->Vmax = max_velocity;
        p_mover->Mred = reduced_mass;

        p_mover->ComputeMass();

        p_mover->CategoryFlag = category;
        p_mover->TrainType = train_type;
        p_mover->SandCapacity = static_cast<int>(sand_capacity);
        p_mover->HeatingPower = heating_power;
        p_mover->LightPower = light_power;

        p_mover->Dim.L = dimensions_length;
        p_mover->Dim.H = dimensions_height;
        p_mover->Dim.W = dimensions_width;
        p_mover->Cx = dimensions_drag_coefficient;
        p_mover->Floor = static_cast<float>(dimensions_floor_height);

        p_mover->BatteryStart = start_mode_map.at(cntrl_battery_start_mode);
        p_mover->GroundRelayStart = start_mode_map.at(cntrl_ground_relay_start_mode);
        p_mover->CompartmentLights.start_type = start_mode_map.at(cntrl_compartment_lights_start_mode);
        p_mover->AutomaticCabActivation = cntrl_automatic_cab_activation;
        p_mover->InactiveCabFlag = cntrl_inactive_cab_flag;

        // FIXME: move to TrainPower
        p_mover->BatteryVoltage = battery_voltage;
        p_mover->NominalBatteryVoltage = static_cast<float>(battery_voltage); // LoadFIZ_Light
    }

    void VehicleController::_do_fetch_config_from_mover(const TMoverParameters *p_mover, Dictionary &p_config) const {
        // Vehicle-wide, not brake-specific - p_mover->Vmax is set from this same max_velocity
        // property (see apply_config() below), so this is a thin alias, not new derivation.
        p_config["max_speed"] = max_velocity;
        p_config["power"] = p_mover->Power;
        p_config["length"] = p_mover->Dim.L;
    }

    void VehicleController::apply_config() {
        if (TMoverParameters *mover = get_mover(); mover != nullptr) {
            _do_update_internal_mover(mover);
            Dictionary new_config;
            _do_fetch_config_from_mover(mover, new_config);
            update_config(new_config);

            /* FIXME: CheckLocomotiveParameters should be called after (re)initialization */
            mover->CheckLocomotiveParameters(initial_velocity != 0.0, 0); // FIXME: brakujace parametery
            initialize_mover_state();
        } else {
            UtilityFunctions::push_warning("VehicleController::apply_config() failed: internal mover not initialized");
        }
    }

    // Original engine: coupler attach/detach sounds (DynObj.cpp:4855-4905) - each request of the mover
    // (TCoupling::sounds) bumps a counter the sound triggers play on; the flags are consumed as there.
    //
    // Consuming is a tick job, not a read job: this clears the mover's flags, so doing it while
    // filling the state dictionary made the events belong to whoever happened to read first.
    void VehicleController::_consume_coupler_sounds(TMoverParameters *p_mover) {
        static const int flags[] = {sound::attachcoupler, sound::attachbrakehose, sound::attachmainhose,
                                    sound::attachcontrol, sound::attachgangway,   sound::attachheating};
        for (TCoupling &coupler: p_mover->Couplers) {
            if (coupler.sounds == sound::none) {
                continue;
            }
            const bool detaching = (coupler.sounds & sound::detach) != 0;
            for (int index = 0; index < 6; ++index) {
                if ((coupler.sounds & flags[index]) != 0) {
                    emit_signal(detaching ? coupler_detached_signal : coupler_attached_signal,
                                static_cast<CouplingElement>(index));
                }
            }
            coupler.sounds = sound::none;
        }
    }

    void VehicleController::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        p_state["mass_total"] = p_mover->TotalMass;
        p_state["velocity"] = p_mover->V;
        p_state["speed"] = p_mover->Vel;
        p_state["tachometer_speed"] = tacho_velocity;
        p_state["tachometer_speed_jump"] = tacho_velocity_jump;
        // tachoclock chunk parameter; 0 keeps the sound stopped (Train.cpp:8323-8335)
        p_state["tachometer_clock_speed"] = tacho_clock_active ? tacho_velocity : 0.0;
        p_state["total_distance"] = p_mover->DistCounter;
        p_state["direction"] = p_mover->DirActive;
        // reverser as the traction side sees it; the smoke emitter tells an idling engine from a
        // pulling one by it (particles.cpp:193)
        p_state["direction_absolute"] = p_mover->DirAbsolute;
        p_state["cabin"] = p_mover->CabActive;
        p_state["cabin_controleable"] = p_mover->IsCabMaster();
        p_state["cabin_occupied"] = p_mover->CabOccupied;

        /* FIXME: move to TrainPower section? */
        p_state["battery_enabled"] = p_mover->Battery;
        p_state["battery_voltage"] = p_mover->BatteryVoltage;

        /* FIXME: move to TrainRadio section? */
        p_state["radio_enabled"] = p_mover->Radio;
        p_state["radio_powered"] = p_mover->Radio && (p_mover->Power24vIsAvailable || p_mover->Power110vIsAvailable);
        p_state["radio_channel"] = radio_channel;

        /* FIXME: move to TrainPower section */
        p_state["power24_voltage"] = p_mover->Power24vVoltage;
        p_state["power24_available"] = p_mover->Power24vIsAvailable;
        p_state["power110_available"] = p_mover->Power110vIsAvailable;
        p_state["current0"] = p_mover->ShowCurrent(0);
        p_state["current1"] = p_mover->ShowCurrent(1);
        p_state["current2"] = p_mover->ShowCurrent(2);
        p_state["relay_novolt"] = p_mover->NoVoltRelay;
        p_state["relay_overvoltage"] = p_mover->OvervoltageRelay;
        p_state["relay_ground"] = p_mover->GroundRelay;
        p_state["train_damage"] = p_mover->DamageFlag;
        p_state["controller_second_position"] = p_mover->ScndCtrlPos;
        p_state["controller_main_position"] = p_mover->MainCtrlPos;
        // joint master controller position - negative range is the local brake (Train.cpp:7699-7714)
        p_state["controller_joint_position"] =
                p_mover->LocalBrakePosA > 0.0
                        ? static_cast<int>(std::round(-p_mover->LocalBrakePosA * LocalBrakePosNo))
                        : (p_mover->CoupledCtrl ? p_mover->MainCtrlPos + p_mover->ScndCtrlPos : p_mover->MainCtrlPos);
        // Diagnostic: the delayed/rate-limited shadow of MainCtrlPos that RList[] resistor
        // lookups actually key off (Mover.cpp's internal auto-relay/resistor-stepping state
        // machine) - a wrong RList[] mapping or array-bounds issue lets this race far ahead of
        // MainCtrlPos, landing on unpopulated (zero-resistance) table slots.
        p_state["controller_main_actual_position"] = p_mover->MainCtrlActualPos;
        p_state["circuit_rlist_size"] = p_mover->RlistSize;
    }

    Dictionary VehicleController::get_config() const {
        return config;
    }

    void VehicleController::update_config(const Dictionary &p_config) {
        config.merge(p_config, true);
        emit_signal(config_changed);
    }

    /// Rebuilt from the mover on the first read after a physics step; the train parts merge their
    /// own keys into it as they process, so those stay where they are
    Dictionary VehicleController::get_state() {
        if (state_dirty) {
            state_dirty = false;
            if (TMoverParameters *mover_ptr = get_mover(); mover_ptr != nullptr) {
                _do_fetch_state_from_mover(mover_ptr, state);
            }
        }
        return state;
    }

    double VehicleController::get_velocity() const {
        return mover != nullptr ? mover->V : 0.0;
    }

    void
    VehicleController::change_track(const String &p_track_name, const float p_track_offset, const int p_track_direction) {
        UtilityFunctions::push_warning(
                vformat("VehicleController::change_track() is managed by RailVehicle3D now: %s / %.3f / %d", p_track_name,
                        p_track_offset, p_track_direction));
    }

    Vector3 VehicleController::get_world_position() const {
        return get_world_transform().get_origin();
    }

    Transform3D VehicleController::get_world_transform() const {
        if (Object *rail_vehicle_physics_server = _get_rail_vehicle_physics_server();
            rail_vehicle_physics_server != nullptr && rid.is_valid()) {
            return rail_vehicle_physics_server->call("controller_get_transform", rid);
        }
        return Transform3D();
    }

    RID VehicleController::get_rid() const {
        return rid;
    }

    void
    VehicleController::emit_command_received_signal(const String &p_command, const Variant &p_p1, const Variant &p_p2) {
        emit_signal(command_received, p_command, p_p1, p_p2);
    }

    void VehicleController::broadcast_command(const String &p_command, const Variant &p_p1, const Variant &p_p2) {
        TrainSystem::get_instance()->broadcast_command(p_command, p_p1, p_p2);
    }

    Variant VehicleController::send_command(const StringName &p_command, const Variant &p_p1, const Variant &p_p2) const {
        return TrainSystem::get_instance()->send_command(train_id, String(p_command), p_p1, p_p2);
    }

    void VehicleController::battery(const bool p_enabled) const {
        mover->BatterySwitch(p_enabled);
    }

    // Original engine: OnCommand_cabactivationenable/disable (Train.cpp:2430-2472)
    void VehicleController::cab_activation(const bool p_enabled) const {
        if (p_enabled) {
            mover->CabActivisation();
            return;
        }
        mover->CabDeactivisation();
    }

    // Original engine: taking over a vehicle activates its cab if the FIZ allows automatic
    // activation (Train.cpp:9086, 9147); otherwise the driver uses cab_activation
    void VehicleController::cab_activation_auto() const {
        mover->CabActivisationAuto(true);
    }

    // Original engine: TTrain::CabChange() (Train.cpp:8516) - steps 1 -> 0 (machine room) -> -1.
    void VehicleController::cab_change(const int p_direction) const {
        mover->CabDeactivisationAuto();
        mover->ChangeCab(p_direction);
        mover->CabActivisationAuto();
    }

    void VehicleController::set_cabin_number(const int p_value) {
        cabin_number = p_value;
    }

    int VehicleController::get_cabin_number() const {
        return cabin_number;
    }

    void VehicleController::main_controller_increase(const int p_step) const {
        const int step = p_step > 0 ? p_step : 1;
        mover->IncMainCtrl(step);
    }

    void VehicleController::main_controller_decrease(const int p_step) const {
        const int step = p_step > 0 ? p_step : 1;
        mover->DecMainCtrl(step);
    }

    // Original engine: OnCommand_secondcontrollerincrease/decrease (Train.cpp:1188, 1349), regular mode
    void VehicleController::second_controller_increase(const int p_step) const {
        const int step = p_step > 0 ? p_step : 1;
        mover->IncScndCtrl(step);
    }

    void VehicleController::second_controller_decrease(const int p_step) const {
        const int step = p_step > 0 ? p_step : 1;
        mover->DecScndCtrl(step);
    }

    void VehicleController::direction_increase() const {
        mover->DirectionForward();
    }

    void VehicleController::direction_decrease() const {
        mover->DirectionBackward();
    }

    void VehicleController::radio_channel_increase(const int p_step) {
        const int step = p_step > 0 ? p_step : 1;
        radio_channel = Math::clamp(radio_channel + step, radio_channel_min, radio_channel_max);
    }

    void VehicleController::radio_channel_decrease(const int p_step) {
        const int step = (p_step != 0) ? p_step : 1;
        radio_channel = Math::clamp(radio_channel - step, radio_channel_min, radio_channel_max);
    }

    void VehicleController::radio_channel_set(const int p_channel) {
        radio_channel = Math::clamp(p_channel, radio_channel_min, radio_channel_max);
    }

    void VehicleController::radio(const bool p_enabled) {
        mover->Radio = p_enabled;
    }
} // namespace godot
