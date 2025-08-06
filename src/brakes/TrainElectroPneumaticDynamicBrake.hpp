#pragma once
#include "../brakes/TrainBrake.hpp"
#include "../core/TrainController.hpp"
#include "../macros.hpp"
#include <godot_cpp/classes/gd_extension.hpp>

namespace godot {
    class TrainController;
    class TrainElectroPneumaticDynamicBrake: public TrainPart {
        GDCLASS(TrainElectroPneumaticDynamicBrake, TrainPart)
        public:
            static void _bind_methods();
            enum CouplerCheck {
                None = 0,
                Front = 1,
                Back = 2,
            };

            void set_ep_brake_force(int p_value);
            void switch_ep_fuse(bool p_value);

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override {};
            void _register_commands() override;
            void _unregister_commands() override;
        private:
            MAKE_MEMBER_GS_NR(CouplerCheck, coupler_check, CouplerCheck::None);
            MAKE_MEMBER_GS(float, ed_braking_ep_delay, 0.0f);
            MAKE_MEMBER_GS(float, min_ep_regenerative_braking, 0.0f);
            MAKE_MEMBER_GS(float, max_ep_brake_engagement_speed, 0.0f);
            MAKE_MEMBER_GS(bool, ep_brake_fuse, false);
    };
} // namespace godot
VARIANT_ENUM_CAST(TrainElectroPneumaticDynamicBrake::CouplerCheck)
