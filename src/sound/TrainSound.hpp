#pragma once
#include "../resources/sound/LayeredSoundResource.hpp"

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_player3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <map>

namespace godot {
    class TrainSound : public Node {
            GDCLASS(TrainSound, Node);

        private:
            std::map<String, Ref<AudioStream>> stream_cache;
        protected:
            static void _bind_methods();

        public:
            float MAX_VOLUME_DB = 0.0;
            float MIN_VOLUME_DB = -80.0;
            String SOUND_ROOT_PATH = "";
            void set_max_volume_db(float max_volume_db);
            float get_max_volume_db();
            void set_min_volume_db(float min_volume_db);
            float get_min_volume_db();
            void set_sound_root_path(String sound_root_path);
            String get_sound_root_path();
            void update_audio_streams_and_volumes(double param_value, const Ref<LayeredSoundResource>& layered_sound_data, AudioStreamPlayer3D* player_1, AudioStreamPlayer3D* player_2);//const NodePath &player_1_path, const NodePath &player_2_path);
            struct SoundDefinition {
                    Ref<LayeredSoundResource> sounds;
            } battery, brake, brake_acc, brake_cylinder_inc, brake_cylinder_dec, brake_hose_attach, brake_hose_detach,
                    compressor, control_attach, control_detach, converter, curve, departure_signal, derail, diesel_inc,
                    door_close, door_open, door_lock, door_step_open, door_step_close, door_run_lock, door_permit,
                    emergency_brake, engine, ep_brake_inc, ep_brake_dec, ep_ctrl_value, fuel_pump, gangway_attach,
                    gangway_detach, heater, heating_attach, heating_detach, horn_high, horn_low, whistle, inverter,
                    loading, main_hose_attach, main_hose_detach, motor_blower, pantograph_up, pantograph_down, sand,
                    small_compressor, start_jolt, traction_motor, traction_motor_ac, transmission, turbo, ventilator,
                    un_brake, un_loading, wheel_flat, wheel_clatter, water_pump, water_heater, compressor_idle,
                    braking_resistor_ventilator;
            std::array<SoundDefinition, 59> sound_definition_list = {
                    battery, brake, brake_acc, brake_cylinder_inc, brake_cylinder_dec, brake_hose_attach, brake_hose_detach,
                    compressor, control_attach, control_detach, converter, curve, departure_signal, derail, diesel_inc,
                    door_close, door_open, door_lock, door_step_open, door_step_close, door_run_lock, door_permit,
                    emergency_brake, engine, ep_brake_inc, ep_brake_dec, ep_ctrl_value, fuel_pump, gangway_attach,
                    gangway_detach, heater, heating_attach, heating_detach, horn_high, horn_low, whistle, inverter,
                    loading, main_hose_attach, main_hose_detach, motor_blower, pantograph_up, pantograph_down, sand,
                    small_compressor, start_jolt, traction_motor, traction_motor_ac, transmission, turbo, ventilator,
                    un_brake, un_loading, wheel_flat, wheel_clatter, water_pump, water_heater, compressor_idle,
                    braking_resistor_ventilator
            };
            // Battery
            void set_battery_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_battery_sounds();
            // Brake
            void set_brake_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_brake_sounds();
            // Brake Acc
            void set_brake_acc_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_brake_acc_sounds();
            // Brake Cylinder Inc
            void set_brake_cylinder_inc_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_brake_cylinder_inc_sounds();
            // Brake Cylinder Dec
            void set_brake_cylinder_dec_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_brake_cylinder_dec_sounds();
            // Brake Hose Attach
            void set_brake_hose_attach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_brake_hose_attach_sounds();
            // Brake Hose Detach
            void set_brake_hose_detach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_brake_hose_detach_sounds();
            // Compressor
            void set_compressor_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_compressor_sounds();
            // Control Attach
            void set_control_attach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_control_attach_sounds();
            // Control Detach
            void set_control_detach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_control_detach_sounds();
            // Converter
            void set_converter_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_converter_sounds();
            // Curve
            void set_curve_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_curve_sounds();
            // Departure Signal
            void set_departure_signal_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_departure_signal_sounds();
            // Derail
            void set_derail_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_derail_sounds();
            // Diesel Inc
            void set_diesel_inc_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_diesel_inc_sounds();
            // Door Close
            void set_door_close_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_close_sounds();
            // Door Open
            void set_door_open_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_open_sounds();
            // Door Lock
            void set_door_lock_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_lock_sounds();
            // Door Step Open
            void set_door_step_open_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_step_open_sounds();
            // Door Step Close
            void set_door_step_close_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_step_close_sounds();
            // Door Run Lock
            void set_door_run_lock_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_run_lock_sounds();
            // Door Permit
            void set_door_permit_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_permit_sounds();
            // Emergency Brake
            void set_emergency_brake_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_emergency_brake_sounds();
            // Engine
            void set_engine_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_engine_sounds();
            // EP Brake Inc
            void set_ep_brake_inc_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_ep_brake_inc_sounds();
            // EP Brake Dec
            void set_ep_brake_dec_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_ep_brake_dec_sounds();
            // EP Ctrl Value
            void set_ep_ctrl_value_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_ep_ctrl_value_sounds();
            // Fuel Pump
            void set_fuel_pump_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_fuel_pump_sounds();
            // Gangway Attach
            void set_gangway_attach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_gangway_attach_sounds();
            // Gangway Detach
            void set_gangway_detach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_gangway_detach_sounds();
            // Heater
            void set_heater_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_heater_sounds();
            // Heating Attach
            void set_heating_attach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_heating_attach_sounds();
            // Heating Detach
            void set_heating_detach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_heating_detach_sounds();
            // Horn High
            void set_horn_high_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_horn_high_sounds();
            // Horn Low
            void set_horn_low_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_horn_low_sounds();
            // Whistle
            void set_whistle_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_whistle_sounds();
            // Inverter
            void set_inverter_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_inverter_sounds();
            // Loading
            void set_loading_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_loading_sounds();
            // Main Hose Attach
            void set_main_hose_attach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_main_hose_attach_sounds();
            // Main Hose Detach
            void set_main_hose_detach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_main_hose_detach_sounds();
            // Motor Blower
            void set_motor_blower_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_motor_blower_sounds();
            // Pantograph Up
            void set_pantograph_up_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_pantograph_up_sounds();
            // Pantograph Down
            void set_pantograph_down_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_pantograph_down_sounds();
            // Sand
            void set_sand_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_sand_sounds();
            // Small Compressor
            void set_small_compressor_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_small_compressor_sounds();
            // Start Jolt
            void set_start_jolt_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_start_jolt_sounds();
            // Traction Motor
            void set_traction_motor_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_traction_motor_sounds();
            // Traction Motor AC
            void set_traction_motor_ac_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_traction_motor_ac_sounds();
            // Transmission
            void set_transmission_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_transmission_sounds();
            // Turbo
            void set_turbo_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_turbo_sounds();
            // Ventilator
            void set_ventilator_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_ventilator_sounds();
            // Un Brake
            void set_un_brake_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_un_brake_sounds();
            // Un Loading
            void set_un_loading_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_un_loading_sounds();
            // Wheel Flat
            void set_wheel_flat_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_wheel_flat_sounds();
            // Wheel Clatter
            void set_wheel_clatter_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_wheel_clatter_sounds();
            // Water Pump
            void set_water_pump_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_water_pump_sounds();
            // Water Heater
            void set_water_heater_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_water_heater_sounds();
            // Compressor Idle
            void set_compressor_idle_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_compressor_idle_sounds();
            // Braking Resistor Ventilator
            void set_braking_resistor_ventilator_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_braking_resistor_ventilator_sounds();
            Ref<AudioStream> get_audio_stream_for_file(const String &p_file_path, bool p_loop_stream = false);
    };
} // namespace godot
