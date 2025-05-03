#pragma once
#include "macros.hpp"

#include <godot_cpp/classes/resource.hpp>

namespace godot {
    class LayeredSoundResource : public Resource {
            GDCLASS(LayeredSoundResource, Resource);

        protected:
            static void _bind_methods();

        public:
            TypedArray<String> starting_sounds;
            TypedArray<String> main_sounds;
            TypedArray<String> ending_sounds;

            MAKE_MEMBER_GS(Vector3, offset, Vector3(0,0,0))
            MAKE_MEMBER_GS(float, range, 0.0)
            MAKE_MEMBER_GS(float, amplitude_factor, 0.0)
            MAKE_MEMBER_GS(float, amplitude_offset, 0.0)
            MAKE_MEMBER_GS(float, frequency_factor, 0.0)
            MAKE_MEMBER_GS(float, frequency_offset, 0.0)
            MAKE_MEMBER_GS(Dictionary, sound_table, {})
            MAKE_MEMBER_GS(Dictionary, pitch_table, {})
            MAKE_MEMBER_GS(float, pitch_variation, 0.0)
            MAKE_MEMBER_GS(float, cross_fade, 0.0)
            MAKE_MEMBER_GS(bool, force_preload, false)

            int current_sound = 0;
            int current_pitch = 0;

            String get_next_sound();
            String get_sound_next_pitch();
            String get_previous_sound();
            String get_sound_previous_pitch();
            String get_sound_pitch(int property_value);
            String get_sound_file_path(int property_value);

            TypedArray<String> get_starting_sounds();
            void set_starting_sounds(const TypedArray<String> &array);
            TypedArray<String> get_main_sounds();
            void set_main_sounds(const TypedArray<String> &array);
            TypedArray<String> get_ending_sounds();
            void set_ending_sounds(const TypedArray<String> &array);
    };
} // namespace godot
