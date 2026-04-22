#pragma once

#include "../maszyna/McZapkie/MOVER.h"
#include "RailVehicleModule.hpp"
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    class LegacyRailVehicle;
    class RailVehicle;

    class LegacyRailVehicleModule : public RailVehicleModule {
            GDCLASS(LegacyRailVehicleModule, RailVehicleModule)

        private:
            Dictionary state;

        protected:
            bool _dirty = false;
            LegacyRailVehicle *legacy_rail_vehicle = nullptr;

            virtual void _do_update_internal_mover(TMoverParameters *mover);
            virtual void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) = 0;
            virtual void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config);
            virtual void _do_process_mover(TMoverParameters *mover, double delta);
            virtual void _process_mover(double delta);
            void _initialize() override;
            void _deinitialize() override;
            void _update(double delta) override;

            TMoverParameters *get_mover();

        public:
            ~LegacyRailVehicleModule() override = default;
            void set_rail_vehicle(RailVehicle *p_rail_vehicle) override;

            void update_mover();
            Dictionary get_mover_state();
            LegacyRailVehicle *get_legacy_rail_vehicle() const;
            virtual Dictionary get_supported_commands();

            static void _bind_methods();
    };
} // namespace godot
