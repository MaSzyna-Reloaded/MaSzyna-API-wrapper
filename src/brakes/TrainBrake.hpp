#pragma once
#include "macros.hpp"
#include "../core/TrainPart.hpp"
#include <godot_cpp/classes/node.hpp>
#include <unordered_map>

#define ASSERT_MOVER_BRAKE(mover_ptr, ...)                                                                             \
    if ((mover_ptr) == nullptr || mover_ptr->Hamulec == nullptr) {                                                     \
        return __VA_ARGS__;                                                                                            \
    }

namespace godot {
    class TrainBrake: public TrainPart {
            GDCLASS(TrainBrake, TrainPart)
        public:
            /**
             * @enum BrakeMethod
             * Enumeration representing various brake methods used in train systems.
             */
            enum BrakeMethod {
                BRAKE_METHOD_P10Bgu,
                BRAKE_METHOD_P10Bg,
                BRAKE_METHOD_D1,
                BRAKE_METHOD_D2,
                BRAKE_METHOD_FR513,
                BRAKE_METHOD_Cosid,
                BRAKE_METHOD_P10yBg,
                BRAKE_METHOD_P10yBgu,
                BRAKE_METHOD_FR510,
                BRAKE_METHOD_D1MG,
            };

            const std::unordered_map<BrakeMethod, int> BrakeMethodMap = {
                {BrakeMethod::BRAKE_METHOD_P10Bgu, 1},
                {BrakeMethod::BRAKE_METHOD_P10Bg, 2},
                {BrakeMethod::BRAKE_METHOD_D1, 9},
                {BrakeMethod::BRAKE_METHOD_D2, 10},
                {BrakeMethod::BRAKE_METHOD_FR513, 11},
                {BrakeMethod::BRAKE_METHOD_Cosid, 12},
                {BrakeMethod::BRAKE_METHOD_P10yBg, 14},
                {BrakeMethod::BRAKE_METHOD_P10yBgu, 16},
                {BrakeMethod::BRAKE_METHOD_FR510, 17},
                {BrakeMethod::BRAKE_METHOD_D1MG, 137},
            };

            enum BrakeHandlePosition {
                BRAKE_HANDLE_POSITION_MIN = 0,
                BRAKE_HANDLE_POSITION_MAX = 1,
                BRAKE_HANDLE_POSITION_DRIVE = 2,
                BRAKE_HANDLE_POSITION_FULL = 3,
                BRAKE_HANDLE_POSITION_EMERGENCY = 4,
            };

            enum CompressorPower {
                COMPRESSOR_POWER_MAIN = 0,
                COMPRESSOR_POWER_UNUSED = 1,
                COMPRESSOR_POWER_CONVERTER = 2,
                COMPRESSOR_POWER_ENGINE,
                COMPRESSOR_POWER_COUPLER1,
                COMPRESSOR_POWER_COUPLER2
            };

            enum TrainBrakeValve {
                BRAKE_VALVE_NO_VALVE,
                BRAKE_VALVE_W,
                BRAKE_VALVE_W_LU_VI,
                BRAKE_VALVE_W_LU_L,
                BRAKE_VALVE_W_LU_XR,
                BRAKE_VALVE_K,
                BRAKE_VALVE_KG,
                BRAKE_VALVE_KP,
                BRAKE_VALVE_KSS,
                BRAKE_VALVE_KKG,
                BRAKE_VALVE_KKP,
                BRAKE_VALVE_KKS,
                BRAKE_VALVE_HIKG1,
                BRAKE_VALVE_HIKSS,
                BRAKE_VALVE_HIKP1,
                BRAKE_VALVE_KE,
                BRAKE_VALVE_SW,
                BRAKE_VALVE_ESTED,
                BRAKE_VALVE_NEST3,
                BRAKE_VALVE_EST3,
                BRAKE_VALVE_LST,
                BRAKE_VALVE_EST4,
                BRAKE_VALVE_EST3AL2,
                BRAKE_VALVE_EP1,
                BRAKE_VALVE_EP2,
                BRAKE_VALVE_M483,
                BRAKE_VALVE_CV1_L_TR,
                BRAKE_VALVE_CV1,
                BRAKE_VALVE_CV1_R,
                BRAKE_VALVE_OTHER
            };

        private:
            const std::unordered_map<BrakeHandlePosition, int> BrakeHandlePositionMap = {
                    {BrakeHandlePosition::BRAKE_HANDLE_POSITION_MIN, Maszyna::bh_MIN},
                    {BrakeHandlePosition::BRAKE_HANDLE_POSITION_MAX, Maszyna::bh_MAX},
                    {BrakeHandlePosition::BRAKE_HANDLE_POSITION_DRIVE, Maszyna::bh_RP},
                    {BrakeHandlePosition::BRAKE_HANDLE_POSITION_FULL, Maszyna::bh_FB},
                    {BrakeHandlePosition::BRAKE_HANDLE_POSITION_EMERGENCY, Maszyna::bh_EB},
            };
            const std::unordered_map<std::string, int> BrakeHandlePositionStringMap = {
                    {"min", Maszyna::bh_MIN}, {"max", Maszyna::bh_MAX}, {"drive", Maszyna::bh_RP},
                    {"full", Maszyna::bh_FB}, {"emergency", Maszyna::bh_EB},
            };
            const std::unordered_map<TBrakeValve, TBrakeSubSystem> BrakeValveToSubsystemMap = {
                    {TBrakeValve::W, TBrakeSubSystem::ss_W},       {TBrakeValve::W_Lu_L, TBrakeSubSystem::ss_W},
                    {TBrakeValve::W_Lu_VI, TBrakeSubSystem::ss_W}, {TBrakeValve::W_Lu_XR, TBrakeSubSystem::ss_W},
                    {TBrakeValve::ESt3, TBrakeSubSystem::ss_ESt},  {TBrakeValve::ESt3AL2, TBrakeSubSystem::ss_ESt},
                    {TBrakeValve::ESt4, TBrakeSubSystem::ss_ESt},  {TBrakeValve::EP2, TBrakeSubSystem::ss_ESt},
                    {TBrakeValve::EP1, TBrakeSubSystem::ss_ESt},   {TBrakeValve::KE, TBrakeSubSystem::ss_KE},
                    {TBrakeValve::CV1, TBrakeSubSystem::ss_Dako},  {TBrakeValve::CV1_L_TR, TBrakeSubSystem::ss_Dako},
                    {TBrakeValve::LSt, TBrakeSubSystem::ss_LSt},   {TBrakeValve::EStED, TBrakeSubSystem::ss_LSt}};

            MAKE_MEMBER_GS_NR(TrainBrakeValve, valve_type, static_cast<TrainBrakeValve>(static_cast<int>(TBrakeValve::NoValve)));
            MAKE_MEMBER_GS(int, valve_size, 0);
            MAKE_MEMBER_GS(int, friction_elements_per_axle, 1);
            MAKE_MEMBER_GS(double, max_brake_force, 1.0);
            MAKE_MEMBER_GS(double, traction_brake_force, 0.0);
            MAKE_MEMBER_GS(double, max_cylinder_pressure, 0.0);
            MAKE_MEMBER_GS(double, max_aux_pressure, 0.0);
            MAKE_MEMBER_GS(double, max_antislip_pressure, 0.0);
            MAKE_MEMBER_GS(double, max_tare_pressure, 0.0);
            MAKE_MEMBER_GS(double, max_medium_pressure, 0.0);
            MAKE_MEMBER_GS(int, cylinder_count, 0);
            MAKE_MEMBER_GS(double, cylinder_radius, 0.0);
            MAKE_MEMBER_GS(double, cylinder_distance, 0.0);
            MAKE_MEMBER_GS(double, cylinder_spring_force, 0.0);
            MAKE_MEMBER_GS(double, piston_stroke_adjuster_resistance, 0.0);
            MAKE_MEMBER_GS(double, cylinder_gear_ratio, 0.0);
            MAKE_MEMBER_GS(double, cylinder_gear_ratio_low, 0.0);
            MAKE_MEMBER_GS(double, cylinder_gear_ratio_high, 0.0);
            MAKE_MEMBER_GS(double, pipe_pressure_max, 5.0);
            MAKE_MEMBER_GS(double, pipe_pressure_min, 3.5);
            MAKE_MEMBER_GS(double, main_tank_volume, 0.0);
            MAKE_MEMBER_GS(double, aux_tank_volume, 0.0);
            MAKE_MEMBER_GS(double, compressor_pressure_cab_a_min, 0.0);
            MAKE_MEMBER_GS(double, compressor_pressure_cab_a_max, 0.0);
            MAKE_MEMBER_GS(double, compressor_pressure_cab_b_min, 0.0);
            MAKE_MEMBER_GS(double, compressor_pressure_cab_b_max, 0.0);
            MAKE_MEMBER_GS(double, compressor_speed, 0.0);
            MAKE_MEMBER_GS(double, rapid_transfer, 1.0);
            MAKE_MEMBER_GS(double, rapid_switching_speed, 55.0);
            MAKE_MEMBER_GS_NR(CompressorPower, compressor_power, COMPRESSOR_POWER_MAIN);
            MAKE_MEMBER_GS_NR(BrakeMethod, brake_method, BRAKE_METHOD_P10Bgu);
            MAKE_MEMBER_GS(double, rig_effectiveness, 0.0);
            MAKE_MEMBER_GS(double, air_leak_multiplier, 1.0);

            MAKE_MEMBER_GS(bool, compressor_tank_valve_active, false);
            MAKE_MEMBER_GS(double, lower_emergency_closing_pressure, -1.0);
            MAKE_MEMBER_GS(double, higher_emergency_closing_pressure, -1.0);
            MAKE_MEMBER_GS(double, main_pipe_blocking_pressure, 0.0);
            MAKE_MEMBER_GS(double, main_pipe_unblocking_pressure, 0.0);
            MAKE_MEMBER_GS(int, main_pipe_minimum_unblocking_handle_position, -3.0);
            MAKE_MEMBER_GS(bool, releaser_enabled_only_at_no_power_pos, false)

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            static void _bind_methods();
            void brake_releaser(bool p_pressed);
            void brake_level_set(double p_level);
            void brake_level_set_position(BrakeHandlePosition p_position);
            void brake_level_set_position_str(const String &p_position);
            void brake_level_increase();
            void brake_level_decrease();
    };
} // namespace godot
VARIANT_ENUM_CAST(TrainBrake::CompressorPower)
VARIANT_ENUM_CAST(TrainBrake::TrainBrakeValve)
VARIANT_ENUM_CAST(TrainBrake::BrakeHandlePosition)
VARIANT_ENUM_CAST(TrainBrake::BrakeMethod)
