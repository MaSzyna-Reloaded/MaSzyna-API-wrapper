#pragma once
#include "TrainSwitch.hpp"


namespace godot {
    class TrainConverterSwitch : public TrainSwitch {
            GDCLASS(TrainConverterSwitch, TrainSwitch)

            static void _bind_methods();

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;

        public:
            TrainConverterSwitch();
            ~TrainConverterSwitch() override = default;
    };
} // namespace godot
