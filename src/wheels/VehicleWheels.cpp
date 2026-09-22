#include "VehicleWheels.hpp"

#include <godot_cpp/core/math.hpp>

namespace godot {
    void VehicleWheels::_bind_methods() {
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, powered_wheel_diameter);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, front_rolling_wheel_diameter);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, rear_rolling_wheel_diameter);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, axle_inertial_moment);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, track_width);
        BIND_PROPERTY(VehicleWheels, Variant::STRING, axle_arrangement);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, bogie_axle_spacing);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, bogie_pivot_spacing);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, minimum_curve_radius);
        BIND_PROPERTY_W_HINT(VehicleWheels, Variant::INT, bearing_type, PROPERTY_HINT_ENUM, "Slide,Roll");

        BIND_ENUM_CONSTANT(BEARING_TYPE_SLIDE);
        BIND_ENUM_CONSTANT(BEARING_TYPE_ROLL);
    }

    void VehicleWheels::_do_update_internal_mover(TMoverParameters *p_mover) {
        const double resolved_powered_wheel_diameter = powered_wheel_diameter > 0.0 ? powered_wheel_diameter : 1.0;
        const double resolved_front_rolling_wheel_diameter =
                front_rolling_wheel_diameter > 0.0 ? front_rolling_wheel_diameter : resolved_powered_wheel_diameter;
        const double resolved_rear_rolling_wheel_diameter =
                rear_rolling_wheel_diameter > 0.0 ? rear_rolling_wheel_diameter : resolved_powered_wheel_diameter;

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

    void VehicleWheels::_declare_state_properties() {
        state_base_index = get_state_property_count();
        declare_state_property("wheel_angle_front_deg", Variant::FLOAT);
        declare_state_property("wheel_angle_powered_deg", Variant::FLOAT);
        declare_state_property("wheel_angle_rear_deg", Variant::FLOAT);
        declare_state_property("wheel_rotation_speed_rps", Variant::FLOAT);
        declare_state_property("wheel_rotation_acceleration_rps2", Variant::FLOAT);
        declare_state_property("slipping_wheels", Variant::BOOL);
        declare_state_property("wheel_flat", Variant::FLOAT);
    }

    Variant VehicleWheels::_get_state_property(const int p_local_index) const {
        const TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return Variant();
        }
        switch (p_local_index - state_base_index) {
            case STATE_ANGLE_FRONT:
                return wheel_angle_front_deg;
            case STATE_ANGLE_POWERED:
                return wheel_angle_powered_deg;
            case STATE_ANGLE_REAR:
                return wheel_angle_rear_deg;
            case STATE_ROTATION_SPEED:
                return mover->nrot;
            case STATE_ROTATION_ACCELERATION:
                return mover->nrot_eps;
            case STATE_SLIPPING:
                return mover->SlippingWheels;
            case STATE_WHEEL_FLAT:
                return mover->WheelFlat;
            default:
                return Variant();
        }
    }

    void VehicleWheels::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        p_config["powered_wheel_diameter"] = p_mover->WheelDiameter;
        p_config["front_rolling_wheel_diameter"] = p_mover->WheelDiameterL;
        p_config["rear_rolling_wheel_diameter"] = p_mover->WheelDiameterT;
        p_config["axle_inertial_moment"] = p_mover->AxleInertialMoment;
        p_config["track_width"] = p_mover->TrackW;
        p_config["axle_arrangement"] = String(p_mover->AxleArangement.c_str());
        p_config["bogie_axle_spacing"] = p_mover->ADist;
        p_config["bogie_pivot_spacing"] = p_mover->BDist;
        p_config["minimum_curve_radius"] = minimum_curve_radius;
        p_config["bearing_type"] = p_mover->BearingType == 0 ? BEARING_TYPE_SLIDE : BEARING_TYPE_ROLL;
        p_config["axles_powered_count"] = p_mover->NPoweredAxles;
        p_config["axles_count"] = p_mover->NAxles;
    }

    void VehicleWheels::_do_process_mover(TMoverParameters *p_mover, const double p_delta) {
        if (p_mover->Vel == 0.0) {
            return;
        }

        wheel_angle_front_deg += Math::rad_to_deg(p_mover->V * p_delta / p_mover->WheelDiameterL);
        wheel_angle_front_deg = Math::wrapf(static_cast<real_t>(wheel_angle_front_deg), 0.0, 360.0);
        wheel_angle_powered_deg += p_mover->nrot * p_delta * 360.0;
        wheel_angle_powered_deg = Math::wrapf(static_cast<real_t>(wheel_angle_powered_deg), 0.0, 360.0);
        wheel_angle_rear_deg += Math::rad_to_deg(p_mover->V * p_delta / p_mover->WheelDiameterT);
        wheel_angle_rear_deg = Math::wrapf(static_cast<real_t>(wheel_angle_rear_deg), 0.0, 360.0);
    }
} // namespace godot
