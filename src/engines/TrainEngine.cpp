#include "TrainEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    class TrainNode;

    TrainEngine::TrainEngine() {
        set_capability("engine");
    }

    void TrainEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("main_switch", "enabled"), &TrainEngine::main_switch);
        BIND_PROPERTY_W_HINT_RES_ARRAY(Variant::ARRAY, "motor_param_table", "motor_param_table", &TrainEngine::set_motor_param_table, &TrainEngine::get_motor_param_table, "motor_param_table", PROPERTY_HINT_TYPE_STRING, "MotorParameter");
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
    }

    void TrainEngine::_do_update_internal_mover(TMoverParameters *mover) {
        mover->EngineType = engine_type_map.at(get_engine_type());

        /* FIXME: for testing purposes */
        mover->GroundRelay = true;
        mover->NoVoltRelay = true;
        mover->OvervoltageRelay = true;
        mover->DamageFlag = 0;
        mover->EngDmgFlag = 0;
        mover->ConvOvldFlag = false;
        /* end testing */

        /* motor param table */
        constexpr int _max = Maszyna::MotorParametersArraySize;
        for (int i = 0; i < std::min(_max, static_cast<int>(motor_param_table.size())); i++) {
            const Ref<MotorParameter> &row = motor_param_table[i];
            if (row == nullptr || !row.is_valid() || row.is_null()) {
                UtilityFunctions::push_warning("[TrainEngine]: motor_param_table property is null at index " + String::num(i));
                return;
            }

            mover->MotorParam[i].mIsat = row->get_saturation_current_multiplier();
            mover->MotorParam[i].fi = row->get_voltage_constant();
            mover->MotorParam[i].mfi = row->get_voltage_constant_multiplier();
            mover->MotorParam[i].Isat = row->get_saturation_current();
            mover->MPTRelay[i].Iup = row->get_shunting_up();//bocznikowanie
            mover->MPTRelay[i].Idown = row->get_shunting_down();//bocznikowanie;
        }
    }

    void TrainEngine::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        const bool previous_main_switch = (state.get("main_switch_enabled", false));
        state["main_switch_enabled"] = mover->Mains;
        state["Mm"] = mover->Mm;
        state["Mw"] = mover->Mw;
        state["Fw"] = mover->Fw;
        state["Ft"] = mover->Ft;
        state["Im"] = mover->Im;
        state["compressor_enabled"] = mover->CompressorFlag;
        state["compressor_allowed"] = mover->CompressorAllow;
        state["engine_power"] = mover->EnginePower;
        state["engine_rpm_count"] = mover->enrot;
        state["engine_rpm_ratio"] = mover->EngineRPMRatio();
        state["engine_current"] = mover->Im;
        state["engine_damage"] = mover->EngDmgFlag;
        state["main_switch_time"] = mover->MainsInitTimeCountdown;
        state["main_no_power_pos"] = mover->IsMainCtrlNoPowerPos();
        state["camshaft_available"] = mover->HasCamshaft;
        state["converter_overload"] = mover->ConvOvldFlag;
        state["line_breaker_delay"] = mover->CtrlDelay;
        state["line_breaker_initial_delay"] = mover->InitialCtrlDelay;
        state["line_breaker_closes_at_no_power"] = mover->LineBreakerClosesOnlyAtNoPowerPos;

        if (!previous_main_switch && (static_cast<bool>(state["main_switch_enabled"]))) {
            emit_signal("engine_start");
        } else if (previous_main_switch && !(static_cast<bool>(state["main_switch_enabled"]))) {
            emit_signal("engine_stop");
        }
    }

    void TrainEngine::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {
        config["main_controller_position_max"] = mover->MainCtrlPosNo;
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
