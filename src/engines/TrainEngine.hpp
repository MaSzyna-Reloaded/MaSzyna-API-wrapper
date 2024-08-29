#pragma once
#include <godot_cpp/classes/node.hpp>
#include "TrainPart.hpp"
#include "maszyna/McZapkie/MOVER.h"


namespace godot {
    class TrainController;
    class TrainEngine : public TrainPart {
            GDCLASS(TrainEngine, TrainPart)

            static void _bind_methods();

        protected:
            virtual TEngineType get_engine_type() = 0;
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;

        public:
            TrainEngine();
            ~TrainEngine() override = default;
    };
} // namespace godot