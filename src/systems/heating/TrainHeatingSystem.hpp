#pragma once

#include "../../core/TrainPart.hpp"
#include "../../macros.hpp"
#include "../../maszyna/McZapkie/MOVER.h"
#include "../../mover_util/power_source/Mod.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class TrainHeatingSystem : public TrainPart {
            GDCLASS(TrainHeatingSystem, TrainPart)

            MAKE_MEMBER_GS_NR_DIRTY(Ref<PowerSource>, power_source, Ref<PowerSource>());
            MAKE_MEMBER_GS_NR_DIRTY(Ref<PowerSource>, alternative_power_source, Ref<PowerSource>());

            static void _bind_methods();

            /** internal helper method, possible to abstract out */
            void reload_power_source(Ref<PowerSource> &power_source, TPowerParameters &src);

            /** internal Godot signal handler, used to notify HeatingSystem about resource config change */
            void on_power_source_change();

            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override;
            void _register_commands() override { /** TBD */ };
            void _unregister_commands() override { /** TBD */ };

        public:
            void _enter_tree() override;
            void _ready() override;
    };

} // namespace godot
