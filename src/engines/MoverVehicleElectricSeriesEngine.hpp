#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "VehicleElectricSeriesEngine.hpp"
#include "MoverEngineBackend.hpp"
#include "MoverElectricEngineBackend.hpp"
#include "MoverElectricTraction.hpp"

namespace godot {
    /* VehicleElectricSeriesEngine on the vendored Mover. It installs the delegates that carry the
     * implementation shared with other engine kinds, and writes the series motor's own
     * configuration into the Mover, which none of those delegates covers. */
    class MoverVehicleElectricSeriesEngine : public VehicleElectricSeriesEngine, public MoverComponent {
            GDCLASS(MoverVehicleElectricSeriesEngine, VehicleElectricSeriesEngine);

        private:
            static void _bind_methods();
            MoverEngineBackend engine_backend_impl{*this};
            MoverElectricEngineBackend electric_backend_impl{*this};
            MoverElectricTraction traction{*this};


        public:
            MoverVehicleElectricSeriesEngine() {
                engine_backend = &engine_backend_impl;
                electric_backend = &electric_backend_impl;
            }

            double get_motor_current() const override;
            double get_circuit_imax() const override;
            bool get_dynamic_brake_active() const override;
            bool get_fuse_active() const override;
            bool get_motor_connectors_open() const override;
            void fuse_reset() override;
            void set_motor_connectors_open(bool p_open) override;
            void _register_commands() override;
            void _unregister_commands() override;
            double get_resistor_fan_rotation() const override;

        protected:
            void _apply_configuration() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
    };
} // namespace godot
