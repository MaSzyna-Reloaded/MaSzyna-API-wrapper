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
    }
    void TrainSpringBrake::activate_spring_brake() {
        TMoverParameters *mover = get_mover();
        mover->SpringBrakeActivate(true);
    }

    void TrainSpringBrake::deactivate_spring_brake() {
        TMoverParameters *mover = get_mover();
        mover->SpringBrakeActivate(false);
    }

    void TrainSpringBrake::spring_brake_on() {
        TMoverParameters *mover = get_mover();
        mover->SpringBrakeShutOff(false);
    }

    void TrainSpringBrake::spring_brake_off() {
        TMoverParameters *mover = get_mover();
        mover->SpringBrakeShutOff(true);
    }

    void TrainSpringBrake::spring_brake_release() {
        TMoverParameters *mover = get_mover();
        mover->SpringBrakeRelease();
    }

    void TrainSpringBrake::_do_update_internal_mover(TMoverParameters *mover) {
        if (!mover->SpringBrake.Cylinder)
            mover->SpringBrake.Cylinder = std::make_shared<TReservoir>();
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
        // mover->SpringBrake.ShuttOff = false;
        // mover->SpringBrake.Activate = false;
        // mover->SpringBrake.IsReady = true;
        TrainPart::_do_update_internal_mover(mover);
    }

    void TrainSpringBrake::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        state["spring_brake/is_ready"] = mover->SpringBrake.IsReady;
        state["spring_brake/shut_off"] = mover->SpringBrake.ShuttOff;
        state["spring_brake/active"] = mover->SpringBrake.Activate;
    }

    void TrainSpringBrake::_register_commands() {
        register_command("activate_spring_brake", Callable(this, "activate_spring_brake"));
        register_command("deactivate_spring_brake", Callable(this, "deactivate_spring_brake"));
        register_command("spring_brake_on", Callable(this, "spring_brake_on"));
        register_command("spring_brake_off", Callable(this, "spring_brake_off"));
        register_command("spring_brake_release", Callable(this, "spring_brake_release"));
        TrainPart::_register_commands();
    }

    void TrainSpringBrake::_unregister_commands() {
        unregister_command("activate_spring_brake", Callable(this, "activate_spring_brake"));
        unregister_command("deactivate_spring_brake", Callable(this, "deactivate_spring_brake"));
        unregister_command("spring_brake_on", Callable(this, "spring_brake_on"));
        unregister_command("spring_brake_off", Callable(this, "spring_brake_off"));
        unregister_command("spring_brake_release", Callable(this, "spring_brake_release"));
        TrainPart::_unregister_commands();
    }
} // namespace godot
