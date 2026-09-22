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

        ClassDB::bind_method(D_METHOD("get_angle_front_deg"), &VehicleWheels::get_angle_front_deg);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "angle_front_deg", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_angle_front_deg");
        ClassDB::bind_method(D_METHOD("get_angle_powered_deg"), &VehicleWheels::get_angle_powered_deg);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "angle_powered_deg", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_angle_powered_deg");
        ClassDB::bind_method(D_METHOD("get_angle_rear_deg"), &VehicleWheels::get_angle_rear_deg);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "angle_rear_deg", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_angle_rear_deg");
        ClassDB::bind_method(D_METHOD("get_rotation_speed_rps"), &VehicleWheels::get_rotation_speed_rps);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "rotation_speed_rps", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rotation_speed_rps");
        ClassDB::bind_method(D_METHOD("get_rotation_acceleration_rps2"), &VehicleWheels::get_rotation_acceleration_rps2);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "rotation_acceleration_rps2", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rotation_acceleration_rps2");
        ClassDB::bind_method(D_METHOD("get_slipping"), &VehicleWheels::get_slipping);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "slipping", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_slipping");
        ClassDB::bind_method(D_METHOD("get_flat"), &VehicleWheels::get_flat);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "flat", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_flat");
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


    double VehicleWheels::get_angle_front_deg() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? wheel_angle_front_deg : 0.0;
    }

    double VehicleWheels::get_angle_powered_deg() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? wheel_angle_powered_deg : 0.0;
    }

    double VehicleWheels::get_angle_rear_deg() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? wheel_angle_rear_deg : 0.0;
    }

    double VehicleWheels::get_rotation_speed_rps() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->nrot : 0.0;
    }

    double VehicleWheels::get_rotation_acceleration_rps2() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->nrot_eps : 0.0;
    }

    bool VehicleWheels::get_slipping() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SlippingWheels : false;
    }

    double VehicleWheels::get_flat() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->WheelFlat : 0.0;
    }

    void VehicleWheels::_fill_state_dictionary(Dictionary &p_state) const {
        // a component without a backend publishes nothing at all, rather than zeroes
        if (get_mover() == nullptr) {
            return;
        }
        p_state["wheel_angle_front_deg"] = get_angle_front_deg();
        p_state["wheel_angle_powered_deg"] = get_angle_powered_deg();
        p_state["wheel_angle_rear_deg"] = get_angle_rear_deg();
        p_state["wheel_rotation_speed_rps"] = get_rotation_speed_rps();
        p_state["wheel_rotation_acceleration_rps2"] = get_rotation_acceleration_rps2();
        p_state["slipping_wheels"] = get_slipping();
        p_state["wheel_flat"] = get_flat();
    }

    void VehicleWheels::_fill_config_dictionary(Dictionary &p_config) const {
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        p_config["powered_wheel_diameter"] = mover->WheelDiameter;
        p_config["front_rolling_wheel_diameter"] = mover->WheelDiameterL;
        p_config["rear_rolling_wheel_diameter"] = mover->WheelDiameterT;
        p_config["axle_inertial_moment"] = mover->AxleInertialMoment;
        p_config["track_width"] = mover->TrackW;
        p_config["axle_arrangement"] = String(mover->AxleArangement.c_str());
        p_config["bogie_axle_spacing"] = mover->ADist;
        p_config["bogie_pivot_spacing"] = mover->BDist;
        p_config["minimum_curve_radius"] = minimum_curve_radius;
        p_config["bearing_type"] = mover->BearingType == 0 ? BEARING_TYPE_SLIDE : BEARING_TYPE_ROLL;
        p_config["axles_powered_count"] = mover->NPoweredAxles;
        p_config["axles_count"] = mover->NAxles;
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
