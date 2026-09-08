#include "TrainEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    class TrainController;
    void TrainEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("main_switch", "enabled"), &TrainEngine::main_switch);
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                TrainEngine, Variant::ARRAY, motor_param_table, PROPERTY_HINT_TYPE_STRING, "MotorParameter");
        BIND_PROPERTY(TrainEngine, Variant::INT, transmission_gear_teeth_motor, "transmission");
        BIND_PROPERTY(TrainEngine, Variant::INT, transmission_gear_teeth_wheel, "transmission");
        BIND_PROPERTY(TrainEngine, Variant::FLOAT, transmission_efficiency, "transmission");
        BIND_PROPERTY(TrainEngine, Variant::FLOAT, maximum_traction_force);
        BIND_PROPERTY(TrainEngine, Variant::FLOAT, motor_blowers_speed, "motor_blowers");
        BIND_PROPERTY(TrainEngine, Variant::FLOAT, motor_blowers_sustain_time, "motor_blowers");
        BIND_PROPERTY(TrainEngine, Variant::FLOAT, motor_blowers_start_velocity, "motor_blowers");
        BIND_PROPERTY(TrainEngine, Variant::BOOL, pressure_switch_present);
        BIND_PROPERTY(TrainEngine, Variant::INT, inverters_count);
        BIND_PROPERTY_W_HINT(
                TrainEngine, Variant::INT, motor_blowers_start_mode, "motor_blowers", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(TrainEngine, Variant::INT, cntrl_main_controller_position_count, "cntrl");
        BIND_PROPERTY(TrainEngine, Variant::INT, cntrl_shunt_controller_position_count, "cntrl");
        BIND_PROPERTY(TrainEngine, Variant::INT, cntrl_direction_change_max_position, "cntrl");
        BIND_PROPERTY(TrainEngine, Variant::BOOL, cntrl_eim_control_additional_zeros, "cntrl");
        BIND_PROPERTY(TrainEngine, Variant::BOOL, cntrl_eim_control_emergency, "cntrl");
        BIND_PROPERTY_W_HINT(TrainEngine, Variant::INT, cntrl_eim_control_type, "cntrl", PROPERTY_HINT_ENUM, "0,1,2,3");
        BIND_PROPERTY_W_HINT(
                TrainEngine, Variant::INT, cntrl_auto_relay_mode, "cntrl", PROPERTY_HINT_ENUM, "No,Yes,Optional");
        BIND_PROPERTY(TrainEngine, Variant::BOOL, cntrl_coupled_controllers, "cntrl");
        BIND_PROPERTY(TrainEngine, Variant::BOOL, cntrl_has_camshaft, "cntrl");
        BIND_PROPERTY(TrainEngine, Variant::BOOL, cntrl_series_shunt_on_series_position, "cntrl");
        BIND_PROPERTY(TrainEngine, Variant::FLOAT, cntrl_initial_controller_delay, "cntrl");
        BIND_PROPERTY(TrainEngine, Variant::FLOAT, cntrl_controller_step_delay, "cntrl");
        BIND_PROPERTY(TrainEngine, Variant::FLOAT, cntrl_controller_step_down_delay, "cntrl");
        BIND_PROPERTY(TrainEngine, Variant::BOOL, cntrl_fast_series_circuit, "cntrl");
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

    void TrainEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        p_mover->EngineType = engine_type_map.at(get_engine_type());

        p_mover->Transmision.NToothM = transmission_gear_teeth_motor;
        p_mover->Transmision.NToothW = transmission_gear_teeth_wheel;
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

        /* FIXME: for testing purposes */
        p_mover->GroundRelay = true;
        p_mover->NoVoltRelay = true;
        p_mover->OvervoltageRelay = true;
        p_mover->DamageFlag = 0;
        p_mover->EngDmgFlag = 0;
        p_mover->ConvOvldFlag = false;
        /* end testing */

        /* motor param table */
        constexpr int MAX = Maszyna::MotorParametersArraySize;
        for (int i = 0; i < std::min(MAX, static_cast<int>(motor_param_table.size())); i++) {
            const Ref<MotorParameter> &row = motor_param_table[i];
            if (row == nullptr || !row.is_valid() || row.is_null()) {
                UtilityFunctions::push_warning(
                        "[TrainEngine]: motor_param_table property is null at index " + String::num(i));
                return;
            }

            p_mover->MotorParam[i].mIsat = row->get_saturation_current_multiplier();
            p_mover->MotorParam[i].fi = row->get_voltage_constant();
            p_mover->MotorParam[i].mfi = row->get_voltage_constant_multiplier();
            p_mover->MotorParam[i].Isat = row->get_saturation_current();
            p_mover->MPTRelay[i].Iup = row->get_shunting_up();     // bocznikowanie
            p_mover->MPTRelay[i].Idown = row->get_shunting_down(); // bocznikowanie;
        }
    }

    void TrainEngine::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        const bool previous_main_switch = (p_state.get("main_switch_enabled", false));
        p_state["main_switch_enabled"] = p_mover->Mains;
        p_state["Mm"] = p_mover->Mm;
        p_state["Mw"] = p_mover->Mw;
        p_state["Fw"] = p_mover->Fw;
        p_state["Ft"] = p_mover->Ft;
        p_state["Im"] = p_mover->Im;
        p_state["compressor_enabled"] = p_mover->CompressorFlag;
        p_state["compressor_allowed"] = p_mover->CompressorAllow;
        p_state["engine_power"] = p_mover->EnginePower;
        p_state["engine_rpm_count"] = p_mover->enrot;
        p_state["engine_rpm_ratio"] = p_mover->EngineRPMRatio();
        p_state["engine_current"] = p_mover->Im;
        p_state["engine_damage"] = p_mover->EngDmgFlag;
        p_state["main_switch_time"] = p_mover->MainsInitTimeCountdown;
        p_state["main_no_power_pos"] = p_mover->IsMainCtrlNoPowerPos();
        p_state["camshaft_available"] = p_mover->HasCamshaft;
        p_state["converter_overload"] = p_mover->ConvOvldFlag;
        p_state["line_breaker_delay"] = p_mover->CtrlDelay;
        p_state["line_breaker_initial_delay"] = p_mover->InitialCtrlDelay;
        p_state["line_breaker_closes_at_no_power"] = p_mover->LineBreakerClosesOnlyAtNoPowerPos;

        if (!previous_main_switch && (static_cast<bool>(p_state["main_switch_enabled"]))) {
            emit_signal("engine_start");
        } else if (previous_main_switch && !(static_cast<bool>(p_state["main_switch_enabled"]))) {
            emit_signal("engine_stop");
        }
    }

    void TrainEngine::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        p_config["main_controller_position_max"] = p_mover->MainCtrlPosNo;
    }

    void TrainEngine::main_switch(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->MainSwitch(p_enabled);
    }

    void TrainEngine::_register_commands() {
        register_command("main_switch", Callable(this, "main_switch"));
    }

    void TrainEngine::_unregister_commands() {
        unregister_command("main_switch", Callable(this, "main_switch"));
    }
} // namespace godot
