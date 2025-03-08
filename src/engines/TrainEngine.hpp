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
            enum EngineType {
                ENGINE_TYPE_NONE,
                ENGINE_TYPE_DUMB,
                ENGINE_TYPE_WHEELS_DRIVEN,
                ENGINE_TYPE_ELECTRIC_SERIES_MOTOR,
                ENGINE_TYPE_ELECTRIC_INDUCTION_MOTOR,
                ENGINE_TYPE_DIESEL,
                ENGINE_TYPE_STEAM,
                ENGINE_TYPE_DIESEL_ELECTRIC,
                ENGINE_TYPE_MAIN
            };

            TypedArray<MotorParameter> get_motor_param_table();
            void set_motor_param_table(const TypedArray<MotorParameter> &p_motor_param_table);
            void main_switch(bool p_enabled);
            static void _bind_methods();
            TypedArray<MotorParameter> motor_param_table;

        protected:
            virtual EngineType get_engine_type() = 0;
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
    };
} // namespace godot

VARIANT_ENUM_CAST(TrainEngine::EngineType);
