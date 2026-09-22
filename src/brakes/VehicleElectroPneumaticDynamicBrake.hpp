#pragma once
#include "../brakes/VehicleBrake.hpp"
#include "../core/VehicleController.hpp"
#include "../macros.hpp"
#include <godot_cpp/classes/gd_extension.hpp>

namespace godot {
    class VehicleController;
    class VehicleElectroPneumaticDynamicBrake : public VehicleComponent {
            GDCLASS(VehicleElectroPneumaticDynamicBrake, VehicleComponent)
            enum StateProperty {
                STATE_COUPLER_CHECK,
                STATE_EP_DELAY,
                STATE_EP_MAX_VEL,
                STATE_EP_MIN_IM,
                STATE_EP_FORCE,
                STATE_EP_FUSE,
            };

        public:
            Variant _get_state_property(int p_local_index) const override;

            static void _bind_methods();
            enum CouplerCheck {
                NONE = 0,
                FRONT = 1,
                BACK = 2,
            };

            void set_ep_brake_force(int p_value);
            void switch_ep_fuse(bool p_value);

        private:
            int state_base_index = 0;

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _declare_state_properties() override;
            void _do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) override {};
            void _register_commands() override;
            void _unregister_commands() override;

        private:
            MAKE_MEMBER_GS_NR(CouplerCheck, coupler_check, CouplerCheck::NONE);
            MAKE_MEMBER_GS(float, electro_pneumatic_brake_delay, 0.0f);
            MAKE_MEMBER_GS(float, electro_pneumatic_min_regenerative_braking, 0.0f);
            MAKE_MEMBER_GS(float, electro_pneumatic_max_ep_brake_engagement_speed, 0.0f);
            MAKE_MEMBER_GS(bool, ep_brake_fuse, false);

            /* Blending: (laczenie trybow hamowania EP+ED) */
            MAKE_MEMBER_GS(double, blending_max_velocity, 0.0);
            MAKE_MEMBER_GS(double, blending_min_velocity, 0.0);
            MAKE_MEMBER_GS(double, blending_reference_velocity, 0.0);
            MAKE_MEMBER_GS(double, blending_max_deceleration, 9.81);
            MAKE_MEMBER_GS(bool, blending_velocity_correction, false);
            MAKE_MEMBER_GS(bool, blending_load_correction, false);
            MAKE_MEMBER_GS(double, blending_min_ed_brake_request, 0.0);
    };
} // namespace godot
VARIANT_ENUM_CAST(VehicleElectroPneumaticDynamicBrake::CouplerCheck)
