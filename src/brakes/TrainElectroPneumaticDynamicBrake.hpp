#pragma once
#include "../brakes/TrainBrake.hpp"
#include "../core/TrainController.hpp"
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

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

            void set_coupler_check(const CouplerCheck check) {
                coupler_check = check;
            }

            CouplerCheck get_coupler_check() {
                return coupler_check;
            }

            void set_min_ep_regenerative_braking(const float p_value) {
                min_ep_regenerative_braking = p_value;
            }

            float get_min_ep_regenerative_braking() {
                return min_ep_regenerative_braking;
            }

            void set_max_ep_brake_engagement_speed(const float p_value) {
                max_ep_brake_engagement_speed = p_value;
            }

            float get_max_ep_brake_engagement_speed() {
                return max_ep_brake_engagement_speed;
            }

            void set_ed_braking_ep_delay(const float p_value) {
                ed_braking_ep_delay = p_value;
            }

            float get_ed_braking_ep_delay() {
                return ed_braking_ep_delay;
            }

            void switch_ep_brake(int p_value);

            void switch_ep_fuse(bool p_value);

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override {};
            void _register_commands() override;
            void _unregister_commands() override;
        private:
            CouplerCheck coupler_check = CouplerCheck::None;
            float min_ep_regenerative_braking = 0.0f;
            float max_ep_brake_engagement_speed = 0.0f;
            float ed_braking_ep_delay = 0.0f;
    };
} // namespace godot
VARIANT_ENUM_CAST(TrainElectroPneumaticDynamicBrake::CouplerCheck)
