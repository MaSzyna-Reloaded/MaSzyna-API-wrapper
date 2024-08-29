#pragma once
#include "TrainSwitch.hpp"


namespace godot {
    class TrainCompressorSwitch : public TrainSwitch {
            GDCLASS(TrainCompressorSwitch, TrainSwitch)

            static void _bind_methods();

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;

        public:
            TrainCompressorSwitch();
            ~TrainCompressorSwitch() override = default;
    };
} // namespace godot
