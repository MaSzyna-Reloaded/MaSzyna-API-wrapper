#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "VehicleElectricEngine.hpp"
#include "VehicleElectricEngineBackend.hpp"

namespace godot {
    class VehicleElectricEngine;
    /* VehicleElectricEngine on the vendored Mover. */
    class MoverElectricEngineBackend : public VehicleElectricEngineBackend {
        private:
            /* The Mover* component that installs this delegate - it reaches the Mover through it. */
            const MoverComponent &owner;

        public:
            explicit MoverElectricEngineBackend(const MoverComponent &p_owner) : owner(p_owner) {}

            bool get_converter_enabled(const VehicleElectricEngine *p_engine) const override;
            bool get_converted_allowed(const VehicleElectricEngine *p_engine) const override;
            double get_converter_time_to_start(const VehicleElectricEngine *p_engine) const override;
            double get_collector_max_voltage(const VehicleElectricEngine *p_engine) const override;
            double get_collector_max_current(const VehicleElectricEngine *p_engine) const override;
            double get_collector_max_lifting(const VehicleElectricEngine *p_engine) const override;
            double get_collector_min_lifting(const VehicleElectricEngine *p_engine) const override;
            double get_collector_sliding_width(const VehicleElectricEngine *p_engine) const override;
            double get_collector_min_main_switch_voltage(const VehicleElectricEngine *p_engine) const override;
            double get_collector_min_pantograph_tank_pressure(const VehicleElectricEngine *p_engine) const override;
            double get_collector_max_pantograph_tank_pressure(const VehicleElectricEngine *p_engine) const override;
            double get_collector_pantograph_tank_pressure(const VehicleElectricEngine *p_engine) const override;
            bool get_collector_pantograph_pressure_switch_armed(const VehicleElectricEngine *p_engine) const override;
            bool get_collector_pantograph_compressor_valve(const VehicleElectricEngine *p_engine) const override;
            bool get_collector_pantograph_compressor_enabled(const VehicleElectricEngine *p_engine) const override;
            bool get_collector_overvoltage_relay(const VehicleElectricEngine *p_engine) const override;
            double get_collector_required_main_switch_voltage(const VehicleElectricEngine *p_engine) const override;
            bool get_collector_valve_active(const VehicleElectricEngine *p_engine) const override;
            bool get_collector_valve_enabled(const VehicleElectricEngine *p_engine) const override;
            bool get_collector_pantographs_dropped(const VehicleElectricEngine *p_engine) const override;
            bool get_collector_pantograph_first_active(const VehicleElectricEngine *p_engine) const override;
            bool get_collector_pantograph_valve_enabled(
                    const VehicleElectricEngine *p_engine,
                    VehicleElectricEngine::PantographSelector p_selector) const override;
            void pantograph_valve_operate(
                    const VehicleElectricEngine *p_engine, VehicleElectricEngine::PantographSelector p_selector,
                    VehicleElectricEngine::ValveOperation p_operation) const override;
            double get_collector_pantograph_first_voltage(const VehicleElectricEngine *p_engine) const override;
            bool get_collector_pantograph_second_active(const VehicleElectricEngine *p_engine) const override;
            double get_collector_pantograph_second_voltage(const VehicleElectricEngine *p_engine) const override;
            double get_collector_voltage(const VehicleElectricEngine *p_engine) const override;
            bool get_contactors_active(const VehicleElectricEngine *p_engine) const override;
            bool get_diff_relay_active(const VehicleElectricEngine *p_engine) const override;
            bool get_resistors_active(const VehicleElectricEngine *p_engine) const override;
            bool get_vent_overload_active(const VehicleElectricEngine *p_engine) const override;
            bool get_highcurrent_active(const VehicleElectricEngine *p_engine) const override;
            bool get_mainbreaker_active(const VehicleElectricEngine *p_engine) const override;
            double get_transducer_input_voltage(const VehicleElectricEngine *p_engine) const override;
            bool get_camshaft_available(const VehicleElectricEngine *p_engine) const override;
            bool get_converter_overload(const VehicleElectricEngine *p_engine) const override;
            double get_line_breaker_delay(const VehicleElectricEngine *p_engine) const override;
            double get_line_breaker_initial_delay(const VehicleElectricEngine *p_engine) const override;
            bool get_line_breaker_closes_at_no_power(const VehicleElectricEngine *p_engine) const override;
            void apply_configuration(const VehicleElectricEngine *p_engine) const override;
            void converter(const VehicleElectricEngine *p_engine, bool p_enabled) const override;
            void converter_fuse_reset(const VehicleElectricEngine *p_engine) const override;
            void pantographs_valve(const VehicleElectricEngine *p_engine, bool p_enabled) const override;
            void pantographs_valve_operate(
                    const VehicleElectricEngine *p_engine,
                    VehicleElectricEngine::ValveOperation p_operation) const override;
            void pantographs_drop_all(const VehicleElectricEngine *p_engine, bool p_enabled) const override;
            void pantograph_compressor(const VehicleElectricEngine *p_engine, bool p_enabled) const override;
            void
            pantograph_compressor_valve(const VehicleElectricEngine *p_engine, bool p_to_compressor) const override;
            void pantograph(
                    const VehicleElectricEngine *p_engine, VehicleElectricEngine::PantographSelector p_selector,
                    bool p_enabled) const override;
            void set_pantograph_wire_voltage(
                    const VehicleElectricEngine *p_engine, VehicleElectricEngine::PantographSelector p_selector,
                    float p_voltage) const override;

            /// Train.cpp:3695 (df5a8a8) - the pantograph compressor starts only below this pressure
            static constexpr double PANTOGRAPH_COMPRESSOR_START_PRESSURE = 4.8;
    };
} // namespace godot
