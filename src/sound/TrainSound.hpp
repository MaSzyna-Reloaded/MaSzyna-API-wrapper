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
            static const char *PRELOAD_PROGRESS_UPDATE;
        protected:
            static void _bind_methods();

        public:
            struct SoundDefinition {
                    Ref<LayeredSoundResource> sounds;
                    NodePath audio_stream_player;
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
            void set_battery_stream_player(const NodePath &stream);
            NodePath get_battery_stream_player();
            // Brake
            void set_brake_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_brake_sounds();
            void set_brake_stream_player(const NodePath &stream);
            NodePath get_brake_stream_player();
            // Brake Acc
            void set_brake_acc_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_brake_acc_sounds();
            void set_brake_acc_stream_player(const NodePath &stream);
            NodePath get_brake_acc_stream_player();
            // Brake Cylinder Inc
            void set_brake_cylinder_inc_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_brake_cylinder_inc_sounds();
            void set_brake_cylinder_inc_stream_player(const NodePath &stream);
            NodePath get_brake_cylinder_inc_stream_player();
            // Brake Cylinder Dec
            void set_brake_cylinder_dec_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_brake_cylinder_dec_sounds();
            void set_brake_cylinder_dec_stream_player(const NodePath &stream);
            NodePath get_brake_cylinder_dec_stream_player();
            // Brake Hose Attach
            void set_brake_hose_attach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_brake_hose_attach_sounds();
            void set_brake_hose_attach_stream_player(const NodePath &stream);
            NodePath get_brake_hose_attach_stream_player();
            // Brake Hose Detach
            void set_brake_hose_detach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_brake_hose_detach_sounds();
            void set_brake_hose_detach_stream_player(const NodePath &stream);
            NodePath get_brake_hose_detach_stream_player();
            // Compressor
            void set_compressor_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_compressor_sounds();
            void set_compressor_stream_player(const NodePath &stream);
            NodePath get_compressor_stream_player();
            // Control Attach
            void set_control_attach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_control_attach_sounds();
            void set_control_attach_stream_player(const NodePath &stream);
            NodePath get_control_attach_stream_player();
            // Control Detach
            void set_control_detach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_control_detach_sounds();
            void set_control_detach_stream_player(const NodePath &stream);
            NodePath get_control_detach_stream_player();
            // Converter
            void set_converter_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_converter_sounds();
            void set_converter_stream_player(const NodePath &stream);
            NodePath get_converter_stream_player();
            // Curve
            void set_curve_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_curve_sounds();
            void set_curve_stream_player(const NodePath &stream);
            NodePath get_curve_stream_player();
            // Departure Signal
            void set_departure_signal_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_departure_signal_sounds();
            void set_departure_signal_stream_player(const NodePath &stream);
            NodePath get_departure_signal_stream_player();
            // Derail
            void set_derail_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_derail_sounds();
            void set_derail_stream_player(const NodePath &stream);
            NodePath get_derail_stream_player();
            // Diesel Inc
            void set_diesel_inc_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_diesel_inc_sounds();
            void set_diesel_inc_stream_player(const NodePath &stream);
            NodePath get_diesel_inc_stream_player();
            // Door Close
            void set_door_close_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_close_sounds();
            void set_door_close_stream_player(const NodePath &stream);
            NodePath get_door_close_stream_player();
            // Door Open
            void set_door_open_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_open_sounds();
            void set_door_open_stream_player(const NodePath &stream);
            NodePath get_door_open_stream_player();
            // Door Lock
            void set_door_lock_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_lock_sounds();
            void set_door_lock_stream_player(const NodePath &stream);
            NodePath get_door_lock_stream_player();
            // Door Step Open
            void set_door_step_open_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_step_open_sounds();
            void set_door_step_open_stream_player(const NodePath &stream);
            NodePath get_door_step_open_stream_player();
            // Door Step Close
            void set_door_step_close_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_step_close_sounds();
            void set_door_step_close_stream_player(const NodePath &stream);
            NodePath get_door_step_close_stream_player();
            // Door Run Lock
            void set_door_run_lock_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_run_lock_sounds();
            void set_door_run_lock_stream_player(const NodePath &stream);
            NodePath get_door_run_lock_stream_player();
            // Door Permit
            void set_door_permit_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_door_permit_sounds();
            void set_door_permit_stream_player(const NodePath &stream);
            NodePath get_door_permit_stream_player();
            // Emergency Brake
            void set_emergency_brake_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_emergency_brake_sounds();
            void set_emergency_brake_stream_player(const NodePath &stream);
            NodePath get_emergency_brake_stream_player();
            // Engine
            void set_engine_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_engine_sounds();
            void set_engine_stream_player(const NodePath &stream);
            NodePath get_engine_stream_player();
            // EP Brake Inc
            void set_ep_brake_inc_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_ep_brake_inc_sounds();
            void set_ep_brake_inc_stream_player(const NodePath &stream);
            NodePath get_ep_brake_inc_stream_player();
            // EP Brake Dec
            void set_ep_brake_dec_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_ep_brake_dec_sounds();
            void set_ep_brake_dec_stream_player(const NodePath &stream);
            NodePath get_ep_brake_dec_stream_player();
            // EP Ctrl Value
            void set_ep_ctrl_value_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_ep_ctrl_value_sounds();
            void set_ep_ctrl_value_stream_player(const NodePath &stream);
            NodePath get_ep_ctrl_value_stream_player();
            // Fuel Pump
            void set_fuel_pump_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_fuel_pump_sounds();
            void set_fuel_pump_stream_player(const NodePath &stream);
            NodePath get_fuel_pump_stream_player();
            // Gangway Attach
            void set_gangway_attach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_gangway_attach_sounds();
            void set_gangway_attach_stream_player(const NodePath &stream);
            NodePath get_gangway_attach_stream_player();
            // Gangway Detach
            void set_gangway_detach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_gangway_detach_sounds();
            void set_gangway_detach_stream_player(const NodePath &stream);
            NodePath get_gangway_detach_stream_player();
            // Heater
            void set_heater_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_heater_sounds();
            void set_heater_stream_player(const NodePath &stream);
            NodePath get_heater_stream_player();
            // Heating Attach
            void set_heating_attach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_heating_attach_sounds();
            void set_heating_attach_stream_player(const NodePath &stream);
            NodePath get_heating_attach_stream_player();
            // Heating Detach
            void set_heating_detach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_heating_detach_sounds();
            void set_heating_detach_stream_player(const NodePath &stream);
            NodePath get_heating_detach_stream_player();
            // Horn High
            void set_horn_high_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_horn_high_sounds();
            void set_horn_high_stream_player(const NodePath &stream);
            NodePath get_horn_high_stream_player();
            // Horn Low
            void set_horn_low_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_horn_low_sounds();
            void set_horn_low_stream_player(const NodePath &stream);
            NodePath get_horn_low_stream_player();
            // Whistle
            void set_whistle_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_whistle_sounds();
            void set_whistle_stream_player(const NodePath &stream);
            NodePath get_whistle_stream_player();
            // Inverter
            void set_inverter_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_inverter_sounds();
            void set_inverter_stream_player(const NodePath &stream);
            NodePath get_inverter_stream_player();
            // Loading
            void set_loading_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_loading_sounds();
            void set_loading_stream_player(const NodePath &stream);
            NodePath get_loading_stream_player();
            // Main Hose Attach
            void set_main_hose_attach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_main_hose_attach_sounds();
            void set_main_hose_attach_stream_player(const NodePath &stream);
            NodePath get_main_hose_attach_stream_player();
            // Main Hose Detach
            void set_main_hose_detach_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_main_hose_detach_sounds();
            void set_main_hose_detach_stream_player(const NodePath &stream);
            NodePath get_main_hose_detach_stream_player();
            // Motor Blower
            void set_motor_blower_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_motor_blower_sounds();
            void set_motor_blower_stream_player(const NodePath &stream);
            NodePath get_motor_blower_stream_player();
            // Pantograph Up
            void set_pantograph_up_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_pantograph_up_sounds();
            void set_pantograph_up_stream_player(const NodePath &stream);
            NodePath get_pantograph_up_stream_player();
            // Pantograph Down
            void set_pantograph_down_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_pantograph_down_sounds();
            void set_pantograph_down_stream_player(const NodePath &stream);
            NodePath get_pantograph_down_stream_player();
            // Sand
            void set_sand_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_sand_sounds();
            void set_sand_stream_player(const NodePath &stream);
            NodePath get_sand_stream_player();
            // Small Compressor
            void set_small_compressor_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_small_compressor_sounds();
            void set_small_compressor_stream_player(const NodePath &stream);
            NodePath get_small_compressor_stream_player();
            // Start Jolt
            void set_start_jolt_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_start_jolt_sounds();
            void set_start_jolt_stream_player(const NodePath &stream);
            NodePath get_start_jolt_stream_player();
            // Traction Motor
            void set_traction_motor_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_traction_motor_sounds();
            void set_traction_motor_stream_player(const NodePath &stream);
            NodePath get_traction_motor_stream_player();
            // Traction Motor AC
            void set_traction_motor_ac_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_traction_motor_ac_sounds();
            void set_traction_motor_ac_stream_player(const NodePath &stream);
            NodePath get_traction_motor_ac_stream_player();
            // Transmission
            void set_transmission_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_transmission_sounds();
            void set_transmission_stream_player(const NodePath &stream);
            NodePath get_transmission_stream_player();
            // Turbo
            void set_turbo_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_turbo_sounds();
            void set_turbo_stream_player(const NodePath &stream);
            NodePath get_turbo_stream_player();
            // Ventilator
            void set_ventilator_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_ventilator_sounds();
            void set_ventilator_stream_player(const NodePath &stream);
            NodePath get_ventilator_stream_player();
            // Un Brake
            void set_un_brake_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_un_brake_sounds();
            void set_un_brake_stream_player(const NodePath &stream);
            NodePath get_un_brake_stream_player();
            // Un Loading
            void set_un_loading_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_un_loading_sounds();
            void set_un_loading_stream_player(const NodePath &stream);
            NodePath get_un_loading_stream_player();
            // Wheel Flat
            void set_wheel_flat_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_wheel_flat_sounds();
            void set_wheel_flat_stream_player(const NodePath &stream);
            NodePath get_wheel_flat_stream_player();
            // Wheel Clatter
            void set_wheel_clatter_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_wheel_clatter_sounds();
            void set_wheel_clatter_stream_player(const NodePath &stream);
            NodePath get_wheel_clatter_stream_player();
            // Water Pump
            void set_water_pump_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_water_pump_sounds();
            void set_water_pump_stream_player(const NodePath &stream);
            NodePath get_water_pump_stream_player();
            // Water Heater
            void set_water_heater_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_water_heater_sounds();
            void set_water_heater_stream_player(const NodePath &stream);
            NodePath get_water_heater_stream_player();
            // Compressor Idle
            void set_compressor_idle_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_compressor_idle_sounds();
            void set_compressor_idle_stream_player(const NodePath &stream);
            NodePath get_compressor_idle_stream_player();
            // Braking Resistor Ventilator
            void set_braking_resistor_ventilator_sounds(const Ref<LayeredSoundResource> &sounds);
            Ref<LayeredSoundResource> get_braking_resistor_ventilator_sounds();
            void set_braking_resistor_ventilator_stream_player(const NodePath &stream);
            NodePath get_braking_resistor_ventilator_stream_player();
            Ref<AudioStream> get_audio_stream_for_file(const String &p_file_path, bool p_loop = false);
            void play(const String &p_type, const String &p_file_path);
            void preload_files(const String& p_base_game_dir);
    };
} // namespace godot
