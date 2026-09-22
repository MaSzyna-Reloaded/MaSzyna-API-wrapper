#pragma once
#include "VehicleDieselEngine.hpp"
#include "MoverEngineBackend.hpp"
#include "MoverDieselEngineBackend.hpp"

namespace godot {
    /* VehicleDieselEngine on the vendored Mover. It owns no logic of its own - it installs the delegates that
     * carry the shared implementation, which is what lets engine kinds share code their
     * interfaces cannot inherit from one another. */
    class MoverVehicleDieselEngine : public VehicleDieselEngine {
            GDCLASS(MoverVehicleDieselEngine, VehicleDieselEngine);

        private:
            static void _bind_methods();
            MoverEngineBackend engine_backend_impl;
            MoverDieselEngineBackend diesel_backend_impl;

        public:
            MoverVehicleDieselEngine() {
                engine_backend = &engine_backend_impl;
                diesel_backend = &diesel_backend_impl;
            }

    };
} // namespace godot
