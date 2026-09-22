#include "MoverVehicleSpringBrake.hpp"
#include "VehicleSpringBrake.hpp"

namespace godot {
    void MoverVehicleSpringBrake::_bind_methods() {}


    void MoverVehicleSpringBrake::set_spring_brake_active(const bool p_active) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->SpringBrakeActivate(p_active);
    }

    void MoverVehicleSpringBrake::set_spring_brake_enabled(const bool p_active) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->SpringBrakeShutOff(p_active);
    }

    void MoverVehicleSpringBrake::spring_brake_release() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->SpringBrakeRelease();
    }

    void MoverVehicleSpringBrake::_do_update_internal_mover(TMoverParameters *p_mover) {
        if (!p_mover->SpringBrake.Cylinder) {
            p_mover->SpringBrake.Cylinder = std::make_shared<TReservoir>();
        }
        p_mover->SpringBrake.Cylinder->CreateCap(get_spring_actuator_chamber_volume());
        p_mover->SpringBrake.MaxBrakeForce = get_pressure_force_coefficient(); // It is equal to brake force
        p_mover->SpringBrake.MaxSetPressure = get_spring_actuator_preload_pressure();
        p_mover->SpringBrake.ResetPressure = get_spring_actuator_max_filling_force();
        p_mover->SpringBrake.MinForcePressure = get_spring_full_balance_pressure();
        p_mover->SpringBrake.PressureOff = get_brake_signal_released_state_pressure();
        p_mover->SpringBrake.PressureOn = get_brake_signal_braked_state_pressure();
        p_mover->SpringBrake.ValveOffArea = get_valve_cross_section_actuator_charge();
        p_mover->SpringBrake.ValveOnArea = get_valve_cross_section_actuator_discharge();
        p_mover->SpringBrake.ValvePNBrakeArea = get_valve_cross_section_pneumatic_brake();
        p_mover->SpringBrake.PNBrakeConnection = p_mover->SpringBrake.ValvePNBrakeArea > 0;
        p_mover->SpringBrake.MultiTractionCoupler = get_required_coupler_connection_method();

        //@TODO: There might be a need to update Spring Brake in the mover internally but it seems to be working as for
        // now
        VehicleComponent::_do_update_internal_mover(p_mover);
    }


    bool MoverVehicleSpringBrake::get_ready() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SpringBrake.IsReady : false;
    }

    bool MoverVehicleSpringBrake::get_shut_off() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SpringBrake.ShuttOff : false;
    }

    bool MoverVehicleSpringBrake::get_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SpringBrake.Activate : false;
    }

    double MoverVehicleSpringBrake::get_cylinder_pressure() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SpringBrake.SBP : 0.0;
    }

    void MoverVehicleSpringBrake::_fill_state_dictionary(Dictionary &p_state) const {
        // a component without a backend publishes nothing at all, rather than zeroes
        if (get_mover() == nullptr) {
            return;
        }
        p_state["spring_brake/is_ready"] = get_ready();
        p_state["spring_brake/shut_off"] = get_shut_off();
        p_state["spring_brake/active"] = get_active();
        p_state["spring_brake/cylinder_pressure"] = get_cylinder_pressure();
    }


} // namespace godot
