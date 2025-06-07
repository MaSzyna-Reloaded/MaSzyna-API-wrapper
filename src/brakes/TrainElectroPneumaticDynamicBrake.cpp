#include "TrainElectroPneumaticDynamicBrake.hpp"

namespace godot {
    void TrainElectroPneumaticDynamicBrake::_bind_methods() {

        ClassDB::bind_method(
                D_METHOD("set_coupler_check", "value"), &TrainElectroPneumaticDynamicBrake::set_coupler_check);
        ClassDB::bind_method(D_METHOD("get_coupler_check"), &TrainElectroPneumaticDynamicBrake::get_coupler_check);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "coupler_check", PROPERTY_HINT_ENUM, "None,Front,Back"), "set_coupler_check",
                "get_coupler_check");

        ClassDB::bind_method(D_METHOD("switch_ep_brake", "value"), &TrainElectroPneumaticDynamicBrake::switch_ep_brake);
        ClassDB::bind_method(D_METHOD("switch_ep_fuse", "value"), &TrainElectroPneumaticDynamicBrake::switch_ep_fuse);

        ClassDB::bind_method(
                D_METHOD("set_min_ep_regenerative_braking", "value"),
                &TrainElectroPneumaticDynamicBrake::set_min_ep_regenerative_braking);
        ClassDB::bind_method(
                D_METHOD("get_min_ep_regenerative_braking"),
                &TrainElectroPneumaticDynamicBrake::get_min_ep_regenerative_braking);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "electro_pneumatic/min_regenerative_braking"),
                "set_min_ep_regenerative_braking", "get_min_ep_regenerative_braking");

        ClassDB::bind_method(
                D_METHOD("set_max_ep_brake_engagement_speed", "value"),
                &TrainElectroPneumaticDynamicBrake::set_max_ep_brake_engagement_speed);
        ClassDB::bind_method(
                D_METHOD("get_max_ep_brake_engagement_speed"),
                &TrainElectroPneumaticDynamicBrake::get_max_ep_brake_engagement_speed);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "electro_pneumatic/max_brake_engagement_speed"),
                "set_max_ep_brake_engagement_speed", "get_max_ep_brake_engagement_speed");

        ClassDB::bind_method(
                D_METHOD("set_ed_braking_ep_delay", "value"),
                &TrainElectroPneumaticDynamicBrake::set_ed_braking_ep_delay);
        ClassDB::bind_method(
                D_METHOD("get_ed_braking_ep_delay"), &TrainElectroPneumaticDynamicBrake::get_ed_braking_ep_delay);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "electro_dynamic/electro_pneumatic_brake_delay"),
                "set_ed_braking_ep_delay", "get_ed_braking_ep_delay");

        BIND_ENUM_CONSTANT(None)
        BIND_ENUM_CONSTANT(Front)
        BIND_ENUM_CONSTANT(Back)
    }

    void TrainElectroPneumaticDynamicBrake::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        state["dcemued/coupler_check"] = mover->DCEMUED_CC;
        state["dcemued/ed_braking_ep_delay"] = mover->DCEMUED_EP_delay;
        state["dcemued/ep_max_brake_engagement_speed"] = mover->DCEMUED_EP_max_Vel;
        state["dcemued/ep_min_regenerative_braking"] = mover->DCEMUED_EP_min_Im;
        state["dcemued/ep_force"] = mover->EpForce;
        state["dcemued/ep_fuse"] = mover->EpFuse;
    }

    void TrainElectroPneumaticDynamicBrake::_register_commands() {
        register_command("switch_ep_brake", Callable(this, "switch_ep_brake"));
        register_command("switch_ep_fuse", Callable(this, "switch_ep_fuse"));
        TrainPart::_register_commands();
    }
    void TrainElectroPneumaticDynamicBrake::_unregister_commands() {
        unregister_command("switch_ep_brake", Callable(this, "switch_ep_brake"));
        unregister_command("switch_ep_fuse", Callable(this, "switch_ep_fuse"));
        TrainPart::_unregister_commands();
    }

    void TrainElectroPneumaticDynamicBrake::_do_update_internal_mover(TMoverParameters *mover) {
        mover->DCEMUED_CC = coupler_check;
        mover->DCEMUED_EP_delay = ed_braking_ep_delay;
        mover->DCEMUED_EP_max_Vel = max_ep_brake_engagement_speed;
        mover->DCEMUED_EP_min_Im = min_ep_regenerative_braking;
    }

    void TrainElectroPneumaticDynamicBrake::switch_ep_brake(const int p_value) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        mover->SwitchEPBrake(p_value);
    }


    void TrainElectroPneumaticDynamicBrake::switch_ep_fuse(const bool p_value) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        mover->EpFuseSwitch(p_value);
    }
} // namespace godot
