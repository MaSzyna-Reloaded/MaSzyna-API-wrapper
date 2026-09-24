#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "VehicleBrake.hpp"

namespace godot {
    /* VehicleBrake on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleBrake : public VehicleBrake {
            GDCLASS(MoverVehicleBrake, VehicleBrake);

        private:
            static void _bind_methods();
        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
            bool get_alarm_chain_pulled() const override;
            double get_air_pressure() const override;
            double get_loco_pressure() const override;
            double get_pipe_brake_pressure() const override;
            double get_pipe_pressure() const override;
            double get_feed_pipe_pressure() const override;
            double get_tank_volume() const override;
            double get_compressor_pressure() const override;
            double get_controller_position() const override;
            double get_controller_position_normalized() const override;
            double get_local_position_normalized() const override;
            int get_manual_position() const override;
            double get_unit_force() const override;
            double get_force_ratio() const override;
            double get_emergency_valve_flow() const override;
            double get_main_valve_flow() const override;
            double get_local_valve_flow() const override;
            double get_loco_pressure_fall_rate() const override;
            double get_loco_pressure_rise_rate() const override;
            double get_control_pressure() const override;
            double get_handle_control_pressure() const override;
            double get_local_aeim_position() const override;
            double get_edb_cylinder_pressure() const override;
            bool get_releaser_active() const override;
        private:
            const std::unordered_map<BrakeMethod, int> brake_method_map = {
                    {BrakeMethod::BRAKE_METHOD_P10_BGU, 1},  {BrakeMethod::BRAKE_METHOD_P10_BG, 2},
                    {BrakeMethod::BRAKE_METHOD_D1, 9},       {BrakeMethod::BRAKE_METHOD_D2, 10},
                    {BrakeMethod::BRAKE_METHOD_FR513, 11},   {BrakeMethod::BRAKE_METHOD_COSID, 12},
                    {BrakeMethod::BRAKE_METHOD_P10Y_BG, 14}, {BrakeMethod::BRAKE_METHOD_P10Y_BGU, 16},
                    {BrakeMethod::BRAKE_METHOD_FR510, 17},   {BrakeMethod::BRAKE_METHOD_D1MG, 137},
            };
            const std::unordered_map<BrakeHandlePosition, int> brake_handle_position_map = {
                    {BrakeHandlePosition::BRAKE_HANDLE_POSITION_MIN, Maszyna::bh_MIN},
                    {BrakeHandlePosition::BRAKE_HANDLE_POSITION_MAX, Maszyna::bh_MAX},
                    {BrakeHandlePosition::BRAKE_HANDLE_POSITION_DRIVE, Maszyna::bh_RP},
                    {BrakeHandlePosition::BRAKE_HANDLE_POSITION_FULL, Maszyna::bh_FB},
                    {BrakeHandlePosition::BRAKE_HANDLE_POSITION_EMERGENCY, Maszyna::bh_EB},
            };
            const std::unordered_map<std::string, int> brake_handle_position_string_map = {
                    {"min", Maszyna::bh_MIN}, {"max", Maszyna::bh_MAX},      {"drive", Maszyna::bh_RP},
                    {"full", Maszyna::bh_FB}, {"emergency", Maszyna::bh_EB},
            };
            const std::unordered_map<TBrakeValve, TBrakeSubSystem> brake_valve_to_subsystem_map = {
                    {TBrakeValve::W, TBrakeSubSystem::ss_W},       {TBrakeValve::W_Lu_L, TBrakeSubSystem::ss_W},
                    {TBrakeValve::W_Lu_VI, TBrakeSubSystem::ss_W}, {TBrakeValve::W_Lu_XR, TBrakeSubSystem::ss_W},
                    {TBrakeValve::ESt3, TBrakeSubSystem::ss_ESt},  {TBrakeValve::ESt3AL2, TBrakeSubSystem::ss_ESt},
                    {TBrakeValve::ESt4, TBrakeSubSystem::ss_ESt},  {TBrakeValve::EP2, TBrakeSubSystem::ss_ESt},
                    {TBrakeValve::EP1, TBrakeSubSystem::ss_ESt},   {TBrakeValve::KE, TBrakeSubSystem::ss_KE},
                    {TBrakeValve::CV1, TBrakeSubSystem::ss_Dako},  {TBrakeValve::CV1_L_TR, TBrakeSubSystem::ss_Dako},
                    {TBrakeValve::LSt, TBrakeSubSystem::ss_LSt},   {TBrakeValve::EStED, TBrakeSubSystem::ss_LSt}};
            const std::unordered_map<BrakePressureTableItem::BrakeType, Maszyna::TBrakeSystem>
                    brake_pressure_table_type_map = {
                            {BrakePressureTableItem::BRAKE_TYPE_PNEUMATIC, Maszyna::TBrakeSystem::Pneumatic},
                            {BrakePressureTableItem::BRAKE_TYPE_ELECTRO_PNEUMATIC,
                             Maszyna::TBrakeSystem::ElectroPneumatic},
                            {BrakePressureTableItem::BRAKE_TYPE_INDIVIDUAL, Maszyna::TBrakeSystem::Individual},
                    };
            const std::unordered_map<BrakeHandleType, Maszyna::TBrakeHandle> brake_handle_type_map = {
                    {BRAKE_HANDLE_TYPE_NO_HANDLE, Maszyna::TBrakeHandle::NoHandle},
                    {BRAKE_HANDLE_TYPE_WESTINGHOUSE, Maszyna::TBrakeHandle::West},
                    {BRAKE_HANDLE_TYPE_FV4A, Maszyna::TBrakeHandle::FV4a},
                    {BRAKE_HANDLE_TYPE_M394, Maszyna::TBrakeHandle::M394},
                    {BRAKE_HANDLE_TYPE_M254, Maszyna::TBrakeHandle::M254},
                    {BRAKE_HANDLE_TYPE_FVE408, Maszyna::TBrakeHandle::FVE408},
                    {BRAKE_HANDLE_TYPE_FVEL6, Maszyna::TBrakeHandle::FVel6},
                    {BRAKE_HANDLE_TYPE_D2, Maszyna::TBrakeHandle::D2},
                    {BRAKE_HANDLE_TYPE_KNORR, Maszyna::TBrakeHandle::Knorr},
                    {BRAKE_HANDLE_TYPE_FD1, Maszyna::TBrakeHandle::FD1},
                    {BRAKE_HANDLE_TYPE_BS2, Maszyna::TBrakeHandle::BS2},
                    {BRAKE_HANDLE_TYPE_TESTH, Maszyna::TBrakeHandle::testH},
                    {BRAKE_HANDLE_TYPE_ST113, Maszyna::TBrakeHandle::St113},
                    {BRAKE_HANDLE_TYPE_MHZ_P, Maszyna::TBrakeHandle::MHZ_P},
                    {BRAKE_HANDLE_TYPE_MHZ_T, Maszyna::TBrakeHandle::MHZ_T},
                    {BRAKE_HANDLE_TYPE_MHZ_EN57, Maszyna::TBrakeHandle::MHZ_EN57},
                    {BRAKE_HANDLE_TYPE_MHZ_K5P, Maszyna::TBrakeHandle::MHZ_K5P},
                    {BRAKE_HANDLE_TYPE_MHZ_K8P, Maszyna::TBrakeHandle::MHZ_K8P},
                    {BRAKE_HANDLE_TYPE_MHZ_6P, Maszyna::TBrakeHandle::MHZ_6P},
            };
            const std::unordered_map<BrakeSystemType, Maszyna::TBrakeSystem> brake_system_type_map = {
                    {BRAKE_SYSTEM_INDIVIDUAL, Maszyna::TBrakeSystem::Individual},
                    {BRAKE_SYSTEM_PNEUMATIC, Maszyna::TBrakeSystem::Pneumatic},
                    {BRAKE_SYSTEM_ELECTRO_PNEUMATIC, Maszyna::TBrakeSystem::ElectroPneumatic},
            };
            const std::unordered_map<LocalBrakeType, Maszyna::TLocalBrake> local_brake_type_map = {
                    {LOCAL_BRAKE_TYPE_NONE, Maszyna::TLocalBrake::NoBrake},
                    {LOCAL_BRAKE_TYPE_MANUAL, Maszyna::TLocalBrake::ManualBrake},
                    {LOCAL_BRAKE_TYPE_PNEUMATIC, Maszyna::TLocalBrake::PneumaticBrake},
                    {LOCAL_BRAKE_TYPE_HYDRAULIC, Maszyna::TLocalBrake::HydraulicBrake},
            };
            bool main_pipe_emergency_cuts_off_handle = false;
        public:
            bool get_main_pipe_emergency_cuts_off_handle() const override {
                return main_pipe_emergency_cuts_off_handle;
            }
            void set_main_pipe_emergency_cuts_off_handle(const bool p_value) override {
                main_pipe_emergency_cuts_off_handle = p_value;
            }
        private:
            double local_brake_pressure_previous = -1.0;
            double local_brake_pressure_change_rate = 0.0;
            static double _controller_position_normalized(const TMoverParameters *p_mover);
            static double _force_ratio(const TMoverParameters *p_mover);
        protected:
            void _apply_configuration() override;
            void _do_process_component(double p_delta) override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
        public:
            void brake_releaser(bool p_pressed) override;
            void brake_level_set(double p_level) override;
            void brake_level_set_position(BrakeHandlePosition p_position) override;
            void brake_level_set_position_str(const String &p_position) override;
            void brake_level_increase() override;
            void brake_level_decrease() override;
            void local_brake_set(double p_level) override;
            void local_brake_increase() override;
            void local_brake_decrease() override;
            void manual_brake_increase() override;
            void manual_brake_decrease() override;
            void auto_rewident(int p_brake_delay) override;
            void brake_level_charging(bool p_active) override;
            void alarm_chain(bool p_pulled) override;
    };
} // namespace godot
