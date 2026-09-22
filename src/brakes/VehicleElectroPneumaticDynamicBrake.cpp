#include "VehicleElectroPneumaticDynamicBrake.hpp"

namespace godot {
    void VehicleElectroPneumaticDynamicBrake::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("set_ep_brake_force", "value"), &VehicleElectroPneumaticDynamicBrake::set_ep_brake_force);
        ClassDB::bind_method(D_METHOD("switch_ep_fuse", "value"), &VehicleElectroPneumaticDynamicBrake::switch_ep_fuse);

        BIND_PROPERTY_W_HINT(
                VehicleElectroPneumaticDynamicBrake, Variant::INT, coupler_check, PROPERTY_HINT_ENUM, "None,Front,Back");
        BIND_PROPERTY(
                VehicleElectroPneumaticDynamicBrake, Variant::FLOAT, electro_pneumatic_min_regenerative_braking,
                "electro_pneumatic");
        BIND_PROPERTY(
                VehicleElectroPneumaticDynamicBrake, Variant::FLOAT, electro_pneumatic_max_ep_brake_engagement_speed,
                "electro_pneumatic");
        BIND_PROPERTY(
                VehicleElectroPneumaticDynamicBrake, Variant::FLOAT, electro_pneumatic_brake_delay, "electro_pneumatic");
        BIND_PROPERTY(VehicleElectroPneumaticDynamicBrake, Variant::BOOL, ep_brake_fuse);
        BIND_PROPERTY(VehicleElectroPneumaticDynamicBrake, Variant::FLOAT, blending_max_velocity, "blending");
        BIND_PROPERTY(VehicleElectroPneumaticDynamicBrake, Variant::FLOAT, blending_min_velocity, "blending");
        BIND_PROPERTY(VehicleElectroPneumaticDynamicBrake, Variant::FLOAT, blending_reference_velocity, "blending");
        BIND_PROPERTY(VehicleElectroPneumaticDynamicBrake, Variant::FLOAT, blending_max_deceleration, "blending");
        BIND_PROPERTY(VehicleElectroPneumaticDynamicBrake, Variant::BOOL, blending_velocity_correction, "blending");
        BIND_PROPERTY(VehicleElectroPneumaticDynamicBrake, Variant::BOOL, blending_load_correction, "blending");
        BIND_PROPERTY(VehicleElectroPneumaticDynamicBrake, Variant::FLOAT, blending_min_ed_brake_request, "blending");

        BIND_ENUM_CONSTANT(NONE)
        BIND_ENUM_CONSTANT(FRONT)
        BIND_ENUM_CONSTANT(BACK)
    }

    void VehicleElectroPneumaticDynamicBrake::_declare_state_properties() {
        state_base_index = get_state_property_count();
        declare_state_property("dcemued/coupler_check", Variant::BOOL);
        declare_state_property("dcemued/ed_braking_ep_delay", Variant::FLOAT);
        declare_state_property("dcemued/ep_max_brake_engagement_speed", Variant::FLOAT);
        declare_state_property("dcemued/ep_min_regenerative_braking", Variant::FLOAT);
        declare_state_property("dcemued/ep_force", Variant::FLOAT);
        declare_state_property("dcemued/ep_fuse", Variant::BOOL);
    }

    Variant VehicleElectroPneumaticDynamicBrake::_get_state_property(const int p_local_index) const {
        const TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return Variant();
        }
        switch (p_local_index - state_base_index) {
            case STATE_COUPLER_CHECK:
                return mover->DCEMUED_CC;
            case STATE_EP_DELAY:
                return mover->DCEMUED_EP_delay;
            case STATE_EP_MAX_VEL:
                return mover->DCEMUED_EP_max_Vel;
            case STATE_EP_MIN_IM:
                return mover->DCEMUED_EP_min_Im;
            case STATE_EP_FORCE:
                return mover->EpForce;
            case STATE_EP_FUSE:
                return mover->EpFuse;
            default:
                return Variant();
        }
    }

    void VehicleElectroPneumaticDynamicBrake::_register_commands() {
        register_command("set_ep_brake_force", Callable(this, "set_ep_brake_force"));
        register_command("switch_ep_fuse", Callable(this, "switch_ep_fuse"));
        VehicleComponent::_register_commands();
    }

    void VehicleElectroPneumaticDynamicBrake::_unregister_commands() {
        unregister_command("set_ep_brake_force", Callable(this, "set_ep_brake_force"));
        unregister_command("switch_ep_fuse", Callable(this, "switch_ep_fuse"));
        VehicleComponent::_unregister_commands();
    }

    void VehicleElectroPneumaticDynamicBrake::_do_update_internal_mover(TMoverParameters *p_mover) {
        p_mover->DCEMUED_CC = coupler_check;
        p_mover->DCEMUED_EP_delay = electro_pneumatic_brake_delay;
        p_mover->DCEMUED_EP_max_Vel = electro_pneumatic_max_ep_brake_engagement_speed;
        p_mover->DCEMUED_EP_min_Im = electro_pneumatic_min_regenerative_braking;
        p_mover->EpFuseSwitch(ep_brake_fuse);

        p_mover->MED_Vmax = blending_max_velocity;
        p_mover->MED_Vmin = blending_min_velocity;
        p_mover->MED_Vref = blending_reference_velocity;
        p_mover->MED_amax = blending_max_deceleration;
        p_mover->MED_EPVC = blending_velocity_correction;
        p_mover->MED_Ncor = blending_load_correction;
        p_mover->MED_MinBrakeReqED = blending_min_ed_brake_request;
    }

    void VehicleElectroPneumaticDynamicBrake::set_ep_brake_force(const int p_value) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        mover->SwitchEPBrake(p_value);
    }


    void VehicleElectroPneumaticDynamicBrake::switch_ep_fuse(const bool p_value) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        mover->EpFuseSwitch(p_value);
    }
} // namespace godot
