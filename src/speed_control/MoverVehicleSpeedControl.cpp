#include "MoverVehicleSpeedControl.hpp"
#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void MoverVehicleSpeedControl::_bind_methods() {}


    void MoverVehicleSpeedControl::_do_update_internal_mover(TMoverParameters *p_mover) {
        ASSERT_MOVER(p_mover);
        VehicleComponent::_do_update_internal_mover(p_mover);

        p_mover->SpeedCtrl = get_speed_control_enabled();
        p_mover->SpeedCtrlDelay = get_delay();
        p_mover->SpeedCtrlTypeTime = get_impulse_lever();
        p_mover->SpeedCtrlAutoTurnOffFlag = get_disables_on();

        constexpr int MAX_PRESET_SPEEDS = 10;
        const int preset_speeds_size = static_cast<int>(get_preset_speeds().size());
        if (preset_speeds_size > MAX_PRESET_SPEEDS) {
            UtilityFunctions::push_warning(
                    "[VehicleSpeedControl]: get_preset_speeds() has " + String::num_int64(preset_speeds_size) +
                    " entries, exceeding the mover's limit of " + String::num_int64(MAX_PRESET_SPEEDS) +
                    "; truncating.");
        }
        for (int i = 0; i < std::min(MAX_PRESET_SPEEDS, preset_speeds_size); i++) {
            p_mover->SpeedCtrlButtons[i] = get_preset_speeds()[i];
        }

        p_mover->SpeedCtrlUnit.ManualStateOverride = get_override_manual_power();
        p_mover->SpeedCtrlUnit.InitialPower = get_initial_power();
        p_mover->SpeedCtrlUnit.FullPowerVelocity = get_full_power_velocity();
        p_mover->SpeedCtrlUnit.StartVelocity = get_start_velocity();
        p_mover->SpeedCtrlUnit.VelocityStep = get_velocity_step();
        p_mover->SpeedCtrlUnit.PowerStep = get_power_step();
        p_mover->SpeedCtrlUnit.MinPower = get_min_power();
        p_mover->SpeedCtrlUnit.MaxPower = get_max_power();
        p_mover->SpeedCtrlUnit.MinVelocity = get_min_velocity();
        p_mover->SpeedCtrlUnit.MaxVelocity = get_max_velocity();
        p_mover->SpeedCtrlUnit.Offset = get_offset();
        p_mover->SpeedCtrlUnit.FactorPpos = get_proportional_gain_positive();
        p_mover->SpeedCtrlUnit.FactorPneg = get_proportional_gain_negative();
        p_mover->SpeedCtrlUnit.FactorIpos = get_integral_gain_positive();
        p_mover->SpeedCtrlUnit.FactorIneg = get_integral_gain_negative();
        p_mover->SpeedCtrlUnit.BrakeIntervention = get_brake_intervention();
        p_mover->SpeedCtrlUnit.BrakeInterventionVel = get_brake_intervention_max_velocity();
        p_mover->SpeedCtrlUnit.PowerUpSpeed = get_power_up_speed();
        p_mover->SpeedCtrlUnit.PowerDownSpeed = get_power_down_speed();
    }


    bool MoverVehicleSpeedControl::get_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SpeedCtrlUnit.IsActive : false;
    }

    double MoverVehicleSpeedControl::get_desired_velocity() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SpeedCtrlUnit.DesiredVelocity : 0.0;
    }

    double MoverVehicleSpeedControl::get_desired_power() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SpeedCtrlUnit.DesiredPower : 0.0;
    }

    double MoverVehicleSpeedControl::get_selected_velocity() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SpeedCtrlValue : 0.0;
    }

    void MoverVehicleSpeedControl::_fill_state_dictionary(Dictionary &p_state) const {
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
