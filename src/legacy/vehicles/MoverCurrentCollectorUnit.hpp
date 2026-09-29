#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleCurrentCollectorUnit.hpp"

namespace godot {
    /* The power source and current collector on the vendored Mover. */
    class MoverCurrentCollectorUnit : public RailVehicleCurrentCollectorUnit {
        private:
            /* The Mover* component that owns this unit - it reaches the Mover through it. */
            const MoverComponent &owner;

        public:
            explicit MoverCurrentCollectorUnit(const MoverComponent &p_owner) : owner(p_owner) {}

            double get_max_voltage() const override;
            double get_max_current() const override;
            double get_max_lifting() const override;
            double get_min_lifting() const override;
            double get_sliding_width() const override;
            double get_min_main_switch_voltage() const override;
            double get_min_pantograph_tank_pressure() const override;
            double get_max_pantograph_tank_pressure() const override;
            double get_pantograph_tank_pressure() const override;
            bool get_pantograph_pressure_switch_armed() const override;
            bool get_pantograph_compressor_valve() const override;
            bool get_pantograph_compressor_enabled() const override;
            bool get_overvoltage_relay() const override;
            double get_required_main_switch_voltage() const override;
            bool get_valve_active() const override;
            bool get_valve_enabled() const override;
            bool get_pantographs_dropped() const override;
            bool get_pantograph_first_active() const override;
            bool get_pantograph_valve_enabled(RailVehicleElectricEngine::PantographSelector p_selector) const override;
            void pantograph_valve_operate(
                    RailVehicleElectricEngine::PantographSelector p_selector,
                    RailVehicleElectricEngine::ValveOperation p_operation) const override;
            double get_pantograph_first_voltage() const override;
            bool get_pantograph_second_active() const override;
            double get_pantograph_second_voltage() const override;
            double get_voltage() const override;
            double get_energy_drawn() const override;
            double get_energy_returned() const override;
            void meter_energy(double p_delta) const override;
            double get_transducer_input_voltage() const override;
            void apply_configuration(const RailVehicleElectricEngine *p_engine) const override;
            void pantographs_valve(bool p_enabled) const override;
            void pantographs_valve_operate(RailVehicleElectricEngine::ValveOperation p_operation) const override;
            void pantographs_drop_all(bool p_enabled) const override;
            void pantograph_compressor(bool p_enabled) const override;
            void pantograph_compressor_valve(bool p_to_compressor) const override;
            void pantograph(RailVehicleElectricEngine::PantographSelector p_selector, bool p_enabled) const override;
            void set_pantograph_wire_voltage(
                    RailVehicleElectricEngine::PantographSelector p_selector, float p_voltage) const override;
            /// Train.cpp:3695 (df5a8a8) - the pantograph compressor starts only below this pressure
            static constexpr double PANTOGRAPH_COMPRESSOR_START_PRESSURE = 4.8;
    };
} // namespace godot
