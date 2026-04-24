#include "TrainWheels.hpp"

#include <cmath>

namespace godot {
    namespace {
        constexpr double WHEEL_LINEAR_TO_DEGREES = 114.59155902616464175359630962821;

        /*
        double wrap_degrees(const double p_angle) {
            double wrapped = std::fmod(p_angle, 360.0);
            if (wrapped < 0.0) {
                wrapped += 360.0;
            }
            return wrapped;
        }
        */
    } // namespace

    void TrainWheels::_bind_methods() {
        BIND_PROPERTY(
                Variant::FLOAT, "powered_wheel_diameter", "powered_wheel_diameter",
                &TrainWheels::set_powered_wheel_diameter, &TrainWheels::get_powered_wheel_diameter, "diameter");
        BIND_PROPERTY(
                Variant::FLOAT, "front_rolling_wheel_diameter", "front_rolling_wheel_diameter",
                &TrainWheels::set_front_rolling_wheel_diameter, &TrainWheels::get_front_rolling_wheel_diameter,
                "diameter");
        BIND_PROPERTY(
                Variant::FLOAT, "rear_rolling_wheel_diameter", "rear_rolling_wheel_diameter",
                &TrainWheels::set_rear_rolling_wheel_diameter, &TrainWheels::get_rear_rolling_wheel_diameter,
                "diameter");
        BIND_PROPERTY(
                Variant::FLOAT, "axle_inertial_moment", "axle_inertial_moment", &TrainWheels::set_axle_inertial_moment,
                &TrainWheels::get_axle_inertial_moment, "moment");
        BIND_PROPERTY(
                Variant::FLOAT, "track_width", "track_width", &TrainWheels::set_track_width,
                &TrainWheels::get_track_width, "width");
        BIND_PROPERTY(
                Variant::STRING, "axle_arrangement", "axle_arrangement", &TrainWheels::set_axle_arrangement,
                &TrainWheels::get_axle_arrangement, "arrangement");
        BIND_PROPERTY(
                Variant::FLOAT, "bogie_axle_spacing", "bogie_axle_spacing", &TrainWheels::set_bogie_axle_spacing,
                &TrainWheels::get_bogie_axle_spacing, "spacing");
        BIND_PROPERTY(
                Variant::FLOAT, "bogie_pivot_spacing", "bogie_pivot_spacing", &TrainWheels::set_bogie_pivot_spacing,
                &TrainWheels::get_bogie_pivot_spacing, "spacing");
        BIND_PROPERTY(
                Variant::FLOAT, "minimum_curve_radius", "minimum_curve_radius", &TrainWheels::set_minimum_curve_radius,
                &TrainWheels::get_minimum_curve_radius, "radius");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "bearing_type", "bearing_type", &TrainWheels::set_bearing_type,
                &TrainWheels::get_bearing_type, "type", PROPERTY_HINT_ENUM, "Slide,Roll");

        BIND_ENUM_CONSTANT(BEARING_TYPE_SLIDE);
        BIND_ENUM_CONSTANT(BEARING_TYPE_ROLL);
    }

    void TrainWheels::_do_update_internal_mover(TMoverParameters *p_mover) {
        const double resolved_powered_wheel_diameter =
                get_powered_wheel_diameter() > 0.0 ? get_powered_wheel_diameter() : 1.0;
        const double resolved_front_rolling_wheel_diameter = get_front_rolling_wheel_diameter() > 0.0
                                                                     ? get_front_rolling_wheel_diameter()
                                                                     : resolved_powered_wheel_diameter;
        const double resolved_rear_rolling_wheel_diameter = get_rear_rolling_wheel_diameter() > 0.0
                                                                    ? get_rear_rolling_wheel_diameter()
                                                                    : resolved_powered_wheel_diameter;

        p_mover->WheelDiameter = static_cast<float>(resolved_powered_wheel_diameter);
        p_mover->WheelDiameterL = static_cast<float>(resolved_front_rolling_wheel_diameter);
        if (p_mover->WheelDiameterL == 0.0) {
            // original LoadFIZ_Wheels workaround
            p_mover->WheelDiameterL = p_mover->WheelDiameter;
        }
        p_mover->WheelDiameterT = static_cast<float>(resolved_rear_rolling_wheel_diameter);
        if (p_mover->WheelDiameterT == 0.0) {
            // original LoadFIZ_Wheels workaround
            p_mover->WheelDiameterT = p_mover->WheelDiameter;
        }
        p_mover->TrackW = static_cast<float>(track_width);
        p_mover->AxleArangement = axle_arrangement.ascii().get_data();
        p_mover->NPoweredAxles = Maszyna::s2NPW(p_mover->AxleArangement);
        p_mover->NAxles = p_mover->NPoweredAxles + Maszyna::s2NNW(p_mover->AxleArangement);
        p_mover->BearingType = bearing_type == BEARING_TYPE_SLIDE ? 0 : 1;
        p_mover->ADist = bogie_axle_spacing;
        p_mover->BDist = bogie_pivot_spacing;

        if (axle_inertial_moment <= 0.0) {
            const double k = 472.0;
            p_mover->AxleInertialMoment = k / 4.0 * std::pow(p_mover->WheelDiameter, 4.0) * p_mover->NAxles;
            // FIXME: THIS IS MODIFICATION OF OTHER SECTION, IT SHOULD BE MOVED TO POST-CONFIG STAGE
            p_mover->Mred = k * std::pow(p_mover->WheelDiameter, 2.0) * p_mover->NAxles;
        } else {
            p_mover->AxleInertialMoment = axle_inertial_moment;
        }
    }

    void TrainWheels::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        p_state["wheel_angle_front_deg"] = wheel_angle_front_deg;
        p_state["wheel_angle_powered_deg"] = wheel_angle_powered_deg;
        p_state["wheel_angle_rear_deg"] = wheel_angle_rear_deg;
        p_state["wheel_rotation_speed_rps"] = p_mover->nrot;
        p_state["wheel_rotation_acceleration_rps2"] = p_mover->nrot_eps;
        p_state["slipping_wheels"] = p_mover->SlippingWheels;
        p_state["wheel_flat"] = p_mover->WheelFlat;
    }

    void TrainWheels::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        p_config["powered_wheel_diameter"] = p_mover->WheelDiameter;
        p_config["front_rolling_wheel_diameter"] = p_mover->WheelDiameterL;
        p_config["rear_rolling_wheel_diameter"] = p_mover->WheelDiameterT;
        p_config["axle_inertial_moment"] = p_mover->AxleInertialMoment;
        p_config["track_width"] = p_mover->TrackW;
        p_config["axle_arrangement"] = String(p_mover->AxleArangement.c_str());
        p_config["bogie_axle_spacing"] = p_mover->ADist;
        p_config["bogie_pivot_spacing"] = p_mover->BDist;
        p_config["minimum_curve_radius"] = get_minimum_curve_radius();
        p_config["bearing_type"] = p_mover->BearingType == 0 ? BEARING_TYPE_SLIDE : BEARING_TYPE_ROLL;
        p_config["axles_powered_count"] = p_mover->NPoweredAxles;
        p_config["axles_count"] = p_mover->NAxles;
    }

    void TrainWheels::_do_process_mover(TMoverParameters *p_mover, const double p_delta) {
        if (p_mover->Vel == 0.0) {
            return;
        }

        wheel_angle_front_deg += WHEEL_LINEAR_TO_DEGREES * p_mover->V * p_delta / p_mover->WheelDiameterL;
        // wheel_angle_front_deg = wrap_degrees(wheel_angle_front_deg);

        wheel_angle_powered_deg += p_mover->nrot * p_delta * 360.0;
        // wheel_angle_powered_deg = wrap_degrees(wheel_angle_powered_deg);

        wheel_angle_rear_deg += WHEEL_LINEAR_TO_DEGREES * p_mover->V * p_delta / p_mover->WheelDiameterT;
        // wheel_angle_rear_deg = wrap_degrees(wheel_angle_rear_deg);
    }
} // namespace godot
