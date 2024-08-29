#pragma once
#include "TrainPart.hpp"


namespace godot {
    class TrainSwitch : public TrainPart {
            GDCLASS(TrainSwitch, TrainPart)

            static void _bind_methods();
            bool pushed = false;

        public:
            void set_pushed(bool p_state);
            bool get_pushed() const;

            TrainSwitch();
            ~TrainSwitch() override = default;
    };
} // namespace godot
