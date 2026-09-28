#pragma once
#include "RailVehicleElectricEngine.hpp"
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    /* What RailVehicleElectricEngine needs a simulation to answer. Series-wound and induction locomotives both run one;
     * same reason. */
    class RailVehicleElectricEngineBackend {
        public:
            virtual ~RailVehicleElectricEngineBackend() = default;

            virtual double get_collector_max_voltage(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_max_current(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_max_lifting(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_min_lifting(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_sliding_width(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_min_main_switch_voltage(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_min_pantograph_tank_pressure(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_max_pantograph_tank_pressure(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_pantograph_tank_pressure(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool
            get_collector_pantograph_pressure_switch_armed(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_pantograph_compressor_valve(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_pantograph_compressor_enabled(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_overvoltage_relay(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_required_main_switch_voltage(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_valve_active(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_valve_enabled(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_pantographs_dropped(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_pantograph_first_active(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_pantograph_valve_enabled(
                    const RailVehicleElectricEngine *p_engine,
                    RailVehicleElectricEngine::PantographSelector p_selector) const = 0;
            virtual void pantograph_valve_operate(
                    const RailVehicleElectricEngine *p_engine, RailVehicleElectricEngine::PantographSelector p_selector,
                    RailVehicleElectricEngine::ValveOperation p_operation) const = 0;
            virtual double get_collector_pantograph_first_voltage(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_collector_pantograph_second_active(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_pantograph_second_voltage(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_collector_voltage(const RailVehicleElectricEngine *p_engine) const = 0;
            /* Energy drawn from the wire and returned to it [kWh], the returned one negative */
            virtual double get_energy_drawn(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_energy_returned(const RailVehicleElectricEngine *p_engine) const = 0;
            /* The energy of p_delta seconds added to the meter */
            virtual void meter_energy(const RailVehicleElectricEngine *p_engine, double p_delta) const = 0;
            virtual bool get_contactors_active(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_diff_relay_active(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_resistors_active(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_vent_overload_active(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_highcurrent_active(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_mainbreaker_active(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_transducer_input_voltage(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_camshaft_available(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_converter_overload(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_line_breaker_delay(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual double get_line_breaker_initial_delay(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual bool get_line_breaker_closes_at_no_power(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual void apply_configuration(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual void converter_fuse_reset(const RailVehicleElectricEngine *p_engine) const = 0;
            virtual void pantographs_valve(const RailVehicleElectricEngine *p_engine, bool p_enabled) const = 0;
            virtual void pantographs_valve_operate(
                    const RailVehicleElectricEngine *p_engine, RailVehicleElectricEngine::ValveOperation p_operation) const = 0;
            virtual void pantographs_drop_all(const RailVehicleElectricEngine *p_engine, bool p_enabled) const = 0;
            virtual void pantograph_compressor(const RailVehicleElectricEngine *p_engine, bool p_enabled) const = 0;
            virtual void
            pantograph_compressor_valve(const RailVehicleElectricEngine *p_engine, bool p_to_compressor) const = 0;
            virtual void pantograph(
                    const RailVehicleElectricEngine *p_engine, RailVehicleElectricEngine::PantographSelector p_selector,
                    bool p_enabled) const = 0;
            virtual void set_pantograph_wire_voltage(
                    const RailVehicleElectricEngine *p_engine, RailVehicleElectricEngine::PantographSelector p_selector,
                    float p_voltage) const = 0;
    };
} // namespace godot
