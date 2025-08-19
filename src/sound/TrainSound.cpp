#include "TrainSound.hpp"
#include <godot_cpp/classes/audio_stream_mp3.hpp>
#include <godot_cpp/classes/audio_stream_ogg_vorbis.hpp>
#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <utility>

namespace godot {
    void TrainSound::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_max_volume_db", "max_volume_db"), &TrainSound::set_max_volume_db);
        ClassDB::bind_method(D_METHOD("get_max_volume_db"), &TrainSound::get_max_volume_db);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_volume_db"), "set_max_volume_db", "get_max_volume_db");
        ClassDB::bind_method(D_METHOD("set_min_volume_db", "min_volume_db"), &TrainSound::set_min_volume_db);
        ClassDB::bind_method(D_METHOD("get_min_volume_db"), &TrainSound::get_min_volume_db);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_volume_db"), "set_min_volume_db", "get_min_volume_db");
        ClassDB::bind_method(D_METHOD("set_sound_root_path", "path"), &TrainSound::set_sound_root_path);
        ClassDB::bind_method(D_METHOD("get_sound_root_path"), &TrainSound::get_sound_root_path);
        ADD_PROPERTY(
                PropertyInfo(Variant::STRING, "sound_root_path", PROPERTY_HINT_DIR), "set_sound_root_path",
                "get_sound_root_path");
        ClassDB::bind_method(
                D_METHOD(
                        "update_audio_streams_and_volumes", "param_values", "layered_sound_data", "player_1",
                        "player_2"),
                &TrainSound::update_audio_streams_and_volumes);
        ClassDB::bind_method(
                D_METHOD("get_audio_stream_for_file", "file_path", "loop"), &TrainSound::get_audio_stream_for_file);
    }

    Ref<AudioStream> TrainSound::get_audio_stream_for_file(const String &p_file_path, bool p_loop_stream) {
        if (!FileAccess::file_exists(p_file_path)) {
            UtilityFunctions::push_error("[TrainSound] Audio file not found at path: " + p_file_path);
            return nullptr;
        }

        if (stream_cache.find(p_file_path) != stream_cache.end()) {
            UtilityFunctions::print_verbose("[TrainSound] Returning already preloaded stream for: " + p_file_path);
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
            const double param_value, const Ref<LayeredSoundResource> &layered_sound_data, AudioStreamPlayer3D *player_1,
            AudioStreamPlayer3D *player_2) {

        Array sorted_parameter_keys = layered_sound_data->get_sound_table().keys();
        Dictionary audio_map = layered_sound_data->get_sound_table();
        const double crossfade_threshold_percent = layered_sound_data->get_cross_fade();
        if (sound_root_path == "") {
            UtilityFunctions::push_error("[TrainSound] Sound root path not set!");
            return;
        }

        if (!sound_root_path.ends_with("/")) {
            UtilityFunctions::push_error("[TrainSound] Sound root path must end with a slash!");
            return;
        }

        if (player_1 == nullptr || player_2 == nullptr) {
            UtilityFunctions::push_error("[TrainSound] One of the required players is not assigned.");
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
            stream_max = get_audio_stream_for_file(file_path_max, true);
        } else {
            param_max = param_min + 1;
        }

        AudioStreamPlayer3D *p_min = nullptr;
        AudioStreamPlayer3D *p_max = nullptr;
        if (idx_min % 2 == 0) {
            p_min = player_1;
            p_max = player_2;
        } else {
            p_min = player_2;
            p_max = player_1;
        }

        if (p_min->get_stream() != stream_min) {
            p_min->set_stream(stream_min);
            p_min->set_max_distance(layered_sound_data->get_range());
            UtilityFunctions::print_verbose(
                    "[TrainSound] Assigned stream_min (" + stream_min->get_path().get_file() + ") to " + p_min->get_name());
            if (!p_min->is_playing()) {
                p_min->play();
            }
        }

        if (stream_max != nullptr) {
            if (p_max->get_stream() != stream_max) {
                p_max->set_stream(stream_max);
                p_max->set_max_distance(layered_sound_data->get_range());
                UtilityFunctions::print_verbose(
                        "[TrainSound] Assigned stream_max (" + stream_max->get_path().get_file() + ") to " + p_max->get_name());
                if (!p_max->is_playing()) {
                    p_max->play();
                }
            }
        } else {
            if (p_max->get_stream() != nullptr) {
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

            if (const double crossfade_start = param_min + ((param_max - param_min) * (crossfade_threshold_percent / 100.0)); param_value >= crossfade_start) {
                double progress = 0.0;
                // Thanks a lot for help, LeD
                progress = Math::clamp(Math::inverse_lerp(param_min, param_max, param_value), 0.0, 1.0);
                p_min->set_volume_db(static_cast<float>(UtilityFunctions::linear_to_db(1.0 - progress)));
                p_max->set_volume_db(static_cast<float>(UtilityFunctions::linear_to_db(progress)));
            }
        } else {
            p_min->set_volume_db(max_volume_db);
            p_max->set_volume_db(min_volume_db);
        }
    }
} // namespace godot
