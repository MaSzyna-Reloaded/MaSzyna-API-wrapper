#pragma once
#include <godot_cpp/classes/node.hpp>
#include "../core/TrainPart.hpp"
#include "../maszyna/McZapkie/MOVER.h"


namespace godot {
    class TrainController;
    class TrainEngine : public TrainPart {
            GDCLASS(TrainEngine, TrainPart)
        public:
            static void _bind_methods();
            TypedArray<Dictionary> motor_param_table;

        protected:
            virtual TEngineType get_engine_type() = 0;
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_process_mover(TMoverParameters *mover, double delta) override;

        public:
            TypedArray<Dictionary> get_motor_param_table();
            void set_motor_param_table(const TypedArray<Dictionary> p_wwlist);
            void _on_command_received(const String &command, const Variant &p1, const Variant &p2) override;

            TrainEngine();
            ~TrainEngine() override = default;
    };
} // namespace godot
