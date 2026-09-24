#pragma once
#include "../core/VehicleComponent.hpp"
#include "macros.hpp"
#include "resources/brakes/BrakePressureTableItem.hpp"
#include "resources/brakes/CompressorListItem.hpp"
#include <godot_cpp/classes/node.hpp>
#include <unordered_map>

namespace godot {
    class VehicleController;
    class VehicleBrake : public VehicleComponent {
            GDCLASS(VehicleBrake, VehicleComponent)
            

        public:
            VehicleComponentType::Type get_component_type() const override {
                return VehicleComponentType::COMPONENT_BRAKES;
            }

        private:
            static void _bind_methods();
        public:
            /* Live state, read straight from the backend - nothing is stored. */
            virtual bool get_alarm_chain_pulled() const = 0;
            virtual double get_air_pressure() const = 0;
            virtual double get_loco_pressure() const = 0;
            virtual double get_pipe_brake_pressure() const = 0;
            virtual double get_pipe_pressure() const = 0;
            virtual double get_feed_pipe_pressure() const = 0;
            virtual double get_tank_volume() const = 0;
            virtual double get_compressor_pressure() const = 0;
            virtual double get_controller_position() const = 0;
            virtual double get_controller_position_normalized() const = 0;
            virtual double get_local_position_normalized() const = 0;
            virtual int get_manual_position() const = 0;
            virtual double get_unit_force() const = 0;
            virtual double get_force_ratio() const = 0;
            virtual double get_emergency_valve_flow() const = 0;
            virtual double get_main_valve_flow() const = 0;
            virtual double get_local_valve_flow() const = 0;
            virtual double get_loco_pressure_fall_rate() const = 0;
            virtual double get_loco_pressure_rise_rate() const = 0;
            virtual double get_control_pressure() const = 0;
            /* Control reservoir of the driver's brake valve (Handle->GetCP(), Train.cpp:8908) */
            virtual double get_handle_control_pressure() const = 0;
            virtual double get_local_aeim_position() const = 0;
            virtual double get_edb_cylinder_pressure() const = 0;
            virtual bool get_releaser_active() const = 0;
            /**
             * @enum BrakeMethod
             * Enumeration representing various brake methods used in train systems.
             */
            enum BrakeMethod {
                BRAKE_METHOD_P10_BGU,
                BRAKE_METHOD_P10_BG,
                BRAKE_METHOD_D1,
                BRAKE_METHOD_D2,
                BRAKE_METHOD_FR513,
                BRAKE_METHOD_COSID,
                BRAKE_METHOD_P10Y_BG,
                BRAKE_METHOD_P10Y_BGU,
                BRAKE_METHOD_FR510,
                BRAKE_METHOD_D1MG,
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
            /* BrakeHandle= / LocBrakeHandle= : shared handle-type enum for both the main and local (independent)
             * brake handles */
            enum BrakeHandleType {
                BRAKE_HANDLE_TYPE_NO_HANDLE,
                BRAKE_HANDLE_TYPE_WESTINGHOUSE,
                BRAKE_HANDLE_TYPE_FV4A,
                BRAKE_HANDLE_TYPE_M394,
                BRAKE_HANDLE_TYPE_M254,
                BRAKE_HANDLE_TYPE_FVE408,
                BRAKE_HANDLE_TYPE_FVEL6,
                BRAKE_HANDLE_TYPE_D2,
                BRAKE_HANDLE_TYPE_KNORR,
                BRAKE_HANDLE_TYPE_FD1,
                BRAKE_HANDLE_TYPE_BS2,
                BRAKE_HANDLE_TYPE_TESTH,
                BRAKE_HANDLE_TYPE_ST113,
                BRAKE_HANDLE_TYPE_MHZ_P,
                BRAKE_HANDLE_TYPE_MHZ_T,
                BRAKE_HANDLE_TYPE_MHZ_EN57,
                BRAKE_HANDLE_TYPE_MHZ_K5P,
                BRAKE_HANDLE_TYPE_MHZ_K8P,
                BRAKE_HANDLE_TYPE_MHZ_6P,
            };
            /* LocalBrake= */
            enum LocalBrakeType {
                LOCAL_BRAKE_TYPE_NONE,
                LOCAL_BRAKE_TYPE_MANUAL,
                LOCAL_BRAKE_TYPE_PNEUMATIC,
                LOCAL_BRAKE_TYPE_HYDRAULIC,
            };
            /* ASB= : anti-skid brake control method */
            enum AntiSkidBrakeType {
                ANTI_SKID_BRAKE_NONE,
                ANTI_SKID_BRAKE_MANUAL,
                ANTI_SKID_BRAKE_AUTOMATIC,
            };
            /* DynamicBrake= */
            enum DynamicBrakeType {
                DYNAMIC_BRAKE_NONE = 0,
                DYNAMIC_BRAKE_PASSIVE = 1,
                DYNAMIC_BRAKE_SWITCH = 2,
                DYNAMIC_BRAKE_REVERSAL = 4,
                DYNAMIC_BRAKE_AUTOMATIC = 8,
            };
            /* BrakeDelays= : possible brake delay settings, named per the FIZ wiki */
            enum BrakeDelaySetting {
                BRAKE_DELAY_G = 1,
                BRAKE_DELAY_P = 2,
                BRAKE_DELAY_R = 4,
                BRAKE_DELAY_GP = 3,
                BRAKE_DELAY_PR = 6,
                BRAKE_DELAY_GPR = 7,
                BRAKE_DELAY_PR_MG = 14,
                BRAKE_DELAY_GPR_MG = 15,
            };
            /* BrakeOpModes= */
            enum BrakeOperationMode {
                BRAKE_OP_MODE_PN = 3,
                BRAKE_OP_MODE_PNEPMED = 15,
            };
            /* BrakeSystem= */
            enum BrakeSystemType {
                BRAKE_SYSTEM_INDIVIDUAL,
                BRAKE_SYSTEM_PNEUMATIC,
                BRAKE_SYSTEM_ELECTRO_PNEUMATIC,
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
            MAKE_MEMBER_GS_NR(
                    TrainBrakeValve, valve_type, BRAKE_VALVE_NO_VALVE);
            MAKE_MEMBER_GS(int, est_valve_size, 0);
            MAKE_MEMBER_GS(int, friction_elements_per_axle, 1);
            MAKE_MEMBER_GS(double, brake_force_max, 1.0);
            MAKE_MEMBER_GS(double, brake_force_traction, 0.0);
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
            MAKE_MEMBER_GS(double, tank_volume_main, 0.0);
            MAKE_MEMBER_GS(double, tank_volume_aux, 0.0);
            MAKE_MEMBER_GS(double, compressor_cab_a_min_pressure, 0.0);
            MAKE_MEMBER_GS(double, compressor_cab_a_max_pressure, 0.0);
            MAKE_MEMBER_GS(double, compressor_cab_b_min_pressure, 0.0);
            MAKE_MEMBER_GS(double, compressor_cab_b_max_pressure, 0.0);
            MAKE_MEMBER_GS(double, compressor_speed, 0.0);
            MAKE_MEMBER_GS(double, rapid_transfer, 1.0);
            MAKE_MEMBER_GS(double, rapid_switching_speed, 55.0);
            MAKE_MEMBER_GS_NR(CompressorPower, compressor_power, COMPRESSOR_POWER_MAIN);
            MAKE_MEMBER_GS_NR(BrakeMethod, brake_method, BRAKE_METHOD_P10_BGU);
            MAKE_MEMBER_GS(double, rig_effectiveness, 0.0);
            MAKE_MEMBER_GS(double, air_leak_multiplier, 1.0);
            MAKE_MEMBER_GS(bool, compressor_tank_valve_active, false);
            MAKE_MEMBER_GS(double, compressor_lower_emergency_closing_pressure, -1.0);
            MAKE_MEMBER_GS(double, compressor_higher_emergency_closing_pressure, -1.0);
            MAKE_MEMBER_GS(double, main_pipe_blocking_pressure, 0.0);
            MAKE_MEMBER_GS(double, main_pipe_unblocking_pressure, 0.0);
            MAKE_MEMBER_GS(int, main_pipe_minimum_unblocking_handle_position, -3.0);
        public:
            virtual bool get_main_pipe_emergency_cuts_off_handle() const = 0;
            virtual void set_main_pipe_emergency_cuts_off_handle(const bool p_value) = 0;
            MAKE_MEMBER_GS(bool, releaser_enabled_only_at_no_power_pos, false)
            MAKE_MEMBER_GS(double, compressor_emergency_valve_area, 0.0);
            MAKE_MEMBER_GS(int, universal_brake_button_1, 0);
            MAKE_MEMBER_GS(int, universal_brake_button_2, 0);
            MAKE_MEMBER_GS(int, universal_brake_button_3, 0);
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<BrakePressureTableItem>, brake_pressure_table)
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<CompressorListItem>, compressor_list)
            /* Cntrl. (czesc dotyczaca hamulca) */
            MAKE_MEMBER_GS_NR(BrakeSystemType, cntrl_brake_system, BRAKE_SYSTEM_PNEUMATIC);
            MAKE_MEMBER_GS(int, cntrl_brake_ctrl_position_count, 6);
            MAKE_MEMBER_GS_NR(BrakeDelaySetting, cntrl_brake_delays, BRAKE_DELAY_GP);
            MAKE_MEMBER_GS(double, cntrl_brake_delay_1, 15.0);
            MAKE_MEMBER_GS(double, cntrl_brake_delay_2, 3.0);
            MAKE_MEMBER_GS(double, cntrl_brake_delay_3, 36.0);
            MAKE_MEMBER_GS(double, cntrl_brake_delay_4, 22.0);
            MAKE_MEMBER_GS_NR(BrakeOperationMode, cntrl_brake_op_modes, BRAKE_OP_MODE_PNEPMED);
            MAKE_MEMBER_GS_NR(BrakeHandleType, cntrl_brake_handle_type, BRAKE_HANDLE_TYPE_FV4A);
            MAKE_MEMBER_GS_NR(AntiSkidBrakeType, cntrl_anti_skid_brake_type, ANTI_SKID_BRAKE_MANUAL);
            MAKE_MEMBER_GS_NR(LocalBrakeType, cntrl_local_brake_type, LOCAL_BRAKE_TYPE_PNEUMATIC);
            MAKE_MEMBER_GS_NR(BrakeHandleType, cntrl_local_brake_handle_type, BRAKE_HANDLE_TYPE_FD1);
            MAKE_MEMBER_GS(bool, cntrl_manual_brake_present, false);
            MAKE_MEMBER_GS_NR(DynamicBrakeType, cntrl_dynamic_brake_type, DYNAMIC_BRAKE_NONE);
            MAKE_MEMBER_GS(bool, cntrl_local_brake_traxx, false);
            MAKE_MEMBER_GS(bool, cntrl_release_parking_by_spring_brake, false);
            MAKE_MEMBER_GS(bool, cntrl_release_parking_by_spring_brake_when_door_open, false);
            MAKE_MEMBER_GS(bool, cntrl_spring_brake_cuts_off_drive, true);
            MAKE_MEMBER_GS(double, cntrl_spring_brake_drive_emergency_velocity, -1.0);
        private:
            /* How much of the maximum force one block is making, 0..1. */
        protected:
            void _register_commands() override;
            void _unregister_commands() override;
        public:
            virtual void brake_releaser(bool p_pressed) = 0;
            virtual void brake_level_set(double p_level) = 0;
            virtual void brake_level_set_position(BrakeHandlePosition p_position) = 0;
            virtual void brake_level_set_position_str(const String &p_position) = 0;
            virtual void brake_level_increase() = 0;
            virtual void brake_level_decrease() = 0;
            virtual void local_brake_set(double p_level) = 0;
            virtual void local_brake_increase() = 0;
            virtual void local_brake_decrease() = 0;
            virtual void manual_brake_increase() = 0;
            virtual void manual_brake_decrease() = 0;
            virtual void auto_rewident(int p_brake_delay) = 0;
            virtual void brake_level_charging(bool p_active) = 0;
            virtual void alarm_chain(bool p_pulled) = 0;
    };
} // namespace godot
VARIANT_ENUM_CAST(VehicleBrake::CompressorPower)
VARIANT_ENUM_CAST(VehicleBrake::TrainBrakeValve)
VARIANT_ENUM_CAST(VehicleBrake::BrakeHandlePosition)
VARIANT_ENUM_CAST(VehicleBrake::BrakeMethod)
VARIANT_ENUM_CAST(VehicleBrake::BrakeHandleType)
VARIANT_ENUM_CAST(VehicleBrake::LocalBrakeType)
VARIANT_ENUM_CAST(VehicleBrake::AntiSkidBrakeType)
VARIANT_ENUM_CAST(VehicleBrake::DynamicBrakeType)
VARIANT_ENUM_CAST(VehicleBrake::BrakeDelaySetting)
VARIANT_ENUM_CAST(VehicleBrake::BrakeOperationMode)
VARIANT_ENUM_CAST(VehicleBrake::BrakeSystemType)
