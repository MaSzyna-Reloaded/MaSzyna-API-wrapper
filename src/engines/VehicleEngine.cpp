#include "VehicleEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    class VehicleController;
    bool VehicleEngine::get_main_switch_enabled() const {
        return engine_backend != nullptr ? engine_backend->get_main_switch_enabled(this) : false;
    }
    bool VehicleEngine::get_main_switch_closable() const {
        return engine_backend != nullptr ? engine_backend->get_main_switch_closable(this) : false;
    }
    double VehicleEngine::get_motor_torque() const {
        return engine_backend != nullptr ? engine_backend->get_motor_torque(this) : 0.0;
    }
    double VehicleEngine::get_wheel_torque() const {
        return engine_backend != nullptr ? engine_backend->get_wheel_torque(this) : 0.0;
    }
    double VehicleEngine::get_wheel_force() const {
        return engine_backend != nullptr ? engine_backend->get_wheel_force(this) : 0.0;
    }
    double VehicleEngine::get_tractive_force() const {
        return engine_backend != nullptr ? engine_backend->get_tractive_force(this) : 0.0;
    }
    bool VehicleEngine::get_compressor_enabled() const {
        return engine_backend != nullptr ? engine_backend->get_compressor_enabled(this) : false;
    }
    bool VehicleEngine::get_compressor_allowed() const {
        return engine_backend != nullptr ? engine_backend->get_compressor_allowed(this) : false;
    }
    double VehicleEngine::get_power() const {
        return engine_backend != nullptr ? engine_backend->get_power(this) : 0.0;
    }
    double VehicleEngine::get_rpm_count() const {
        return engine_backend != nullptr ? engine_backend->get_rpm_count(this) : 0.0;
    }
    double VehicleEngine::get_rpm_ratio() const {
        return engine_backend != nullptr ? engine_backend->get_rpm_ratio(this) : 0.0;
    }
    double VehicleEngine::get_circuit_nmax_rpm() const {
        return engine_backend != nullptr ? engine_backend->get_circuit_nmax_rpm(this) : 0.0;
    }
    int VehicleEngine::get_damage() const {
        return engine_backend != nullptr ? engine_backend->get_damage(this) : 0;
    }
    double VehicleEngine::get_main_switch_time() const {
        return engine_backend != nullptr ? engine_backend->get_main_switch_time(this) : 0.0;
    }
    bool VehicleEngine::get_main_no_power_pos() const {
        return engine_backend != nullptr ? engine_backend->get_main_no_power_pos(this) : false;
    }
    void VehicleEngine::_apply_configuration() {
        VehicleComponent::_apply_configuration();
        if (engine_backend != nullptr) {
            engine_backend->apply_configuration(this);
        }
    }
    void VehicleEngine::_fill_config_dictionary(Dictionary &p_config) const {
        VehicleComponent::_fill_config_dictionary(p_config);
        if (engine_backend != nullptr) {
            engine_backend->fill_config(this, p_config);
        }
    }

    void VehicleEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("main_switch", "enabled"), &VehicleEngine::main_switch);
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleEngine, Variant::ARRAY, motor_param_table, PROPERTY_HINT_TYPE_STRING, "MotorParameter");
        BIND_PROPERTY(VehicleEngine, Variant::INT, transmission_gear_teeth_motor, "transmission");
        BIND_PROPERTY(VehicleEngine, Variant::INT, transmission_gear_teeth_wheel, "transmission");
        BIND_PROPERTY(VehicleEngine, Variant::FLOAT, transmission_efficiency, "transmission");
        BIND_PROPERTY(VehicleEngine, Variant::FLOAT, maximum_traction_force);
        BIND_PROPERTY(VehicleEngine, Variant::FLOAT, motor_blowers_speed, "motor_blowers");
        BIND_PROPERTY(VehicleEngine, Variant::FLOAT, motor_blowers_sustain_time, "motor_blowers");
        BIND_PROPERTY(VehicleEngine, Variant::FLOAT, motor_blowers_start_velocity, "motor_blowers");
        BIND_PROPERTY(VehicleEngine, Variant::BOOL, pressure_switch_present);
        BIND_PROPERTY(VehicleEngine, Variant::INT, inverters_count);
        BIND_PROPERTY_W_HINT(
                VehicleEngine, Variant::INT, motor_blowers_start_mode, "motor_blowers", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(VehicleEngine, Variant::INT, cntrl_main_controller_position_count, "cntrl");
        BIND_PROPERTY(VehicleEngine, Variant::INT, cntrl_shunt_controller_position_count, "cntrl");
        BIND_PROPERTY(VehicleEngine, Variant::INT, cntrl_direction_change_max_position, "cntrl");
        BIND_PROPERTY(VehicleEngine, Variant::BOOL, cntrl_eim_control_additional_zeros, "cntrl");
        BIND_PROPERTY(VehicleEngine, Variant::BOOL, cntrl_eim_control_emergency, "cntrl");
        BIND_PROPERTY_W_HINT(VehicleEngine, Variant::INT, cntrl_eim_control_type, "cntrl", PROPERTY_HINT_ENUM, "0,1,2,3");
        BIND_PROPERTY_W_HINT(
                VehicleEngine, Variant::INT, cntrl_auto_relay_mode, "cntrl", PROPERTY_HINT_ENUM, "No,Yes,Optional");
        BIND_PROPERTY(VehicleEngine, Variant::BOOL, cntrl_coupled_controllers, "cntrl");
        BIND_PROPERTY(VehicleEngine, Variant::BOOL, cntrl_has_camshaft, "cntrl");
        BIND_PROPERTY(VehicleEngine, Variant::BOOL, cntrl_series_shunt_on_series_position, "cntrl");
        BIND_PROPERTY(VehicleEngine, Variant::FLOAT, cntrl_initial_controller_delay, "cntrl");
        BIND_PROPERTY(VehicleEngine, Variant::FLOAT, cntrl_controller_step_delay, "cntrl");
        BIND_PROPERTY(VehicleEngine, Variant::FLOAT, cntrl_controller_step_down_delay, "cntrl");
        BIND_PROPERTY(VehicleEngine, Variant::BOOL, cntrl_fast_series_circuit, "cntrl");
        ADD_SIGNAL(MethodInfo("engine_start"));
        ADD_SIGNAL(MethodInfo("engine_stop"));

        BIND_ENUM_CONSTANT(NONE);
        BIND_ENUM_CONSTANT(DUMB);
        BIND_ENUM_CONSTANT(WHEELS_DRIVEN);
        BIND_ENUM_CONSTANT(ELECTRIC_SERIES_MOTOR);
        BIND_ENUM_CONSTANT(ELECTRIC_INDUCTION_MOTOR);
        BIND_ENUM_CONSTANT(DIESEL);
        BIND_ENUM_CONSTANT(STEAM);
        BIND_ENUM_CONSTANT(DIESEL_ELECTRIC);
        BIND_ENUM_CONSTANT(MAIN);

        BIND_ENUM_CONSTANT(START_MODE_DISABLED);
        BIND_ENUM_CONSTANT(START_MODE_MANUAL);
        BIND_ENUM_CONSTANT(START_MODE_AUTOMATIC);
        BIND_ENUM_CONSTANT(START_MODE_MANUAL_WITH_AUTO_FALLBACK);
        BIND_ENUM_CONSTANT(START_MODE_CONVERTER);
        BIND_ENUM_CONSTANT(START_MODE_BATTERY);
        BIND_ENUM_CONSTANT(START_MODE_DIRECTION);

        BIND_ENUM_CONSTANT(EIM_CONTROL_TYPE_0);
        BIND_ENUM_CONSTANT(EIM_CONTROL_TYPE_1);
        BIND_ENUM_CONSTANT(EIM_CONTROL_TYPE_2);
        BIND_ENUM_CONSTANT(EIM_CONTROL_TYPE_3);

        BIND_ENUM_CONSTANT(AUTO_RELAY_NO);
        BIND_ENUM_CONSTANT(AUTO_RELAY_YES);
        BIND_ENUM_CONSTANT(AUTO_RELAY_OPTIONAL);

        ClassDB::bind_method(D_METHOD("get_main_switch_enabled"), &VehicleEngine::get_main_switch_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "main_switch_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_main_switch_enabled");
        ClassDB::bind_method(D_METHOD("get_main_switch_closable"), &VehicleEngine::get_main_switch_closable);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "main_switch_closable", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_main_switch_closable");
        ClassDB::bind_method(D_METHOD("get_type"), &VehicleEngine::get_type);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "type", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_type");
        ClassDB::bind_method(D_METHOD("get_motor_torque"), &VehicleEngine::get_motor_torque);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "motor_torque", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_motor_torque");
        ClassDB::bind_method(D_METHOD("get_wheel_torque"), &VehicleEngine::get_wheel_torque);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "wheel_torque", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_wheel_torque");
        ClassDB::bind_method(D_METHOD("get_wheel_force"), &VehicleEngine::get_wheel_force);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "wheel_force", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_wheel_force");
        ClassDB::bind_method(D_METHOD("get_tractive_force"), &VehicleEngine::get_tractive_force);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "tractive_force", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_tractive_force");
        ClassDB::bind_method(D_METHOD("get_compressor_enabled"), &VehicleEngine::get_compressor_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "compressor_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_compressor_enabled");
        ClassDB::bind_method(D_METHOD("get_compressor_allowed"), &VehicleEngine::get_compressor_allowed);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "compressor_allowed", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_compressor_allowed");
        ClassDB::bind_method(D_METHOD("get_power"), &VehicleEngine::get_power);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_power");
        ClassDB::bind_method(D_METHOD("get_rpm_count"), &VehicleEngine::get_rpm_count);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "rpm_count", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rpm_count");
        ClassDB::bind_method(D_METHOD("get_rpm_ratio"), &VehicleEngine::get_rpm_ratio);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "rpm_ratio", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rpm_ratio");
        ClassDB::bind_method(D_METHOD("get_circuit_nmax_rpm"), &VehicleEngine::get_circuit_nmax_rpm);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "circuit_nmax_rpm", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_circuit_nmax_rpm");
        ClassDB::bind_method(D_METHOD("get_damage"), &VehicleEngine::get_damage);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "damage", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_damage");
        ClassDB::bind_method(D_METHOD("get_main_switch_time"), &VehicleEngine::get_main_switch_time);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "main_switch_time", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_main_switch_time");
        ClassDB::bind_method(D_METHOD("get_main_no_power_pos"), &VehicleEngine::get_main_no_power_pos);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "main_no_power_pos", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_main_no_power_pos");
    }

    // Original engine: the main switch closing and opening is what "the engine started/stopped"
    // means here. Detected once per tick against this part's own member - it
    // used to be compared against the state dictionary while that dictionary was being filled,
    // so the signal fired on a read rather than on a change.
    void VehicleEngine::_do_process_component(const double p_delta) {
        if (engine_backend != nullptr) {
            engine_backend->process(this, p_delta);
        }
        const bool main_switch_enabled = get_main_switch_enabled();
        if (previous_main_switch == main_switch_enabled) {
            return;
        }
        previous_main_switch = main_switch_enabled;
        emit_signal(previous_main_switch ? "engine_start" : "engine_stop");
    }


    int VehicleEngine::get_type() const {
        return get_engine_type();
    }

    void VehicleEngine::_fill_state_dictionary(Dictionary &p_state) const {
        if (!is_simulation_ready()) {
            return;
        }
        p_state["main_switch_enabled"] = get_main_switch_enabled();
        p_state["main_switch_closable"] = get_main_switch_closable();
        p_state["engine_type"] = get_type();
        p_state["Mm"] = get_motor_torque();
        p_state["Mw"] = get_wheel_torque();
        p_state["Fw"] = get_wheel_force();
        p_state["Ft"] = get_tractive_force();
        p_state["compressor_enabled"] = get_compressor_enabled();
        p_state["compressor_allowed"] = get_compressor_allowed();
        p_state["engine_power"] = get_power();
        p_state["engine_rpm_count"] = get_rpm_count();
        p_state["engine_rpm_ratio"] = get_rpm_ratio();
        p_state["circuit_nmax_rpm"] = get_circuit_nmax_rpm();
        p_state["engine_damage"] = get_damage();
        p_state["main_switch_time"] = get_main_switch_time();
        p_state["main_no_power_pos"] = get_main_no_power_pos();
    }

    bool VehicleEngine::main_switch(const bool p_enabled) {
        return engine_backend != nullptr ? engine_backend->main_switch(this, p_enabled) : false;
    }

    void VehicleEngine::_register_commands() {
        register_command("main_switch", Callable(this, "main_switch"));
    }

    void VehicleEngine::_unregister_commands() {
        unregister_command("main_switch", Callable(this, "main_switch"));
    }
} // namespace godot
