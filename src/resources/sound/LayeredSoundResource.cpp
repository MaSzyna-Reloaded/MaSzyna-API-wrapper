#include "LayeredSoundResource.hpp"
namespace godot {

    void LayeredSoundResource::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_starting_sounds"), &LayeredSoundResource::get_starting_sounds);
        ClassDB::bind_method(D_METHOD("set_starting_sounds", "starting_sounds"), &LayeredSoundResource::set_starting_sounds);
        ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "starting_sounds"), "set_starting_sounds", "get_starting_sounds");

        ClassDB::bind_method(D_METHOD("get_main_sounds"), &LayeredSoundResource::get_main_sounds);
        ClassDB::bind_method(D_METHOD("set_main_sounds", "main_sounds"), &LayeredSoundResource::set_main_sounds);
        ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "main_sounds"), "set_main_sounds", "get_main_sounds");

        ClassDB::bind_method(D_METHOD("get_ending_sounds"), &LayeredSoundResource::get_ending_sounds);
        ClassDB::bind_method(D_METHOD("set_ending_sounds", "Ending_sounds"), &LayeredSoundResource::set_ending_sounds);
        ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "ending_sounds"), "set_ending_sounds", "get_ending_sounds");

        ClassDB::bind_method(D_METHOD("get_range"), &LayeredSoundResource::get_range);
        ClassDB::bind_method(D_METHOD("set_range"), &LayeredSoundResource::set_range);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "range"), "set_range", "get_range");

        ClassDB::bind_method(D_METHOD("get_amplitude_factor"), &LayeredSoundResource::get_amplitude_factor);
        ClassDB::bind_method(D_METHOD("set_amplitude_factor", "amplitude_factor"), &LayeredSoundResource::set_amplitude_factor);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "amplitude_factor"), "set_amplitude_factor", "get_amplitude_factor");

        ClassDB::bind_method(D_METHOD("get_amplitude_offset"), &LayeredSoundResource::get_amplitude_offset);
        ClassDB::bind_method(D_METHOD("set_amplitude_offset", "amplitude_offset"), &LayeredSoundResource::set_amplitude_offset);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "amplitude_offset"), "set_amplitude_offset", "get_amplitude_offset");

        ClassDB::bind_method(D_METHOD("get_frequency_factor"), &LayeredSoundResource::get_frequency_factor);
        ClassDB::bind_method(D_METHOD("set_frequency_factor", "frequency_factor"), &LayeredSoundResource::set_frequency_factor);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "frequency_factor"), "set_frequency_factor", "get_frequency_factor");

        ClassDB::bind_method(D_METHOD("get_frequency_offset"), &LayeredSoundResource::get_frequency_offset);
        ClassDB::bind_method(D_METHOD("set_frequency_offset", "frequency_offset"), &LayeredSoundResource::set_frequency_offset);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "frequency_offset"), "set_frequency_offset", "get_frequency_offset");

        ClassDB::bind_method(D_METHOD("get_sound_table"), &LayeredSoundResource::get_sound_table);
        ClassDB::bind_method(D_METHOD("set_sound_table", "sound_table"), &LayeredSoundResource::set_sound_table);
        ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "sound_table"), "set_sound_table", "get_sound_table");

        ClassDB::bind_method(D_METHOD("get_pitch_table"), &LayeredSoundResource::get_pitch_table);
        ClassDB::bind_method(D_METHOD("set_pitch_table", "pitch_table"), &LayeredSoundResource::set_pitch_table);
        ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "pitch_table"), "set_pitch_table", "get_pitch_table");

        ClassDB::bind_method(D_METHOD("get_pitch_variation"), &LayeredSoundResource::get_pitch_variation);
        ClassDB::bind_method(D_METHOD("set_pitch_variation", "pitch_variation"), &LayeredSoundResource::set_pitch_variation);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "pitch_variation"), "set_pitch_variation", "get_pitch_variation");

        ClassDB::bind_method(D_METHOD("get_cross_fade"), &LayeredSoundResource::get_cross_fade);
        ClassDB::bind_method(D_METHOD("set_cross_fade", "cross_fade"), &LayeredSoundResource::set_cross_fade);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cross_fade"), "set_cross_fade", "get_cross_fade");

        ClassDB::bind_method(D_METHOD("set_offset", "offset"), &LayeredSoundResource::set_offset);
        ClassDB::bind_method(D_METHOD("get_offset"), &LayeredSoundResource::get_offset);
        ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "offset"), "set_offset", "get_offset");

        ClassDB::bind_method(D_METHOD("get_next_sound"), &LayeredSoundResource::get_next_sound);
        ClassDB::bind_method(D_METHOD("get_sound_next_pitch"), &LayeredSoundResource::get_sound_next_pitch);
        ClassDB::bind_method(D_METHOD("get_previous_sound"), &LayeredSoundResource::get_previous_sound);
        ClassDB::bind_method(D_METHOD("get_sound_previous_pitch"), &LayeredSoundResource::get_sound_previous_pitch);
        ClassDB::bind_method(D_METHOD("get_sound_pitch", "pitch_table_index"), &LayeredSoundResource::get_sound_pitch);
        ClassDB::bind_method(D_METHOD("get_sound_file_path", "sound_table_index"), &LayeredSoundResource::get_sound_file_path);
        ClassDB::bind_method(D_METHOD("set_force_preload", "force_preload"), &LayeredSoundResource::set_force_preload);
        ClassDB::bind_method(D_METHOD("get_force_preload"), &LayeredSoundResource::get_force_preload);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "force_preload"), "set_force_preload", "get_force_preload");
    }
    TypedArray<String> LayeredSoundResource::get_starting_sounds() {
        return starting_sounds;
    }
    void LayeredSoundResource::set_starting_sounds(const TypedArray<String> &array) {
        starting_sounds = array;
    }
    TypedArray<String> LayeredSoundResource::get_main_sounds() {
        return main_sounds;
    }
    void LayeredSoundResource::set_main_sounds(const TypedArray<String> &array) {
        main_sounds = array;
    }
    TypedArray<String> LayeredSoundResource::get_ending_sounds() {
        return ending_sounds;
    }
    void LayeredSoundResource::set_ending_sounds(const TypedArray<String> &array) {
        ending_sounds = array;
    }
    String LayeredSoundResource::get_next_sound() {
        return sound_table[current_sound++];
    }
    String LayeredSoundResource::get_sound_next_pitch() {
        return pitch_table[current_pitch++];
    }
    String LayeredSoundResource::get_previous_sound() {
        return sound_table[current_sound--];
    }
    String LayeredSoundResource::get_sound_previous_pitch() {
        return pitch_table[current_pitch--];
    }
    String LayeredSoundResource::get_sound_pitch(const int property_value) {
        return pitch_table[property_value];
    }
    String LayeredSoundResource::get_sound_file_path(const int property_value) {
        return sound_table[property_value];
    }
} // namespace godot
