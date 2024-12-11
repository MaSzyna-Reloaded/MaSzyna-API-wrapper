#pragma once

#include "../maszyna/McZapkie/MOVER.h"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    class LegacyRailVehicle;

    class LegacyRailVehicleModule : public Node {
            GDCLASS(LegacyRailVehicleModule, Node)

        private:
            Dictionary state;

        protected:
            bool _dirty = false;
            LegacyRailVehicle *legacy_rail_vehicle_node = nullptr;

            void _notification(int p_what);

            virtual void _do_update_internal_mover(TMoverParameters *mover);
            virtual void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) = 0;
            virtual void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config);
            virtual void _do_process_mover(TMoverParameters *mover, double delta);
            virtual void _process_mover(double delta);

            TMoverParameters *get_mover();

        public:
            ~LegacyRailVehicleModule() override = default;
            void _process(double delta) override;

            void update_mover();
            Dictionary get_mover_state();
            LegacyRailVehicle *get_legacy_rail_vehicle_node() const;

            static void _bind_methods();
    };
} // namespace godot
