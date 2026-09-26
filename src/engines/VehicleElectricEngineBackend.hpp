#pragma once
#include "VehicleElectricEngine.hpp"
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    /* What VehicleElectricEngine needs a simulation to answer. Series-wound and induction locomotives both run one;
     * same reason. */
    class VehicleElectricEngineBackend {
        public:
            virtual ~VehicleElectricEngineBackend() = default;

            virtual bool get_converter_enabled(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_converted_allowed(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_converter_time_to_start(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_max_voltage(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_max_current(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_max_lifting(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_min_lifting(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_sliding_width(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_min_main_switch_voltage(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_min_pantograph_tank_pressure(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_max_pantograph_tank_pressure(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_pantograph_tank_pressure(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool
            get_collector_pantograph_pressure_switch_armed(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_pantograph_compressor_valve(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_pantograph_compressor_enabled(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_overvoltage_relay(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_required_main_switch_voltage(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_valve_active(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_valve_enabled(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_pantographs_dropped(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_pantograph_first_active(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_pantograph_valve_enabled(
                    const VehicleElectricEngine *p_engine,
                    VehicleElectricEngine::PantographSelector p_selector) const = 0;
            virtual void pantograph_valve_operate(
                    const VehicleElectricEngine *p_engine, VehicleElectricEngine::PantographSelector p_selector,
                    VehicleElectricEngine::ValveOperation p_operation) const = 0;
            virtual double get_collector_pantograph_first_voltage(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_pantograph_second_active(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_pantograph_second_voltage(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_voltage(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_contactors_active(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_diff_relay_active(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_resistors_active(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_vent_overload_active(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_highcurrent_active(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_mainbreaker_active(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_transducer_input_voltage(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_camshaft_available(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_converter_overload(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_line_breaker_delay(const VehicleElectricEngine *p_engine) const = 0;
            virtual double get_line_breaker_initial_delay(const VehicleElectricEngine *p_engine) const = 0;
            virtual bool get_line_breaker_closes_at_no_power(const VehicleElectricEngine *p_engine) const = 0;
            virtual void apply_configuration(const VehicleElectricEngine *p_engine) const = 0;
            virtual void converter(const VehicleElectricEngine *p_engine, bool p_enabled) const = 0;
            virtual void converter_fuse_reset(const VehicleElectricEngine *p_engine) const = 0;
            virtual void pantographs_valve(const VehicleElectricEngine *p_engine, bool p_enabled) const = 0;
            virtual void pantographs_valve_operate(
                    const VehicleElectricEngine *p_engine, VehicleElectricEngine::ValveOperation p_operation) const = 0;
            virtual void pantographs_drop_all(const VehicleElectricEngine *p_engine, bool p_enabled) const = 0;
            virtual void pantograph_compressor(const VehicleElectricEngine *p_engine, bool p_enabled) const = 0;
            virtual void
            pantograph_compressor_valve(const VehicleElectricEngine *p_engine, bool p_to_compressor) const = 0;
            virtual void pantograph(
                    const VehicleElectricEngine *p_engine, VehicleElectricEngine::PantographSelector p_selector,
                    bool p_enabled) const = 0;
            virtual void set_pantograph_wire_voltage(
                    const VehicleElectricEngine *p_engine, VehicleElectricEngine::PantographSelector p_selector,
                    float p_voltage) const = 0;
    };
} // namespace godot
