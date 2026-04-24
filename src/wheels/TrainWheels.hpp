#pragma once

#include "../core/TrainPart.hpp"
#include "../macros.hpp"

namespace godot {
    class TrainWheels : public TrainPart {
            GDCLASS(TrainWheels, TrainPart)

        public:
            enum BearingType {
                BEARING_TYPE_SLIDE = 0,
                BEARING_TYPE_ROLL = 1,
            };

        private:
            double wheel_angle_front_deg = 0.0;
            double wheel_angle_powered_deg = 0.0;
            double wheel_angle_rear_deg = 0.0;

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override;
            void _do_process_mover(TMoverParameters *mover, double delta) override;

        public:
            static void _bind_methods();

            MAKE_MEMBER_GS_DIRTY(double, powered_wheel_diameter, 0.0);
            MAKE_MEMBER_GS_DIRTY(double, front_rolling_wheel_diameter, 0.0);
            MAKE_MEMBER_GS_DIRTY(double, rear_rolling_wheel_diameter, 0.0);
            MAKE_MEMBER_GS_DIRTY(double, axle_inertial_moment, 0.0);
            MAKE_MEMBER_GS_DIRTY(double, track_width, 1.435);
            MAKE_MEMBER_GS_DIRTY(String, axle_arrangement, "");
            MAKE_MEMBER_GS_DIRTY(double, bogie_axle_spacing, 0.0);
            MAKE_MEMBER_GS_DIRTY(double, bogie_pivot_spacing, 0.0);
            MAKE_MEMBER_GS_DIRTY(double, minimum_curve_radius, 0.0);
            MAKE_MEMBER_GS_DIRTY(int, bearing_type, BEARING_TYPE_ROLL);
    };
} // namespace godot

VARIANT_ENUM_CAST(godot::TrainWheels::BearingType);
