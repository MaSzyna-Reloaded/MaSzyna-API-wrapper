#include "TrainWheels.hpp"

#include <cmath>

namespace godot {
    namespace {
        constexpr double WHEEL_LINEAR_TO_DEGREES = 114.59155902616464175359630962821;

        double wrap_degrees(const double angle) {
            double wrapped = std::fmod(angle, 360.0);
            if (wrapped < 0.0) {
                wrapped += 360.0;
            }
            return wrapped;
        }
    } // namespace

    void TrainWheels::_bind_methods() {
        BIND_PROPERTY(Variant::FLOAT, "powered_wheel_diameter", "powered_wheel_diameter", &TrainWheels::set_powered_wheel_diameter, &TrainWheels::get_powered_wheel_diameter, "diameter");
        BIND_PROPERTY(Variant::FLOAT, "front_rolling_wheel_diameter", "front_rolling_wheel_diameter", &TrainWheels::set_front_rolling_wheel_diameter, &TrainWheels::get_front_rolling_wheel_diameter, "diameter");
        BIND_PROPERTY(Variant::FLOAT, "rear_rolling_wheel_diameter", "rear_rolling_wheel_diameter", &TrainWheels::set_rear_rolling_wheel_diameter, &TrainWheels::get_rear_rolling_wheel_diameter, "diameter");
        BIND_PROPERTY(Variant::FLOAT, "axle_inertial_moment", "axle_inertial_moment", &TrainWheels::set_axle_inertial_moment, &TrainWheels::get_axle_inertial_moment, "moment");
        BIND_PROPERTY(Variant::FLOAT, "track_width", "track_width", &TrainWheels::set_track_width, &TrainWheels::get_track_width, "width");
        BIND_PROPERTY(Variant::STRING, "axle_arrangement", "axle_arrangement", &TrainWheels::set_axle_arrangement, &TrainWheels::get_axle_arrangement, "arrangement");
        BIND_PROPERTY(Variant::FLOAT, "bogie_axle_spacing", "bogie_axle_spacing", &TrainWheels::set_bogie_axle_spacing, &TrainWheels::get_bogie_axle_spacing, "spacing");
        BIND_PROPERTY(Variant::FLOAT, "bogie_pivot_spacing", "bogie_pivot_spacing", &TrainWheels::set_bogie_pivot_spacing, &TrainWheels::get_bogie_pivot_spacing, "spacing");
        BIND_PROPERTY(Variant::FLOAT, "minimum_curve_radius", "minimum_curve_radius", &TrainWheels::set_minimum_curve_radius, &TrainWheels::get_minimum_curve_radius, "radius");
        BIND_PROPERTY_W_HINT(Variant::INT, "bearing_type", "bearing_type", &TrainWheels::set_bearing_type, &TrainWheels::get_bearing_type, "type", PROPERTY_HINT_ENUM, "Slide,Roll");

        BIND_ENUM_CONSTANT(BEARING_TYPE_SLIDE);
        BIND_ENUM_CONSTANT(BEARING_TYPE_ROLL);
    }

    void TrainWheels::_do_update_internal_mover(TMoverParameters *mover) {
        const double resolved_powered_wheel_diameter = get_powered_wheel_diameter() > 0.0 ? get_powered_wheel_diameter() : 1.0;
        const double resolved_front_rolling_wheel_diameter =
                get_front_rolling_wheel_diameter() > 0.0 ? get_front_rolling_wheel_diameter() : resolved_powered_wheel_diameter;
        const double resolved_rear_rolling_wheel_diameter =
                get_rear_rolling_wheel_diameter() > 0.0 ? get_rear_rolling_wheel_diameter() : resolved_powered_wheel_diameter;

        mover->WheelDiameter = static_cast<float>(resolved_powered_wheel_diameter);
        mover->WheelDiameterL = static_cast<float>(resolved_front_rolling_wheel_diameter);
        mover->WheelDiameterT = static_cast<float>(resolved_rear_rolling_wheel_diameter);
        mover->TrackW = static_cast<float>(get_track_width());
        mover->AxleArangement = get_axle_arrangement().ascii().get_data();
        mover->NPoweredAxles = Maszyna::s2NPW(mover->AxleArangement);
        mover->NAxles = mover->NPoweredAxles + Maszyna::s2NNW(mover->AxleArangement);
        mover->BearingType = get_bearing_type() == BEARING_TYPE_SLIDE ? 0 : 1;
        mover->ADist = get_bogie_axle_spacing();
        mover->BDist = get_bogie_pivot_spacing();

        if (get_axle_inertial_moment() > 0.0) {
            mover->AxleInertialMoment = get_axle_inertial_moment();
            mover->Mred = 0.0;
        } else {
            const double k = 472.0;
            mover->AxleInertialMoment = k / 4.0 * std::pow(mover->WheelDiameter, 4.0) * mover->NAxles;
            mover->Mred = k * std::pow(mover->WheelDiameter, 2.0) * mover->NAxles;
        }
    }

    void TrainWheels::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        state["wheel_angle_front_deg"] = wheel_angle_front_deg;
        state["wheel_angle_powered_deg"] = wheel_angle_powered_deg;
        state["wheel_angle_rear_deg"] = wheel_angle_rear_deg;
        state["wheel_rotation_speed_rps"] = mover->nrot;
        state["wheel_rotation_acceleration_rps2"] = mover->nrot_eps;
        state["slipping_wheels"] = mover->SlippingWheels;
        state["wheel_flat"] = mover->WheelFlat;
    }

    void TrainWheels::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {
        config["powered_wheel_diameter"] = mover->WheelDiameter;
        config["front_rolling_wheel_diameter"] = mover->WheelDiameterL;
        config["rear_rolling_wheel_diameter"] = mover->WheelDiameterT;
        config["axle_inertial_moment"] = mover->AxleInertialMoment;
        config["track_width"] = mover->TrackW;
        config["axle_arrangement"] = String(mover->AxleArangement.c_str());
        config["bogie_axle_spacing"] = mover->ADist;
        config["bogie_pivot_spacing"] = mover->BDist;
        config["minimum_curve_radius"] = get_minimum_curve_radius();
        config["bearing_type"] = mover->BearingType == 0 ? BEARING_TYPE_SLIDE : BEARING_TYPE_ROLL;
        config["axles_powered_count"] = mover->NPoweredAxles;
        config["axles_count"] = mover->NAxles;
    }

    void TrainWheels::_do_process_mover(TMoverParameters *mover, const double delta) {
        if (mover->Vel == 0.0) {
            return;
        }

        if (mover->WheelDiameterL > 0.0) {
            wheel_angle_front_deg += WHEEL_LINEAR_TO_DEGREES * mover->V * delta / mover->WheelDiameterL;
            wheel_angle_front_deg = wrap_degrees(wheel_angle_front_deg);
        }

        wheel_angle_powered_deg += mover->nrot * delta * 360.0;
        wheel_angle_powered_deg = wrap_degrees(wheel_angle_powered_deg);

        if (mover->WheelDiameterT > 0.0) {
            wheel_angle_rear_deg += WHEEL_LINEAR_TO_DEGREES * mover->V * delta / mover->WheelDiameterT;
            wheel_angle_rear_deg = wrap_degrees(wheel_angle_rear_deg);
        }
    }
} // namespace godot
