//
// Created by karol on 25.08.2024.
//

#pragma once
#include <godot_cpp/classes/node.hpp>
#include "../core/TrainPart.hpp"


namespace godot {
    class TrainController;
    class TrainBrake final : public TrainPart {
            GDCLASS(TrainBrake, TrainPart)
        private:
            // BrakeValve -> BrakeValve
            int valve = static_cast<int>(TBrakeValve::NoValve);
            int friction_elements_per_axle = 1;         // NBpA -> NBpA
            double max_brake_force = 1;                 // MBF -> MaxBrakeForce
            int valve_size = 0;                         // Size -> BrakeValveSize
            double track_brake_force = 0.0;             // TBF -> TrackBrakeForce
            double max_pressure = 0.0;                  // MaxBP -> MaxBrakePress[3]
            double max_antislip_pressure = 0.0;         // MaxASBP -> MaxBrakePress[4]
            int cylinders_count = 0;                    // BCN -> BrakeCylNo
            double cylinder_radius = 0.0;               // BCR -> BrakeCylRadius
            double cylinder_distance = 0.0;             // BCD -> BrakeCylDist
            double cylinder_spring_force = 0.0;         // BCS -> BrakeCylSpring
            double slck_adjustment_force = 0.0;         // BSA -> BrakeSlckAdj
            double cylinder_gear_ratio = 0.0;           // BCM -> BrakeCylMult[0]
            double cylinder_gear_ratio_low = 0.0;       // BCMlo -> BrakeCylMult[1]
            double cylinder_gear_ratio_high = 0.0;      // BCMHi -> BrakeCylMult[2]
            double pipe_pressure_max = 5.0;             // HiPP -> HighPipePress
            double pipe_pressure_min = 3.5;             // LoPP -> LowPipePress
            double main_tank_volume = 0.0;              // Vv -> VeselVolume
            double aux_tank_volume = 0.0;               // BVV -> BrakeVVolume
            double compressor_pressure_min = 0.0;       // MinCP -> MinCompressor
            double compressor_pressure_max = 0.0;       // MaxCP -> MaxCompressor
            double compressor_pressure_cab_b_min = 0.0; // MinCP_B -> MinCompressor_cabB
            double compressor_pressure_cab_b_max = 0.0; // MaxCP_B -> MaxCompressor_cabB
            double compressor_speed = 0.0;              // CompressorSpeed -> CompressorSpeed
            int compressor_power = 0;                   // CompressorPower
            double rig_effectiveness = 0.0;             // BRE -> BrakeRigEff effectiveness

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;

        public:
            enum CompressorPower {
                COMPRESSOR_POWER_MAIN = 0,
                COMPRESSOR_POWER_UNUSED = 1,
                COMPRESSOR_POWER_CONVERTER = 2,
                COMPRESSOR_POWER_ENGINE,
                COMPRESSOR_POWER_COUPLER1,
                COMPRESSOR_POWER_COUPLER2
            };
            static void _bind_methods();

            void set_valve(int p_valve);
            int get_valve() const;
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

            void set_compressor_power(int p_compressor_power);
            int get_compressor_power() const;

            void set_rig_effectiveness(double p_rig_effectiveness);
            double get_rig_effectiveness() const;

            TrainBrake();
            ~TrainBrake() override = default;
    };
} // namespace godot
VARIANT_ENUM_CAST(TrainBrake::CompressorPower)
