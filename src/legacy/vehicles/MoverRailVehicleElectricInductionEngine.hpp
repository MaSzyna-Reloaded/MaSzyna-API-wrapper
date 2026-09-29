#pragma once
#include "MoverCircuitUnit.hpp"
#include "MoverCurrentCollectorUnit.hpp"
#include "MoverDriveUnit.hpp"
#include "MoverTractionMotorsUnit.hpp"
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleElectricInductionEngine.hpp"

namespace godot {
    /* RailVehicleElectricInductionEngine on the vendored Mover. It owns the units the engine is
     * composed of, and writes the induction motor's own
     * configuration into the Mover, which none of the units covers. */
    class MoverRailVehicleElectricInductionEngine : public RailVehicleElectricInductionEngine, public MoverComponent {
            GDCLASS(MoverRailVehicleElectricInductionEngine, RailVehicleElectricInductionEngine);

        private:
            static void _bind_methods();
            MoverDriveUnit drive_unit_impl{*this};
            MoverCurrentCollectorUnit current_collector_unit_impl{*this};
            MoverCircuitUnit circuit_unit_impl{*this};
            MoverTractionMotorsUnit traction_motors_unit_impl{*this};


        public:
            MoverRailVehicleElectricInductionEngine() {
                traction_motors_unit = &traction_motors_unit_impl;
                drive_unit = &drive_unit_impl;
                current_collector_unit = &current_collector_unit_impl;
                circuit_unit = &circuit_unit_impl;
            }

            TypedArray<RailVehicleInverter> get_inverters() const override;

        protected:
            void _apply_configuration() override;
    };
} // namespace godot
