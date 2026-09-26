#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "MoverElectricEngineBackend.hpp"
#include "MoverElectricTraction.hpp"
#include "MoverEngineBackend.hpp"
#include "VehicleElectricInductionEngine.hpp"

namespace godot {
    /* VehicleElectricInductionEngine on the vendored Mover. It installs the delegates that carry
     * the implementation shared with other engine kinds, and writes the induction motor's own
     * configuration into the Mover, which none of those delegates covers. */
    class MoverVehicleElectricInductionEngine : public VehicleElectricInductionEngine, public MoverComponent {
            GDCLASS(MoverVehicleElectricInductionEngine, VehicleElectricInductionEngine);

        private:
            static void _bind_methods();
            MoverEngineBackend engine_backend_impl{*this};
            MoverElectricEngineBackend electric_backend_impl{*this};
            MoverElectricTraction traction{*this};


        public:
            MoverVehicleElectricInductionEngine() {
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

        protected:
            void _apply_configuration() override;
    };
} // namespace godot
