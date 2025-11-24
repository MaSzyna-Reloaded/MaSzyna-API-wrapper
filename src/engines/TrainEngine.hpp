#pragma once
#include "../core/TrainPart.hpp"
#include "../maszyna/McZapkie/MOVER.h"
#include "resources/engines/MotorParameter.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class TrainEngine : public TrainPart {
            GDCLASS(TrainEngine, TrainPart)
        public:
            TrainEngine();
            enum EngineType {
                NONE,
                DUMB,
                WHEELS_DRIVEN,
                ELECTRIC_SERIES_MOTOR,
                ELECTRIC_INDUCTION_MOTOR,
                DIESEL,
                STEAM,
                DIESEL_ELECTRIC,
                MAIN
            };

            const std::map<EngineType, TEngineType> engine_type_map = {
                    {NONE, TEngineType::None},
                    {DUMB, TEngineType::Dumb},
                    {WHEELS_DRIVEN, TEngineType::WheelsDriven},
                    {ELECTRIC_SERIES_MOTOR, TEngineType::ElectricSeriesMotor},
                    {ELECTRIC_INDUCTION_MOTOR, TEngineType::ElectricInductionMotor},
                    {DIESEL, TEngineType::DieselEngine},
                    {STEAM, TEngineType::SteamEngine},
                    {DIESEL_ELECTRIC, TEngineType::DieselElectric},
                    {MAIN, TEngineType::Main}
            };


            TypedArray<MotorParameter> get_motor_param_table() {
                return motor_param_table;
            }

            void set_motor_param_table(const TypedArray<MotorParameter> &p_motor_param_table) {
                motor_param_table.clear();
                motor_param_table.append_array(p_motor_param_table);
            }

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
