#pragma once
#include "../resources/sound/LayeredSoundResource.hpp"

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_player3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <map>

namespace godot {
    class TrainSound : public Node {
            GDCLASS(TrainSound, Node);

        private:
            std::map<String, Ref<AudioStream>> stream_cache;
        protected:
            static const char *SOUND_ENDED_SIGNAL;
            static void _bind_methods();
            void _notification(int what);

        public:
            enum SoundState {
                STATE_STOPPED,
                STATE_STARTING,
                STATE_PLAYING,
                STATE_ENDING
            };

            enum SoundType {
                SOUND_BATTERY,
                SOUND_BRAKE,
                SOUND_BRAKE_ACC,
                SOUND_BRAKE_CYLINDER_INC,
                SOUND_BRAKE_CYLINDER_DEC,
                SOUND_BRAKE_HOSE_ATTACH,
                SOUND_BRAKE_HOSE_DETACH,
                SOUND_COMPRESSOR,
                SOUND_CONTROL_ATTACH,
                SOUND_CONTROL_DETACH,
                SOUND_CONVERTER,
                SOUND_CURVE,
                SOUND_DEPARTURE_SIGNAL,
                SOUND_DERAIL,
                SOUND_DIESEL_INC,
                SOUND_DOOR_CLOSE,
                SOUND_DOOR_OPEN,
                SOUND_DOOR_LOCK,
                SOUND_DOOR_STEP_OPEN,
                SOUND_DOOR_STEP_CLOSE,
                SOUND_DOOR_RUN_LOCK,
                SOUND_DOOR_PERMIT,
                SOUND_EMERGENCY_BRAKE,
                SOUND_ENGINE,
                SOUND_EP_BRAKE_INC,
                SOUND_EP_BRAKE_DEC,
                SOUND_EP_CTRL_VALUE,
                SOUND_FUEL_PUMP,
                SOUND_GANGWAY_ATTACH,
                SOUND_GANGWAY_DETACH,
                SOUND_HEATER,
                SOUND_HEATING_ATTACH,
                SOUND_HEATING_DETACH,
                SOUND_HORN_HIGH,
                SOUND_HORN_LOW,
                SOUND_WHISTLE,
                SOUND_INVERTER,
                SOUND_LOADING,
                SOUND_MAIN_HOSE_ATTACH,
                SOUND_MAIN_HOSE_DETACH,
                SOUND_MOTOR_BLOWER,
                SOUND_PANTOGRAPH_UP,
                SOUND_PANTOGRAPH_DOWN,
                SOUND_SAND,
                SOUND_SMALL_COMPRESSOR,
                SOUND_START_JOLT,
                SOUND_TRACTION_MOTOR,
                SOUND_TRACTION_MOTOR_AC,
                SOUND_TRANSMISSION,
                SOUND_TURBO,
                SOUND_VENTILATOR,
                SOUND_UN_BRAKE,
                SOUND_UN_LOADING,
                SOUND_WHEEL_FLAT,
                SOUND_WHEEL_CLATTER,
                SOUND_WATER_PUMP,
                SOUND_WATER_HEATER,
                SOUND_COMPRESSOR_IDLE,
                SOUND_BRAKING_RESISTOR_VENTILATOR,
                SOUND_COUNT
            };

            struct SoundDefinition {
                    Ref<LayeredSoundResource> resource;
                    SoundState state = STATE_STOPPED;
                    StringName name;
            };

        private:
            std::map<StringName, SoundDefinition> sound_definitions;
            void _setup_sound_definitions();
            void _on_player_finished(const StringName& p_sound_name);

        public:
            void stop_all() const;
            void pause_all() const;
            void resume_all() const;

            void play_sound(const StringName &p_name);
            void play_sound_type(SoundType p_type);
            void stop_sound(const StringName &p_name);
            void stop_sound_type(SoundType p_type);
            void update_sound(const StringName &p_name, double p_param_value);
            void update_sound_type(SoundType p_type, double p_param_value);

            MAKE_MEMBER_GS(String, sound_root_path, "")
            MAKE_MEMBER_GS(float, max_volume_db, 0.0)
            MAKE_MEMBER_GS(float, min_volume_db, -80.0)
            
            // Backward compatibility
            void update_audio_streams_and_volumes(double param_value, const Ref<LayeredSoundResource>& layered_sound_data, AudioStreamPlayer3D* player, AudioStreamPlayer3D* player_2);
            
            Ref<AudioStream> get_audio_stream_for_file(const String &p_file_path, bool p_loop_stream = false);

            // Accessors for the 59 sounds (needed for property binding)
            #define DEFINE_SOUND_GETSET(name) \
                Ref<LayeredSoundResource> get_##name() const { return _get_sound_resource(#name); } \
                void set_##name(const Ref<LayeredSoundResource>& p_res) { _set_sound_resource(#name, p_res); }

            DEFINE_SOUND_GETSET(battery) DEFINE_SOUND_GETSET(brake) DEFINE_SOUND_GETSET(brake_acc)
            DEFINE_SOUND_GETSET(brake_cylinder_inc) DEFINE_SOUND_GETSET(brake_cylinder_dec)
            DEFINE_SOUND_GETSET(brake_hose_attach) DEFINE_SOUND_GETSET(brake_hose_detach)
            DEFINE_SOUND_GETSET(compressor) DEFINE_SOUND_GETSET(control_attach)
            DEFINE_SOUND_GETSET(control_detach) DEFINE_SOUND_GETSET(converter)
            DEFINE_SOUND_GETSET(curve) DEFINE_SOUND_GETSET(departure_signal)
            DEFINE_SOUND_GETSET(derail) DEFINE_SOUND_GETSET(diesel_inc)
            DEFINE_SOUND_GETSET(door_close) DEFINE_SOUND_GETSET(door_open)
            DEFINE_SOUND_GETSET(door_lock) DEFINE_SOUND_GETSET(door_step_open)
            DEFINE_SOUND_GETSET(door_step_close) DEFINE_SOUND_GETSET(door_run_lock)
            DEFINE_SOUND_GETSET(door_permit) DEFINE_SOUND_GETSET(emergency_brake)
            DEFINE_SOUND_GETSET(engine) DEFINE_SOUND_GETSET(ep_brake_inc)
            DEFINE_SOUND_GETSET(ep_brake_dec) DEFINE_SOUND_GETSET(ep_ctrl_value)
            DEFINE_SOUND_GETSET(fuel_pump) DEFINE_SOUND_GETSET(gangway_attach)
            DEFINE_SOUND_GETSET(gangway_detach) DEFINE_SOUND_GETSET(heater)
            DEFINE_SOUND_GETSET(heating_attach) DEFINE_SOUND_GETSET(heating_detach)
            DEFINE_SOUND_GETSET(horn_high) DEFINE_SOUND_GETSET(horn_low)
            DEFINE_SOUND_GETSET(whistle) DEFINE_SOUND_GETSET(inverter)
            DEFINE_SOUND_GETSET(loading) DEFINE_SOUND_GETSET(main_hose_attach)
            DEFINE_SOUND_GETSET(main_hose_detach) DEFINE_SOUND_GETSET(motor_blower)
            DEFINE_SOUND_GETSET(pantograph_up) DEFINE_SOUND_GETSET(pantograph_down)
            DEFINE_SOUND_GETSET(sand) DEFINE_SOUND_GETSET(small_compressor)
            DEFINE_SOUND_GETSET(start_jolt) DEFINE_SOUND_GETSET(traction_motor)
            DEFINE_SOUND_GETSET(traction_motor_ac) DEFINE_SOUND_GETSET(transmission)
            DEFINE_SOUND_GETSET(turbo) DEFINE_SOUND_GETSET(ventilator)
            DEFINE_SOUND_GETSET(un_brake) DEFINE_SOUND_GETSET(un_loading)
            DEFINE_SOUND_GETSET(wheel_flat) DEFINE_SOUND_GETSET(wheel_clatter)
            DEFINE_SOUND_GETSET(water_pump) DEFINE_SOUND_GETSET(water_heater)
            DEFINE_SOUND_GETSET(compressor_idle) DEFINE_SOUND_GETSET(braking_resistor_ventilator)

        private:
            Ref<LayeredSoundResource> _get_sound_resource(const StringName &p_name) const;
            void _set_sound_resource(const StringName &p_name, const Ref<LayeredSoundResource> &p_res);

            static StringName _get_sound_name_from_type(SoundType p_type);
    };
} // namespace godot

VARIANT_ENUM_CAST(godot::TrainSound::SoundType);
