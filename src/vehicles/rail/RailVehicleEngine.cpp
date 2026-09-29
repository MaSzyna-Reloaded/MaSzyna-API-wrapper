#include "RailVehicleEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    class VehicleController;
    bool RailVehicleEngine::get_main_switch_enabled() const {
        return drive_unit != nullptr ? drive_unit->get_main_switch_enabled() : false;
    }
    bool RailVehicleEngine::get_main_switch_closable() const {
        return drive_unit != nullptr ? drive_unit->get_main_switch_closable() : false;
    }
    double RailVehicleEngine::get_motor_torque() const {
        return drive_unit != nullptr ? drive_unit->get_motor_torque() : 0.0;
    }
    double RailVehicleEngine::get_wheel_torque() const {
        return drive_unit != nullptr ? drive_unit->get_wheel_torque() : 0.0;
    }
    double RailVehicleEngine::get_wheel_force() const {
        return drive_unit != nullptr ? drive_unit->get_wheel_force() : 0.0;
    }
    double RailVehicleEngine::get_tractive_force() const {
        return drive_unit != nullptr ? drive_unit->get_tractive_force() : 0.0;
    }
    double RailVehicleEngine::get_power() const {
        return drive_unit != nullptr ? drive_unit->get_power() : 0.0;
    }
    double RailVehicleEngine::get_rpm_count() const {
        return drive_unit != nullptr ? drive_unit->get_rpm_count() : 0.0;
    }
    double RailVehicleEngine::get_rpm_ratio() const {
        return drive_unit != nullptr ? drive_unit->get_rpm_ratio() : 0.0;
    }
    double RailVehicleEngine::get_circuit_nmax_rpm() const {
        return drive_unit != nullptr ? drive_unit->get_circuit_nmax_rpm() : 0.0;
    }
    int RailVehicleEngine::get_damage() const {
        return drive_unit != nullptr ? drive_unit->get_damage() : 0;
    }
    double RailVehicleEngine::get_main_switch_time() const {
        return drive_unit != nullptr ? drive_unit->get_main_switch_time() : 0.0;
    }
    bool RailVehicleEngine::get_main_no_power_pos() const {
        return drive_unit != nullptr ? drive_unit->get_main_no_power_pos() : false;
    }

    double RailVehicleEngine::get_eimic_real() const {
        return drive_unit != nullptr ? drive_unit->get_eimic_real() : 0.0;
    }

    bool RailVehicleEngine::get_motor_overload_relay_high_threshold() const {
        return drive_unit != nullptr ? drive_unit->get_motor_overload_relay_high_threshold() : false;
    }
    void RailVehicleEngine::_apply_configuration() {
        VehicleComponent::_apply_configuration();
        if (drive_unit != nullptr) {
            drive_unit->apply_configuration(this);
        }
    }
    void RailVehicleEngine::_fill_config_dictionary(Dictionary &p_config) const {
        VehicleComponent::_fill_config_dictionary(p_config);
        if (drive_unit != nullptr) {
            drive_unit->fill_config(p_config);
        }
    }

    void RailVehicleEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("main_switch", "enabled"), &RailVehicleEngine::main_switch);
        ClassDB::bind_method(
                D_METHOD("motor_overload_relay_threshold", "high"), &RailVehicleEngine::motor_overload_relay_threshold);
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                RailVehicleEngine, Variant::ARRAY, motor_param_table, PROPERTY_HINT_TYPE_STRING,
                "RailVehicleMotorParameter");
        BIND_PROPERTY(RailVehicleEngine, Variant::INT, transmission_gear_teeth_motor, "transmission");
        BIND_PROPERTY(RailVehicleEngine, Variant::INT, transmission_gear_teeth_wheel, "transmission");
        BIND_PROPERTY(RailVehicleEngine, Variant::FLOAT, transmission_efficiency, "transmission");
        BIND_PROPERTY(RailVehicleEngine, Variant::FLOAT, maximum_traction_force);
        BIND_PROPERTY(RailVehicleEngine, Variant::FLOAT, motor_blowers_speed, "motor_blowers");
        BIND_PROPERTY(RailVehicleEngine, Variant::FLOAT, motor_blowers_sustain_time, "motor_blowers");
        BIND_PROPERTY(RailVehicleEngine, Variant::FLOAT, motor_blowers_start_velocity, "motor_blowers");
        BIND_PROPERTY(RailVehicleEngine, Variant::BOOL, pressure_switch_present);
        BIND_PROPERTY(RailVehicleEngine, Variant::INT, inverters_count);
        BIND_PROPERTY_W_HINT(
                RailVehicleEngine, Variant::INT, motor_blowers_start_mode, "motor_blowers", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(RailVehicleEngine, Variant::BOOL, cntrl_eim_control_additional_zeros, "cntrl");
        BIND_PROPERTY(RailVehicleEngine, Variant::BOOL, cntrl_eim_control_emergency, "cntrl");
        BIND_PROPERTY_W_HINT(
                RailVehicleEngine, Variant::INT, cntrl_eim_control_type, "cntrl", PROPERTY_HINT_ENUM, "0,1,2,3");
        BIND_PROPERTY_W_HINT(
                RailVehicleEngine, Variant::INT, cntrl_auto_relay_mode, "cntrl", PROPERTY_HINT_ENUM, "No,Yes,Optional");
        BIND_PROPERTY(RailVehicleEngine, Variant::BOOL, cntrl_has_camshaft, "cntrl");
        BIND_PROPERTY(RailVehicleEngine, Variant::BOOL, cntrl_series_shunt_on_series_position, "cntrl");
        BIND_PROPERTY(RailVehicleEngine, Variant::BOOL, cntrl_fast_series_circuit, "cntrl");
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


        BIND_ENUM_CONSTANT(EIM_CONTROL_TYPE_0);
        BIND_ENUM_CONSTANT(EIM_CONTROL_TYPE_1);
        BIND_ENUM_CONSTANT(EIM_CONTROL_TYPE_2);
        BIND_ENUM_CONSTANT(EIM_CONTROL_TYPE_3);

        BIND_ENUM_CONSTANT(AUTO_RELAY_NO);
        BIND_ENUM_CONSTANT(AUTO_RELAY_YES);
        BIND_ENUM_CONSTANT(AUTO_RELAY_OPTIONAL);

        ClassDB::bind_method(D_METHOD("get_main_switch_enabled"), &RailVehicleEngine::get_main_switch_enabled);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "main_switch_enabled", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_main_switch_enabled");
        ClassDB::bind_method(D_METHOD("get_main_switch_closable"), &RailVehicleEngine::get_main_switch_closable);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "main_switch_closable", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_main_switch_closable");
        ClassDB::bind_method(D_METHOD("get_type"), &RailVehicleEngine::get_type);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "type", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_type");
        ClassDB::bind_method(D_METHOD("get_motor_torque"), &RailVehicleEngine::get_motor_torque);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "motor_torque", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_motor_torque");
        ClassDB::bind_method(D_METHOD("get_wheel_torque"), &RailVehicleEngine::get_wheel_torque);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "wheel_torque", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_wheel_torque");
        ClassDB::bind_method(D_METHOD("get_wheel_force"), &RailVehicleEngine::get_wheel_force);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "wheel_force", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_wheel_force");
        ClassDB::bind_method(D_METHOD("get_tractive_force"), &RailVehicleEngine::get_tractive_force);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "tractive_force", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_tractive_force");
        ClassDB::bind_method(D_METHOD("get_power"), &RailVehicleEngine::get_power);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "power", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_power");
        ClassDB::bind_method(D_METHOD("get_rpm_count"), &RailVehicleEngine::get_rpm_count);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "rpm_count", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rpm_count");
        ClassDB::bind_method(D_METHOD("get_rpm_ratio"), &RailVehicleEngine::get_rpm_ratio);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "rpm_ratio", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rpm_ratio");
        ClassDB::bind_method(D_METHOD("get_circuit_nmax_rpm"), &RailVehicleEngine::get_circuit_nmax_rpm);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "circuit_nmax_rpm", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_circuit_nmax_rpm");
        ClassDB::bind_method(D_METHOD("get_damage"), &RailVehicleEngine::get_damage);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "damage", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_damage");
        ClassDB::bind_method(D_METHOD("get_main_switch_time"), &RailVehicleEngine::get_main_switch_time);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "main_switch_time", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_main_switch_time");
        ClassDB::bind_method(D_METHOD("get_main_no_power_pos"), &RailVehicleEngine::get_main_no_power_pos);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "main_no_power_pos", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_main_no_power_pos");
        ClassDB::bind_method(
                D_METHOD("get_motor_overload_relay_high_threshold"),
                &RailVehicleEngine::get_motor_overload_relay_high_threshold);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "motor_overload_relay_high_threshold", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_motor_overload_relay_high_threshold");
        ClassDB::bind_method(D_METHOD("get_eimic_real"), &RailVehicleEngine::get_eimic_real);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "eimic_real", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_eimic_real");
    }

    // Original engine: the main switch closing and opening is what "the engine started/stopped"
    // means here. Detected once per tick against this part's own member - it
    // used to be compared against the state dictionary while that dictionary was being filled,
    // so the signal fired on a read rather than on a change.
    void RailVehicleEngine::_do_process_component(const double p_delta) {
        if (drive_unit != nullptr) {
            drive_unit->process(this, p_delta);
        }
        const bool main_switch_enabled = get_main_switch_enabled();
        if (previous_main_switch == main_switch_enabled) {
            return;
        }
        previous_main_switch = main_switch_enabled;
        emit_signal(previous_main_switch ? "engine_start" : "engine_stop");
    }


    void RailVehicleEngine::_fill_state_dictionary(Dictionary &p_state) const {
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
        p_state["engine_power"] = get_power();
        p_state["engine_rpm_count"] = get_rpm_count();
        p_state["engine_rpm_ratio"] = get_rpm_ratio();
        p_state["circuit_nmax_rpm"] = get_circuit_nmax_rpm();
        p_state["engine_damage"] = get_damage();
        p_state["main_switch_time"] = get_main_switch_time();
        p_state["main_no_power_pos"] = get_main_no_power_pos();
        p_state["motor_overload_relay_high_threshold"] = get_motor_overload_relay_high_threshold();
        p_state["eimic_real"] = get_eimic_real();
    }

    bool RailVehicleEngine::main_switch(const bool p_enabled) {
        return drive_unit != nullptr ? drive_unit->main_switch(p_enabled) : false;
    }

    bool RailVehicleEngine::motor_overload_relay_threshold(const bool p_high) {
        return drive_unit != nullptr ? drive_unit->motor_overload_relay_threshold(p_high) : false;
    }


    void RailVehicleEngine::_register_commands() {
        register_command("main_switch", Callable(this, "main_switch"));
        register_command("motor_overload_relay_threshold", Callable(this, "motor_overload_relay_threshold"));
    }

    void RailVehicleEngine::_unregister_commands() {
        unregister_command("main_switch");
        unregister_command("motor_overload_relay_threshold");
    }
} // namespace godot
