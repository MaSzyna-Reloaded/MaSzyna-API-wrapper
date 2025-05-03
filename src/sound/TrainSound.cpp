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
        BIND_PROPERTY(Variant::FLOAT, "max_volume_db", "max_volume_db", &TrainSound::set_max_volume_db, &TrainSound::get_max_volume_db, "max_volume_db");
        BIND_PROPERTY(Variant::FLOAT, "min_volume_db", "min_volume_db", &TrainSound::set_min_volume_db, &TrainSound::get_min_volume_db, "min_volume_db");
        ClassDB::bind_method(D_METHOD("set_sound_root_path", "path"), &TrainSound::set_sound_root_path);
        ClassDB::bind_method(D_METHOD("get_sound_root_path"), &TrainSound::get_sound_root_path);
        ADD_PROPERTY(
                PropertyInfo(Variant::STRING, "sound_root_path", PROPERTY_HINT_DIR), "set_sound_root_path",
                "get_sound_root_path");
        ClassDB::bind_method(
                D_METHOD(
                        "update_audio_streams_and_volumes", "param_values", "layered_sound_data", "player",
                        "player_2"),
                &TrainSound::update_audio_streams_and_volumes);
        ClassDB::bind_method(
        D_METHOD("get_audio_stream_for_file", "file_path", "loop"), &TrainSound::get_audio_stream_for_file);
        ADD_SIGNAL(MethodInfo(SOUND_ENDED_SIGNAL));
        ClassDB::bind_method(D_METHOD("stop_all"), &TrainSound::stop_all);
        ClassDB::bind_method(D_METHOD("pause_all"), &TrainSound::pause_all);
        ClassDB::bind_method(D_METHOD("resume_all"), &TrainSound::resume_all);

        ClassDB::bind_method(D_METHOD("play_sound", "name"), &TrainSound::play_sound);
        ClassDB::bind_method(D_METHOD("stop_sound", "name"), &TrainSound::stop_sound);
        ClassDB::bind_method(D_METHOD("update_sound", "name", "param_value"), &TrainSound::update_sound);
        ClassDB::bind_method(D_METHOD("_on_player_finished", "sound_name"), &TrainSound::_on_player_finished);

        #define BIND_SOUND_PROPERTY(name) \
            ClassDB::bind_method(D_METHOD("get_" #name), &TrainSound::get_##name); \
            ClassDB::bind_method(D_METHOD("set_" #name, "res"), &TrainSound::set_##name); \
            ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sounds/" #name, PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"), "set_" #name, "get_" #name);

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
            // The scene tree will free children automatically; just clear raw pointers to avoid dangling references.
            auto_player_1 = nullptr;
            auto_player_2 = nullptr;
            for (auto &pair : sound_definitions) {
                pair.second.player_1 = nullptr;
                pair.second.player_2 = nullptr;
            }
            stream_cache.clear();
        }
    }

    void TrainSound::stop_all() const {
        if (auto_player_1 != nullptr) {
            auto_player_1->stop();
        }
        if (auto_player_2 != nullptr) {
            auto_player_2->stop();
        }
        for (const auto &[fst, snd] : sound_definitions) {
            if (snd.player_1 != nullptr) { snd.player_1->stop();
}
            if (snd.player_2 != nullptr) { snd.player_2->stop();
}
        }
    }

    void TrainSound::pause_all() const {
        if (auto_player_1 != nullptr) {
            auto_player_1->set_stream_paused(true);
        }
        if (auto_player_2 != nullptr) {
            auto_player_2->set_stream_paused(true);
        }
        for (const auto &[fst, snd] : sound_definitions) {
            if (snd.player_1 != nullptr) { snd.player_1->set_stream_paused(true);
}
            if (snd.player_2 != nullptr) { snd.player_2->set_stream_paused(true);
}
        }
    }

    void TrainSound::resume_all() const {
        if (auto_player_1 != nullptr) {
            auto_player_1->set_stream_paused(false);
        }
        if (auto_player_2 != nullptr) {
            auto_player_2->set_stream_paused(false);
        }
        for (const auto &[fst, snd] : sound_definitions) {
            if (snd.player_1 != nullptr) { snd.player_1->set_stream_paused(false);
}
            if (snd.player_2 != nullptr) { snd.player_2->set_stream_paused(false);
}
        }
    }

    void TrainSound::play_sound(const StringName &p_name) {
        if (sound_definitions.count(p_name) == 0u) { return;
}
        SoundDefinition &def = sound_definitions[p_name];
        if (!def.resource.is_valid()) { return;
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
                    def.player_1->set_stream(stream);
                    def.player_1->set_max_distance(def.resource->get_range());
                    def.player_1->set_volume_db(max_volume_db);
                    def.player_1->play();
                    return;
                }
            }
            // If no starting sounds or load failed, go straight to playing
            def.state = STATE_PLAYING;
            update_sound(p_name, 0.0);
        }
    }

    void TrainSound::stop_sound(const StringName &p_name) {
        if (!sound_definitions.count(p_name)) return;
        SoundDefinition &def = sound_definitions[p_name];
        if (!def.resource.is_valid()) return;

        if (def.state == STATE_PLAYING || def.state == STATE_STARTING) {
            TypedArray<String> enders = def.resource->get_ending_sounds();
            if (enders.size() > 0) {
                String ender_file = enders[UtilityFunctions::randi() % enders.size()];
                String full_path = sound_root_path + (sound_root_path.ends_with("/") ? "" : "/") + ender_file;
                Ref<AudioStream> stream = get_audio_stream_for_file(full_path, false);
                if (stream.is_valid()) {
                    def.state = STATE_ENDING;
                    def.player_1->stop();
                    def.player_2->stop();
                    def.player_1->set_stream(stream);
                    def.player_1->set_max_distance(def.resource->get_range());
                    def.player_1->set_volume_db(max_volume_db);
                    def.player_1->play();
                    return;
                }
            }
            def.state = STATE_STOPPED;
            def.player_1->stop();
            def.player_2->stop();
        }
    }

    void TrainSound::update_sound(const StringName &p_name, double p_param_value) {
        if (!sound_definitions.count(p_name)) return;
        SoundDefinition &def = sound_definitions[p_name];
        if (!def.resource.is_valid()) return;

        if (def.state == STATE_PLAYING) {
            update_audio_streams_and_volumes(p_param_value, def.resource, def.player_1, def.player_2);
        }
    }

    void TrainSound::_on_player_finished(StringName p_sound_name) {
        if (!sound_definitions.count(p_sound_name)) return;
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
        // We create players for each definition that has a resource.
        for (auto &pair : sound_definitions) {
            if (pair.second.player_1 == nullptr) {
                pair.second.player_1 = memnew(AudioStreamPlayer3D);
                pair.second.player_1->set_name(String(pair.first) + "_player_1");
                add_child(pair.second.player_1);
                pair.second.player_1->connect("finished", Callable(this, "_on_player_finished").bind(pair.first));

                pair.second.player_2 = memnew(AudioStreamPlayer3D);
                pair.second.player_2->set_name(String(pair.first) + "_player_2");
                add_child(pair.second.player_2);
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
        auto_player_1 = player;
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

        // Lazily create secondary player if not provided
        if (player_2 == nullptr) {
            if (auto_player_2 == nullptr) {
                auto_player_2 = memnew(AudioStreamPlayer3D);
                add_child(auto_player_2);
                UtilityFunctions::print_verbose(
                        "[TrainSound] Created secondary AudioStreamPlayer3D automatically as a child node.");
            }
            player_2 = auto_player_2;
        } else {
            auto_player_2 = player_2;
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
        const int param_min = static_cast<int>(key_min);
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
            const Variant key_max = sorted_parameter_keys[idx_max];
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
            const Variant key_max = sorted_parameter_keys[idx_min + 1];
            double base_pitch_max =
                    pitch_table.has(key_max) ? static_cast<double>(pitch_table[key_max]) : static_cast<double>(param_max);
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

            // Compute crossfade window based on threshold percent
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
