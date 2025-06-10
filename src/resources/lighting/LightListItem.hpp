#pragma once
#include "macros.hpp"
#include <godot_cpp/classes/resource.hpp>

namespace godot {
    class LightListItem : public Resource {
            GDCLASS(LightListItem, Resource);

        public:
            static void _bind_methods();
            MAKE_MEMBER_GS(bool, cab_a_head_light, false);
            MAKE_MEMBER_GS(bool, cab_a_left_white_signal, false);
            MAKE_MEMBER_GS(bool, cab_a_left_red_signal, false);
            MAKE_MEMBER_GS(bool, cab_a_right_white_signal, false);
            MAKE_MEMBER_GS(bool, cab_a_right_red_signal, false);
            MAKE_MEMBER_GS(bool, cab_b_head_light, false);
            MAKE_MEMBER_GS(bool, cab_b_left_white_signal, false);
            MAKE_MEMBER_GS(bool, cab_b_left_red_signal, false);
            MAKE_MEMBER_GS(bool, cab_b_right_white_signal, false);
            MAKE_MEMBER_GS(bool, cab_b_right_red_signal, false);
    };
} // namespace godot
