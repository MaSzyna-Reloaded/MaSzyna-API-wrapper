#pragma once

#include "../maszyna/McZapkie/MOVER.h"
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    class LegacyRailVehicle;
    class RailVehicle;

    class LegacyRailVehicleModule : public Resource {
            GDCLASS(LegacyRailVehicleModule, Resource)

        protected:
            Dictionary state;
            Ref<RailVehicle> rail_vehicle;

            virtual void _do_update_internal_mover(TMoverParameters *mover);
            virtual void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) = 0;
            virtual void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config);
            virtual void _do_process_mover(TMoverParameters *mover, double delta);
            virtual void _process_mover(double delta);
            virtual void _initialize();
            virtual void _finalize();
            virtual void _update(double delta);

            TMoverParameters *get_mover();
            LegacyRailVehicle *get_legacy_rail_vehicle() const;

        public:
            ~LegacyRailVehicleModule() override = default;
            void set_rail_vehicle(const Ref<RailVehicle> &p_rail_vehicle);
            Ref<RailVehicle> get_rail_vehicle() const;
            virtual void initialize();
            void finalize();
            virtual void update(double delta);

            void update_mover();
            virtual Dictionary get_mover_state();
            virtual Dictionary get_supported_commands();

            static void _bind_methods();
    };
} // namespace godot
