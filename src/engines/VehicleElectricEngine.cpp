#include "VehicleElectricEngine.hpp"
#include "MoverElectricEngineBackend.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>

#include <algorithm>
#include <cmath>

namespace godot {
    bool VehicleElectricEngine::get_converter_enabled() const {
        return electric_backend != nullptr ? electric_backend->get_converter_enabled(get_mover()) : false;
    }
    bool VehicleElectricEngine::get_converted_allowed() const {
        return electric_backend != nullptr ? electric_backend->get_converted_allowed(get_mover()) : false;
    }
    double VehicleElectricEngine::get_converter_time_to_start() const {
        return electric_backend != nullptr ? electric_backend->get_converter_time_to_start(get_mover()) : 0.0;
    }
    double VehicleElectricEngine::get_collector_max_voltage() const {
        return electric_backend != nullptr ? electric_backend->get_collector_max_voltage(get_mover()) : 0.0;
    }
    double VehicleElectricEngine::get_collector_max_current() const {
        return electric_backend != nullptr ? electric_backend->get_collector_max_current(get_mover()) : 0.0;
    }
    double VehicleElectricEngine::get_collector_max_lifting() const {
        return electric_backend != nullptr ? electric_backend->get_collector_max_lifting(get_mover()) : 0.0;
    }
    double VehicleElectricEngine::get_collector_min_lifting() const {
        return electric_backend != nullptr ? electric_backend->get_collector_min_lifting(get_mover()) : 0.0;
    }
    double VehicleElectricEngine::get_collector_sliding_width() const {
        return electric_backend != nullptr ? electric_backend->get_collector_sliding_width(get_mover()) : 0.0;
    }
    double VehicleElectricEngine::get_collector_min_main_switch_voltage() const {
        return electric_backend != nullptr ? electric_backend->get_collector_min_main_switch_voltage(get_mover()) : 0.0;
    }
    double VehicleElectricEngine::get_collector_min_pantograph_tank_pressure() const {
        return electric_backend != nullptr ? electric_backend->get_collector_min_pantograph_tank_pressure(get_mover()) : 0.0;
    }
    double VehicleElectricEngine::get_collector_max_pantograph_tank_pressure() const {
        return electric_backend != nullptr ? electric_backend->get_collector_max_pantograph_tank_pressure(get_mover()) : 0.0;
    }
    double VehicleElectricEngine::get_collector_pantograph_tank_pressure() const {
        return electric_backend != nullptr ? electric_backend->get_collector_pantograph_tank_pressure(get_mover()) : 0.0;
    }
    bool VehicleElectricEngine::get_collector_pantograph_pressure_switch_armed() const {
        return electric_backend != nullptr ? electric_backend->get_collector_pantograph_pressure_switch_armed(get_mover()) : false;
    }
    bool VehicleElectricEngine::get_collector_pantograph_compressor_valve() const {
        return electric_backend != nullptr ? electric_backend->get_collector_pantograph_compressor_valve(get_mover()) : false;
    }
    bool VehicleElectricEngine::get_collector_overvoltage_relay() const {
        return electric_backend != nullptr ? electric_backend->get_collector_overvoltage_relay(get_mover()) : false;
    }
    double VehicleElectricEngine::get_collector_required_main_switch_voltage() const {
        return electric_backend != nullptr ? electric_backend->get_collector_required_main_switch_voltage(get_mover()) : 0.0;
    }
    bool VehicleElectricEngine::get_collector_valve_active() const {
        return electric_backend != nullptr ? electric_backend->get_collector_valve_active(get_mover()) : false;
    }
    bool VehicleElectricEngine::get_collector_pantographs_dropped() const {
        return electric_backend != nullptr ? electric_backend->get_collector_pantographs_dropped(get_mover()) : false;
    }
    bool VehicleElectricEngine::get_collector_pantograph_first_active() const {
        return electric_backend != nullptr ? electric_backend->get_collector_pantograph_first_active(get_mover()) : false;
    }
    double VehicleElectricEngine::get_collector_pantograph_first_voltage() const {
        return electric_backend != nullptr ? electric_backend->get_collector_pantograph_first_voltage(get_mover()) : 0.0;
    }
    bool VehicleElectricEngine::get_collector_pantograph_second_active() const {
        return electric_backend != nullptr ? electric_backend->get_collector_pantograph_second_active(get_mover()) : false;
    }
    double VehicleElectricEngine::get_collector_pantograph_second_voltage() const {
        return electric_backend != nullptr ? electric_backend->get_collector_pantograph_second_voltage(get_mover()) : 0.0;
    }
    double VehicleElectricEngine::get_collector_voltage() const {
        return electric_backend != nullptr ? electric_backend->get_collector_voltage(get_mover()) : 0.0;
    }
    bool VehicleElectricEngine::get_contactors_active() const {
        return electric_backend != nullptr ? electric_backend->get_contactors_active(get_mover()) : false;
    }
    bool VehicleElectricEngine::get_diff_relay_active() const {
        return electric_backend != nullptr ? electric_backend->get_diff_relay_active(get_mover()) : false;
    }
    bool VehicleElectricEngine::get_resistors_active() const {
        return electric_backend != nullptr ? electric_backend->get_resistors_active(get_mover()) : false;
    }
    bool VehicleElectricEngine::get_vent_overload_active() const {
        return electric_backend != nullptr ? electric_backend->get_vent_overload_active(get_mover()) : false;
    }
    bool VehicleElectricEngine::get_highcurrent_active() const {
        return electric_backend != nullptr ? electric_backend->get_highcurrent_active(get_mover()) : false;
    }
    bool VehicleElectricEngine::get_mainbreaker_active() const {
        return electric_backend != nullptr ? electric_backend->get_mainbreaker_active(get_mover()) : false;
    }
    double VehicleElectricEngine::get_transducer_input_voltage() const {
        return electric_backend != nullptr ? electric_backend->get_transducer_input_voltage(get_mover()) : 0.0;
    }
    bool VehicleElectricEngine::get_camshaft_available() const {
        return electric_backend != nullptr ? electric_backend->get_camshaft_available(get_mover()) : false;
    }
    bool VehicleElectricEngine::get_converter_overload() const {
        return electric_backend != nullptr ? electric_backend->get_converter_overload(get_mover()) : false;
    }
    double VehicleElectricEngine::get_line_breaker_delay() const {
        return electric_backend != nullptr ? electric_backend->get_line_breaker_delay(get_mover()) : 0.0;
    }
    double VehicleElectricEngine::get_line_breaker_initial_delay() const {
        return electric_backend != nullptr ? electric_backend->get_line_breaker_initial_delay(get_mover()) : 0.0;
    }
    bool VehicleElectricEngine::get_line_breaker_closes_at_no_power() const {
        return electric_backend != nullptr ? electric_backend->get_line_breaker_closes_at_no_power(get_mover()) : false;
    }
    void VehicleElectricEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        VehicleEngine::_do_update_internal_mover(p_mover);
        if (electric_backend != nullptr) {
            electric_backend->update_mover(this, p_mover);
        }
    }

    void VehicleElectricEngine::_bind_methods() {
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, power_source, "power", PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::INT, power_current_collector_number_of_collectors,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_max_voltage, "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_max_current, "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_max_collector_lifting,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_min_collector_lifting,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_sliding_width, "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_min_main_switch_voltage,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_min_pantograph_tank_pressure,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_max_pantograph_tank_pressure,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::BOOL, power_current_collector_overvoltage_relay,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_required_main_switch_voltage,
                "power/current_collector");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, power_transducer_input_voltage, "power/transducer");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, power_accumulator_recharge_source, "power/accumulator",
                PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, power_cable_source, "power/power_cable", PROPERTY_HINT_ENUM,
                enum_hint(
                        {{"NoPower", VehicleController::POWER_TYPE_NONE},
                         {"BioPower", VehicleController::POWER_TYPE_BIO},
                         {"MechPower", VehicleController::POWER_TYPE_MECH},
                         {"ElectricPower", VehicleController::POWER_TYPE_ELECTRIC},
                         {"SteamPower", VehicleController::POWER_TYPE_STEAM}}));
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, power_cable_steam_pressure, "power/power_cable");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, power_current_collector_physical_layout, "power/current_collector",
                PROPERTY_HINT_FLAGS, "Front,Rear");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_resistance, "circuit");
        BIND_PROPERTY(VehicleElectricEngine, Variant::INT, circuit_imax_low, "circuit");
        BIND_PROPERTY(VehicleElectricEngine, Variant::INT, circuit_imax_high, "circuit");
        BIND_PROPERTY(VehicleElectricEngine, Variant::INT, circuit_imin_low, "circuit");
        BIND_PROPERTY(VehicleElectricEngine, Variant::INT, circuit_imin_high, "circuit");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_sum, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_diff, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_min_current, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_max_current, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::INT, circuit_tuhex_stages, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_sum_1, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_sum_2, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_sum_3, "circuit/tuhex");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, cntrl_converter_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, cntrl_converter_start_delay, "cntrl");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, cntrl_converter_overload_relay_start_mode, "cntrl",
                PROPERTY_HINT_ENUM, "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(VehicleElectricEngine, Variant::BOOL, cntrl_converter_overload_relay_off_when_main_is_off, "cntrl");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, cntrl_pantograph_compressor_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(VehicleElectricEngine, Variant::BOOL, cntrl_pantograph_auto_valve, "cntrl");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, cntrl_main_switch_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        ClassDB::bind_method(D_METHOD("compressor", "enabled"), &VehicleElectricEngine::compressor);
        ClassDB::bind_method(D_METHOD("converter", "enabled"), &VehicleElectricEngine::converter);
        ClassDB::bind_method(D_METHOD("converter_fuse_reset"), &VehicleElectricEngine::converter_fuse_reset);
        ClassDB::bind_method(D_METHOD("pantographs_valve", "enabled"), &VehicleElectricEngine::pantographs_valve);
        ClassDB::bind_method(
                D_METHOD("pantographs_drop_all", "enabled"), &VehicleElectricEngine::pantographs_drop_all);
        ClassDB::bind_method(
                D_METHOD("pantograph_compressor", "enabled"), &VehicleElectricEngine::pantograph_compressor);
        ClassDB::bind_method(
                D_METHOD("pantograph_compressor_valve", "to_compressor"),
                &VehicleElectricEngine::pantograph_compressor_valve);
        ClassDB::bind_method(D_METHOD("pantograph", "selector", "enabled"), &VehicleElectricEngine::pantograph);
        ClassDB::bind_method(
                D_METHOD("set_pantograph_wire_voltage", "selector", "voltage"),
                &VehicleElectricEngine::set_pantograph_wire_voltage);

        BIND_ENUM_CONSTANT(PANTOGRAPH_FIRST);
        BIND_ENUM_CONSTANT(PANTOGRAPH_SECOND);

        ClassDB::bind_method(D_METHOD("get_converter_enabled"), &VehicleElectricEngine::get_converter_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "converter_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_converter_enabled");
        ClassDB::bind_method(D_METHOD("get_converted_allowed"), &VehicleElectricEngine::get_converted_allowed);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "converted_allowed", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_converted_allowed");
        ClassDB::bind_method(D_METHOD("get_converter_time_to_start"), &VehicleElectricEngine::get_converter_time_to_start);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "converter_time_to_start", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_converter_time_to_start");
        ClassDB::bind_method(D_METHOD("get_collector_max_voltage"), &VehicleElectricEngine::get_collector_max_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "collector_max_voltage", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_max_voltage");
        ClassDB::bind_method(D_METHOD("get_collector_max_current"), &VehicleElectricEngine::get_collector_max_current);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "collector_max_current", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_max_current");
        ClassDB::bind_method(D_METHOD("get_collector_max_lifting"), &VehicleElectricEngine::get_collector_max_lifting);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "collector_max_lifting", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_max_lifting");
        ClassDB::bind_method(D_METHOD("get_collector_min_lifting"), &VehicleElectricEngine::get_collector_min_lifting);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "collector_min_lifting", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_min_lifting");
        ClassDB::bind_method(D_METHOD("get_collector_sliding_width"), &VehicleElectricEngine::get_collector_sliding_width);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "collector_sliding_width", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_sliding_width");
        ClassDB::bind_method(D_METHOD("get_collector_min_main_switch_voltage"), &VehicleElectricEngine::get_collector_min_main_switch_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "collector_min_main_switch_voltage", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_min_main_switch_voltage");
        ClassDB::bind_method(D_METHOD("get_collector_min_pantograph_tank_pressure"), &VehicleElectricEngine::get_collector_min_pantograph_tank_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "collector_min_pantograph_tank_pressure", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_min_pantograph_tank_pressure");
        ClassDB::bind_method(D_METHOD("get_collector_max_pantograph_tank_pressure"), &VehicleElectricEngine::get_collector_max_pantograph_tank_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "collector_max_pantograph_tank_pressure", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_max_pantograph_tank_pressure");
        ClassDB::bind_method(D_METHOD("get_collector_pantograph_tank_pressure"), &VehicleElectricEngine::get_collector_pantograph_tank_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "collector_pantograph_tank_pressure", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_pantograph_tank_pressure");
        ClassDB::bind_method(D_METHOD("get_collector_pantograph_pressure_switch_armed"), &VehicleElectricEngine::get_collector_pantograph_pressure_switch_armed);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "collector_pantograph_pressure_switch_armed", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_pantograph_pressure_switch_armed");
        ClassDB::bind_method(D_METHOD("get_collector_pantograph_compressor_valve"), &VehicleElectricEngine::get_collector_pantograph_compressor_valve);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "collector_pantograph_compressor_valve", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_pantograph_compressor_valve");
        ClassDB::bind_method(D_METHOD("get_collector_overvoltage_relay"), &VehicleElectricEngine::get_collector_overvoltage_relay);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "collector_overvoltage_relay", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_overvoltage_relay");
        ClassDB::bind_method(D_METHOD("get_collector_required_main_switch_voltage"), &VehicleElectricEngine::get_collector_required_main_switch_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "collector_required_main_switch_voltage", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_required_main_switch_voltage");
        ClassDB::bind_method(D_METHOD("get_collector_valve_active"), &VehicleElectricEngine::get_collector_valve_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "collector_valve_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_valve_active");
        ClassDB::bind_method(D_METHOD("get_collector_pantographs_dropped"), &VehicleElectricEngine::get_collector_pantographs_dropped);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "collector_pantographs_dropped", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_pantographs_dropped");
        ClassDB::bind_method(D_METHOD("get_collector_pantograph_first_active"), &VehicleElectricEngine::get_collector_pantograph_first_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "collector_pantograph_first_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_pantograph_first_active");
        ClassDB::bind_method(D_METHOD("get_collector_pantograph_first_voltage"), &VehicleElectricEngine::get_collector_pantograph_first_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "collector_pantograph_first_voltage", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_pantograph_first_voltage");
        ClassDB::bind_method(D_METHOD("get_collector_pantograph_second_active"), &VehicleElectricEngine::get_collector_pantograph_second_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "collector_pantograph_second_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_pantograph_second_active");
        ClassDB::bind_method(D_METHOD("get_collector_pantograph_second_voltage"), &VehicleElectricEngine::get_collector_pantograph_second_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "collector_pantograph_second_voltage", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_pantograph_second_voltage");
        ClassDB::bind_method(D_METHOD("has_accumulator"), &VehicleElectricEngine::has_accumulator);
        ClassDB::bind_method(D_METHOD("has_power_cable"), &VehicleElectricEngine::has_power_cable);
        ClassDB::bind_method(D_METHOD("get_collector_voltage"), &VehicleElectricEngine::get_collector_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "collector_voltage", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_collector_voltage");
        ClassDB::bind_method(D_METHOD("get_contactors_active"), &VehicleElectricEngine::get_contactors_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "contactors_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_contactors_active");
        ClassDB::bind_method(D_METHOD("get_diff_relay_active"), &VehicleElectricEngine::get_diff_relay_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "diff_relay_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_diff_relay_active");
        ClassDB::bind_method(D_METHOD("get_resistors_active"), &VehicleElectricEngine::get_resistors_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "resistors_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_resistors_active");
        ClassDB::bind_method(D_METHOD("get_vent_overload_active"), &VehicleElectricEngine::get_vent_overload_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "vent_overload_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_vent_overload_active");
        ClassDB::bind_method(D_METHOD("get_highcurrent_active"), &VehicleElectricEngine::get_highcurrent_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "highcurrent_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_highcurrent_active");
        ClassDB::bind_method(D_METHOD("get_mainbreaker_active"), &VehicleElectricEngine::get_mainbreaker_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "mainbreaker_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_mainbreaker_active");
        ClassDB::bind_method(D_METHOD("get_transducer_input_voltage"), &VehicleElectricEngine::get_transducer_input_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "transducer_input_voltage", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_transducer_input_voltage");

        ClassDB::bind_method(D_METHOD("get_camshaft_available"), &VehicleElectricEngine::get_camshaft_available);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "camshaft_available", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_camshaft_available");
        ClassDB::bind_method(D_METHOD("get_converter_overload"), &VehicleElectricEngine::get_converter_overload);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "converter_overload", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_converter_overload");
        ClassDB::bind_method(D_METHOD("get_line_breaker_delay"), &VehicleElectricEngine::get_line_breaker_delay);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "line_breaker_delay", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_line_breaker_delay");
        ClassDB::bind_method(D_METHOD("get_line_breaker_initial_delay"), &VehicleElectricEngine::get_line_breaker_initial_delay);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "line_breaker_initial_delay", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_line_breaker_initial_delay");
        ClassDB::bind_method(D_METHOD("get_line_breaker_closes_at_no_power"), &VehicleElectricEngine::get_line_breaker_closes_at_no_power);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "line_breaker_closes_at_no_power", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_line_breaker_closes_at_no_power");
        ClassDB::bind_method(D_METHOD("get_motor_current"), &VehicleElectricEngine::get_motor_current);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "motor_current", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_motor_current");
        ClassDB::bind_method(D_METHOD("get_circuit_imax"), &VehicleElectricEngine::get_circuit_imax);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "circuit_imax", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_circuit_imax");
        ClassDB::bind_method(D_METHOD("get_dynamic_brake_active"), &VehicleElectricEngine::get_dynamic_brake_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "dynamic_brake_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_dynamic_brake_active");
        ClassDB::bind_method(D_METHOD("get_fuse_active"), &VehicleElectricEngine::get_fuse_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "fuse_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_fuse_active");
        ClassDB::bind_method(D_METHOD("get_motor_connectors_open"), &VehicleElectricEngine::get_motor_connectors_open);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "motor_connectors_open", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_motor_connectors_open");
    }


    bool VehicleElectricEngine::has_accumulator() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr && mover->EnginePowerSource.SourceType == TPowerSource::Accumulator;
    }

    bool VehicleElectricEngine::has_power_cable() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr && mover->EnginePowerSource.SourceType == TPowerSource::PowerCable;
    }

    void VehicleElectricEngine::_fill_state_dictionary(Dictionary &p_state) const {
        p_state["camshaft_available"] = get_camshaft_available();
        p_state["converter_overload"] = get_converter_overload();
        p_state["line_breaker_delay"] = get_line_breaker_delay();
        p_state["line_breaker_initial_delay"] = get_line_breaker_initial_delay();
        p_state["line_breaker_closes_at_no_power"] = get_line_breaker_closes_at_no_power();
        p_state["Im"] = get_motor_current();
        p_state["circuit_imax"] = get_circuit_imax();
        p_state["dynamic_brake_active"] = get_dynamic_brake_active();
        p_state["fuse_active"] = get_fuse_active();
        p_state["motor_connectors_open"] = get_motor_connectors_open();
        VehicleEngine::_fill_state_dictionary(p_state);
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        p_state["converter_enabled"] = get_converter_enabled();
        p_state["converted_allowed"] = get_converted_allowed();
        p_state["converter_time_to_start"] = get_converter_time_to_start();
        p_state["power_source"] = get_power_source();
        if (has_accumulator()) {
            p_state["accumulator/recharge_source"] = get_power_accumulator_recharge_source();
        }
        p_state["current_collector/max_voltage"] = get_collector_max_voltage();
        p_state["current_collector/max_current"] = get_collector_max_current();
        p_state["current_collector/max_collector_lifting"] = get_collector_max_lifting();
        p_state["current_collector/min_collector_lifting"] = get_collector_min_lifting();
        p_state["current_collector/collector_sliding_width"] = get_collector_sliding_width();
        p_state["current_collector/min_main_switch_voltage"] = get_collector_min_main_switch_voltage();
        p_state["current_collector/min_pantograph_tank_pressure"] = get_collector_min_pantograph_tank_pressure();
        p_state["current_collector/max_pantograph_tank_pressure"] = get_collector_max_pantograph_tank_pressure();
        p_state["current_collector/pantograph_tank_pressure"] = get_collector_pantograph_tank_pressure();
        p_state["current_collector/pantograph_pressure_switch_armed"] = get_collector_pantograph_pressure_switch_armed();
        p_state["current_collector/pantograph_compressor_valve"] = get_collector_pantograph_compressor_valve();
        p_state["current_collector/overvoltage_relay"] = get_collector_overvoltage_relay();
        p_state["current_collector/required_main_switch_voltage"] = get_collector_required_main_switch_voltage();
        p_state["current_collector/valve_active"] = get_collector_valve_active();
        p_state["current_collector/pantographs_dropped"] = get_collector_pantographs_dropped();
        p_state["current_collector/pantograph_first_active"] = get_collector_pantograph_first_active();
        p_state["current_collector/pantograph_first_voltage"] = get_collector_pantograph_first_voltage();
        p_state["current_collector/pantograph_second_active"] = get_collector_pantograph_second_active();
        p_state["current_collector/pantograph_second_voltage"] = get_collector_pantograph_second_voltage();
        p_state["current_collector/voltage"] = get_collector_voltage();
        p_state["indicators/contactors_active"] = get_contactors_active();
        p_state["indicators/diff_relay_active"] = get_diff_relay_active();
        p_state["indicators/resistors_active"] = get_resistors_active();
        p_state["indicators/vent_overload_active"] = get_vent_overload_active();
        p_state["indicators/highcurrent_active"] = get_highcurrent_active();
        p_state["indicators/mainbreaker_active"] = get_mainbreaker_active();
        p_state["transducer/input_voltage"] = get_transducer_input_voltage();
        if (has_power_cable()) {
            p_state["power_cable/source"] = get_power_cable_source();
        }
        if (has_power_cable()) {
            p_state["power_cable/steam_pressure"] = get_power_cable_steam_pressure();
        }
    }

    void VehicleElectricEngine::converter(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->ConverterSwitch(p_enabled);
    }

    void VehicleElectricEngine::compressor(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->CompressorSwitch(p_enabled);
    }

    void VehicleElectricEngine::converter_fuse_reset() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_converteroverloadrelayreset (Train.cpp:3567-3585) ->
        // RelayReset(relay_t::primaryconverteroverload), "converterfuse_bt:"/ggConverterFuseButton
        // (Train.cpp:10053) - the converter-specific counterpart to fuse_reset()/FuseOn() above.
        mover->RelayReset(Maszyna::primaryconverteroverload);
    }

    void VehicleElectricEngine::pantographs_valve(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->OperatePantographsValve(p_enabled ? Maszyna::operation_t::enable : Maszyna::operation_t::disable);
    }

    // Train.cpp:3336 OnCommand_pantographlowerall
    void VehicleElectricEngine::pantographs_drop_all(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->DropAllPantographs(p_enabled);
    }

    void VehicleElectricEngine::pantograph_compressor(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_pantographcompressoractivate (Train.cpp:2912) - runs while held,
        // starting only with low enough pressure and live 24V power
        if (!p_enabled) {
            mover->PantCompFlag = false;
            return;
        }
        if (mover->PantPress < 4.8 && mover->Power24vIsAvailable) {
            mover->PantCompFlag = true;
        }
    }

    void VehicleElectricEngine::pantograph_compressor_valve(const bool p_to_compressor) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_pantographcompressorvalveenable/disable (Train.cpp:2869-2909)
        mover->bPantKurek3 = !p_to_compressor;
    }

    void VehicleElectricEngine::pantograph(const PantographSelector p_selector, const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        const Maszyna::end end = (p_selector == PANTOGRAPH_FIRST) ? Maszyna::end::front : Maszyna::end::rear;
        mover->OperatePantographValve(end, p_enabled ? Maszyna::operation_t::enable : Maszyna::operation_t::disable);
        // The mover also gates every pantograph on a separate master air valve (PantsValve -
        // PantographsCheck(), Mover.cpp) that the original engine opens from its own
        // "raise selected pantograph" key command (Train.cpp's
        // OnCommand_pantographraiseselected calls OperatePantographsValve itself) for vehicles -
        // like every one wired through this wrapper's cabin today - that have no separate
        // master-valve switch of their own ("pantvalves_sw:"/ggPantValvesButton in Train.cpp is
        // genuinely absent from this vehicle's cabin, and nothing here has a keybind path
        // either). Without this, no pantograph could ever be raised through the cabin, on any
        // vehicle. Only opened here, never closed: PantographsCheck() ANDs it with each
        // pantograph's own individual valve, so leaving it open doesn't keep a lowered
        // pantograph powered - closing it here on every lower would also drop any OTHER,
        // still-raised pantograph sharing the same master valve on a two-pantograph vehicle.
        if (p_enabled) {
            mover->OperatePantographsValve(Maszyna::operation_t::enable);
        }
    }

    void VehicleElectricEngine::set_pantograph_wire_voltage(const PantographSelector p_selector, const float p_voltage) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        // Written straight to the mover, like the other per-frame-relevant setters above
        // (pantograph(), pantographs_valve()) - _do_update_internal_mover() only runs when the
        // controller is dirty (effectively once, at startup), so stashing this in a member for
        // that path to pick up later would mean every subsequent frame's wire voltage is ignored.
        if (p_selector == PANTOGRAPH_FIRST) {
            pantograph_first_wire_voltage = p_voltage;
            mover->Pantographs[0].voltage = p_voltage;
            mover->PantFrontVolt = mover->Pantographs[0].is_active ? p_voltage : 0.0;
        } else {
            pantograph_second_wire_voltage = p_voltage;
            mover->Pantographs[1].voltage = p_voltage;
            mover->PantRearVolt = mover->Pantographs[1].is_active ? p_voltage : 0.0;
        }
        mover->PantographVoltage = std::max(std::fabs(mover->PantFrontVolt), std::fabs(mover->PantRearVolt));
    }

    void VehicleElectricEngine::_register_commands() {
        VehicleEngine::_register_commands();
        register_command("converter", Callable(this, "converter"));
        register_command("converter_fuse_reset", Callable(this, "converter_fuse_reset"));
        register_command("compressor", Callable(this, "compressor"));
        register_command("pantographs_valve", Callable(this, "pantographs_valve"));
        register_command("pantographs_drop_all", Callable(this, "pantographs_drop_all"));
        register_command("pantograph_compressor", Callable(this, "pantograph_compressor"));
        register_command("pantograph_compressor_valve", Callable(this, "pantograph_compressor_valve"));
        register_command("pantograph", Callable(this, "pantograph"));
    }

    void VehicleElectricEngine::_unregister_commands() {
        VehicleEngine::_unregister_commands();
        unregister_command("converter", Callable(this, "converter"));
        unregister_command("converter_fuse_reset", Callable(this, "converter_fuse_reset"));
        unregister_command("compressor", Callable(this, "compressor"));
        unregister_command("pantographs_valve", Callable(this, "pantographs_valve"));
        unregister_command("pantographs_drop_all", Callable(this, "pantographs_drop_all"));
        unregister_command("pantograph_compressor", Callable(this, "pantograph_compressor"));
        unregister_command("pantograph_compressor_valve", Callable(this, "pantograph_compressor_valve"));
        unregister_command("pantograph", Callable(this, "pantograph"));
    }


    void VehicleElectricEngine::set_power_source(const VehicleController::TrainPowerSource p_source) {
        power_source = p_source;
        dirty = true;
    }

    VehicleController::TrainPowerSource VehicleElectricEngine::get_power_source() const {
        return power_source;
    }
} // namespace godot
