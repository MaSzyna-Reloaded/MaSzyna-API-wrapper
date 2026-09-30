#include "RailVehicleController.hpp"
#include "vehicles/base/VehicleComponent.hpp"
#include "vehicles/base/VehicleServer.hpp"

namespace godot {
    const char *RailVehicleController::power_changed_signal = "power_changed";
    const char *RailVehicleController::cabin_occupied_changed = "cabin_occupied_changed";
    const char *RailVehicleController::trainset_changed_signal = "trainset_changed";
    const char *RailVehicleController::coupler_attached_signal = "coupler_attached";
    const char *RailVehicleController::coupler_detached_signal = "coupler_detached";

    Dictionary RailVehicleController::get_state() {
        VehicleServer *server = VehicleServer::get_instance();
        if (server == nullptr) {
            return Dictionary();
        }
        // An unknown or invalid handle has no vehicle, so the server answers an empty dictionary
        // on its own - there is no second way of producing the state here.
        return server->vehicle_dump_state(get_rid());
    }

    Ref<VehicleComponent> RailVehicleController::get_rail_component(const RailVehicleComponentType::Type p_type) const {
        return _get_component_of_type(p_type);
    }

    TypedArray<VehicleComponent>
    RailVehicleController::find_rail_components(const RailVehicleComponentType::Type p_type) const {
        return _find_components_of_type(p_type);
    }

    void RailVehicleController::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_rail_component", "type"), &RailVehicleController::get_rail_component);
        ClassDB::bind_method(D_METHOD("find_rail_components", "type"), &RailVehicleController::find_rail_components);
        ClassDB::bind_method(D_METHOD("battery", "enabled"), &RailVehicleController::battery);
        ClassDB::bind_method(D_METHOD("converter", "enabled"), &RailVehicleController::converter);
        ClassDB::bind_method(D_METHOD("cab_activation", "enabled"), &RailVehicleController::cab_activation);
        ClassDB::bind_method(D_METHOD("cab_activation_auto"), &RailVehicleController::cab_activation_auto);
        ClassDB::bind_method(D_METHOD("cab_change", "direction"), &RailVehicleController::cab_change);
        ClassDB::bind_method(D_METHOD("ground_relay_reset"), &RailVehicleController::ground_relay_reset);
        ClassDB::bind_method(D_METHOD("antislip"), &RailVehicleController::antislip);
        ClassDB::bind_method(
                D_METHOD("main_controller_increase", "step"), &RailVehicleController::main_controller_increase,
                DEFVAL(1));
        ClassDB::bind_method(
                D_METHOD("main_controller_decrease", "step"), &RailVehicleController::main_controller_decrease,
                DEFVAL(1));
        ClassDB::bind_method(
                D_METHOD("second_controller_increase", "step"), &RailVehicleController::second_controller_increase,
                DEFVAL(1));
        ClassDB::bind_method(
                D_METHOD("second_controller_decrease", "step"), &RailVehicleController::second_controller_decrease,
                DEFVAL(1));
        ClassDB::bind_method(D_METHOD("direction_increase"), &RailVehicleController::direction_increase);
        ClassDB::bind_method(D_METHOD("direction_decrease"), &RailVehicleController::direction_decrease);
        ClassDB::bind_method(
                D_METHOD("distance_counter_activate", "pressed"), &RailVehicleController::distance_counter_activate);
        ClassDB::bind_method(D_METHOD("process_movement", "delta"), &RailVehicleController::process_movement);
        ClassDB::bind_method(D_METHOD("update_location"), &RailVehicleController::update_location);
        ClassDB::bind_method(
                D_METHOD("update_neighbour", "end", "other", "other_end", "track_distance"),
                &RailVehicleController::update_neighbour);
        ClassDB::bind_method(D_METHOD("compute_forces", "delta"), &RailVehicleController::compute_forces);
        ClassDB::bind_method(D_METHOD("compute_movement", "delta"), &RailVehicleController::compute_movement);
        ClassDB::bind_method(D_METHOD("compute_fast_movement", "delta"), &RailVehicleController::compute_fast_movement);
        ClassDB::bind_method(
                D_METHOD("couple", "other", "end", "other_end", "coupling_type"), &RailVehicleController::couple);
        ClassDB::bind_method(D_METHOD("uncouple", "end"), &RailVehicleController::uncouple);
        ClassDB::bind_method(D_METHOD("is_coupled", "end"), &RailVehicleController::is_coupled);
        ClassDB::bind_method(D_METHOD("is_coupled_by", "end", "element"), &RailVehicleController::is_coupled_by);
        ClassDB::bind_method(D_METHOD("get_coupled_controller", "end"), &RailVehicleController::get_coupled_controller);
        ClassDB::bind_method(D_METHOD("get_coupled_end", "end"), &RailVehicleController::get_coupled_end);
        ClassDB::bind_method(D_METHOD("coupler_connect", "where"), &RailVehicleController::coupler_connect);
        ClassDB::bind_method(D_METHOD("coupler_disconnect", "where"), &RailVehicleController::coupler_disconnect);
        /* FIXME: move to TrainPower section? */
        BIND_PROPERTY_W_HINT(RailVehicleController, Variant::FLOAT, battery_voltage, PROPERTY_HINT_RANGE, "0,500,1");
        BIND_PROPERTY_W_HINT(
                RailVehicleController, Variant::INT, train_type, PROPERTY_HINT_ENUM,
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
        BIND_PROPERTY(RailVehicleController, Variant::STRING, type_name);
        BIND_PROPERTY(RailVehicleController, Variant::STRING, load_name);
        BIND_PROPERTY(RailVehicleController, Variant::FLOAT, load_amount);
        BIND_PROPERTY(RailVehicleController, Variant::FLOAT, reduced_mass);
        BIND_PROPERTY(RailVehicleController, Variant::FLOAT, sand_capacity);
        BIND_PROPERTY(RailVehicleController, Variant::FLOAT, heating_power);
        BIND_PROPERTY(RailVehicleController, Variant::FLOAT, light_power);
        BIND_PROPERTY_W_HINT(
                RailVehicleController, Variant::INT, cntrl_battery_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY_W_HINT(
                RailVehicleController, Variant::INT, cntrl_converter_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(RailVehicleController, Variant::FLOAT, cntrl_converter_start_delay, "cntrl");
        BIND_PROPERTY_W_HINT(
                RailVehicleController, Variant::INT, cntrl_ground_relay_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY_W_HINT(
                RailVehicleController, Variant::INT, cntrl_compartment_lights_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(RailVehicleController, Variant::BOOL, cntrl_automatic_cab_activation, "cntrl");
        BIND_PROPERTY_W_HINT(
                RailVehicleController, Variant::INT, cntrl_inactive_cab_flag, "cntrl", PROPERTY_HINT_FLAGS,
                "Emergency Brake,Toggle Mirrors,Raise Second Pantograph,End Of Train Lights,Grant Both Side Permits,"
                "Apply Spring Brake,Release Spring Brake,Reset Direction");

        ADD_SIGNAL(MethodInfo(power_changed_signal, PropertyInfo(Variant::BOOL, "is_powered")));
        ADD_SIGNAL(MethodInfo(cabin_occupied_changed, PropertyInfo(Variant::INT, "cabin_occupied")));
        ADD_SIGNAL(MethodInfo(trainset_changed_signal));
        const String coupling_element_hint = enum_hint(
                {{"Coupler", COUPLING_ELEMENT_COUPLER},
                 {"BrakeHose", COUPLING_ELEMENT_BRAKEHOSE},
                 {"MainHose", COUPLING_ELEMENT_MAINHOSE},
                 {"Control", COUPLING_ELEMENT_CONTROL},
                 {"Gangway", COUPLING_ELEMENT_GANGWAY},
                 {"Heating", COUPLING_ELEMENT_HEATING},
                 {"Permanent", COUPLING_ELEMENT_PERMANENT}});
        ADD_SIGNAL(MethodInfo(
                coupler_attached_signal,
                PropertyInfo(Variant::INT, "element", PROPERTY_HINT_ENUM, coupling_element_hint)));
        ADD_SIGNAL(MethodInfo(
                coupler_detached_signal,
                PropertyInfo(Variant::INT, "element", PROPERTY_HINT_ENUM, coupling_element_hint)));

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
        BIND_ENUM_CONSTANT(COUPLING_ELEMENT_PERMANENT);

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

        ClassDB::bind_method(D_METHOD("get_tachometer_speed"), &RailVehicleController::get_tachometer_speed);
        ClassDB::bind_method(D_METHOD("get_tachometer_speed_jump"), &RailVehicleController::get_tachometer_speed_jump);
        ClassDB::bind_method(
                D_METHOD("get_tachometer_clock_speed"), &RailVehicleController::get_tachometer_clock_speed);
        ClassDB::bind_method(D_METHOD("get_direction_absolute"), &RailVehicleController::get_direction_absolute);
        ClassDB::bind_method(D_METHOD("get_cabin"), &RailVehicleController::get_cabin);
        ClassDB::bind_method(D_METHOD("get_cabin_controleable"), &RailVehicleController::get_cabin_controleable);
        ClassDB::bind_method(D_METHOD("get_cabin_occupied"), &RailVehicleController::get_cabin_occupied);
        ClassDB::bind_method(D_METHOD("get_live_battery_voltage"), &RailVehicleController::get_live_battery_voltage);
        ClassDB::bind_method(D_METHOD("get_battery_enabled"), &RailVehicleController::get_battery_enabled);
        ClassDB::bind_method(D_METHOD("get_converter_enabled"), &RailVehicleController::get_converter_enabled);
        ClassDB::bind_method(D_METHOD("get_converter_allowed"), &RailVehicleController::get_converter_allowed);
        ClassDB::bind_method(
                D_METHOD("get_converter_time_to_start"), &RailVehicleController::get_converter_time_to_start);
        ClassDB::bind_method(D_METHOD("get_distance_counter"), &RailVehicleController::get_distance_counter);
        ClassDB::bind_method(D_METHOD("get_power24_voltage"), &RailVehicleController::get_power24_voltage);
        ClassDB::bind_method(D_METHOD("get_power24_available"), &RailVehicleController::get_power24_available);
        ClassDB::bind_method(D_METHOD("get_power110_available"), &RailVehicleController::get_power110_available);
        ClassDB::bind_method(D_METHOD("get_current0"), &RailVehicleController::get_current0);
        ClassDB::bind_method(D_METHOD("get_current1"), &RailVehicleController::get_current1);
        ClassDB::bind_method(D_METHOD("get_current2"), &RailVehicleController::get_current2);
        ClassDB::bind_method(D_METHOD("get_relay_novolt"), &RailVehicleController::get_relay_novolt);
        ClassDB::bind_method(D_METHOD("get_relay_overvoltage"), &RailVehicleController::get_relay_overvoltage);
        ClassDB::bind_method(D_METHOD("get_relay_ground"), &RailVehicleController::get_relay_ground);
        ClassDB::bind_method(D_METHOD("get_train_damage"), &RailVehicleController::get_train_damage);
        ClassDB::bind_method(
                D_METHOD("get_controller_second_position"), &RailVehicleController::get_controller_second_position);
        ClassDB::bind_method(
                D_METHOD("get_controller_main_position"), &RailVehicleController::get_controller_main_position);
        ClassDB::bind_method(
                D_METHOD("get_controller_joint_position"), &RailVehicleController::get_controller_joint_position);
        ClassDB::bind_method(
                D_METHOD("get_controller_second_actual_position"),
                &RailVehicleController::get_controller_second_actual_position);
        ClassDB::bind_method(D_METHOD("get_mass_reduced"), &RailVehicleController::get_mass_reduced);
        ClassDB::bind_method(
                D_METHOD("get_controller_main_actual_position"),
                &RailVehicleController::get_controller_main_actual_position);
        ClassDB::bind_method(
                D_METHOD("get_controller_main_delayed"), &RailVehicleController::get_controller_main_delayed);
        ClassDB::bind_method(D_METHOD("get_coupler_stretched"), &RailVehicleController::get_coupler_stretched);
        ClassDB::bind_method(
                D_METHOD("get_controller_main_no_power_position"),
                &RailVehicleController::get_controller_main_no_power_position);
        ClassDB::bind_method(D_METHOD("get_radio_stop_active"), &RailVehicleController::get_radio_stop_active);
        ClassDB::bind_method(D_METHOD("get_circuit_rlist_size"), &RailVehicleController::get_circuit_rlist_size);
    }

    void RailVehicleController::_register_commands() {
        register_command("battery", Callable(this, "battery"));
        register_command("converter", Callable(this, "converter"));
        register_command("cab_change", Callable(this, "cab_change"));
        register_command("ground_relay_reset", Callable(this, "ground_relay_reset"));
        register_command("antislip", Callable(this, "antislip"));
        register_command("cab_activation", Callable(this, "cab_activation"));
        register_command("cab_activation_auto", Callable(this, "cab_activation_auto"));
        register_command("main_controller_increase", Callable(this, "main_controller_increase"));
        register_command("main_controller_decrease", Callable(this, "main_controller_decrease"));
        register_command("second_controller_increase", Callable(this, "second_controller_increase"));
        register_command("second_controller_decrease", Callable(this, "second_controller_decrease"));
        register_command("direction_increase", Callable(this, "direction_increase"));
        register_command("direction_decrease", Callable(this, "direction_decrease"));
        register_command("distance_counter_activate", Callable(this, "distance_counter_activate"));
        register_command("coupler_connect", Callable(this, "coupler_connect"));
        register_command("coupler_disconnect", Callable(this, "coupler_disconnect"));
    }

    void RailVehicleController::_unregister_commands() {
        unregister_command("battery");
        unregister_command("converter");
        unregister_command("cab_change");
        unregister_command("ground_relay_reset");
        unregister_command("antislip");
        unregister_command("cab_activation");
        unregister_command("cab_activation_auto");
        unregister_command("main_controller_increase");
        unregister_command("main_controller_decrease");
        unregister_command("second_controller_increase");
        unregister_command("second_controller_decrease");
        unregister_command("direction_increase");
        unregister_command("direction_decrease");
        unregister_command("distance_counter_activate");
        unregister_command("coupler_connect");
        unregister_command("coupler_disconnect");
    }

    /* The low voltage and the occupied cab are decided every step, from the live getters, as the
     * vehicle's own signals are (VehicleController::update_state()). */
    void RailVehicleController::update_state() {
        if (!is_simulation_ready()) {
            return;
        }
        VehicleController::update_state();
        const bool new_is_powered = get_power24_available() || get_power110_available();
        if (prev_is_powered != new_is_powered) {
            prev_is_powered = new_is_powered; // FIXME: I don't like this
            emit_signal(power_changed_signal, prev_is_powered);
        }

        if (const int new_cabin_occupied = get_cabin_occupied(); prev_cabin_occupied != new_cabin_occupied) {
            prev_cabin_occupied = new_cabin_occupied;
            emit_signal(cabin_occupied_changed, new_cabin_occupied);
        }
    }

    /* Listeners expect the low voltage's initial value, as they get the roof light's. */
    void RailVehicleController::initialize() {
        VehicleController::initialize();
        emit_signal(power_changed_signal, prev_is_powered);
    }

    void RailVehicleController::_fill_state_dictionary(Dictionary &p_state) const {
        VehicleController::_fill_state_dictionary(p_state);
        if (!is_simulation_ready()) {
            return;
        }
        p_state["tachometer_speed"] = get_tachometer_speed();
        p_state["tachometer_speed_jump"] = get_tachometer_speed_jump();
        p_state["tachometer_clock_speed"] = get_tachometer_clock_speed();
        p_state["direction_absolute"] = get_direction_absolute();
        p_state["cabin"] = get_cabin();
        p_state["cabin_controleable"] = get_cabin_controleable();
        p_state["cabin_occupied"] = get_cabin_occupied();
        p_state["battery_enabled"] = get_battery_enabled();
        p_state["battery_voltage"] = get_live_battery_voltage();
        p_state["converter_enabled"] = get_converter_enabled();
        p_state["converter_allowed"] = get_converter_allowed();
        p_state["converter_time_to_start"] = get_converter_time_to_start();
        p_state["distance_counter"] = get_distance_counter();
        p_state["power24_voltage"] = get_power24_voltage();
        p_state["power24_available"] = get_power24_available();
        p_state["power110_available"] = get_power110_available();
        p_state["current0"] = get_current0();
        p_state["current1"] = get_current1();
        p_state["current2"] = get_current2();
        p_state["relay_novolt"] = get_relay_novolt();
        p_state["relay_overvoltage"] = get_relay_overvoltage();
        p_state["relay_ground"] = get_relay_ground();
        p_state["train_damage"] = get_train_damage();
        p_state["controller_second_position"] = get_controller_second_position();
        p_state["controller_main_position"] = get_controller_main_position();
        p_state["controller_joint_position"] = get_controller_joint_position();
        p_state["controller_main_actual_position"] = get_controller_main_actual_position();
        p_state["controller_second_actual_position"] = get_controller_second_actual_position();
        p_state["mass_reduced"] = get_mass_reduced();
        p_state["controller_main_delayed"] = get_controller_main_delayed();
        p_state["coupler_stretched"] = get_coupler_stretched();
        p_state["controller_main_no_power_position"] = get_controller_main_no_power_position();
        p_state["radio_stop_active"] = get_radio_stop_active();
        p_state["circuit_rlist_size"] = get_circuit_rlist_size();
    }

} // namespace godot
