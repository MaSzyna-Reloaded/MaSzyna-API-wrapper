#pragma once

#include "../maszyna/McZapkie/MOVER.h"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    class LegacyRailVehicle;
    class RailVehicle;

    class LegacyRailVehicleModule : public Resource {
            GDCLASS(LegacyRailVehicleModule, Resource)

        private:
            Dictionary state;

        protected:
            bool _dirty = false;
            LegacyRailVehicle *legacy_rail_vehicle_node = nullptr;
            Node *runtime_host = nullptr;

            virtual void _do_update_internal_mover(TMoverParameters *mover);
            virtual void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) = 0;
            virtual void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config);
            virtual void _do_process_mover(TMoverParameters *mover, double delta);
            virtual void _process_mover(double delta);
            virtual void _enter_tree();
            virtual void _ready();
            virtual void _exit_tree();

            TMoverParameters *get_mover();

        public:
            ~LegacyRailVehicleModule() override = default;
            void enter_tree(RailVehicle *p_vehicle, Node *p_host);
            void ready();
            void exit_tree();
            virtual void process(double delta);

            void update_mover();
            Dictionary get_mover_state();
            LegacyRailVehicle *get_legacy_rail_vehicle_node() const;
            Node *get_runtime_host() const;
            virtual Dictionary get_supported_commands();

            static void _bind_methods();
    };
} // namespace godot
