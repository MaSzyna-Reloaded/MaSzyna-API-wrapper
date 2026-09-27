#include "RailVehicleDieselEngine.hpp"
#include "macros.hpp"

#include <algorithm>
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    double RailVehicleDieselEngine::get_rpm() const {
        return diesel_backend != nullptr ? diesel_backend->get_rpm(this) : 0.0;
    }
    bool RailVehicleDieselEngine::get_oil_pump_active() const {
        return diesel_backend != nullptr ? diesel_backend->get_oil_pump_active(this) : false;
    }
    bool RailVehicleDieselEngine::get_oil_pump_disabled() const {
        return diesel_backend != nullptr ? diesel_backend->get_oil_pump_disabled(this) : false;
    }
    double RailVehicleDieselEngine::get_oil_pump_pressure() const {
        return diesel_backend != nullptr ? diesel_backend->get_oil_pump_pressure(this) : 0.0;
    }
    bool RailVehicleDieselEngine::get_fuel_pump_active() const {
        return diesel_backend != nullptr ? diesel_backend->get_fuel_pump_active(this) : false;
    }
    bool RailVehicleDieselEngine::get_fuel_pump_disabled() const {
        return diesel_backend != nullptr ? diesel_backend->get_fuel_pump_disabled(this) : false;
    }
    bool RailVehicleDieselEngine::get_fuel_pump_enabled() const {
        return diesel_backend != nullptr ? diesel_backend->get_fuel_pump_enabled(this) : false;
    }
    bool RailVehicleDieselEngine::get_oil_pump_enabled() const {
        return diesel_backend != nullptr ? diesel_backend->get_oil_pump_enabled(this) : false;
    }
    bool RailVehicleDieselEngine::get_heat_malfunction() const {
        return diesel_backend != nullptr ? diesel_backend->get_heat_malfunction(this) : false;
    }
    bool RailVehicleDieselEngine::get_startup() const {
        return diesel_backend != nullptr ? diesel_backend->get_startup(this) : false;
    }
    bool RailVehicleDieselEngine::get_ignition() const {
        return diesel_backend != nullptr ? diesel_backend->get_ignition(this) : false;
    }
    bool RailVehicleDieselEngine::get_spinup() const {
        return diesel_backend != nullptr ? diesel_backend->get_spinup(this) : false;
    }
    double RailVehicleDieselEngine::get_output_power() const {
        return diesel_backend != nullptr ? diesel_backend->get_output_power(this) : 0.0;
    }
    double RailVehicleDieselEngine::get_torque() const {
        return diesel_backend != nullptr ? diesel_backend->get_torque(this) : 0.0;
    }
    double RailVehicleDieselEngine::get_fill() const {
        return diesel_backend != nullptr ? diesel_backend->get_fill(this) : 0.0;
    }
    double RailVehicleDieselEngine::get_max_rpm() const {
        return diesel_backend != nullptr ? diesel_backend->get_max_rpm(this) : 0.0;
    }
    void RailVehicleDieselEngine::_apply_configuration() {
        RailVehicleEngine::_apply_configuration();
        if (diesel_backend != nullptr) {
            diesel_backend->apply_configuration(this);
        }
    }
    void RailVehicleDieselEngine::_fill_config_dictionary(Dictionary &p_config) const {
        RailVehicleEngine::_fill_config_dictionary(p_config);
        if (diesel_backend != nullptr) {
            diesel_backend->fill_config(this, p_config);
        }
    }

    void RailVehicleDieselEngine::_bind_methods() {
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, oil_pump_pressure_minimum, "oil_pump");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, oil_pump_pressure_maximum, "oil_pump");
        BIND_PROPERTY_W_HINT(
                RailVehicleDieselEngine, Variant::INT, fuel_pump_start_mode, "fuel_pump", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY_W_HINT(
                RailVehicleDieselEngine, Variant::INT, oil_pump_start_mode, "oil_pump", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY_W_HINT(
                RailVehicleDieselEngine, Variant::INT, water_pump_start_mode, "water_pump", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, mechanical_min_rpm, "mechanical");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, mechanical_max_rpm, "mechanical");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, mechanical_fuel_cutoff_rpm, "mechanical");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, mechanical_inertia, "mechanical");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, mechanical_clutch_engage_speed, "mechanical/clutch");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, mechanical_clutch_disengage_speed, "mechanical/clutch");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, mechanical_min_rpm_hydro_drive, "mechanical");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, mechanical_min_rpm_hydro_drive_factor, "mechanical");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, mechanical_min_rpm_retarder, "mechanical");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, mechanical_nominal_max_rpm, "mechanical");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, mechanical_regulator_acceleration, "mechanical");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, mechanical_rpm_decrease_rate, "mechanical");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, mechanical_shunt_mode_ratio, "mechanical");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, clutch_min_velocity_full_engage, "mechanical/clutch");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, clutch_diameter, "mechanical/clutch");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, clutch_max_force, "mechanical/clutch");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, clutch_friction, "mechanical/clutch");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, torque_converter_unlock_velocity, "torque_converter");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, retarder_engage_velocity, "retarder");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::BOOL, retarder_clutch, "retarder");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, retarder_clutch_speed, "retarder");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::BOOL, retarder_with_individual, "retarder");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::BOOL, torque_converter_present, "torque_converter");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, torque_converter_max_torque_ratio, "torque_converter");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, torque_converter_coupling_point, "torque_converter");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, torque_converter_lockup_torque, "torque_converter");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, torque_converter_lockup_rate, "torque_converter");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, torque_converter_unlock_rate, "torque_converter");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, torque_converter_fill_rate_increase, "torque_converter");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, torque_converter_fill_rate_decrease, "torque_converter");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, torque_converter_torque_in_in, "torque_converter");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, torque_converter_torque_in_out, "torque_converter");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, torque_converter_torque_out_out, "torque_converter");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, torque_converter_lockup_speed, "torque_converter");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, torque_converter_unlock_speed, "torque_converter");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                RailVehicleDieselEngine, Variant::ARRAY, torque_converter_table, "torque_converter",
                PROPERTY_HINT_TYPE_STRING, "VehicleCurvePointItem");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                RailVehicleDieselEngine, Variant::ARRAY, vel2nmax_table, PROPERTY_HINT_TYPE_STRING, "VehicleCurvePointItem");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::BOOL, retarder_present, "retarder");
        BIND_PROPERTY_W_HINT(
                RailVehicleDieselEngine, Variant::INT, retarder_placement, "retarder", PROPERTY_HINT_ENUM,
                "AfterGearbox,BetweenGearboxAndTC,BetweenTCAndEngine");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, retarder_torque_in_in, "retarder");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, retarder_max_torque, "retarder");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, retarder_max_power, "retarder");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, retarder_fill_rate_increase, "retarder");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, retarder_fill_rate_decrease, "retarder");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, retarder_min_velocity, "retarder");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, throttle_table_max_torque, "throttle_table_positions");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, throttle_table_max_torque_rpm, "throttle_table_positions");
        BIND_PROPERTY(RailVehicleDieselEngine, Variant::FLOAT, throttle_table_max_rpm_torque, "throttle_table_positions");
        BIND_PROPERTY(
                RailVehicleDieselEngine, Variant::FLOAT, throttle_table_nominal_fuel_dose, "throttle_table_positions");
        BIND_PROPERTY(
                RailVehicleDieselEngine, Variant::FLOAT, throttle_table_resistance_torque, "throttle_table_positions");
        BIND_PROPERTY(
                RailVehicleDieselEngine, Variant::FLOAT, throttle_table_nominal_fuel_consumption_rate,
                "throttle_table_positions");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                RailVehicleDieselEngine, Variant::ARRAY, throttle_table_positions, "throttle_table_positions",
                PROPERTY_HINT_TYPE_STRING, "RailVehicleThrottlePositionItem");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                RailVehicleDieselEngine, Variant::ARRAY, torque_table, PROPERTY_HINT_TYPE_STRING, "VehicleCurvePointItem");
        ClassDB::bind_method(D_METHOD("fuel_pump", "enabled"), &RailVehicleDieselEngine::fuel_pump);
        ClassDB::bind_method(D_METHOD("oil_pump", "enabled"), &RailVehicleDieselEngine::oil_pump);
        ClassDB::bind_method(D_METHOD("fuel_pump_switch_off", "enabled"), &RailVehicleDieselEngine::fuel_pump_switch_off);
        ClassDB::bind_method(D_METHOD("oil_pump_switch_off", "enabled"), &RailVehicleDieselEngine::oil_pump_switch_off);
        ClassDB::bind_method(D_METHOD("get_fuel_pump_enabled"), &RailVehicleDieselEngine::get_fuel_pump_enabled);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "fuel_pump_enabled", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_fuel_pump_enabled");
        ClassDB::bind_method(D_METHOD("get_oil_pump_enabled"), &RailVehicleDieselEngine::get_oil_pump_enabled);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "oil_pump_enabled", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_oil_pump_enabled");
        ClassDB::bind_method(D_METHOD("get_heat_malfunction"), &RailVehicleDieselEngine::get_heat_malfunction);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "heat_malfunction", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_heat_malfunction");

        BIND_ENUM_CONSTANT(RETARDER_PLACEMENT_AFTER_GEARBOX);
        BIND_ENUM_CONSTANT(RETARDER_PLACEMENT_BETWEEN_GEARBOX_AND_TC);
        BIND_ENUM_CONSTANT(RETARDER_PLACEMENT_BETWEEN_TC_AND_ENGINE);

        ClassDB::bind_method(D_METHOD("get_rpm"), &RailVehicleDieselEngine::get_rpm);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "rpm", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rpm");
        ClassDB::bind_method(D_METHOD("get_oil_pump_active"), &RailVehicleDieselEngine::get_oil_pump_active);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "oil_pump_active", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_oil_pump_active");
        ClassDB::bind_method(D_METHOD("get_oil_pump_disabled"), &RailVehicleDieselEngine::get_oil_pump_disabled);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "oil_pump_disabled", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_oil_pump_disabled");
        ClassDB::bind_method(D_METHOD("get_oil_pump_pressure"), &RailVehicleDieselEngine::get_oil_pump_pressure);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "oil_pump_pressure", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_oil_pump_pressure");
        ClassDB::bind_method(D_METHOD("get_fuel_pump_active"), &RailVehicleDieselEngine::get_fuel_pump_active);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "fuel_pump_active", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_fuel_pump_active");
        ClassDB::bind_method(D_METHOD("get_fuel_pump_disabled"), &RailVehicleDieselEngine::get_fuel_pump_disabled);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "fuel_pump_disabled", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_fuel_pump_disabled");
        ClassDB::bind_method(D_METHOD("get_startup"), &RailVehicleDieselEngine::get_startup);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "startup", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_startup");
        ClassDB::bind_method(D_METHOD("get_ignition"), &RailVehicleDieselEngine::get_ignition);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "ignition", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_ignition");
        ClassDB::bind_method(D_METHOD("get_spinup"), &RailVehicleDieselEngine::get_spinup);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "spinup", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_spinup");
        ClassDB::bind_method(D_METHOD("get_output_power"), &RailVehicleDieselEngine::get_output_power);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "output_power", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_output_power");
        ClassDB::bind_method(D_METHOD("get_torque"), &RailVehicleDieselEngine::get_torque);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "torque", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_torque");
        ClassDB::bind_method(D_METHOD("get_fill"), &RailVehicleDieselEngine::get_fill);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "fill", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_fill");
        ClassDB::bind_method(D_METHOD("get_max_rpm"), &RailVehicleDieselEngine::get_max_rpm);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "max_rpm", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_max_rpm");
    }

    RailVehicleEngine::EngineType RailVehicleDieselEngine::get_engine_type() const {
        return RailVehicleEngine::EngineType::DIESEL;
    }


    void RailVehicleDieselEngine::_fill_state_dictionary(Dictionary &p_state) const {
        RailVehicleEngine::_fill_state_dictionary(p_state);
        if (!is_simulation_ready()) {
            return;
        }
        p_state["engine_rpm"] = get_rpm();
        p_state["oil_pump_active"] = get_oil_pump_active();
        p_state["oil_pump_disabled"] = get_oil_pump_disabled();
        p_state["oil_pump_pressure"] = get_oil_pump_pressure();
        p_state["fuel_pump_active"] = get_fuel_pump_active();
        p_state["fuel_pump_disabled"] = get_fuel_pump_disabled();
        p_state["diesel_heat_malfunction"] = get_heat_malfunction();
        p_state["fuel_pump_enabled"] = get_fuel_pump_enabled();
        p_state["oil_pump_enabled"] = get_oil_pump_enabled();
        p_state["diesel_startup"] = get_startup();
        p_state["diesel_ignition"] = get_ignition();
        p_state["diesel_spinup"] = get_spinup();
        p_state["diesel_power"] = get_output_power();
        p_state["diesel_torque"] = get_torque();
        p_state["diesel_fill"] = get_fill();
        p_state["diesel_max_rpm"] = get_max_rpm();
    }

    void RailVehicleDieselEngine::oil_pump(const bool p_enabled) {
        if (diesel_backend != nullptr) {
            diesel_backend->oil_pump(this, p_enabled);
        }
    }

    void RailVehicleDieselEngine::fuel_pump(const bool p_enabled) {
        if (diesel_backend != nullptr) {
            diesel_backend->fuel_pump(this, p_enabled);
        }
    }

    void RailVehicleDieselEngine::oil_pump_switch_off(const bool p_enabled) {
        if (diesel_backend != nullptr) {
            diesel_backend->oil_pump_switch_off(this, p_enabled);
        }
    }

    void RailVehicleDieselEngine::fuel_pump_switch_off(const bool p_enabled) {
        if (diesel_backend != nullptr) {
            diesel_backend->fuel_pump_switch_off(this, p_enabled);
        }
    }

    void RailVehicleDieselEngine::_register_commands() {
        RailVehicleEngine::_register_commands();
        register_command("oil_pump", Callable(this, "oil_pump"));
        register_command("fuel_pump", Callable(this, "fuel_pump"));
        register_command("oil_pump_switch_off", Callable(this, "oil_pump_switch_off"));
        register_command("fuel_pump_switch_off", Callable(this, "fuel_pump_switch_off"));
    }

    void RailVehicleDieselEngine::_unregister_commands() {
        RailVehicleEngine::_unregister_commands();
        unregister_command("oil_pump", Callable(this, "oil_pump"));
        unregister_command("fuel_pump", Callable(this, "fuel_pump"));
        unregister_command("oil_pump_switch_off", Callable(this, "oil_pump_switch_off"));
        unregister_command("fuel_pump_switch_off", Callable(this, "fuel_pump_switch_off"));
    }
} // namespace godot
