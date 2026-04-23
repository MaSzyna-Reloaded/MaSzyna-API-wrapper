#include "TrainEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    TrainEngine::~TrainEngine() {
        motor_param_table.clear();
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

    void TrainEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        p_mover->EngineType = engine_type_map.at(get_engine_type());

        /* FIXME: for testing purposes */
        p_mover->GroundRelay = true;
        p_mover->NoVoltRelay = true;
        p_mover->OvervoltageRelay = true;
        p_mover->ConvOvldFlag = false;
        /* end testing */

        /* motor param table */
        constexpr int MAX = Maszyna::MotorParametersArraySize;
        for (int i = 0; i < std::min(MAX, static_cast<int>(motor_param_table.size())); i++) {
            const Ref<MotorParameter> &row = motor_param_table[i];
            if (row == nullptr || !row.is_valid() || row.is_null()) {
                UtilityFunctions::push_warning("[TrainEngine]: motor_param_table property is null at index " + String::num(i));
                return;
            }

            p_mover->MotorParam[i].mIsat = row->get_saturation_current_multiplier();
            p_mover->MotorParam[i].fi = row->get_voltage_constant();
            p_mover->MotorParam[i].mfi = row->get_voltage_constant_multiplier();
            p_mover->MotorParam[i].Isat = row->get_saturation_current();
            p_mover->MPTRelay[i].Iup = row->get_shunting_up();//bocznikowanie
            p_mover->MPTRelay[i].Idown = row->get_shunting_down();//bocznikowanie;
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

    TypedArray<TrainCommand> TrainEngine::get_supported_commands() {
        TypedArray<TrainCommand> commands = TrainPart::get_supported_commands();
        commands.append(make_train_command("main_switch", Callable(this, "main_switch")));
        return commands;
    }
} // namespace godot
