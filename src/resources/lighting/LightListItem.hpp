#pragma once
#include <godot_cpp/classes/resource.hpp>

namespace godot {
    class LightListItem : public Resource {
            GDCLASS(LightListItem, Resource);

        public:
            static void _bind_methods();
            void set_cabin_a_head_light(bool p_head_light);
            bool get_cabin_a_head_light() const;
            void set_cabin_a_left_white_signal(bool p_left_white_signal);
            bool get_cabin_a_left_white_signal() const;
            void set_cabin_a_left_red_signal(bool p_left_red_signal);
            bool get_cabin_a_left_red_signal() const;
            void set_cabin_a_right_white_signal(bool p_right_white_signal);
            bool get_cabin_a_right_white_signal() const;
            void set_cabin_a_right_red_signal(bool p_right_red_signal);
            bool get_cabin_a_right_red_signal() const;
            void set_cabin_b_head_light(bool p_head_light);
            bool get_cabin_b_head_light() const;
            void set_cabin_b_left_white_signal(bool p_left_white_signal);
            bool get_cabin_b_left_white_signal() const;
            void set_cabin_b_left_red_signal(bool p_left_red_signal);
            bool get_cabin_b_left_red_signal() const;
            void set_cabin_b_right_white_signal(bool p_right_white_signal);
            bool get_cabin_b_right_white_signal() const;
            void set_cabin_b_right_red_signal(bool p_right_red_signal);
            bool get_cabin_b_right_red_signal() const;

            struct CabinLights {
                bool head_light = false;
                bool left_white_signal = false;
                bool left_red_signal = false;
                bool right_white_signal = false;
                bool right_red_signal = false;
            } cabin_a, cabin_b;

    };
} // namespace godot
