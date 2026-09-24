#include "VehicleDieselEngine.hpp"
#include "macros.hpp"

#include <algorithm>
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    double VehicleDieselEngine::get_rpm() const {
        return diesel_backend != nullptr ? diesel_backend->get_rpm(this) : 0.0;
    }
    bool VehicleDieselEngine::get_oil_pump_active() const {
        return diesel_backend != nullptr ? diesel_backend->get_oil_pump_active(this) : false;
    }
    bool VehicleDieselEngine::get_oil_pump_disabled() const {
        return diesel_backend != nullptr ? diesel_backend->get_oil_pump_disabled(this) : false;
    }
    double VehicleDieselEngine::get_oil_pump_pressure() const {
        return diesel_backend != nullptr ? diesel_backend->get_oil_pump_pressure(this) : 0.0;
    }
    bool VehicleDieselEngine::get_fuel_pump_active() const {
        return diesel_backend != nullptr ? diesel_backend->get_fuel_pump_active(this) : false;
    }
    bool VehicleDieselEngine::get_fuel_pump_disabled() const {
        return diesel_backend != nullptr ? diesel_backend->get_fuel_pump_disabled(this) : false;
    }
    bool VehicleDieselEngine::get_startup() const {
        return diesel_backend != nullptr ? diesel_backend->get_startup(this) : false;
    }
    bool VehicleDieselEngine::get_ignition() const {
        return diesel_backend != nullptr ? diesel_backend->get_ignition(this) : false;
    }
    bool VehicleDieselEngine::get_spinup() const {
        return diesel_backend != nullptr ? diesel_backend->get_spinup(this) : false;
    }
    double VehicleDieselEngine::get_output_power() const {
        return diesel_backend != nullptr ? diesel_backend->get_output_power(this) : 0.0;
    }
    double VehicleDieselEngine::get_torque() const {
        return diesel_backend != nullptr ? diesel_backend->get_torque(this) : 0.0;
    }
    double VehicleDieselEngine::get_fill() const {
        return diesel_backend != nullptr ? diesel_backend->get_fill(this) : 0.0;
    }
    double VehicleDieselEngine::get_max_rpm() const {
        return diesel_backend != nullptr ? diesel_backend->get_max_rpm(this) : 0.0;
    }
    void VehicleDieselEngine::_apply_configuration() {
        VehicleEngine::_apply_configuration();
        if (diesel_backend != nullptr) {
            diesel_backend->apply_configuration(this);
        }
    }
    void VehicleDieselEngine::_fill_config_dictionary(Dictionary &p_config) const {
        VehicleEngine::_fill_config_dictionary(p_config);
        if (diesel_backend != nullptr) {
            diesel_backend->fill_config(this, p_config);
        }
    }

    void VehicleDieselEngine::_bind_methods() {
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, oil_pump_pressure_minimum, "oil_pump");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, oil_pump_pressure_maximum, "oil_pump");
        BIND_PROPERTY_W_HINT(
                VehicleDieselEngine, Variant::INT, fuel_pump_start_mode, "fuel_pump", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY_W_HINT(
                VehicleDieselEngine, Variant::INT, oil_pump_start_mode, "oil_pump", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY_W_HINT(
                VehicleDieselEngine, Variant::INT, water_pump_start_mode, "water_pump", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, mechanical_min_rpm, "mechanical");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, mechanical_max_rpm, "mechanical");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, mechanical_fuel_cutoff_rpm, "mechanical");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, mechanical_inertia, "mechanical");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, mechanical_clutch_engage_speed, "mechanical/clutch");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, mechanical_clutch_disengage_speed, "mechanical/clutch");
        BIND_PROPERTY(VehicleDieselEngine, Variant::BOOL, torque_converter_present, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_max_torque_ratio, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_coupling_point, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_lockup_torque, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_lockup_rate, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_unlock_rate, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_fill_rate_increase, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_fill_rate_decrease, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_torque_in_in, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_torque_in_out, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_torque_out_out, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_lockup_speed, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_unlock_speed, "torque_converter");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleDieselEngine, Variant::ARRAY, torque_converter_table, "torque_converter",
                PROPERTY_HINT_TYPE_STRING, "CurvePointItem");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleDieselEngine, Variant::ARRAY, vel2nmax_table, PROPERTY_HINT_TYPE_STRING, "CurvePointItem");
        BIND_PROPERTY(VehicleDieselEngine, Variant::BOOL, retarder_present, "retarder");
        BIND_PROPERTY_W_HINT(
                VehicleDieselEngine, Variant::INT, retarder_placement, "retarder", PROPERTY_HINT_ENUM,
                "AfterGearbox,BetweenGearboxAndTC,BetweenTCAndEngine");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, retarder_torque_in_in, "retarder");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, retarder_max_torque, "retarder");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, retarder_max_power, "retarder");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, retarder_fill_rate_increase, "retarder");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, retarder_fill_rate_decrease, "retarder");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, retarder_min_velocity, "retarder");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, throttle_table_max_torque, "throttle_table_positions");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, throttle_table_max_torque_rpm, "throttle_table_positions");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, throttle_table_max_rpm_torque, "throttle_table_positions");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, throttle_table_nominal_fuel_dose, "throttle_table_positions");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, throttle_table_resistance_torque, "throttle_table_positions");
        BIND_PROPERTY(
                VehicleDieselEngine, Variant::FLOAT, throttle_table_nominal_fuel_consumption_rate,
                "throttle_table_positions");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleDieselEngine, Variant::ARRAY, throttle_table_positions, "throttle_table_positions",
                PROPERTY_HINT_TYPE_STRING, "ThrottlePositionItem");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleDieselEngine, Variant::ARRAY, torque_table, PROPERTY_HINT_TYPE_STRING, "CurvePointItem");
        ClassDB::bind_method(D_METHOD("fuel_pump", "enabled"), &VehicleDieselEngine::fuel_pump);
        ClassDB::bind_method(D_METHOD("oil_pump", "enabled"), &VehicleDieselEngine::oil_pump);

        BIND_ENUM_CONSTANT(RETARDER_PLACEMENT_AFTER_GEARBOX);
        BIND_ENUM_CONSTANT(RETARDER_PLACEMENT_BETWEEN_GEARBOX_AND_TC);
        BIND_ENUM_CONSTANT(RETARDER_PLACEMENT_BETWEEN_TC_AND_ENGINE);

        ClassDB::bind_method(D_METHOD("get_rpm"), &VehicleDieselEngine::get_rpm);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "rpm", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rpm");
        ClassDB::bind_method(D_METHOD("get_oil_pump_active"), &VehicleDieselEngine::get_oil_pump_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "oil_pump_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_oil_pump_active");
        ClassDB::bind_method(D_METHOD("get_oil_pump_disabled"), &VehicleDieselEngine::get_oil_pump_disabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "oil_pump_disabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_oil_pump_disabled");
        ClassDB::bind_method(D_METHOD("get_oil_pump_pressure"), &VehicleDieselEngine::get_oil_pump_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "oil_pump_pressure", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_oil_pump_pressure");
        ClassDB::bind_method(D_METHOD("get_fuel_pump_active"), &VehicleDieselEngine::get_fuel_pump_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "fuel_pump_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_fuel_pump_active");
        ClassDB::bind_method(D_METHOD("get_fuel_pump_disabled"), &VehicleDieselEngine::get_fuel_pump_disabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "fuel_pump_disabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_fuel_pump_disabled");
        ClassDB::bind_method(D_METHOD("get_startup"), &VehicleDieselEngine::get_startup);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "startup", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_startup");
        ClassDB::bind_method(D_METHOD("get_ignition"), &VehicleDieselEngine::get_ignition);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "ignition", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_ignition");
        ClassDB::bind_method(D_METHOD("get_spinup"), &VehicleDieselEngine::get_spinup);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "spinup", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_spinup");
        ClassDB::bind_method(D_METHOD("get_output_power"), &VehicleDieselEngine::get_output_power);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "output_power", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_output_power");
        ClassDB::bind_method(D_METHOD("get_torque"), &VehicleDieselEngine::get_torque);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "torque", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_torque");
        ClassDB::bind_method(D_METHOD("get_fill"), &VehicleDieselEngine::get_fill);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "fill", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_fill");
        ClassDB::bind_method(D_METHOD("get_max_rpm"), &VehicleDieselEngine::get_max_rpm);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "max_rpm", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_max_rpm");
    }

    VehicleEngine::EngineType VehicleDieselEngine::get_engine_type() const {
        return VehicleEngine::EngineType::DIESEL;
    }


    void VehicleDieselEngine::_fill_state_dictionary(Dictionary &p_state) const {
        VehicleEngine::_fill_state_dictionary(p_state);
        if (!is_simulation_ready()) {
            return;
        }
        p_state["engine_rpm"] = get_rpm();
        p_state["oil_pump_active"] = get_oil_pump_active();
        p_state["oil_pump_disabled"] = get_oil_pump_disabled();
        p_state["oil_pump_pressure"] = get_oil_pump_pressure();
        p_state["fuel_pump_active"] = get_fuel_pump_active();
        p_state["fuel_pump_disabled"] = get_fuel_pump_disabled();
        p_state["diesel_startup"] = get_startup();
        p_state["diesel_ignition"] = get_ignition();
        p_state["diesel_spinup"] = get_spinup();
        p_state["diesel_power"] = get_output_power();
        p_state["diesel_torque"] = get_torque();
        p_state["diesel_fill"] = get_fill();
        p_state["diesel_max_rpm"] = get_max_rpm();
    }

    void VehicleDieselEngine::oil_pump(const bool p_enabled) {
        if (diesel_backend != nullptr) {
            diesel_backend->oil_pump(this, p_enabled);
        }
    }

    void VehicleDieselEngine::fuel_pump(const bool p_enabled) {
        if (diesel_backend != nullptr) {
            diesel_backend->fuel_pump(this, p_enabled);
        }
    }

    void VehicleDieselEngine::_register_commands() {
        VehicleEngine::_register_commands();
        register_command("oil_pump", Callable(this, "oil_pump"));
        register_command("fuel_pump", Callable(this, "fuel_pump"));
    }

    void VehicleDieselEngine::_unregister_commands() {
        VehicleEngine::_unregister_commands();
        unregister_command("oil_pump", Callable(this, "oil_pump"));
        unregister_command("fuel_pump", Callable(this, "fuel_pump"));
    }
} // namespace godot
