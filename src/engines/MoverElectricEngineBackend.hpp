#pragma once
#include "VehicleElectricEngineBackend.hpp"
#include "VehicleElectricEngine.hpp"

namespace godot {
    /* VehicleElectricEngine on the vendored Mover. */
    class MoverElectricEngineBackend : public VehicleElectricEngineBackend {
        public:
            bool get_converter_enabled(const TMoverParameters *p_mover) const override;
            bool get_converted_allowed(const TMoverParameters *p_mover) const override;
            double get_converter_time_to_start(const TMoverParameters *p_mover) const override;
            double get_collector_max_voltage(const TMoverParameters *p_mover) const override;
            double get_collector_max_current(const TMoverParameters *p_mover) const override;
            double get_collector_max_lifting(const TMoverParameters *p_mover) const override;
            double get_collector_min_lifting(const TMoverParameters *p_mover) const override;
            double get_collector_sliding_width(const TMoverParameters *p_mover) const override;
            double get_collector_min_main_switch_voltage(const TMoverParameters *p_mover) const override;
            double get_collector_min_pantograph_tank_pressure(const TMoverParameters *p_mover) const override;
            double get_collector_max_pantograph_tank_pressure(const TMoverParameters *p_mover) const override;
            double get_collector_pantograph_tank_pressure(const TMoverParameters *p_mover) const override;
            bool get_collector_pantograph_pressure_switch_armed(const TMoverParameters *p_mover) const override;
            bool get_collector_pantograph_compressor_valve(const TMoverParameters *p_mover) const override;
            bool get_collector_overvoltage_relay(const TMoverParameters *p_mover) const override;
            double get_collector_required_main_switch_voltage(const TMoverParameters *p_mover) const override;
            bool get_collector_valve_active(const TMoverParameters *p_mover) const override;
            bool get_collector_pantographs_dropped(const TMoverParameters *p_mover) const override;
            bool get_collector_pantograph_first_active(const TMoverParameters *p_mover) const override;
            double get_collector_pantograph_first_voltage(const TMoverParameters *p_mover) const override;
            bool get_collector_pantograph_second_active(const TMoverParameters *p_mover) const override;
            double get_collector_pantograph_second_voltage(const TMoverParameters *p_mover) const override;
            double get_collector_voltage(const TMoverParameters *p_mover) const override;
            bool get_contactors_active(const TMoverParameters *p_mover) const override;
            bool get_diff_relay_active(const TMoverParameters *p_mover) const override;
            bool get_resistors_active(const TMoverParameters *p_mover) const override;
            bool get_vent_overload_active(const TMoverParameters *p_mover) const override;
            bool get_highcurrent_active(const TMoverParameters *p_mover) const override;
            bool get_mainbreaker_active(const TMoverParameters *p_mover) const override;
            double get_transducer_input_voltage(const TMoverParameters *p_mover) const override;
            bool get_camshaft_available(const TMoverParameters *p_mover) const override;
            bool get_converter_overload(const TMoverParameters *p_mover) const override;
            double get_line_breaker_delay(const TMoverParameters *p_mover) const override;
            double get_line_breaker_initial_delay(const TMoverParameters *p_mover) const override;
            bool get_line_breaker_closes_at_no_power(const TMoverParameters *p_mover) const override;
            void update_mover(const VehicleElectricEngine *p_engine, TMoverParameters *p_mover) const override;
    };
} // namespace godot
