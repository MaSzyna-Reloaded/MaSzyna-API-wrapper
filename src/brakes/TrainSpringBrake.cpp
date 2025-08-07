#include "TrainSpringBrake.hpp"

namespace godot {
    void TrainSpringBrake::_bind_methods() {
        BIND_PROPERTY(
                Variant::FLOAT, "spring_actuator_chamber_volume", "spring/actuator/chamber_volume",
                &TrainSpringBrake::set_spring_actuator_chamber_volume,
                &TrainSpringBrake::get_spring_actuator_chamber_volume, "volume")
        BIND_PROPERTY(
                Variant::FLOAT, "max_spring_actuator_filling_force", "spring/actuator/max_filling_force",
                &TrainSpringBrake::set_max_spring_actuator_filling_force,
                &TrainSpringBrake::get_max_spring_actuator_filling_force, "value")
        BIND_PROPERTY(
                Variant::FLOAT, "pressure_force_coefficient", "pressure_force_coefficient",
                &TrainSpringBrake::set_pressure_force_coefficient, &TrainSpringBrake::get_pressure_force_coefficient,
                "value")
        BIND_PROPERTY(
                Variant::FLOAT, "spring_actuator_preload_pressure", "spring/actuator/preload_pressure",
                &TrainSpringBrake::set_spring_actuator_preload_pressure,
                &TrainSpringBrake::get_spring_actuator_preload_pressure, "value")
        BIND_PROPERTY(
                Variant::FLOAT, "spring_full_balance_pressure", "spring/full_balance_pressure",
                &TrainSpringBrake::set_spring_full_balance_pressure,
                &TrainSpringBrake::get_spring_full_balance_pressure, "value")
        BIND_PROPERTY(
                Variant::FLOAT, "brake_signal_released_state_pressure", "brake_signal/released_state_pressure",
                &TrainSpringBrake::set_brake_signal_released_state_pressure,
                &TrainSpringBrake::get_brake_signal_released_state_pressure, "value")
        BIND_PROPERTY(
                Variant::FLOAT, "brake_signal_braked_state_pressure", "brake_signal/braked_state_pressure",
                &TrainSpringBrake::set_brake_signal_braked_state_pressure,
                &TrainSpringBrake::get_brake_signal_braked_state_pressure, "value")
        BIND_PROPERTY(
                Variant::FLOAT, "actuator_discharge_valve_cross_section", "valve_cross_section/actuator_discharge",
                &TrainSpringBrake::set_actuator_discharge_valve_cross_section,
                &TrainSpringBrake::get_actuator_discharge_valve_cross_section, "value")
        BIND_PROPERTY(
                Variant::FLOAT, "actuator_charge_valve_cross_section", "valve_cross_section/actuator_charge",
                &TrainSpringBrake::set_actuator_charge_valve_cross_section,
                &TrainSpringBrake::get_actuator_charge_valve_cross_section, "value")
        BIND_PROPERTY(
                Variant::FLOAT, "pneumatic_brake_valve_cross_section", "valve_cross_section/pneumatic_brake",
                &TrainSpringBrake::set_pneumatic_brake_valve_cross_section,
                &TrainSpringBrake::get_pneumatic_brake_valve_cross_section, "value")
        BIND_PROPERTY(
                Variant::INT, "required_coupler_connection_method", "required_coupler_connection_method",
                &TrainSpringBrake::set_required_coupler_connection_method,
                &TrainSpringBrake::get_required_coupler_connection_method, "value")

        ClassDB::bind_method(D_METHOD("set_spring_brake_active", "active"), &TrainSpringBrake::set_spring_brake_active);
        ClassDB::bind_method(D_METHOD("set_spring_brake_enabled", "enabled"), &TrainSpringBrake::set_spring_brake_enabled);
        ClassDB::bind_method(D_METHOD("spring_brake_release"), &TrainSpringBrake::spring_brake_release);
    }

    void TrainSpringBrake::set_spring_brake_active(const bool p_active) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->SpringBrakeActivate(p_active);
    }

    void TrainSpringBrake::set_spring_brake_enabled(const bool p_active) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->SpringBrakeShutOff(p_active);
    }

    void TrainSpringBrake::spring_brake_release() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->SpringBrakeRelease();
    }

    void TrainSpringBrake::_do_update_internal_mover(TMoverParameters *mover) {
        if (!mover->SpringBrake.Cylinder) {
            mover->SpringBrake.Cylinder = std::make_shared<TReservoir>();
        }
        mover->SpringBrake.Cylinder->CreateCap(spring_actuator_chamber_volume);
        mover->SpringBrake.MaxBrakeForce = pressure_force_coefficient; // It is equal to brake force
        mover->SpringBrake.MaxSetPressure = spring_actuator_preload_pressure;
        mover->SpringBrake.ResetPressure = max_spring_actuator_filling_force;
        mover->SpringBrake.MinForcePressure = spring_full_balance_pressure;
        mover->SpringBrake.PressureOff = brake_signal_released_state_pressure;
        mover->SpringBrake.PressureOn = brake_signal_braked_state_pressure;
        mover->SpringBrake.ValveOffArea = actuator_charge_valve_cross_section;
        mover->SpringBrake.ValveOnArea = actuator_discharge_valve_cross_section;
        mover->SpringBrake.ValvePNBrakeArea = pneumatic_brake_valve_cross_section;
        mover->SpringBrake.PNBrakeConnection = mover->SpringBrake.ValvePNBrakeArea > 0;
        mover->SpringBrake.MultiTractionCoupler = required_coupler_connection_method;

        //@TODO: There might be a need to update Spring Brake in the mover internally but it seems to be working as for now
        TrainPart::_do_update_internal_mover(mover);
    }

    void TrainSpringBrake::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        state["spring_brake/is_ready"] = mover->SpringBrake.IsReady;
        state["spring_brake/shut_off"] = mover->SpringBrake.ShuttOff;
        state["spring_brake/active"] = mover->SpringBrake.Activate;
        state["spring_brake/cylinder_pressure"] = mover->SpringBrake.SBP;
    }

    void TrainSpringBrake::_register_commands() {
        register_command("set_spring_brake_active", Callable(this, "set_spring_brake_active"));
        register_command("set_spring_brake_enabled", Callable(this, "set_spring_brake_enabled"));
        register_command("spring_brake_release", Callable(this, "spring_brake_release"));
    }

    void TrainSpringBrake::_unregister_commands() {
        unregister_command("set_spring_brake_active", Callable(this, "set_spring_brake_active"));
        unregister_command("set_spring_brake_enabled", Callable(this, "set_spring_brake_enabled"));
        unregister_command("spring_brake_release", Callable(this, "spring_brake_release"));
    }
} // namespace godot
