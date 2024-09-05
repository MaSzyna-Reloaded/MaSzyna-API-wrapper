#include "TrainEngine.hpp"

namespace godot {
    class TrainController;

    TrainEngine::TrainEngine() = default;

    void TrainEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_main_switch_pressed"), &TrainEngine::set_main_switch_pressed);
        ClassDB::bind_method(D_METHOD("get_main_switch_pressed"), &TrainEngine::get_main_switch_pressed);

        ClassDB::bind_method(D_METHOD("set_compressor_switch_pressed"), &TrainEngine::set_compressor_switch_pressed);
        ClassDB::bind_method(D_METHOD("get_compressor_switch_pressed"), &TrainEngine::get_compressor_switch_pressed);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "switches/main"), "set_main_switch_pressed", "get_main_switch_pressed");
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "switches/compressor"), "set_compressor_switch_pressed",
                "get_compressor_switch_pressed");

        ClassDB::bind_method(D_METHOD("set_motor_param_table"), &TrainEngine::set_motor_param_table);
        ClassDB::bind_method(D_METHOD("get_motor_param_table"), &TrainEngine::get_motor_param_table);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::ARRAY, "motor_param_table", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT,
                        "TypedArray<Array>"),
                "set_motor_param_table", "get_motor_param_table");
    }

    void TrainEngine::_do_update_internal_mover(TMoverParameters *mover) {
        mover->EngineType = get_engine_type();

        /* FIXME: for testing purposes */
        mover->GroundRelay = true;
        mover->NoVoltRelay = true;
        mover->OvervoltageRelay = true;
        mover->DamageFlag = false;
        mover->EngDmgFlag = false;
        mover->ConvOvldFlag = false;
        /* end testing */

        /* motor param table */
        const int _max = Maszyna::MotorParametersArraySize;
        for (int i = 0; i < std::min(_max, (int)motor_param_table.size()); i++) {
            mover->MotorParam[i].mIsat = motor_param_table[i].get("misat");
            mover->MotorParam[i].fi = motor_param_table[i].get("fi");
            mover->MotorParam[i].mfi = motor_param_table[i].get("mfi");
            mover->MotorParam[i].Isat = motor_param_table[i].get("isat");
            mover->MPTRelay[i].Iup = motor_param_table[i].get("iup");
            mover->MPTRelay[i].Idown = motor_param_table[i].get("idown");
        }
    }

    void TrainEngine::_do_process_mover(TMoverParameters *mover, const double delta) {
        mover->MainSwitch(main_switch_pressed);
        mover->CompressorSwitch(compressor_switch_pressed);
    }

    void TrainEngine::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        state["Mm"] = mover->Mm;
        state["Mw"] = mover->Mw;
        state["Fw"] = mover->Fw;
        state["Ft"] = mover->Ft;
        state["Im"] = mover->Im;
        state["compressor_enabled"] = mover->CompressorFlag;
        state["compressor_allowed"] = mover->CompressorAllow;
        state["engine_power"] = mover->EnginePower;
        state["engine_rpm_count"] = mover->enrot;
        state["engine_current"] = mover->Im;
        state["engine_damage"] = mover->EngDmgFlag;
        state["main_switch_time"] = mover->MainsInitTimeCountdown;
        state["main_no_power_pos"] = mover->IsMainCtrlNoPowerPos();
        state["camshaft_available"] = mover->HasCamshaft;
        state["converter_overload"] = mover->ConvOvldFlag;
        state["line_breaker_delay"] = mover->CtrlDelay;
        state["line_breaker_initial_delay"] = mover->InitialCtrlDelay;
        state["line_breaker_closes_at_no_power"] = mover->LineBreakerClosesOnlyAtNoPowerPos;
    }

    void TrainEngine::set_main_switch_pressed(const bool p_state) {
        main_switch_pressed = p_state;
        _dirty = true;
    }

    bool TrainEngine::get_main_switch_pressed() const {
        return main_switch_pressed;
    }

    void TrainEngine::set_compressor_switch_pressed(const bool p_state) {
        compressor_switch_pressed = p_state;
        _dirty = true;
    }

    bool TrainEngine::get_compressor_switch_pressed() const {
        return compressor_switch_pressed;
    }

    TypedArray<Dictionary> TrainEngine::get_motor_param_table() {
        return motor_param_table;
    }

    void TrainEngine::set_motor_param_table(const TypedArray<Dictionary> p_value) {
        motor_param_table.clear();
        motor_param_table.append_array(p_value);
    }

} // namespace godot
