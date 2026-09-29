#pragma once
#include "RailVehicleElectricEngine.hpp"

namespace godot {
    /* The power an electric engine is fed with (FIZ "Power:", EnginePowerSource): the current
     * collector - pantographs, their valves, tank and compressor (TCurrentCollector) - the
     * transducer and the energy meter.
     *
     * A unit the catenary-fed engines are composed of, with an implementation per simulation.
     * Not a Godot class - Godot sees only the engine. */
    class RailVehicleCurrentCollectorUnit {
        public:
            virtual ~RailVehicleCurrentCollectorUnit() = default;

            virtual double get_max_voltage() const = 0;
            virtual double get_max_current() const = 0;
            virtual double get_max_lifting() const = 0;
            virtual double get_min_lifting() const = 0;
            virtual double get_sliding_width() const = 0;
            virtual double get_min_main_switch_voltage() const = 0;
            virtual double get_min_pantograph_tank_pressure() const = 0;
            virtual double get_max_pantograph_tank_pressure() const = 0;
            virtual double get_pantograph_tank_pressure() const = 0;
            virtual bool get_pantograph_pressure_switch_armed() const = 0;
            virtual bool get_pantograph_compressor_valve() const = 0;
            virtual bool get_pantograph_compressor_enabled() const = 0;
            virtual bool get_overvoltage_relay() const = 0;
            virtual double get_required_main_switch_voltage() const = 0;
            virtual bool get_valve_active() const = 0;
            virtual bool get_valve_enabled() const = 0;
            virtual bool get_pantographs_dropped() const = 0;
            virtual bool get_pantograph_first_active() const = 0;
            virtual bool
            get_pantograph_valve_enabled(RailVehicleElectricEngine::PantographSelector p_selector) const = 0;
            virtual void pantograph_valve_operate(
                    RailVehicleElectricEngine::PantographSelector p_selector,
                    RailVehicleElectricEngine::ValveOperation p_operation) const = 0;
            virtual double get_pantograph_first_voltage() const = 0;
            virtual bool get_pantograph_second_active() const = 0;
            virtual double get_pantograph_second_voltage() const = 0;
            virtual double get_voltage() const = 0;
            /* Energy drawn from the wire and returned to it [kWh], the returned one negative */
            virtual double get_energy_drawn() const = 0;
            virtual double get_energy_returned() const = 0;
            /* The energy of p_delta seconds added to the meter */
            virtual void meter_energy(double p_delta) const = 0;
            virtual double get_transducer_input_voltage() const = 0;
            virtual void apply_configuration(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual void pantographs_valve(bool p_enabled) const = 0;
            virtual void pantographs_valve_operate(RailVehicleElectricEngine::ValveOperation p_operation) const = 0;
            virtual void pantographs_drop_all(bool p_enabled) const = 0;
            virtual void pantograph_compressor(bool p_enabled) const = 0;
            virtual void pantograph_compressor_valve(bool p_to_compressor) const = 0;
            virtual void pantograph(RailVehicleElectricEngine::PantographSelector p_selector, bool p_enabled) const = 0;
            virtual void set_pantograph_wire_voltage(
                    RailVehicleElectricEngine::PantographSelector p_selector, float p_voltage) const = 0;
    };
} // namespace godot
