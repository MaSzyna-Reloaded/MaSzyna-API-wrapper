#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "MoverDieselEngineBackend.hpp"
#include "MoverEngineBackend.hpp"
#include "vehicles/rail/RailVehicleDieselEngine.hpp"

namespace godot {
    /* RailVehicleDieselEngine on the vendored Mover. It owns no logic of its own - it installs the delegates that
     * carry the shared implementation, which is what lets engine kinds share code their
     * interfaces cannot inherit from one another. */
    class MoverRailVehicleDieselEngine : public RailVehicleDieselEngine, public MoverComponent {
            GDCLASS(MoverRailVehicleDieselEngine, RailVehicleDieselEngine);

        private:
            static void _bind_methods();
            MoverEngineBackend engine_backend_impl{*this};
            MoverDieselEngineBackend diesel_backend_impl{*this};

        public:
            MoverRailVehicleDieselEngine() {
                engine_backend = &engine_backend_impl;
                diesel_backend = &diesel_backend_impl;
            }
    };
} // namespace godot
