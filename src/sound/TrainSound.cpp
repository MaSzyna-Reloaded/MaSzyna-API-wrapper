#include "TrainSound.hpp"
#include <godot_cpp/classes/audio_stream_mp3.hpp>
#include <godot_cpp/classes/audio_stream_ogg_vorbis.hpp>
#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/classes/file_access.hpp>
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
    }

    void TrainSound::_notification(const int what) {
        if (what == NOTIFICATION_PREDELETE) {
            // The scene tree will free children automatically; just clear raw pointers to avoid dangling references.
            auto_player_1 = nullptr;
            auto_player_2 = nullptr;
            stream_cache.clear();
        }
    }

    void TrainSound::stop_all() const {
        auto_player_1->stop();
        auto_player_2->stop();
    }

    void TrainSound::pause_all() const {
        auto_player_1->set_stream_paused(true);
        auto_player_2->set_stream_paused(true);
    }

    void TrainSound::resume_all() const {
        auto_player_1->set_stream_paused(false);
        auto_player_2->set_stream_paused(false);
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
            AudioStreamPlayer3D *player_2 = nullptr) {
        auto_player_1 = player;
        Array sorted_parameter_keys = layered_sound_data->get_sound_table().keys();
        Dictionary audio_map = layered_sound_data->get_sound_table();
        const double crossfade_threshold_percent = layered_sound_data->get_cross_fade();
        if (sound_root_path == "") {
            UtilityFunctions::push_error("[TrainSound] Sound root path not set!");
            return;
        }

        if (!sound_root_path.ends_with("/")) {
            sound_root_path += "/";//Just make sure the path ends with backslash
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
                UtilityFunctions::print_verbose("[TrainSound] Created secondary AudioStreamPlayer3D automatically as a child node.");
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

        const int param_min = sorted_parameter_keys[idx_min];
        if (!audio_map.has(param_min)) {
            UtilityFunctions::push_error("[TrainSound] Key ", Variant(param_min), " not found in audio_map");
            return;
        }

        const String file_path_min = sound_root_path + static_cast<String>(audio_map[param_min]);
        const Ref<AudioStream> stream_min = get_audio_stream_for_file(file_path_min, true);
        int param_max = 0;
        Ref<AudioStream> stream_max;
        if (const int idx_max = idx_min + 1; idx_max < sorted_parameter_keys.size()) {
            param_max = static_cast<int>(sorted_parameter_keys[idx_max]);
            if (!audio_map.has(param_max)) {
                UtilityFunctions::push_error("[TrainSound] Key ", Variant(param_max), " not found in audio_map");
                return;
            }

            const String file_path_max = sound_root_path + static_cast<String>(audio_map[param_max]);
            // if (param_max == static_cast<int>(audio_map.keys()[audio_map.size()])) {//@TODO: experimental
            //     emit_signal(SOUND_ENDED_SIGNAL);
            // }

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
                    "[TrainSound] Assigned stream_min (" + stream_min->get_path() + ") to " + p_min->get_name());
            if (!p_min->is_playing()) {
                p_min->play();
            }
        }

        if (stream_max != nullptr) {
            if (p_max->get_stream() != stream_max) {
                p_max->set_stream(stream_max);
                p_max->set_max_distance(layered_sound_data->get_range());
                UtilityFunctions::print_verbose(
                        "[TrainSound] Assigned stream_max (" + stream_max->get_path() + ") to " + p_max->get_name());
                if (!p_max->is_playing()) {
                    p_max->play();
                }
            }
        } else {
            if (p_max->get_stream() != nullptr) {
                //Clear stream from the highest parameter range
                p_max->stop();
                p_max->set_stream(nullptr);
                UtilityFunctions::print_verbose("[TrainSound] Cleared stream from " + p_max->get_name() + " (highest parameter range)");
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
