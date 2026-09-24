#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "MoverDieselEngineBackend.hpp"
#include "MoverElectricTraction.hpp"
#include "MoverEngineBackend.hpp"
#include "VehicleDieselElectricEngine.hpp"

namespace godot {
    /* VehicleDieselElectricEngine on the vendored Mover. It installs the delegates that carry the
     * implementation shared with other engine kinds, and writes the diesel-electric engine's own
     * configuration into the Mover, which none of those delegates covers. */
    class MoverVehicleDieselElectricEngine : public VehicleDieselElectricEngine, public MoverComponent {
            GDCLASS(MoverVehicleDieselElectricEngine, VehicleDieselElectricEngine);

        private:
            static void _bind_methods();
            MoverEngineBackend engine_backend_impl{*this};
            MoverDieselEngineBackend diesel_backend_impl{*this};
            MoverElectricTraction traction{*this};


        public:
            MoverVehicleDieselElectricEngine() {
                engine_backend = &engine_backend_impl;
                diesel_backend = &diesel_backend_impl;
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
            /// Mover.cpp:8551-8552 (readWWList) - the shunting power bounds of a WWList row
            static constexpr double WWLIST_SHUNT_POWER_DIVISOR = 47.6;

            void _apply_configuration() override;
    };
} // namespace godot
