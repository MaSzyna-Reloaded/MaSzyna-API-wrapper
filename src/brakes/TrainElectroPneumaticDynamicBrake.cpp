#include "TrainElectroPneumaticDynamicBrake.hpp"

namespace godot {
    void TrainElectroPneumaticDynamicBrake::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_ep_brake_force", "value"), &TrainElectroPneumaticDynamicBrake::set_ep_brake_force);
        ClassDB::bind_method(D_METHOD("switch_ep_fuse", "value"), &TrainElectroPneumaticDynamicBrake::switch_ep_fuse);

        BIND_PROPERTY_W_HINT(Variant::INT, "coupler_check", "coupler_check", &TrainElectroPneumaticDynamicBrake::set_coupler_check, &TrainElectroPneumaticDynamicBrake::get_coupler_check, "value", PROPERTY_HINT_ENUM, "None,Front,Back");
        BIND_PROPERTY(Variant::FLOAT, "min_regenerative_braking", "electro_pneumatic/min_regenerative_braking", &TrainElectroPneumaticDynamicBrake::set_min_ep_regenerative_braking, &TrainElectroPneumaticDynamicBrake::get_min_ep_regenerative_braking, "value");
        BIND_PROPERTY(Variant::FLOAT, "max_ep_brake_engagement_speed", "electro_pneumatic/max_ep_brake_engagement_speed", &TrainElectroPneumaticDynamicBrake::set_max_ep_brake_engagement_speed, &TrainElectroPneumaticDynamicBrake::get_max_ep_brake_engagement_speed, "value");
        BIND_PROPERTY(Variant::FLOAT, "electro_pneumatic_brake_delay", "electro_pneumatic/electro_pneumatic_brake_delay", &TrainElectroPneumaticDynamicBrake::set_ed_braking_ep_delay, &TrainElectroPneumaticDynamicBrake::get_ed_braking_ep_delay, "value");
        BIND_PROPERTY(Variant::BOOL, "ep_brake_fuse", "ep_brake_fuse", &TrainElectroPneumaticDynamicBrake::set_ep_brake_fuse, &TrainElectroPneumaticDynamicBrake::get_ep_brake_fuse, "fuse_state");

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

    TypedArray<TrainCommand> TrainElectroPneumaticDynamicBrake::get_supported_commands() {
        TypedArray<TrainCommand> commands = TrainPart::get_supported_commands();
        commands.append(make_train_command("set_ep_brake_force", Callable(this, "set_ep_brake_force")));
        commands.append(make_train_command("switch_ep_fuse", Callable(this, "switch_ep_fuse")));
        return commands;
    }

    void TrainElectroPneumaticDynamicBrake::_do_update_internal_mover(TMoverParameters *p_mover) {
        p_mover->DCEMUED_CC = coupler_check;
        p_mover->DCEMUED_EP_delay = ed_braking_ep_delay;
        p_mover->DCEMUED_EP_max_Vel = max_ep_brake_engagement_speed;
        p_mover->DCEMUED_EP_min_Im = min_ep_regenerative_braking;
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
