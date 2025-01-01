#pragma once
#include "../core/TrainPart.hpp"
#include <godot_cpp/classes/node.hpp>
#include <unordered_map>

#define ASSERT_MOVER_BRAKE(mover_ptr, ...)                                                                             \
    if ((mover_ptr) == nullptr || mover_ptr->Hamulec == nullptr) {                                                     \
        return __VA_ARGS__;                                                                                            \
    }

namespace godot {
    class TrainController;
    class TrainBrake: public TrainPart {
            GDCLASS(TrainBrake, TrainPart)
        public:
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
                    {"min", Maszyna::bh_MIN}, {"max", Maszyna::bh_MAX},      {"drive", Maszyna::bh_RP},
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

            // BrakeValve -> BrakeValve
            // assuming same int values between our TrainBrakeValve and mover's TBrakeValve
            TrainBrakeValve valve = static_cast<TrainBrakeValve>(static_cast<int>(TBrakeValve::NoValve));

            int friction_elements_per_axle = 1;                       // NBpA -> NBpA
            double max_brake_force = 1;                               // MBF -> MaxBrakeForce
            int valve_size = 0;                                       // Size -> BrakeValveSize
            double track_brake_force = 0.0;                           // TBF -> TrackBrakeForce
            double max_pressure = 0.0;                                // MaxBP -> MaxBrakePress[3]
            double max_pressure_aux = 0.0;                            // MaxLBP -> MaxBrakePress[0]
            double max_antislip_pressure = 0.0;                       // MaxASBP -> MaxBrakePress[4]
            double max_pressure_tare = 0.0;                           // TareMaxBP -> MaxBrakePress[1]
            double max_pressure_medium = 0.0;                         // MedMaxBP -> MaxBrakePress[2]
            int cylinders_count = 0;                                  // BCN -> BrakeCylNo
            double cylinder_radius = 0.0;                             // BCR -> BrakeCylRadius
            double cylinder_distance = 0.0;                           // BCD -> BrakeCylDist
            double cylinder_spring_force = 0.0;                       // BCS -> BrakeCylSpring
            double slck_adjustment_force = 0.0;                       // BSA -> BrakeSlckAdj
            double cylinder_gear_ratio = 0.0;                         // BCM -> BrakeCylMult[0]
            double cylinder_gear_ratio_low = 0.0;                     // BCMlo -> BrakeCylMult[1]
            double cylinder_gear_ratio_high = 0.0;                    // BCMHi -> BrakeCylMult[2]
            double pipe_pressure_max = 5.0;                           // HiPP -> HighPipePress
            double pipe_pressure_min = 3.5;                           // LoPP -> LowPipePress
            double main_tank_volume = 0.0;                            // Vv -> VeselVolume
            double aux_tank_volume = 0.0;                             // BVV -> BrakeVVolume
            double compressor_pressure_min = 0.0;                     // MinCP -> MinCompressor
            double compressor_pressure_max = 0.0;                     // MaxCP -> MaxCompressor
            double compressor_pressure_cab_b_min = 0.0;               // MinCP_B -> MinCompressor_cabB
            double compressor_pressure_cab_b_max = 0.0;               // MaxCP_B -> MaxCompressor_cabB
            double compressor_speed = 0.0;                            // CompressorSpeed -> CompressorSpeed
            CompressorPower compressor_power = COMPRESSOR_POWER_MAIN; // CompressorPower
            double rig_effectiveness = 0.0;                           // BRE -> BrakeRigEff effectiveness

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            static void _bind_methods();

            void set_valve(TrainBrakeValve p_valve);
            TrainBrakeValve get_valve() const;

            void set_friction_elements_per_axle(int p_friction_elements_per_axle);
            int get_friction_elements_per_axle() const;

            void set_max_brake_force(double p_max_brake_force);
            double get_max_brake_force() const;

            void set_valve_size(int p_valve_size);
            int get_valve_size() const;

            void set_track_brake_force(double p_track_brake_force);
            double get_track_brake_force() const;

            void set_max_pressure(double p_max_pressure);
            double get_max_pressure() const;

            void set_max_pressure_aux(double p_value);
            double get_max_pressure_aux() const;

            void set_max_pressure_tare(double p_value);
            double get_max_pressure_tare() const;

            void set_max_pressure_medium(double p_value);
            double get_max_pressure_medium() const;

            void set_max_antislip_pressure(double p_max_antislip_pressure);
            double get_max_antislip_pressure() const;

            void set_cylinders_count(int p_cylinders_count);
            int get_cylinders_count() const;

            void set_cylinder_radius(double p_cylinder_radius);
            double get_cylinder_radius() const;

            void set_cylinder_distance(double p_cylinder_distance);
            double get_cylinder_distance() const;

            void set_cylinder_spring_force(double p_cylinder_spring_force);
            double get_cylinder_spring_force() const;

            void set_slck_adjustment_force(double p_slck_adjustment_force);
            double get_slck_adjustment_force() const;

            void set_cylinder_gear_ratio(double p_cylinder_gear_ratio);
            double get_cylinder_gear_ratio() const;

            void set_cylinder_gear_ratio_low(double p_cylinder_gear_ratio_low);
            double get_cylinder_gear_ratio_low() const;

            void set_cylinder_gear_ratio_high(double p_cylinder_gear_ratio_high);
            double get_cylinder_gear_ratio_high() const;

            void set_pipe_pressure_max(double p_pipe_pressure_max);
            double get_pipe_pressure_max() const;

            void set_pipe_pressure_min(double p_pipe_pressure_min);
            double get_pipe_pressure_min() const;

            void set_main_tank_volume(double p_main_tank_volume);
            double get_main_tank_volume() const;

            void set_aux_tank_volume(double p_aux_tank_volume);
            double get_aux_tank_volume() const;

            void set_compressor_pressure_min(double p_compressor_pressure_min);
            double get_compressor_pressure_min() const;

            void set_compressor_pressure_max(double p_compressor_pressure_max);
            double get_compressor_pressure_max() const;

            void set_compressor_pressure_cab_b_min(double p_compressor_pressure_cab_b_min);
            double get_compressor_pressure_cab_b_min() const;

            void set_compressor_pressure_cab_b_max(double p_compressor_pressure_cab_b_max);
            double get_compressor_pressure_cab_b_max() const;

            void set_compressor_speed(double p_compressor_speed);
            double get_compressor_speed() const;

            void set_compressor_power(CompressorPower p_compressor_power);
            CompressorPower get_compressor_power() const;

            void set_rig_effectiveness(double p_rig_effectiveness);
            double get_rig_effectiveness() const;

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
