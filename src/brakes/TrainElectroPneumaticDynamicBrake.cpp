#include "TrainElectroPneumaticDynamicBrake.hpp"

namespace godot {
    void TrainElectroPneumaticDynamicBrake::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("set_ep_brake_force", "value"), &TrainElectroPneumaticDynamicBrake::set_ep_brake_force);
        ClassDB::bind_method(D_METHOD("switch_ep_fuse", "value"), &TrainElectroPneumaticDynamicBrake::switch_ep_fuse);

        BIND_PROPERTY_W_HINT(
                TrainElectroPneumaticDynamicBrake, Variant::INT, coupler_check, PROPERTY_HINT_ENUM, "None,Front,Back");
        BIND_PROPERTY(
                TrainElectroPneumaticDynamicBrake, Variant::FLOAT, electro_pneumatic_min_regenerative_braking,
                "electro_pneumatic");
        BIND_PROPERTY(
                TrainElectroPneumaticDynamicBrake, Variant::FLOAT, electro_pneumatic_max_ep_brake_engagement_speed,
                "electro_pneumatic");
        BIND_PROPERTY(
                TrainElectroPneumaticDynamicBrake, Variant::FLOAT, electro_pneumatic_brake_delay, "electro_pneumatic");
        BIND_PROPERTY(TrainElectroPneumaticDynamicBrake, Variant::BOOL, ep_brake_fuse);

        BIND_ENUM_CONSTANT(NONE)
        BIND_ENUM_CONSTANT(FRONT)
        BIND_ENUM_CONSTANT(BACK)
    }

    void TrainElectroPneumaticDynamicBrake::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        p_state["dcemued/coupler_check"] = p_mover->DCEMUED_CC;
        p_state["dcemued/ed_braking_ep_delay"] = p_mover->DCEMUED_EP_delay;
        p_state["dcemued/ep_max_brake_engagement_speed"] = p_mover->DCEMUED_EP_max_Vel;
        p_state["dcemued/ep_min_regenerative_braking"] = p_mover->DCEMUED_EP_min_Im;
        p_state["dcemued/ep_force"] = p_mover->EpForce;
        p_state["dcemued/ep_fuse"] = p_mover->EpFuse;
    }

    void TrainElectroPneumaticDynamicBrake::_register_commands() {
        register_command("set_ep_brake_force", Callable(this, "set_ep_brake_force"));
        register_command("switch_ep_fuse", Callable(this, "switch_ep_fuse"));
        TrainPart::_register_commands();
    }

    void TrainElectroPneumaticDynamicBrake::_unregister_commands() {
        unregister_command("set_ep_brake_force", Callable(this, "set_ep_brake_force"));
        unregister_command("switch_ep_fuse", Callable(this, "switch_ep_fuse"));
        TrainPart::_unregister_commands();
    }

    void TrainElectroPneumaticDynamicBrake::_do_update_internal_mover(TMoverParameters *p_mover) {
        p_mover->DCEMUED_CC = coupler_check;
        p_mover->DCEMUED_EP_delay = electro_pneumatic_brake_delay;
        p_mover->DCEMUED_EP_max_Vel = electro_pneumatic_max_ep_brake_engagement_speed;
        p_mover->DCEMUED_EP_min_Im = electro_pneumatic_min_regenerative_braking;
        p_mover->EpFuseSwitch(ep_brake_fuse);
    }

    void TrainElectroPneumaticDynamicBrake::set_ep_brake_force(const int p_value) {
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
