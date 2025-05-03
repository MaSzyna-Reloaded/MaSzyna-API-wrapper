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
            // Lazily created secondary player when not provided by caller
            AudioStreamPlayer3D *auto_player_1 = nullptr;
            AudioStreamPlayer3D *auto_player_2 = nullptr;
        protected:
            static const char *SOUND_ENDED_SIGNAL;
            static void _bind_methods();
            void _notification(int what);

        public:
            void stop_all() const;
            void pause_all() const;
            void resume_all() const;
            MAKE_MEMBER_GS(String, sound_root_path, "")
            MAKE_MEMBER_GS(float, max_volume_db, 0.0)
            MAKE_MEMBER_GS(float, min_volume_db, -80.0)
            // Old signature kept for backward compatibility. If player_2 is null, a secondary player will be created automatically.
            void update_audio_streams_and_volumes(double param_value, const Ref<LayeredSoundResource>& layered_sound_data, AudioStreamPlayer3D* player, AudioStreamPlayer3D* player_2);
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
            Ref<AudioStream> get_audio_stream_for_file(const String &p_file_path, bool p_loop_stream = false);
    };
} // namespace godot
