#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "TrainEngine.hpp"
#include "macros.hpp"
#include "resources/engines/WWListItem.hpp"

namespace godot {
    class TrainDieselEngine : public TrainEngine {
            GDCLASS(TrainDieselEngine, TrainEngine)
        private:
            static void _bind_methods();
            MAKE_MEMBER_GS(float, oil_min_pressure, 0.0);
            MAKE_MEMBER_GS(float, oil_max_pressure, 0.65);
            MAKE_MEMBER_GS(double, traction_force_max, 0.0);
            TypedArray<WWListItem> wwlist;

        protected:
            EngineType get_engine_type() override;
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _register_commands() override;
            void _unregister_commands() override;


        public:
            TrainDieselEngine();
            TypedArray<WWListItem> get_wwlist() {
                return wwlist;
            }

            void set_wwlist(const TypedArray<WWListItem>& p_wwlist) {
                wwlist.clear();
                wwlist.append_array(p_wwlist);
            }

            void oil_pump(bool p_enabled);
            void fuel_pump(bool p_enabled);
    };
} // namespace godot
