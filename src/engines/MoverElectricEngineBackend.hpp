#pragma once
#include "VehicleElectricEngineBackend.hpp"
#include "VehicleElectricEngine.hpp"

namespace godot {
    class VehicleElectricEngine;
    /* VehicleElectricEngine on the vendored Mover. */
    class MoverElectricEngineBackend : public VehicleElectricEngineBackend {
        public:
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
            bool get_collector_overvoltage_relay(const VehicleElectricEngine *p_engine) const override;
            double get_collector_required_main_switch_voltage(const VehicleElectricEngine *p_engine) const override;
            bool get_collector_valve_active(const VehicleElectricEngine *p_engine) const override;
            bool get_collector_pantographs_dropped(const VehicleElectricEngine *p_engine) const override;
            bool get_collector_pantograph_first_active(const VehicleElectricEngine *p_engine) const override;
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
    };
} // namespace godot
