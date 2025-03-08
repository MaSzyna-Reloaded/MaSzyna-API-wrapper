#pragma once
#include <godot_cpp/classes/resource.hpp>

namespace godot {
    class LightListItem : public Resource {
            GDCLASS(LightListItem, Resource);

        public:
            static void _bind_methods();
            bool head_light = false;
            bool left_white_signal = false;
            bool left_red_signal = false;
            bool right_white_signal = false;
            bool right_red_signal = false;

            void set_head_light(bool p_head_light);
            bool get_head_light() const;
            void set_left_white_signal(bool p_left_white_signal);
            bool get_left_white_signal() const;
            void set_left_red_signal(bool p_left_red_signal);
            bool get_left_red_signal() const;
            void set_right_white_signal(bool p_right_white_signal);
            bool get_right_white_signal() const;
            void set_right_red_signal(bool p_right_red_signal);
            bool get_right_red_signal() const;
    };
} // namespace godot
