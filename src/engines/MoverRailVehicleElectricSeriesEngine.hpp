#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "MoverElectricEngineBackend.hpp"
#include "MoverElectricTraction.hpp"
#include "MoverEngineBackend.hpp"
#include "RailVehicleElectricSeriesEngine.hpp"

namespace godot {
    /* RailVehicleElectricSeriesEngine on the vendored Mover. It installs the delegates that carry the
     * implementation shared with other engine kinds, and writes the series motor's own
     * configuration into the Mover, which none of those delegates covers. */
    class MoverRailVehicleElectricSeriesEngine : public RailVehicleElectricSeriesEngine, public MoverComponent {
            GDCLASS(MoverRailVehicleElectricSeriesEngine, RailVehicleElectricSeriesEngine);

        private:
            static void _bind_methods();
            MoverEngineBackend engine_backend_impl{*this};
            MoverElectricEngineBackend electric_backend_impl{*this};
            MoverElectricTraction traction{*this};


        public:
            MoverRailVehicleElectricSeriesEngine() {
                engine_backend = &engine_backend_impl;
                electric_backend = &electric_backend_impl;
            }

            double get_motor_current() const override;
            double get_circuit_imax() const override;
            bool get_dynamic_brake_active() const override;
            bool get_fuse_active() const override;
            bool get_motor_connectors_open() const override;
            bool is_line_contactor_closed() const override;
            bool is_pressure_switch_tripped() const override;
            void fuse_reset() override;
            void set_motor_connectors_open(bool p_open) override;
            void _register_commands() override;
            void _unregister_commands() override;
            double get_resistor_fan_rotation() const override;
            double get_circuit_imin() const override;
            double get_engine_voltage() const override;
            double get_next_position_velocity(bool p_main_controller) const override;

        protected:
            void _apply_configuration() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
    };
} // namespace godot
