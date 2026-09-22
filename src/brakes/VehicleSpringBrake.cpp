#include "VehicleSpringBrake.hpp"

namespace godot {
    void VehicleSpringBrake::_bind_methods() {
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, spring_actuator_chamber_volume, "spring/actuator")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, spring_actuator_max_filling_force, "spring/actuator")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, pressure_force_coefficient)
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, spring_actuator_preload_pressure, "spring/actuator")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, spring_full_balance_pressure, "spring")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, brake_signal_released_state_pressure, "brake_signal")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, brake_signal_braked_state_pressure, "brake_signal")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, valve_cross_section_actuator_discharge, "valve_cross_section")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, valve_cross_section_actuator_charge, "valve_cross_section")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, valve_cross_section_pneumatic_brake, "valve_cross_section")
        BIND_PROPERTY(VehicleSpringBrake, Variant::INT, required_coupler_connection_method)

        ClassDB::bind_method(D_METHOD("set_spring_brake_active", "active"), &VehicleSpringBrake::set_spring_brake_active);
        ClassDB::bind_method(
                D_METHOD("set_spring_brake_enabled", "enabled"), &VehicleSpringBrake::set_spring_brake_enabled);
        ClassDB::bind_method(D_METHOD("spring_brake_release"), &VehicleSpringBrake::spring_brake_release);
    }

    void VehicleSpringBrake::set_spring_brake_active(const bool p_active) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->SpringBrakeActivate(p_active);
    }

    void VehicleSpringBrake::set_spring_brake_enabled(const bool p_active) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->SpringBrakeShutOff(p_active);
    }

    void VehicleSpringBrake::spring_brake_release() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->SpringBrakeRelease();
    }

    void VehicleSpringBrake::_do_update_internal_mover(TMoverParameters *p_mover) {
        if (!p_mover->SpringBrake.Cylinder) {
            p_mover->SpringBrake.Cylinder = std::make_shared<TReservoir>();
        }
        p_mover->SpringBrake.Cylinder->CreateCap(spring_actuator_chamber_volume);
        p_mover->SpringBrake.MaxBrakeForce = pressure_force_coefficient; // It is equal to brake force
        p_mover->SpringBrake.MaxSetPressure = spring_actuator_preload_pressure;
        p_mover->SpringBrake.ResetPressure = spring_actuator_max_filling_force;
        p_mover->SpringBrake.MinForcePressure = spring_full_balance_pressure;
        p_mover->SpringBrake.PressureOff = brake_signal_released_state_pressure;
        p_mover->SpringBrake.PressureOn = brake_signal_braked_state_pressure;
        p_mover->SpringBrake.ValveOffArea = valve_cross_section_actuator_charge;
        p_mover->SpringBrake.ValveOnArea = valve_cross_section_actuator_discharge;
        p_mover->SpringBrake.ValvePNBrakeArea = valve_cross_section_pneumatic_brake;
        p_mover->SpringBrake.PNBrakeConnection = p_mover->SpringBrake.ValvePNBrakeArea > 0;
        p_mover->SpringBrake.MultiTractionCoupler = required_coupler_connection_method;

        //@TODO: There might be a need to update Spring Brake in the mover internally but it seems to be working as for
        // now
        VehicleComponent::_do_update_internal_mover(p_mover);
    }

    void VehicleSpringBrake::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        p_state["spring_brake/is_ready"] = p_mover->SpringBrake.IsReady;
        p_state["spring_brake/shut_off"] = p_mover->SpringBrake.ShuttOff;
        p_state["spring_brake/active"] = p_mover->SpringBrake.Activate;
        p_state["spring_brake/cylinder_pressure"] = p_mover->SpringBrake.SBP;
    }

    void VehicleSpringBrake::_register_commands() {
        register_command("set_spring_brake_active", Callable(this, "set_spring_brake_active"));
        register_command("set_spring_brake_enabled", Callable(this, "set_spring_brake_enabled"));
        register_command("spring_brake_release", Callable(this, "spring_brake_release"));
    }

    void VehicleSpringBrake::_unregister_commands() {
        unregister_command("set_spring_brake_active", Callable(this, "set_spring_brake_active"));
        unregister_command("set_spring_brake_enabled", Callable(this, "set_spring_brake_enabled"));
        unregister_command("spring_brake_release", Callable(this, "spring_brake_release"));
    }
} // namespace godot
