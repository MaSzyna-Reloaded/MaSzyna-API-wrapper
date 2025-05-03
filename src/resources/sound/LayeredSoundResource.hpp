#pragma once
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
            double range;
            struct {
                    double factor = 1.0;
                    double offset = 0;
            } amplitude, frequency;

            Vector3 offset;
            int current_sound = 0;
            int current_pitch = 0;
            Dictionary sound_table;
            Dictionary pitch_table;
            double pitch_variation;
            double cross_fade;
            bool force_preload = true;

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
            double get_range();
            void set_range(double value);
            double get_amplitude_factor();
            void set_amplitude_factor(double value);
            double get_amplitude_offset();
            void set_amplitude_offset(double value);
            double get_frequency_factor();
            void set_frequency_factor(double value);
            double get_frequency_offset();
            void set_frequency_offset(double value);
            Dictionary get_sound_table();
            void set_sound_table(const Dictionary &array);
            Dictionary get_pitch_table();
            void set_pitch_table(const Dictionary &array);
            double get_pitch_variation();
            void set_pitch_variation(double value);
            double get_cross_fade();
            void set_cross_fade(double value);
            Vector3 get_offset();
            void set_offset(Vector3 p_offset);
            void set_force_preload(bool p_force_preload);
            bool get_force_preload();
    };
} // namespace godot
