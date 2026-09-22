#include "VehicleEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    class VehicleController;
    void VehicleEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("main_switch", "enabled"), &VehicleEngine::main_switch);
        ClassDB::bind_method(D_METHOD("fuse_reset"), &VehicleEngine::fuse_reset);
        ClassDB::bind_method(D_METHOD("motor_connectors_open", "open"), &VehicleEngine::motor_connectors_open);
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
        ClassDB::bind_method(D_METHOD("get_motor_current"), &VehicleEngine::get_motor_current);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "motor_current", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_motor_current");
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
        ClassDB::bind_method(D_METHOD("get_dynamic_brake_active"), &VehicleEngine::get_dynamic_brake_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "dynamic_brake_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_dynamic_brake_active");
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
        ClassDB::bind_method(D_METHOD("get_current"), &VehicleEngine::get_current);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "current", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_current");
        ClassDB::bind_method(D_METHOD("get_circuit_imax"), &VehicleEngine::get_circuit_imax);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "circuit_imax", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_circuit_imax");
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
        ClassDB::bind_method(D_METHOD("get_camshaft_available"), &VehicleEngine::get_camshaft_available);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "camshaft_available", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_camshaft_available");
        ClassDB::bind_method(D_METHOD("get_converter_overload"), &VehicleEngine::get_converter_overload);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "converter_overload", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_converter_overload");
        ClassDB::bind_method(D_METHOD("get_line_breaker_delay"), &VehicleEngine::get_line_breaker_delay);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "line_breaker_delay", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_line_breaker_delay");
        ClassDB::bind_method(D_METHOD("get_line_breaker_initial_delay"), &VehicleEngine::get_line_breaker_initial_delay);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "line_breaker_initial_delay", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_line_breaker_initial_delay");
        ClassDB::bind_method(D_METHOD("get_line_breaker_closes_at_no_power"), &VehicleEngine::get_line_breaker_closes_at_no_power);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "line_breaker_closes_at_no_power", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_line_breaker_closes_at_no_power");
        ClassDB::bind_method(D_METHOD("get_fuse_active"), &VehicleEngine::get_fuse_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "fuse_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_fuse_active");
        ClassDB::bind_method(D_METHOD("get_motor_connectors_open"), &VehicleEngine::get_motor_connectors_open);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "motor_connectors_open", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_motor_connectors_open");
    }

    void VehicleEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        p_mover->EngineType = engine_type_map.at(get_engine_type());

        p_mover->Transmision.NToothM = transmission_gear_teeth_motor;
        p_mover->Transmision.NToothW = transmission_gear_teeth_wheel;
        // Original engine: LoadFIZ_Engine (Mover.cpp) derives Ratio from the teeth counts
        // itself right after parsing "Trans=" - NToothM/NToothW alone are never read anywhere
        // else in Mover.cpp. Without this, Transmision.Ratio stays at its compiled default
        // (1.0), silently dropping the real gear ratio out of Mw/Fw/Ft (ElectricSeriesMotor
        // case, Mover.cpp ~line 5791) and undertractioning every geared vehicle.
        p_mover->Transmision.Ratio =
                transmission_gear_teeth_motor > 0
                        ? static_cast<double>(transmission_gear_teeth_wheel) / transmission_gear_teeth_motor
                        : 1.0;
        p_mover->Transmision.Efficiency = transmission_efficiency;
        p_mover->Ftmax = maximum_traction_force;
        p_mover->HasControlPressureSwitch = pressure_switch_present;
        p_mover->InvertersNo = inverters_count;
        for (auto &fan: p_mover->MotorBlowers) {
            fan.speed = static_cast<float>(motor_blowers_speed);
            fan.sustain_time = static_cast<float>(motor_blowers_sustain_time);
            fan.min_start_velocity = static_cast<float>(motor_blowers_start_velocity);
            fan.start_type = start_mode_map.at(motor_blowers_start_mode);
        }

        p_mover->MainCtrlPosNo = cntrl_main_controller_position_count;
        p_mover->ScndCtrlPosNo = cntrl_shunt_controller_position_count;
        p_mover->MainCtrlMaxDirChangePos = cntrl_direction_change_max_position;
        p_mover->EIMCtrlAdditionalZeros = cntrl_eim_control_additional_zeros;
        p_mover->EIMCtrlEmergency = cntrl_eim_control_emergency;
        p_mover->EIMCtrlType = cntrl_eim_control_type;
        p_mover->AutoRelayType = cntrl_auto_relay_mode;
        p_mover->CoupledCtrl = cntrl_coupled_controllers;
        p_mover->HasCamshaft = cntrl_has_camshaft;
        p_mover->ScndS = cntrl_series_shunt_on_series_position;
        p_mover->InitialCtrlDelay = cntrl_initial_controller_delay;
        p_mover->CtrlDelay = cntrl_controller_step_delay;
        p_mover->CtrlDownDelay = cntrl_controller_step_down_delay;
        p_mover->FastSerialCircuit = static_cast<int>(cntrl_fast_series_circuit);

        // Original engine: GroundRelay/NoVoltRelay/OvervoltageRelay/DamageFlag/EngDmgFlag/
        // ConvOvldFlag are all live, self-computed Mover state (relay checks recomputed every
        // Update() tick from real voltage/current, e.g. the ElectricSeriesMotor NoVoltRelay/
        // OvervoltageRelay block, Mover.cpp ~5627) and already default to their "healthy" values
        // in TMoverParameters's own constructor (MOVER.h:1590/1600/1601/401/1517/1518/1589).
        // This method reruns on every dirty-flag config reapply (not just once at startup - see
        // [[mover-parity-check]]), so force-resetting them here on every rerun was silently
        // wiping real relay trips/damage the simulation had legitimately produced since the last
        // reapply - matching the "traction voltage drops and the engine cuts out, needs the main
        // switch re-engaged" symptom (NoVoltRelay/OvervoltageRelay flip Mains off for real, then
        // a later config reapply cosmetically closes the relay again without also restoring
        // Mains, so the panel looks fine but the loco is still dead).

        /* motor param table */
        constexpr int MAX = Maszyna::MotorParametersArraySize;
        for (int i = 0; i < std::min(MAX, static_cast<int>(motor_param_table.size())); i++) {
            const Ref<MotorParameter> &row = motor_param_table[i];
            if (row == nullptr || !row.is_valid() || row.is_null()) {
                UtilityFunctions::push_warning(
                        "[VehicleEngine]: motor_param_table property is null at index " + String::num(i));
                return;
            }

            p_mover->MotorParam[i].mIsat = row->get_saturation_current_multiplier();
            p_mover->MotorParam[i].fi = row->get_voltage_constant();
            p_mover->MotorParam[i].mfi = row->get_voltage_constant_multiplier();
            p_mover->MotorParam[i].Isat = row->get_saturation_current();
            // readMPT0's default case (Mover.cpp:8948, what "MotorParamTable0:" rows actually go
            // through) reads these two as real columns, unlike readMPTElectricSeries - see
            // FizTrainEngineCommon.parse_motor_param_row's doc comment for the full story. fi0 in
            // particular feeds Current()'s back-EMF term (Mover.cpp:389, "U1 = U + Mn*n*fi0*fi"),
            // so leaving it at TMotorParameters's compiled-zero default here (matching the
            // never-set case for the OTHER reader) would silently kill that back-EMF term.
            p_mover->MotorParam[i].mfi0 = row->get_initial_voltage_constant_multiplier();
            p_mover->MotorParam[i].fi0 = row->get_initial_voltage_constant();
            p_mover->MPTRelay[i].Iup = row->get_shunting_up();     // bocznikowanie
            p_mover->MPTRelay[i].Idown = row->get_shunting_down(); // bocznikowanie;
        }
    }

    // Original engine: the main switch closing and opening is what "the engine started/stopped"
    // means here (Mains, Mover.cpp). Detected once per tick against this part's own member - it
    // used to be compared against the state dictionary while that dictionary was being filled,
    // so the signal fired on a read rather than on a change.
    void VehicleEngine::_do_process_mover(TMoverParameters *p_mover, double p_delta) {
        if (previous_main_switch == p_mover->Mains) {
            return;
        }
        previous_main_switch = p_mover->Mains;
        emit_signal(previous_main_switch ? "engine_start" : "engine_stop");
    }


    bool VehicleEngine::get_main_switch_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Mains : false;
    }

    bool VehicleEngine::get_main_switch_closable() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->MainSwitchCheck() : false;
    }

    int VehicleEngine::get_type() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? get_engine_type() : 0;
    }

    double VehicleEngine::get_motor_torque() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Mm : 0.0;
    }

    double VehicleEngine::get_wheel_torque() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Mw : 0.0;
    }

    double VehicleEngine::get_wheel_force() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Fw : 0.0;
    }

    double VehicleEngine::get_tractive_force() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Ft : 0.0;
    }

    double VehicleEngine::get_motor_current() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Im : 0.0;
    }

    bool VehicleEngine::get_compressor_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->CompressorFlag : false;
    }

    bool VehicleEngine::get_compressor_allowed() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->CompressorAllow : false;
    }

    double VehicleEngine::get_power() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EnginePower : 0.0;
    }

    bool VehicleEngine::get_dynamic_brake_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->DynamicBrakeFlag : false;
    }

    double VehicleEngine::get_rpm_count() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->enrot : 0.0;
    }

    double VehicleEngine::get_rpm_ratio() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EngineRPMRatio() : 0.0;
    }

    double VehicleEngine::get_current() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Im : 0.0;
    }

    double VehicleEngine::get_circuit_imax() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Imax : 0.0;
    }

    double VehicleEngine::get_circuit_nmax_rpm() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->nmax * 60.0 : 0.0;
    }

    int VehicleEngine::get_damage() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EngDmgFlag : 0;
    }

    double VehicleEngine::get_main_switch_time() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->MainsInitTimeCountdown : 0.0;
    }

    bool VehicleEngine::get_main_no_power_pos() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->IsMainCtrlNoPowerPos() : false;
    }

    bool VehicleEngine::get_camshaft_available() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->HasCamshaft : false;
    }

    bool VehicleEngine::get_converter_overload() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->ConvOvldFlag : false;
    }

    double VehicleEngine::get_line_breaker_delay() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->CtrlDelay : 0.0;
    }

    double VehicleEngine::get_line_breaker_initial_delay() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->InitialCtrlDelay : 0.0;
    }

    bool VehicleEngine::get_line_breaker_closes_at_no_power() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->LineBreakerClosesOnlyAtNoPowerPos : false;
    }

    bool VehicleEngine::get_fuse_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->FuseFlag : false;
    }

    bool VehicleEngine::get_motor_connectors_open() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->StLinSwitchOff : false;
    }

    void VehicleEngine::_fill_state_dictionary(Dictionary &p_state) const {
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        p_state["main_switch_enabled"] = get_main_switch_enabled();
        p_state["main_switch_closable"] = get_main_switch_closable();
        p_state["engine_type"] = get_type();
        p_state["Mm"] = get_motor_torque();
        p_state["Mw"] = get_wheel_torque();
        p_state["Fw"] = get_wheel_force();
        p_state["Ft"] = get_tractive_force();
        p_state["Im"] = get_motor_current();
        p_state["compressor_enabled"] = get_compressor_enabled();
        p_state["compressor_allowed"] = get_compressor_allowed();
        p_state["engine_power"] = get_power();
        p_state["dynamic_brake_active"] = get_dynamic_brake_active();
        p_state["engine_rpm_count"] = get_rpm_count();
        p_state["engine_rpm_ratio"] = get_rpm_ratio();
        p_state["engine_current"] = get_current();
        p_state["circuit_imax"] = get_circuit_imax();
        p_state["circuit_nmax_rpm"] = get_circuit_nmax_rpm();
        p_state["engine_damage"] = get_damage();
        p_state["main_switch_time"] = get_main_switch_time();
        p_state["main_no_power_pos"] = get_main_no_power_pos();
        p_state["camshaft_available"] = get_camshaft_available();
        p_state["converter_overload"] = get_converter_overload();
        p_state["line_breaker_delay"] = get_line_breaker_delay();
        p_state["line_breaker_initial_delay"] = get_line_breaker_initial_delay();
        p_state["line_breaker_closes_at_no_power"] = get_line_breaker_closes_at_no_power();
        p_state["fuse_active"] = get_fuse_active();
        p_state["motor_connectors_open"] = get_motor_connectors_open();
    }

    void VehicleEngine::_fill_config_dictionary(Dictionary &p_config) const {
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        p_config["main_controller_position_max"] = mover->MainCtrlPosNo;
        p_config["second_controller_position_max"] = mover->ScndCtrlPosNo;
        p_config["transmission_ratio"] = mover->Transmision.Ratio;
    }

    bool VehicleEngine::main_switch(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return false;
        }
        return mover->MainSwitch(p_enabled);
    }

    void VehicleEngine::fuse_reset() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_motoroverloadrelayreset (Train.cpp:4061) calls this same
        // FuseOn() on press - "zbij nadmiarowy", clearing the overload/fast-fuse trip
        // (MoverParameters->FuseFlag) that blocks Mains/converter/compressor from re-enabling.
        mover->FuseOn();
    }

    void VehicleEngine::motor_connectors_open(const bool p_open) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_motorconnectorsopen/close (Train.cpp:3947-4008) - a plain
        // field flip, no dedicated setter method exists on the vendored Mover for this one.
        mover->StLinSwitchOff = p_open;
    }

    void VehicleEngine::_register_commands() {
        register_command("main_switch", Callable(this, "main_switch"));
        register_command("fuse_reset", Callable(this, "fuse_reset"));
        register_command("motor_connectors_open", Callable(this, "motor_connectors_open"));
    }

    void VehicleEngine::_unregister_commands() {
        unregister_command("main_switch", Callable(this, "main_switch"));
        unregister_command("fuse_reset", Callable(this, "fuse_reset"));
        unregister_command("motor_connectors_open", Callable(this, "motor_connectors_open"));
    }
} // namespace godot
