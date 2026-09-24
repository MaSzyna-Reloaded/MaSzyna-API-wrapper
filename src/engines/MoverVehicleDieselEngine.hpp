#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "MoverDieselEngineBackend.hpp"
#include "MoverEngineBackend.hpp"
#include "VehicleDieselEngine.hpp"

namespace godot {
    /* VehicleDieselEngine on the vendored Mover. It owns no logic of its own - it installs the delegates that
     * carry the shared implementation, which is what lets engine kinds share code their
     * interfaces cannot inherit from one another. */
    class MoverVehicleDieselEngine : public VehicleDieselEngine, public MoverComponent {
            GDCLASS(MoverVehicleDieselEngine, VehicleDieselEngine);

        private:
            static void _bind_methods();
            MoverEngineBackend engine_backend_impl{*this};
            MoverDieselEngineBackend diesel_backend_impl{*this};

        public:
            MoverVehicleDieselEngine() {
                engine_backend = &engine_backend_impl;
                diesel_backend = &diesel_backend_impl;
            }
    };
} // namespace godot
