#include "VehicleSpeedControl.hpp"
#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void VehicleSpeedControl::_bind_methods() {
        BIND_PROPERTY(VehicleSpeedControl, Variant::BOOL, speed_control_enabled);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, delay);
        BIND_PROPERTY(VehicleSpeedControl, Variant::BOOL, impulse_lever);
        BIND_PROPERTY_W_HINT(
                VehicleSpeedControl, Variant::INT, disables_on, PROPERTY_HINT_FLAGS, "Main Controller Movement,Braking");
        BIND_PROPERTY(VehicleSpeedControl, Variant::PACKED_FLOAT64_ARRAY, preset_speeds);
        BIND_PROPERTY(VehicleSpeedControl, Variant::BOOL, override_manual_power);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, initial_power);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, full_power_velocity);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, start_velocity);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, velocity_step);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, power_step);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, min_power);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, max_power);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, min_velocity);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, max_velocity);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, offset);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, proportional_gain_positive);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, proportional_gain_negative);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, integral_gain_positive);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, integral_gain_negative);
        BIND_PROPERTY(VehicleSpeedControl, Variant::BOOL, brake_intervention);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, brake_intervention_max_velocity);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, power_up_speed);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, power_down_speed);

        ClassDB::bind_method(D_METHOD("get_active"), &VehicleSpeedControl::get_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_active");
        ClassDB::bind_method(D_METHOD("get_desired_velocity"), &VehicleSpeedControl::get_desired_velocity);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "desired_velocity", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_desired_velocity");
        ClassDB::bind_method(D_METHOD("get_desired_power"), &VehicleSpeedControl::get_desired_power);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "desired_power", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_desired_power");
        ClassDB::bind_method(D_METHOD("get_selected_velocity"), &VehicleSpeedControl::get_selected_velocity);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "selected_velocity", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_selected_velocity");
    }

    void VehicleSpeedControl::_do_update_internal_mover(TMoverParameters *p_mover) {
        ASSERT_MOVER(p_mover);
        VehicleComponent::_do_update_internal_mover(p_mover);

        p_mover->SpeedCtrl = speed_control_enabled;
        p_mover->SpeedCtrlDelay = delay;
        p_mover->SpeedCtrlTypeTime = impulse_lever;
        p_mover->SpeedCtrlAutoTurnOffFlag = disables_on;

        constexpr int MAX_PRESET_SPEEDS = 10;
        const int preset_speeds_size = static_cast<int>(preset_speeds.size());
        if (preset_speeds_size > MAX_PRESET_SPEEDS) {
            UtilityFunctions::push_warning(
                    "[VehicleSpeedControl]: preset_speeds has " + String::num_int64(preset_speeds_size) +
                    " entries, exceeding the mover's limit of " + String::num_int64(MAX_PRESET_SPEEDS) +
                    "; truncating.");
        }
        for (int i = 0; i < std::min(MAX_PRESET_SPEEDS, preset_speeds_size); i++) {
            p_mover->SpeedCtrlButtons[i] = preset_speeds[i];
        }

        p_mover->SpeedCtrlUnit.ManualStateOverride = override_manual_power;
        p_mover->SpeedCtrlUnit.InitialPower = initial_power;
        p_mover->SpeedCtrlUnit.FullPowerVelocity = full_power_velocity;
        p_mover->SpeedCtrlUnit.StartVelocity = start_velocity;
        p_mover->SpeedCtrlUnit.VelocityStep = velocity_step;
        p_mover->SpeedCtrlUnit.PowerStep = power_step;
        p_mover->SpeedCtrlUnit.MinPower = min_power;
        p_mover->SpeedCtrlUnit.MaxPower = max_power;
        p_mover->SpeedCtrlUnit.MinVelocity = min_velocity;
        p_mover->SpeedCtrlUnit.MaxVelocity = max_velocity;
        p_mover->SpeedCtrlUnit.Offset = offset;
        p_mover->SpeedCtrlUnit.FactorPpos = proportional_gain_positive;
        p_mover->SpeedCtrlUnit.FactorPneg = proportional_gain_negative;
        p_mover->SpeedCtrlUnit.FactorIpos = integral_gain_positive;
        p_mover->SpeedCtrlUnit.FactorIneg = integral_gain_negative;
        p_mover->SpeedCtrlUnit.BrakeIntervention = brake_intervention;
        p_mover->SpeedCtrlUnit.BrakeInterventionVel = brake_intervention_max_velocity;
        p_mover->SpeedCtrlUnit.PowerUpSpeed = power_up_speed;
        p_mover->SpeedCtrlUnit.PowerDownSpeed = power_down_speed;
    }


    bool VehicleSpeedControl::get_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SpeedCtrlUnit.IsActive : false;
    }

    double VehicleSpeedControl::get_desired_velocity() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SpeedCtrlUnit.DesiredVelocity : 0.0;
    }

    double VehicleSpeedControl::get_desired_power() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SpeedCtrlUnit.DesiredPower : 0.0;
    }

    double VehicleSpeedControl::get_selected_velocity() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SpeedCtrlValue : 0.0;
    }

    void VehicleSpeedControl::_fill_state_dictionary(Dictionary &p_state) const {
        // a component without a backend publishes nothing at all, rather than zeroes
        if (get_mover() == nullptr) {
            return;
        }
        p_state["speed_control/active"] = get_active();
        p_state["speed_control/desired_velocity"] = get_desired_velocity();
        p_state["speed_control/desired_power"] = get_desired_power();
        p_state["speed_control/selected_velocity"] = get_selected_velocity();
    }
} // namespace godot
