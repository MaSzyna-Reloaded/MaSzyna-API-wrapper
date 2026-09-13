#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "TrainEngine.hpp"
#include "macros.hpp"
#include "resources/engines/WWListItem.hpp"

namespace godot {
    class TrainController;

    class TrainDieselEngine : public TrainEngine {
            GDCLASS(TrainDieselEngine, TrainEngine)
        private:
            static void _bind_methods();
            MAKE_MEMBER_GS(float, oil_pump_pressure_minimum, 0.0);
            MAKE_MEMBER_GS(float, oil_pump_pressure_maximum, 0.65);
            MAKE_MEMBER_GS(double, maximum_traction_force, 0.0);
            TypedArray<WWListItem> wwlist;

        protected:
            EngineType get_engine_type() override;
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) override;
            void _register_commands() override;
            void _unregister_commands() override;


        public:
            TypedArray<WWListItem> get_wwlist() {
                return wwlist;
            }

            void set_wwlist(const TypedArray<WWListItem> &p_wwlist) {
                wwlist.clear();
                wwlist.append_array(p_wwlist);
            }

            void oil_pump(bool p_enabled);
            void fuel_pump(bool p_enabled);
    };
} // namespace godot
