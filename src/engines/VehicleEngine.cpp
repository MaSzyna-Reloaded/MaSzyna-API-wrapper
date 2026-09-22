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

    void VehicleEngine::_declare_state_properties() {
        state_base_index = get_state_property_count();
        declare_state_property("main_switch_enabled", Variant::BOOL);
        declare_state_property("main_switch_closable", Variant::BOOL);
        declare_state_property("engine_type", Variant::INT);
        declare_state_property("Mm", Variant::FLOAT);
        declare_state_property("Mw", Variant::FLOAT);
        declare_state_property("Fw", Variant::FLOAT);
        declare_state_property("Ft", Variant::FLOAT);
        declare_state_property("Im", Variant::FLOAT);
        declare_state_property("compressor_enabled", Variant::BOOL);
        declare_state_property("compressor_allowed", Variant::BOOL);
        declare_state_property("engine_power", Variant::FLOAT);
        declare_state_property("dynamic_brake_active", Variant::BOOL);
        declare_state_property("engine_rpm_count", Variant::FLOAT);
        declare_state_property("engine_rpm_ratio", Variant::FLOAT);
        declare_state_property("engine_current", Variant::FLOAT);
        declare_state_property("circuit_imax", Variant::FLOAT);
        declare_state_property("circuit_nmax_rpm", Variant::FLOAT);
        declare_state_property("engine_damage", Variant::INT);
        declare_state_property("main_switch_time", Variant::FLOAT);
        declare_state_property("main_no_power_pos", Variant::BOOL);
        declare_state_property("camshaft_available", Variant::BOOL);
        declare_state_property("converter_overload", Variant::BOOL);
        declare_state_property("line_breaker_delay", Variant::FLOAT);
        declare_state_property("line_breaker_initial_delay", Variant::FLOAT);
        declare_state_property("line_breaker_closes_at_no_power", Variant::BOOL);
        declare_state_property("fuse_active", Variant::BOOL);
        declare_state_property("motor_connectors_open", Variant::BOOL);
    }

    Variant VehicleEngine::_get_state_property(const int p_local_index) const {
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return Variant();
        }
        switch (p_local_index - state_base_index) {
            case STATE_MAIN_SWITCH_ENABLED:
                return mover->Mains;
            case STATE_MAIN_SWITCH_CLOSABLE:
                return mover->MainSwitchCheck();
            case STATE_ENGINE_TYPE:
                return get_engine_type();
            case STATE_MM:
                return mover->Mm;
            case STATE_MW:
                return mover->Mw;
            case STATE_FW:
                return mover->Fw;
            case STATE_FT:
                return mover->Ft;
            case STATE_IM:
                return mover->Im;
            case STATE_COMPRESSOR_ENABLED:
                return mover->CompressorFlag;
            case STATE_COMPRESSOR_ALLOWED:
                return mover->CompressorAllow;
            case STATE_ENGINE_POWER:
                return mover->EnginePower;
            case STATE_DYNAMIC_BRAKE_ACTIVE:
                return mover->DynamicBrakeFlag;
            case STATE_ENGINE_RPM_COUNT:
                return mover->enrot;
            case STATE_ENGINE_RPM_RATIO:
                return mover->EngineRPMRatio();
            case STATE_ENGINE_CURRENT:
                return mover->Im;
            case STATE_CIRCUIT_IMAX:
                return mover->Imax;
            case STATE_CIRCUIT_NMAX_RPM:
                return mover->nmax * 60.0;
            case STATE_ENGINE_DAMAGE:
                return mover->EngDmgFlag;
            case STATE_MAIN_SWITCH_TIME:
                return mover->MainsInitTimeCountdown;
            case STATE_MAIN_NO_POWER_POS:
                return mover->IsMainCtrlNoPowerPos();
            case STATE_CAMSHAFT_AVAILABLE:
                return mover->HasCamshaft;
            case STATE_CONVERTER_OVERLOAD:
                return mover->ConvOvldFlag;
            case STATE_LINE_BREAKER_DELAY:
                return mover->CtrlDelay;
            case STATE_LINE_BREAKER_INITIAL_DELAY:
                return mover->InitialCtrlDelay;
            case STATE_LINE_BREAKER_CLOSES_AT_NO_POWER:
                return mover->LineBreakerClosesOnlyAtNoPowerPos;
            case STATE_FUSE_ACTIVE:
                return mover->FuseFlag;
            case STATE_MOTOR_CONNECTORS_OPEN:
                return mover->StLinSwitchOff;
            default:
                return Variant();
        }
    }

    void VehicleEngine::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        p_config["main_controller_position_max"] = p_mover->MainCtrlPosNo;
        p_config["second_controller_position_max"] = p_mover->ScndCtrlPosNo;
        p_config["transmission_ratio"] = p_mover->Transmision.Ratio;
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
