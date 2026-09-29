#pragma once
#include "MoverDieselEngineUnit.hpp"
#include "MoverDriveUnit.hpp"
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleDieselEngine.hpp"

namespace godot {
    /* RailVehicleDieselEngine on the vendored Mover. It owns no logic of its own - only the units
     * the engine is composed of, whose implementations other engine kinds share. */
    class MoverRailVehicleDieselEngine : public RailVehicleDieselEngine, public MoverComponent {
            GDCLASS(MoverRailVehicleDieselEngine, RailVehicleDieselEngine);

        private:
            static void _bind_methods();
            MoverDriveUnit drive_unit_impl{*this};
            MoverDieselEngineUnit diesel_engine_unit_impl{*this};

        public:
            MoverRailVehicleDieselEngine() {
                drive_unit = &drive_unit_impl;
                diesel_engine_unit = &diesel_engine_unit_impl;
            }
    };
} // namespace godot
