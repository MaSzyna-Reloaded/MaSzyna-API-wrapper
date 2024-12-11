#pragma once

#include "LegacyRailVehicle.hpp"

namespace godot {
    class LegacyWagon : public LegacyRailVehicle {
            GDCLASS(LegacyWagon, LegacyRailVehicle)

        protected:
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;

        public:
            static void _bind_methods();
    };
} // namespace godot
