#include <godot_cpp/classes/node.hpp>

#include "../maszyna/McZapkie/MOVER.h"
#include "TrainConverterSwitch.hpp"

namespace godot {
    TrainConverterSwitch::TrainConverterSwitch() = default;

    void TrainConverterSwitch::_bind_methods() {}

    void TrainConverterSwitch::_do_update_internal_mover(TMoverParameters *mover) {
        mover->CompressorSwitch(get_pushed());
    };
    void TrainConverterSwitch::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        state["enabled"] = mover->ConverterFlag;
        state["allowed"] = mover->ConverterAllow;
        state["time_to_start"] = mover->ConverterStartDelayTimer;
    };

} // namespace godot
