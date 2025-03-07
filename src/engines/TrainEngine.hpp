#pragma once
#include "../core/TrainPart.hpp"
#include "../maszyna/McZapkie/MOVER.h"
#include "resources/engines/MotorParameter.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class TrainController;
    class TrainEngine : public TrainPart {
            GDCLASS(TrainEngine, TrainPart)
        public:
            static void _bind_methods();
            TypedArray<MotorParameter> motor_param_table;

        protected:
            virtual TEngineType get_engine_type() = 0;
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            TypedArray<MotorParameter> get_motor_param_table();
            void set_motor_param_table(const TypedArray<MotorParameter> &p_motor_param_table);
            void main_switch(bool p_enabled);
    };
} // namespace godot
