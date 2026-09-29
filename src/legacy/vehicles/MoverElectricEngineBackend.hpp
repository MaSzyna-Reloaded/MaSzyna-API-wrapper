#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleElectricEngine.hpp"
#include "vehicles/rail/RailVehicleElectricEngineBackend.hpp"

namespace godot {
    class RailVehicleElectricEngine;
    /* RailVehicleElectricEngine on the vendored Mover. */
    class MoverElectricEngineBackend : public RailVehicleElectricEngineBackend {
        private:
            /* The Mover* component that installs this delegate - it reaches the Mover through it. */
            const MoverComponent &owner;

        public:
            explicit MoverElectricEngineBackend(const MoverComponent &p_owner) : owner(p_owner) {}

            double get_collector_max_voltage(const RailVehicleElectricEngine *p_engine) const override;
            double get_collector_max_current(const RailVehicleElectricEngine *p_engine) const override;
            double get_collector_max_lifting(const RailVehicleElectricEngine *p_engine) const override;
            double get_collector_min_lifting(const RailVehicleElectricEngine *p_engine) const override;
            double get_collector_sliding_width(const RailVehicleElectricEngine *p_engine) const override;
            double get_collector_min_main_switch_voltage(const RailVehicleElectricEngine *p_engine) const override;
            double get_collector_min_pantograph_tank_pressure(const RailVehicleElectricEngine *p_engine) const override;
            double get_collector_max_pantograph_tank_pressure(const RailVehicleElectricEngine *p_engine) const override;
            double get_collector_pantograph_tank_pressure(const RailVehicleElectricEngine *p_engine) const override;
            bool
            get_collector_pantograph_pressure_switch_armed(const RailVehicleElectricEngine *p_engine) const override;
            bool get_collector_pantograph_compressor_valve(const RailVehicleElectricEngine *p_engine) const override;
            bool get_collector_pantograph_compressor_enabled(const RailVehicleElectricEngine *p_engine) const override;
            bool get_collector_overvoltage_relay(const RailVehicleElectricEngine *p_engine) const override;
            double get_collector_required_main_switch_voltage(const RailVehicleElectricEngine *p_engine) const override;
            bool get_collector_valve_active(const RailVehicleElectricEngine *p_engine) const override;
            bool get_collector_valve_enabled(const RailVehicleElectricEngine *p_engine) const override;
            bool get_collector_pantographs_dropped(const RailVehicleElectricEngine *p_engine) const override;
            bool get_collector_pantograph_first_active(const RailVehicleElectricEngine *p_engine) const override;
            bool get_collector_pantograph_valve_enabled(
                    const RailVehicleElectricEngine *p_engine,
                    RailVehicleElectricEngine::PantographSelector p_selector) const override;
            void pantograph_valve_operate(
                    const RailVehicleElectricEngine *p_engine, RailVehicleElectricEngine::PantographSelector p_selector,
                    RailVehicleElectricEngine::ValveOperation p_operation) const override;
            double get_collector_pantograph_first_voltage(const RailVehicleElectricEngine *p_engine) const override;
            bool get_collector_pantograph_second_active(const RailVehicleElectricEngine *p_engine) const override;
            double get_collector_pantograph_second_voltage(const RailVehicleElectricEngine *p_engine) const override;
            double get_collector_voltage(const RailVehicleElectricEngine *p_engine) const override;
            double get_energy_drawn(const RailVehicleElectricEngine *p_engine) const override;
            double get_energy_returned(const RailVehicleElectricEngine *p_engine) const override;
            void meter_energy(const RailVehicleElectricEngine *p_engine, double p_delta) const override;
            bool get_contactors_active(const RailVehicleElectricEngine *p_engine) const override;
            bool get_diff_relay_active(const RailVehicleElectricEngine *p_engine) const override;
            bool get_resistors_active(const RailVehicleElectricEngine *p_engine) const override;
            bool get_vent_overload_active(const RailVehicleElectricEngine *p_engine) const override;
            bool get_highcurrent_active(const RailVehicleElectricEngine *p_engine) const override;
            bool get_mainbreaker_active(const RailVehicleElectricEngine *p_engine) const override;
            double get_transducer_input_voltage(const RailVehicleElectricEngine *p_engine) const override;
            bool get_camshaft_available(const RailVehicleElectricEngine *p_engine) const override;
            bool get_converter_overload(const RailVehicleElectricEngine *p_engine) const override;
            double get_line_breaker_delay(const RailVehicleElectricEngine *p_engine) const override;
            double get_line_breaker_initial_delay(const RailVehicleElectricEngine *p_engine) const override;
            bool get_line_breaker_closes_at_no_power(const RailVehicleElectricEngine *p_engine) const override;
            void apply_configuration(const RailVehicleElectricEngine *p_engine) const override;
            void converter_fuse_reset(const RailVehicleElectricEngine *p_engine) const override;
            void pantographs_valve(const RailVehicleElectricEngine *p_engine, bool p_enabled) const override;
            void pantographs_valve_operate(
                    const RailVehicleElectricEngine *p_engine,
                    RailVehicleElectricEngine::ValveOperation p_operation) const override;
            void pantographs_drop_all(const RailVehicleElectricEngine *p_engine, bool p_enabled) const override;
            void pantograph_compressor(const RailVehicleElectricEngine *p_engine, bool p_enabled) const override;
            void
            pantograph_compressor_valve(const RailVehicleElectricEngine *p_engine, bool p_to_compressor) const override;
            void pantograph(
                    const RailVehicleElectricEngine *p_engine, RailVehicleElectricEngine::PantographSelector p_selector,
                    bool p_enabled) const override;
            void set_pantograph_wire_voltage(
                    const RailVehicleElectricEngine *p_engine, RailVehicleElectricEngine::PantographSelector p_selector,
                    float p_voltage) const override;

            /// Train.cpp:3695 (df5a8a8) - the pantograph compressor starts only below this pressure
            static constexpr double PANTOGRAPH_COMPRESSOR_START_PRESSURE = 4.8;
    };
} // namespace godot
