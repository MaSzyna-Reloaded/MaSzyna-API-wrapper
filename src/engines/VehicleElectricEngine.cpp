#include "VehicleElectricEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>

#include <algorithm>
#include <cmath>

namespace godot {
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
    }


    bool VehicleElectricEngine::get_converter_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->ConverterFlag : false;
    }

    bool VehicleElectricEngine::get_converted_allowed() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->ConverterAllow : false;
    }

    double VehicleElectricEngine::get_converter_time_to_start() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->ConverterStartDelayTimer : 0.0;
    }

    double VehicleElectricEngine::get_collector_max_voltage() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EnginePowerSource.MaxVoltage : 0.0;
    }

    double VehicleElectricEngine::get_collector_max_current() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EnginePowerSource.MaxCurrent : 0.0;
    }

    double VehicleElectricEngine::get_collector_max_lifting() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EnginePowerSource.CollectorParameters.MaxH : 0.0;
    }

    double VehicleElectricEngine::get_collector_min_lifting() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EnginePowerSource.CollectorParameters.MinH : 0.0;
    }

    double VehicleElectricEngine::get_collector_sliding_width() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EnginePowerSource.CollectorParameters.CSW : 0.0;
    }

    double VehicleElectricEngine::get_collector_min_main_switch_voltage() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EnginePowerSource.CollectorParameters.MinV : 0.0;
    }

    double VehicleElectricEngine::get_collector_min_pantograph_tank_pressure() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EnginePowerSource.CollectorParameters.MinPress : 0.0;
    }

    double VehicleElectricEngine::get_collector_max_pantograph_tank_pressure() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EnginePowerSource.CollectorParameters.MaxPress : 0.0;
    }

    double VehicleElectricEngine::get_collector_pantograph_tank_pressure() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->PantPress : 0.0;
    }

    bool VehicleElectricEngine::get_collector_pantograph_pressure_switch_armed() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->PantPressSwitchActive : false;
    }

    bool VehicleElectricEngine::get_collector_pantograph_compressor_valve() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? !mover->bPantKurek3 : false;
    }

    bool VehicleElectricEngine::get_collector_overvoltage_relay() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EnginePowerSource.CollectorParameters.OVP : false;
    }

    double VehicleElectricEngine::get_collector_required_main_switch_voltage() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EnginePowerSource.CollectorParameters.InsetV : 0.0;
    }

    bool VehicleElectricEngine::get_collector_valve_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->PantsValve.is_active : false;
    }

    bool VehicleElectricEngine::get_collector_pantographs_dropped() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->PantAllDown : false;
    }

    bool VehicleElectricEngine::get_collector_pantograph_first_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Pantographs[0].is_active : false;
    }

    double VehicleElectricEngine::get_collector_pantograph_first_voltage() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Pantographs[0].voltage : 0.0;
    }

    bool VehicleElectricEngine::get_collector_pantograph_second_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Pantographs[1].is_active : false;
    }

    double VehicleElectricEngine::get_collector_pantograph_second_voltage() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Pantographs[1].voltage : 0.0;
    }

    double VehicleElectricEngine::get_collector_voltage() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->PantographVoltage : 0.0;
    }

    bool VehicleElectricEngine::get_contactors_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? (mover->StLinFlag || mover->ControlPressureSwitch) ? false : (mover->BrakePress < 1.0) : false;
    }

    bool VehicleElectricEngine::get_diff_relay_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? (mover->GroundRelay || mover->ControlPressureSwitch) ? false : (mover->BrakePress < 1.0) : false;
    }

    bool VehicleElectricEngine::get_resistors_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->StLinFlag ? mover->ResistorsFlagCheck() : false : false;
    }

    bool VehicleElectricEngine::get_vent_overload_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? (mover->RventRot < 5.0) && mover->ResistorsFlagCheck() : false;
    }

    bool VehicleElectricEngine::get_highcurrent_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? !(mover->Imax < mover->ImaxHi) : false;
    }

    bool VehicleElectricEngine::get_mainbreaker_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Mains : false;
    }

    double VehicleElectricEngine::get_transducer_input_voltage() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EnginePowerSource.Transducer.InputVoltage : 0.0;
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

    void VehicleElectricEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        VehicleEngine::_do_update_internal_mover(p_mover);
        // Pantographs[*].voltage/PantFrontVolt/PantRearVolt/PantographVoltage are NOT set here:
        // this only runs when the controller is dirty (effectively once, at startup), but wire
        // voltage changes every frame as the vehicle moves - see set_pantograph_wire_voltage(),
        // which writes them straight to the mover instead.
        p_mover->EnginePowerSource.SourceType = train_controller_node->power_source_map.at(power_source);

        switch (power_source) {
            case VehicleController::POWER_SOURCE_INTERNAL: {
                const std::map<VehicleController::TrainPowerType, TPowerType>::const_iterator lookup =
                        train_controller_node->power_type_map.find(power_cable_source);
                p_mover->EnginePowerSource.PowerType =
                        lookup != train_controller_node->power_type_map.end() ? lookup->second : TPowerType::NoPower;
                break;
            }
            case VehicleController::POWER_SOURCE_TRANSDUCER: {
                p_mover->EnginePowerSource.Transducer.InputVoltage = power_transducer_input_voltage;
                break;
            }
            case VehicleController::POWER_SOURCE_GENERATOR: {
                // engine_revolutions is an uninitialized raw pointer on a fresh TMoverParameters
                // (MOVER.h:551) - nothing currently dereferences EnginePowerSource's copy of it,
                // but HeatingPowerSource's copy does (see VehicleHeating.cpp), so it's pointed at
                // enrot (the vehicle's own engine revolutions counter) here too, defensively.
                engine_generator &generator_params{p_mover->EnginePowerSource.EngineGenerator};
                generator_params.engine_revolutions = &p_mover->enrot;
                break;
            }
            case VehicleController::POWER_SOURCE_ACCUMULATOR: {
                p_mover->EnginePowerSource.RAccumulator.RechargeSource =
                        train_controller_node->power_source_map.at(power_accumulator_recharge_source);
                break;
            }
            case VehicleController::POWER_SOURCE_CURRENTCOLLECTOR: {
                p_mover->EnginePowerSource.CollectorParameters.MinH = power_current_collector_min_collector_lifting;
                p_mover->EnginePowerSource.CollectorParameters.MaxH = power_current_collector_max_collector_lifting;
                p_mover->EnginePowerSource.CollectorParameters.CSW = power_current_collector_sliding_width;
                p_mover->EnginePowerSource.CollectorParameters.MinV = power_current_collector_min_main_switch_voltage;
                p_mover->EnginePowerSource.CollectorParameters.MinPress =
                        power_current_collector_min_pantograph_tank_pressure;
                p_mover->EnginePowerSource.CollectorParameters.MaxPress =
                        power_current_collector_max_pantograph_tank_pressure;
                p_mover->EnginePowerSource.CollectorParameters.OVP = power_current_collector_overvoltage_relay;
                p_mover->EnginePowerSource.CollectorParameters.CollectorsNo =
                        power_current_collector_number_of_collectors;
                p_mover->EnginePowerSource.MaxVoltage = power_current_collector_max_voltage;
                p_mover->EnginePowerSource.MaxCurrent = power_current_collector_max_current;
                p_mover->EnginePowerSource.CollectorParameters.InsetV =
                        power_current_collector_required_main_switch_voltage;
                p_mover->EnginePowerSource.CollectorParameters.PhysicalLayout = power_current_collector_physical_layout;
                break;
            }
            case VehicleController::POWER_SOURCE_POWERCABLE: {
                p_mover->EnginePowerSource.RPowerCable.PowerTrans =
                        train_controller_node->power_type_map.at(power_cable_source);
                if (p_mover->EnginePowerSource.RPowerCable.PowerTrans == TPowerType::SteamPower) {
                    p_mover->EnginePowerSource.RPowerCable.SteamPressure = power_cable_steam_pressure;
                }
                break;
            }
            case VehicleController::POWER_SOURCE_HEATER:; // Not finished on MaSzyna's side
            case VehicleController::POWER_SOURCE_NOT_DEFINED:;
            default:;
        }

        /* Circuit: (elektryczny obwod napedowy), tylko pojazdy elektryczne i spalinowo-elektryczne */
        p_mover->CircuitRes = circuit_resistance;
        p_mover->ImaxLo = circuit_imax_low;
        p_mover->ImaxHi = circuit_imax_high;
        p_mover->IminLo = circuit_imin_low;
        p_mover->IminHi = circuit_imin_high;
        p_mover->TUHEX_Sum = circuit_tuhex_sum;
        p_mover->TUHEX_Diff = circuit_tuhex_diff;
        p_mover->TUHEX_MinIw = circuit_tuhex_min_current;
        p_mover->TUHEX_MaxIw = circuit_tuhex_max_current;
        p_mover->TUHEX_Stages = circuit_tuhex_stages;
        p_mover->TUHEX_Sum1 = circuit_tuhex_sum_1;
        p_mover->TUHEX_Sum2 = circuit_tuhex_sum_2;
        p_mover->TUHEX_Sum3 = circuit_tuhex_sum_3;

        p_mover->ConverterStart = start_mode_map.at(cntrl_converter_start_mode);
        p_mover->ConverterStartDelay = static_cast<float>(cntrl_converter_start_delay);
        p_mover->ConverterOverloadRelayStart = start_mode_map.at(cntrl_converter_overload_relay_start_mode);
        p_mover->ConverterOverloadRelayOffWhenMainIsOff = cntrl_converter_overload_relay_off_when_main_is_off;
        p_mover->PantographCompressorStart = start_mode_map.at(cntrl_pantograph_compressor_start_mode);
        p_mover->PantAutoValve = cntrl_pantograph_auto_valve;
        p_mover->MainsStart = start_mode_map.at(cntrl_main_switch_start_mode);
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
