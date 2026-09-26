#include "VehicleElectroPneumaticDynamicBrake.hpp"

namespace godot {
    void VehicleElectroPneumaticDynamicBrake::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("set_ep_brake_force", "value"), &VehicleElectroPneumaticDynamicBrake::set_ep_brake_force);
        ClassDB::bind_method(D_METHOD("switch_ep_fuse", "value"), &VehicleElectroPneumaticDynamicBrake::switch_ep_fuse);

        BIND_PROPERTY_W_HINT(
                VehicleElectroPneumaticDynamicBrake, Variant::INT, coupler_check, PROPERTY_HINT_ENUM,
                "None,Front,Back");
        BIND_PROPERTY(
                VehicleElectroPneumaticDynamicBrake, Variant::FLOAT, electro_pneumatic_min_regenerative_braking,
                "electro_pneumatic");
        BIND_PROPERTY(
                VehicleElectroPneumaticDynamicBrake, Variant::FLOAT, electro_pneumatic_max_ep_brake_engagement_speed,
                "electro_pneumatic");
        BIND_PROPERTY(
                VehicleElectroPneumaticDynamicBrake, Variant::FLOAT, electro_pneumatic_brake_delay,
                "electro_pneumatic");
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

        ClassDB::bind_method(
                D_METHOD("get_ed_braking_ep_delay"), &VehicleElectroPneumaticDynamicBrake::get_ed_braking_ep_delay);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "ed_braking_ep_delay", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_ed_braking_ep_delay");
        ClassDB::bind_method(
                D_METHOD("get_ep_max_brake_engagement_speed"),
                &VehicleElectroPneumaticDynamicBrake::get_ep_max_brake_engagement_speed);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "ep_max_brake_engagement_speed", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_ep_max_brake_engagement_speed");
        ClassDB::bind_method(
                D_METHOD("get_ep_min_regenerative_braking"),
                &VehicleElectroPneumaticDynamicBrake::get_ep_min_regenerative_braking);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "ep_min_regenerative_braking", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_ep_min_regenerative_braking");
        ClassDB::bind_method(D_METHOD("get_ep_force"), &VehicleElectroPneumaticDynamicBrake::get_ep_force);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "ep_force", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_ep_force");
        ClassDB::bind_method(D_METHOD("get_ep_fuse"), &VehicleElectroPneumaticDynamicBrake::get_ep_fuse);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "ep_fuse", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_ep_fuse");
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
} // namespace godot
