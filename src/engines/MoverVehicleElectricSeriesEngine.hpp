#pragma once
#include "VehicleElectricSeriesEngine.hpp"
#include "MoverEngineBackend.hpp"
#include "MoverElectricEngineBackend.hpp"
#include "MoverElectricTraction.hpp"

namespace godot {
    /* VehicleElectricSeriesEngine on the vendored Mover. It owns no logic of its own - it installs the delegates that
     * carry the shared implementation, which is what lets engine kinds share code their
     * interfaces cannot inherit from one another. */
    class MoverVehicleElectricSeriesEngine : public VehicleElectricSeriesEngine {
            GDCLASS(MoverVehicleElectricSeriesEngine, VehicleElectricSeriesEngine);

        private:
            static void _bind_methods();
            MoverEngineBackend engine_backend_impl;
            MoverElectricEngineBackend electric_backend_impl;
            MoverElectricTraction traction;


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
    };
} // namespace godot
