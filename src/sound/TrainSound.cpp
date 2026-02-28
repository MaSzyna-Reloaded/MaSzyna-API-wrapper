#include "TrainSound.hpp"
#include <godot_cpp/classes/audio_stream_mp3.hpp>
#include <godot_cpp/classes/audio_stream_ogg_vorbis.hpp>
#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <utility>

namespace godot {
    const char *TrainSound::SOUND_ENDED_SIGNAL = "sound_ended";

    void TrainSound::_bind_methods() {
        BIND_PROPERTY(
                Variant::FLOAT, "max_volume_db", "max_volume_db", &TrainSound::set_max_volume_db,
                &TrainSound::get_max_volume_db, "max_volume_db");
        BIND_PROPERTY(
                Variant::FLOAT, "min_volume_db", "min_volume_db", &TrainSound::set_min_volume_db,
                &TrainSound::get_min_volume_db, "min_volume_db");
        ClassDB::bind_method(D_METHOD("set_sound_root_path", "path"), &TrainSound::set_sound_root_path);
        ClassDB::bind_method(D_METHOD("get_sound_root_path"), &TrainSound::get_sound_root_path);
        ADD_PROPERTY(
                PropertyInfo(Variant::STRING, "sound_root_path", PROPERTY_HINT_DIR), "set_sound_root_path",
                "get_sound_root_path");
        ClassDB::bind_method(
                D_METHOD(
                        "update_audio_streams_and_volumes", "param_values", "layered_sound_data", "player", "player_2"),
                &TrainSound::update_audio_streams_and_volumes);
        ClassDB::bind_method(
                D_METHOD("get_audio_stream_for_file", "file_path", "loop"), &TrainSound::get_audio_stream_for_file);
        ADD_SIGNAL(MethodInfo(SOUND_ENDED_SIGNAL));
        ClassDB::bind_method(D_METHOD("stop_all"), &TrainSound::stop_all);
        ClassDB::bind_method(D_METHOD("pause_all"), &TrainSound::pause_all);
        ClassDB::bind_method(D_METHOD("resume_all"), &TrainSound::resume_all);

        ClassDB::bind_method(D_METHOD("play_sound", "name"), &TrainSound::play_sound);
        ClassDB::bind_method(D_METHOD("play_sound_type", "type"), &TrainSound::play_sound_type);
        ClassDB::bind_method(D_METHOD("stop_sound", "name"), &TrainSound::stop_sound);
        ClassDB::bind_method(D_METHOD("stop_sound_type", "type"), &TrainSound::stop_sound_type);
        ClassDB::bind_method(D_METHOD("update_sound", "name", "param_value"), &TrainSound::update_sound);
        ClassDB::bind_method(D_METHOD("update_sound_type", "type", "param_value"), &TrainSound::update_sound_type);
        ClassDB::bind_method(D_METHOD("_on_player_finished", "sound_name"), &TrainSound::_on_player_finished);

        BIND_ENUM_CONSTANT(SOUND_BATTERY);
        BIND_ENUM_CONSTANT(SOUND_BRAKE);
        BIND_ENUM_CONSTANT(SOUND_BRAKE_ACC);
        BIND_ENUM_CONSTANT(SOUND_BRAKE_CYLINDER_INC);
        BIND_ENUM_CONSTANT(SOUND_BRAKE_CYLINDER_DEC);
        BIND_ENUM_CONSTANT(SOUND_BRAKE_HOSE_ATTACH);
        BIND_ENUM_CONSTANT(SOUND_BRAKE_HOSE_DETACH);
        BIND_ENUM_CONSTANT(SOUND_COMPRESSOR);
        BIND_ENUM_CONSTANT(SOUND_CONTROL_ATTACH);
        BIND_ENUM_CONSTANT(SOUND_CONTROL_DETACH);
        BIND_ENUM_CONSTANT(SOUND_CONVERTER);
        BIND_ENUM_CONSTANT(SOUND_CURVE);
        BIND_ENUM_CONSTANT(SOUND_DEPARTURE_SIGNAL);
        BIND_ENUM_CONSTANT(SOUND_DERAIL);
        BIND_ENUM_CONSTANT(SOUND_DIESEL_INC);
        BIND_ENUM_CONSTANT(SOUND_DOOR_CLOSE);
        BIND_ENUM_CONSTANT(SOUND_DOOR_OPEN);
        BIND_ENUM_CONSTANT(SOUND_DOOR_LOCK);
        BIND_ENUM_CONSTANT(SOUND_DOOR_STEP_OPEN);
        BIND_ENUM_CONSTANT(SOUND_DOOR_STEP_CLOSE);
        BIND_ENUM_CONSTANT(SOUND_DOOR_RUN_LOCK);
        BIND_ENUM_CONSTANT(SOUND_DOOR_PERMIT);
        BIND_ENUM_CONSTANT(SOUND_EMERGENCY_BRAKE);
        BIND_ENUM_CONSTANT(SOUND_ENGINE);
        BIND_ENUM_CONSTANT(SOUND_EP_BRAKE_INC);
        BIND_ENUM_CONSTANT(SOUND_EP_BRAKE_DEC);
        BIND_ENUM_CONSTANT(SOUND_EP_CTRL_VALUE);
        BIND_ENUM_CONSTANT(SOUND_FUEL_PUMP);
        BIND_ENUM_CONSTANT(SOUND_GANGWAY_ATTACH);
        BIND_ENUM_CONSTANT(SOUND_GANGWAY_DETACH);
        BIND_ENUM_CONSTANT(SOUND_HEATER);
        BIND_ENUM_CONSTANT(SOUND_HEATING_ATTACH);
        BIND_ENUM_CONSTANT(SOUND_HEATING_DETACH);
        BIND_ENUM_CONSTANT(SOUND_HORN_HIGH);
        BIND_ENUM_CONSTANT(SOUND_HORN_LOW);
        BIND_ENUM_CONSTANT(SOUND_WHISTLE);
        BIND_ENUM_CONSTANT(SOUND_INVERTER);
        BIND_ENUM_CONSTANT(SOUND_LOADING);
        BIND_ENUM_CONSTANT(SOUND_MAIN_HOSE_ATTACH);
        BIND_ENUM_CONSTANT(SOUND_MAIN_HOSE_DETACH);
        BIND_ENUM_CONSTANT(SOUND_MOTOR_BLOWER);
        BIND_ENUM_CONSTANT(SOUND_PANTOGRAPH_UP);
        BIND_ENUM_CONSTANT(SOUND_PANTOGRAPH_DOWN);
        BIND_ENUM_CONSTANT(SOUND_SAND);
        BIND_ENUM_CONSTANT(SOUND_SMALL_COMPRESSOR);
        BIND_ENUM_CONSTANT(SOUND_START_JOLT);
        BIND_ENUM_CONSTANT(SOUND_TRACTION_MOTOR);
        BIND_ENUM_CONSTANT(SOUND_TRACTION_MOTOR_AC);
        BIND_ENUM_CONSTANT(SOUND_TRANSMISSION);
        BIND_ENUM_CONSTANT(SOUND_TURBO);
        BIND_ENUM_CONSTANT(SOUND_VENTILATOR);
        BIND_ENUM_CONSTANT(SOUND_UN_BRAKE);
        BIND_ENUM_CONSTANT(SOUND_UN_LOADING);
        BIND_ENUM_CONSTANT(SOUND_WHEEL_FLAT);
        BIND_ENUM_CONSTANT(SOUND_WHEEL_CLATTER);
        BIND_ENUM_CONSTANT(SOUND_WATER_PUMP);
        BIND_ENUM_CONSTANT(SOUND_WATER_HEATER);
        BIND_ENUM_CONSTANT(SOUND_COMPRESSOR_IDLE);
        BIND_ENUM_CONSTANT(SOUND_BRAKING_RESISTOR_VENTILATOR);
        BIND_ENUM_CONSTANT(SOUND_COUNT);

#define BIND_SOUND_PROPERTY(name)                                                                                      \
    ClassDB::bind_method(D_METHOD("get_" #name), &TrainSound::get_##name);                                             \
    ClassDB::bind_method(D_METHOD("set_" #name, "res"), &TrainSound::set_##name);                                      \
    ADD_PROPERTY(                                                                                                      \
            PropertyInfo(Variant::OBJECT, "sounds/" #name, PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),       \
            "set_" #name, "get_" #name);

        ADD_GROUP("Sounds", "sounds/");
        BIND_SOUND_PROPERTY(battery) BIND_SOUND_PROPERTY(brake) BIND_SOUND_PROPERTY(brake_acc)
        BIND_SOUND_PROPERTY(brake_cylinder_inc) BIND_SOUND_PROPERTY(brake_cylinder_dec)
        BIND_SOUND_PROPERTY(brake_hose_attach) BIND_SOUND_PROPERTY(brake_hose_detach)
        BIND_SOUND_PROPERTY(compressor) BIND_SOUND_PROPERTY(control_attach)
        BIND_SOUND_PROPERTY(control_detach) BIND_SOUND_PROPERTY(converter)
        BIND_SOUND_PROPERTY(curve) BIND_SOUND_PROPERTY(departure_signal)
        BIND_SOUND_PROPERTY(derail) BIND_SOUND_PROPERTY(diesel_inc)
        BIND_SOUND_PROPERTY(door_close) BIND_SOUND_PROPERTY(door_open)
        BIND_SOUND_PROPERTY(door_lock) BIND_SOUND_PROPERTY(door_step_open)
        BIND_SOUND_PROPERTY(door_step_close) BIND_SOUND_PROPERTY(door_run_lock)
        BIND_SOUND_PROPERTY(door_permit) BIND_SOUND_PROPERTY(emergency_brake)
        BIND_SOUND_PROPERTY(engine) BIND_SOUND_PROPERTY(ep_brake_inc)
        BIND_SOUND_PROPERTY(ep_brake_dec) BIND_SOUND_PROPERTY(ep_ctrl_value)
        BIND_SOUND_PROPERTY(fuel_pump) BIND_SOUND_PROPERTY(gangway_attach)
        BIND_SOUND_PROPERTY(gangway_detach) BIND_SOUND_PROPERTY(heater)
        BIND_SOUND_PROPERTY(heating_attach) BIND_SOUND_PROPERTY(heating_detach)
        BIND_SOUND_PROPERTY(horn_high) BIND_SOUND_PROPERTY(horn_low)
        BIND_SOUND_PROPERTY(whistle) BIND_SOUND_PROPERTY(inverter)
        BIND_SOUND_PROPERTY(loading) BIND_SOUND_PROPERTY(main_hose_attach)
        BIND_SOUND_PROPERTY(main_hose_detach) BIND_SOUND_PROPERTY(motor_blower)
        BIND_SOUND_PROPERTY(pantograph_up) BIND_SOUND_PROPERTY(pantograph_down)
        BIND_SOUND_PROPERTY(sand) BIND_SOUND_PROPERTY(small_compressor)
        BIND_SOUND_PROPERTY(start_jolt) BIND_SOUND_PROPERTY(traction_motor)
        BIND_SOUND_PROPERTY(traction_motor_ac) BIND_SOUND_PROPERTY(transmission)
        BIND_SOUND_PROPERTY(turbo) BIND_SOUND_PROPERTY(ventilator)
        BIND_SOUND_PROPERTY(un_brake) BIND_SOUND_PROPERTY(un_loading)
        BIND_SOUND_PROPERTY(wheel_flat) BIND_SOUND_PROPERTY(wheel_clatter)
        BIND_SOUND_PROPERTY(water_pump) BIND_SOUND_PROPERTY(water_heater)
        BIND_SOUND_PROPERTY(compressor_idle) BIND_SOUND_PROPERTY(braking_resistor_ventilator)
    }

    void TrainSound::_notification(const int what) {
        if (what == NOTIFICATION_POST_ENTER_TREE) {
            _setup_sound_definitions();
        }
        if (what == NOTIFICATION_PREDELETE) {
            // The scene tree will free children automatically.
            stream_cache.clear();
        }
    }

    void TrainSound::stop_all() const {
        for (const auto &[fst, snd]: sound_definitions) {
            if (!snd.resource.is_valid()) {
                continue;
            }
            if (!snd.resource->get_audio_stream_player_1().is_empty()) {
                if (AudioStreamPlayer3D *p1 = get_node<AudioStreamPlayer3D>(snd.resource->get_audio_stream_player_1())) {
                    p1->stop();
                }
            }
            if (!snd.resource->get_audio_stream_player_2().is_empty()) {
                if (AudioStreamPlayer3D *p2 = get_node<AudioStreamPlayer3D>(snd.resource->get_audio_stream_player_2())) {
                    p2->stop();
                }
            }
        }
    }

    void TrainSound::pause_all() const {
        for (const auto &[fst, snd]: sound_definitions) {
            if (!snd.resource.is_valid()) {
                continue;
            }
            if (!snd.resource->get_audio_stream_player_1().is_empty()) {
                if (AudioStreamPlayer3D *p1 = get_node<AudioStreamPlayer3D>(snd.resource->get_audio_stream_player_1())) {
                    p1->set_stream_paused(true);
                }
            }
            if (!snd.resource->get_audio_stream_player_2().is_empty()) {
                if (AudioStreamPlayer3D *p2 = get_node<AudioStreamPlayer3D>(snd.resource->get_audio_stream_player_2())) {
                    p2->set_stream_paused(true);
                }
            }
        }
    }

    void TrainSound::resume_all() const {
        for (const auto &[fst, snd]: sound_definitions) {
            if (!snd.resource.is_valid()) {
                continue;
            }
            if (!snd.resource->get_audio_stream_player_1().is_empty()) {
                if (AudioStreamPlayer3D *p1 = get_node<AudioStreamPlayer3D>(snd.resource->get_audio_stream_player_1())) {
                    p1->set_stream_paused(false);
                }
            }
            if (!snd.resource->get_audio_stream_player_2().is_empty()) {
                if (AudioStreamPlayer3D *p2 = get_node<AudioStreamPlayer3D>(snd.resource->get_audio_stream_player_2())) {
                    p2->set_stream_paused(false);
                }
            }
        }
    }

    void TrainSound::play_sound(const StringName &p_name) {
        if (sound_definitions.count(p_name) == 0u) {
            return;
        }
        SoundDefinition &def = sound_definitions[p_name];
        if (!def.resource.is_valid()) {
            return;
        }

        AudioStreamPlayer3D *p1 = nullptr;
        if (!def.resource->get_audio_stream_player_1().is_empty()) {
            p1 = get_node<AudioStreamPlayer3D>(def.resource->get_audio_stream_player_1());
        }

        if (p1 == nullptr) {
            UtilityFunctions::push_error("[TrainSound] Primary player is not assigned for sound: ", p_name);
            return;
        }

        if (def.state == STATE_STOPPED) {
            const TypedArray<String> starters = def.resource->get_starting_sounds();
            if (starters.size() > 0) {
                // Pick a starting sound
                const String starter_file = starters.get(UtilityFunctions::randi() % starters.size());
                const String full_path = sound_root_path + (sound_root_path.ends_with("/") ? "" : "/") + starter_file;
                const Ref<AudioStream> stream = get_audio_stream_for_file(full_path, false);
                if (stream.is_valid()) {
                    def.state = STATE_STARTING;
                    p1->set_stream(stream);
                    p1->set_max_distance(def.resource->get_range());
                    p1->set_volume_db(max_volume_db);
                    p1->play();
                    return;
                }
            }
            // If no starting sounds or load failed, go straight to playing
            def.state = STATE_PLAYING;
            update_sound(p_name, 0.0);
        }
    }

    void TrainSound::play_sound_type(const SoundType p_type) {
        play_sound(_get_sound_name_from_type(p_type));
    }

    void TrainSound::stop_sound(const StringName &p_name) {
        if (sound_definitions.count(p_name) == 0u) {
            return;
        }
        SoundDefinition &def = sound_definitions[p_name];
        if (!def.resource.is_valid()) {
            return;
        }

        AudioStreamPlayer3D *p1 = nullptr;
        if (!def.resource->get_audio_stream_player_1().is_empty()) {
            p1 = get_node<AudioStreamPlayer3D>(def.resource->get_audio_stream_player_1());
        }
        AudioStreamPlayer3D *p2 = nullptr;
        if (!def.resource->get_audio_stream_player_2().is_empty()) {
            p2 = get_node<AudioStreamPlayer3D>(def.resource->get_audio_stream_player_2());
        }

        if (p1 == nullptr) {
            return;
        }

        if (def.state == STATE_PLAYING || def.state == STATE_STARTING) {
            TypedArray<String> enders = def.resource->get_ending_sounds();
            if (enders.size() > 0) {
                const String ender_file = enders[UtilityFunctions::randi() % enders.size()];
                const String full_path = sound_root_path + (sound_root_path.ends_with("/") ? "" : "/") + ender_file;
                const Ref<AudioStream> stream = get_audio_stream_for_file(full_path, false);
                if (stream.is_valid()) {
                    def.state = STATE_ENDING;
                    p1->stop();
                    if (p2) {
                        p2->stop();
                    }
                    p1->set_stream(stream);
                    p1->set_max_distance(def.resource->get_range());
                    p1->set_volume_db(max_volume_db);
                    p1->play();
                    return;
                }
            }
            def.state = STATE_STOPPED;
            p1->stop();
            if (p2) {
                p2->stop();
            }
        }
    }

    void TrainSound::stop_sound_type(SoundType p_type) {
        stop_sound(_get_sound_name_from_type(p_type));
    }

    void TrainSound::update_sound(const StringName &p_name, double p_param_value) {
        if (sound_definitions.count(p_name) == 0u) {
            return;
        }
        const SoundDefinition &def = sound_definitions[p_name];
        if (!def.resource.is_valid()) {
            return;
        }

        if (def.state == STATE_PLAYING) {
            AudioStreamPlayer3D *p1 = nullptr;
            if (!def.resource->get_audio_stream_player_1().is_empty()) {
                p1 = get_node<AudioStreamPlayer3D>(def.resource->get_audio_stream_player_1());
            }
            AudioStreamPlayer3D *p2 = nullptr;
            if (!def.resource->get_audio_stream_player_2().is_empty()) {
                p2 = get_node<AudioStreamPlayer3D>(def.resource->get_audio_stream_player_2());
            }

            if (p1 == nullptr) {
                return;
            }

            update_audio_streams_and_volumes(p_param_value, def.resource, p1, p2);
        }
    }

    void TrainSound::update_sound_type(SoundType p_type, double p_param_value) {
        update_sound(_get_sound_name_from_type(p_type), p_param_value);
    }

    void TrainSound::_on_player_finished(const StringName &p_sound_name) {
        if (sound_definitions.count(p_sound_name) == 0u) {
            return;
        }
        SoundDefinition &def = sound_definitions[p_sound_name];

        if (def.state == STATE_STARTING) {
            def.state = STATE_PLAYING;
            // Trigger initial update
            update_sound(p_sound_name, 0.0);
        } else if (def.state == STATE_ENDING) {
            def.state = STATE_STOPPED;
            emit_signal(SOUND_ENDED_SIGNAL);
        }
    }

    void TrainSound::_setup_sound_definitions() {
        // This is called when the node enters the tree.
        // We no longer create players automatically.
        // We check if players can be resolved from NodePaths in resources and connect finished signals.
        for (auto &[fst, snd]: sound_definitions) {
            if (snd.resource.is_valid() && !snd.resource->get_audio_stream_player_1().is_empty()) {
                if (AudioStreamPlayer3D *p1 = get_node<AudioStreamPlayer3D>(snd.resource->get_audio_stream_player_1())) {
                    if (!p1->is_connected("finished", Callable(this, "_on_player_finished").bind(fst))) {
                        p1->connect("finished", Callable(this, "_on_player_finished").bind(fst));
                    }
                }
            }
        }
    }

    Ref<LayeredSoundResource> TrainSound::_get_sound_resource(const StringName &p_name) const {
        if (sound_definitions.count(p_name) != 0u) {
            return sound_definitions.at(p_name).resource;
        }
        return nullptr;
    }

    void TrainSound::_set_sound_resource(const StringName &p_name, const Ref<LayeredSoundResource> &p_res) {
        sound_definitions[p_name].resource = p_res;
        sound_definitions[p_name].name = p_name;
        if (is_inside_tree()) {
            _setup_sound_definitions();
        }
    }

    StringName TrainSound::_get_sound_name_from_type(const SoundType p_type) {
        switch (p_type) {
            case SOUND_BATTERY:
                return "battery";
            case SOUND_BRAKE:
                return "brake";
            case SOUND_BRAKE_ACC:
                return "brake_acc";
            case SOUND_BRAKE_CYLINDER_INC:
                return "brake_cylinder_inc";
            case SOUND_BRAKE_CYLINDER_DEC:
                return "brake_cylinder_dec";
            case SOUND_BRAKE_HOSE_ATTACH:
                return "brake_hose_attach";
            case SOUND_BRAKE_HOSE_DETACH:
                return "brake_hose_detach";
            case SOUND_COMPRESSOR:
                return "compressor";
            case SOUND_CONTROL_ATTACH:
                return "control_attach";
            case SOUND_CONTROL_DETACH:
                return "control_detach";
            case SOUND_CONVERTER:
                return "converter";
            case SOUND_CURVE:
                return "curve";
            case SOUND_DEPARTURE_SIGNAL:
                return "departure_signal";
            case SOUND_DERAIL:
                return "derail";
            case SOUND_DIESEL_INC:
                return "diesel_inc";
            case SOUND_DOOR_CLOSE:
                return "door_close";
            case SOUND_DOOR_OPEN:
                return "door_open";
            case SOUND_DOOR_LOCK:
                return "door_lock";
            case SOUND_DOOR_STEP_OPEN:
                return "door_step_open";
            case SOUND_DOOR_STEP_CLOSE:
                return "door_step_close";
            case SOUND_DOOR_RUN_LOCK:
                return "door_run_lock";
            case SOUND_DOOR_PERMIT:
                return "door_permit";
            case SOUND_EMERGENCY_BRAKE:
                return "emergency_brake";
            case SOUND_ENGINE:
                return "engine";
            case SOUND_EP_BRAKE_INC:
                return "ep_brake_inc";
            case SOUND_EP_BRAKE_DEC:
                return "ep_brake_dec";
            case SOUND_EP_CTRL_VALUE:
                return "ep_ctrl_value";
            case SOUND_FUEL_PUMP:
                return "fuel_pump";
            case SOUND_GANGWAY_ATTACH:
                return "gangway_attach";
            case SOUND_GANGWAY_DETACH:
                return "gangway_detach";
            case SOUND_HEATER:
                return "heater";
            case SOUND_HEATING_ATTACH:
                return "heating_attach";
            case SOUND_HEATING_DETACH:
                return "heating_detach";
            case SOUND_HORN_HIGH:
                return "horn_high";
            case SOUND_HORN_LOW:
                return "horn_low";
            case SOUND_WHISTLE:
                return "whistle";
            case SOUND_INVERTER:
                return "inverter";
            case SOUND_LOADING:
                return "loading";
            case SOUND_MAIN_HOSE_ATTACH:
                return "main_hose_attach";
            case SOUND_MAIN_HOSE_DETACH:
                return "main_hose_detach";
            case SOUND_MOTOR_BLOWER:
                return "motor_blower";
            case SOUND_PANTOGRAPH_UP:
                return "pantograph_up";
            case SOUND_PANTOGRAPH_DOWN:
                return "pantograph_down";
            case SOUND_SAND:
                return "sand";
            case SOUND_SMALL_COMPRESSOR:
                return "small_compressor";
            case SOUND_START_JOLT:
                return "start_jolt";
            case SOUND_TRACTION_MOTOR:
                return "traction_motor";
            case SOUND_TRACTION_MOTOR_AC:
                return "traction_motor_ac";
            case SOUND_TRANSMISSION:
                return "transmission";
            case SOUND_TURBO:
                return "turbo";
            case SOUND_VENTILATOR:
                return "ventilator";
            case SOUND_UN_BRAKE:
                return "un_brake";
            case SOUND_UN_LOADING:
                return "un_loading";
            case SOUND_WHEEL_FLAT:
                return "wheel_flat";
            case SOUND_WHEEL_CLATTER:
                return "wheel_clatter";
            case SOUND_WATER_PUMP:
                return "water_pump";
            case SOUND_WATER_HEATER:
                return "water_heater";
            case SOUND_COMPRESSOR_IDLE:
                return "compressor_idle";
            case SOUND_BRAKING_RESISTOR_VENTILATOR:
                return "braking_resistor_ventilator";
            default:
                return "";
        }
    }

    Ref<AudioStream> TrainSound::get_audio_stream_for_file(const String &p_file_path, const bool p_loop_stream) {
        if (!FileAccess::file_exists(p_file_path)) {
            UtilityFunctions::push_error("[TrainSound] Audio file not found at path: " + p_file_path);
            return nullptr;
        }

        if (stream_cache.find(p_file_path) != stream_cache.end()) {
            return stream_cache[p_file_path];
        }

        UtilityFunctions::print_verbose("[TrainSound] Generating AudioStream stream for: " + p_file_path);
        Ref<AudioStream> _audio_steam = nullptr;
        if (const String extension = p_file_path.get_extension().to_lower(); extension == "wav") {
            const Ref<AudioStreamWAV> _stream = AudioStreamWAV::load_from_file(p_file_path);
            if (p_loop_stream) {
                _stream->set_loop_mode(AudioStreamWAV::LOOP_FORWARD);
            } else {
                _stream->set_loop_mode(AudioStreamWAV::LOOP_DISABLED);
            }
            _audio_steam = _stream;
        } else if (extension == "ogg") {
            const Ref<AudioStreamOggVorbis> _stream = AudioStreamOggVorbis::load_from_file(p_file_path);
            _stream->set_loop(p_loop_stream);
            _audio_steam = _stream;
        } else if (extension == "mp3") {
            const Ref<AudioStreamMP3> _stream = AudioStreamMP3::load_from_file(p_file_path);
            _stream->set_loop(p_loop_stream);
            _audio_steam = _stream;
        } else {
            UtilityFunctions::push_warning("[TrainSound] Unsupported file extension for: " + p_file_path);
            return nullptr;
        }

        stream_cache[p_file_path] = _audio_steam;
        return _audio_steam;
    }

    void TrainSound::update_audio_streams_and_volumes(
            const double param_value, const Ref<LayeredSoundResource> &layered_sound_data, AudioStreamPlayer3D *player,
            AudioStreamPlayer3D *player_2) {
        Array sorted_parameter_keys = layered_sound_data->get_sound_table().keys();
        sorted_parameter_keys.sort(); // Ensure keys are sorted for correct range finding

        Dictionary audio_map = layered_sound_data->get_sound_table();
        Dictionary pitch_table = layered_sound_data->get_pitch_table();

        const double crossfade_threshold_percent = layered_sound_data->get_cross_fade();
        if (sound_root_path == "") {
            UtilityFunctions::push_error("[TrainSound] Sound root path not set!");
            return;
        }

        if (!sound_root_path.ends_with("/")) {
            sound_root_path += "/"; // Just make sure the path ends with backslash
        }

        if (player == nullptr) {
            UtilityFunctions::push_error("[TrainSound] Primary player is not assigned.");
            return;
        }

        if (player_2 == nullptr) {
            UtilityFunctions::push_error("[TrainSound] Secondary player is not assigned for crossfade.");
            return;
        }

        int idx_min = -1;
        for (int i = static_cast<int>(sorted_parameter_keys.size()) - 1; i >= 0; --i) {
            if (param_value >= static_cast<double>(sorted_parameter_keys[i])) {
                idx_min = i;
                break;
            }
        }

        if (idx_min == -1) {
            idx_min = 0;
        }

        const Variant key_min = sorted_parameter_keys[idx_min];
        const int param_min = key_min;
        if (!audio_map.has(key_min)) {
            UtilityFunctions::push_error("[TrainSound] Key ", key_min, " not found in audio_map");
            return;
        }

        const String file_path_min = sound_root_path + static_cast<String>(audio_map[key_min]);
        const Ref<AudioStream> stream_min = get_audio_stream_for_file(file_path_min, true);
        int param_max = 0;
        String file_path_max;
        Ref<AudioStream> stream_max;
        if (const int idx_max = idx_min + 1; idx_max < sorted_parameter_keys.size()) {
            const Variant& key_max = sorted_parameter_keys[idx_max];
            param_max = static_cast<int>(key_max);
            if (!audio_map.has(key_max)) {
                UtilityFunctions::push_error("[TrainSound] Key ", key_max, " not found in audio_map");
                return;
            }

            file_path_max = sound_root_path + static_cast<String>(audio_map[key_max]);
            stream_max = get_audio_stream_for_file(file_path_max, true);
        } else {
            param_max = param_min + 1;
        }

        AudioStreamPlayer3D *p_min = nullptr;
        AudioStreamPlayer3D *p_max = nullptr;
        if (idx_min % 2 == 0) {
            p_min = player;
            p_max = player_2;
        } else {
            p_min = player_2;
            p_max = player;
        }

        if (p_min->get_stream() != stream_min) {
            p_min->set_stream(stream_min);
            p_min->set_max_distance(layered_sound_data->get_range());
            UtilityFunctions::print_verbose(
                    "[TrainSound] Assigned stream_min (" + file_path_min + ") to " + p_min->get_name());
            if (!p_min->is_playing()) {
                p_min->play();
            }
        }

        // Apply pitch for p_min
        double base_pitch_min =
                pitch_table.has(key_min) ? static_cast<double>(pitch_table[key_min]) : static_cast<double>(param_min);
        // Avoid division by zero
        if (Math::is_zero_approx(base_pitch_min)) {
            base_pitch_min = 1.0;
        }

        if (idx_min == 0 && param_value < base_pitch_min) {
            p_min->set_pitch_scale(1.0f);
        } else {
            // Ensure pitch is always positive to avoid Godot errors
            p_min->set_pitch_scale(static_cast<float>(Math::max(0.01, param_value / base_pitch_min)));
        }

        if (stream_max != nullptr) {
            if (p_max->get_stream() != stream_max) {
                p_max->set_stream(stream_max);
                p_max->set_max_distance(layered_sound_data->get_range());
                UtilityFunctions::print_verbose(
                        "[TrainSound] Assigned stream_max (" + file_path_max + ") to " + p_max->get_name());
                if (!p_max->is_playing()) {
                    p_max->play();
                }
            }
            // Apply pitch for p_max
            const Variant &key_max = sorted_parameter_keys[idx_min + 1];
            double base_pitch_max = pitch_table.has(key_max) ? static_cast<double>(pitch_table[key_max])
                                                             : static_cast<double>(param_max);
            if (Math::is_zero_approx(base_pitch_max)) {
                base_pitch_max = 1.0;
            }
            p_max->set_pitch_scale(static_cast<float>(Math::max(0.01, param_value / base_pitch_max)));

        } else {
            if (p_max->get_stream() != nullptr) {
                // Clear stream from the highest parameter range
                p_max->stop();
                p_max->set_stream(nullptr);
                UtilityFunctions::print_verbose(
                        "[TrainSound] Cleared stream from " + p_max->get_name() + " (highest parameter range)");
            }
        }

        if (stream_max != nullptr && param_max > param_min) {
            if (crossfade_threshold_percent > 100 || crossfade_threshold_percent < 0) {
                UtilityFunctions::push_error(
                        "[TrainSound] Crossfade threshold must be within a range between 0 and 100");
                return;
            }

            // Compute a crossfade window based on threshold percent
            const double width = (param_max - param_min) * (crossfade_threshold_percent / 100.0);
            // Anchor the crossfade at the top of the range, so blending happens as we approach the next layer
            const double crossfade_end = param_max;

            if (const double crossfade_start = crossfade_end - width; param_value < crossfade_start) {
                // Before crossfade window: only lower stream audible
                p_min->set_volume_db(max_volume_db);
                p_max->set_volume_db(min_volume_db);
            } else if (param_value >= crossfade_end) {
                // After crossfade window (at or beyond the boundary): only upper stream audible
                p_min->set_volume_db(min_volume_db);
                p_max->set_volume_db(max_volume_db);
            } else {
                // Within crossfade window: smooth equal-power crossfade
                const double t = Math::clamp((param_value - crossfade_start) / Math::max(width, 1e-6), 0.0, 1.0);
                const double gain_min = Math::cos(t * M_PI_2);
                const double gain_max = Math::sin(t * M_PI_2);
                const double base_db = max_volume_db;
                const double db_min = base_db + UtilityFunctions::linear_to_db(gain_min);
                const double db_max = base_db + UtilityFunctions::linear_to_db(gain_max);
                p_min->set_volume_db(static_cast<float>(Math::max(db_min, static_cast<double>(min_volume_db))));
                p_max->set_volume_db(static_cast<float>(Math::max(db_max, static_cast<double>(min_volume_db))));
            }
        } else {
            p_min->set_volume_db(max_volume_db);
            p_max->set_volume_db(min_volume_db);
        }
    }
} // namespace godot
