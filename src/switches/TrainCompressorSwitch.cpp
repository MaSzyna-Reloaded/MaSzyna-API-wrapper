#include <godot_cpp/classes/node.hpp>

#include "../maszyna/McZapkie/MOVER.h"
#include "TrainCompressorSwitch.hpp"

namespace godot {
    TrainCompressorSwitch::TrainCompressorSwitch() = default;

    void TrainCompressorSwitch::_bind_methods() {}

    void TrainCompressorSwitch::_do_update_internal_mover(TMoverParameters *mover) {
        mover->CompressorSwitch(get_pushed());
    };

    void TrainCompressorSwitch::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        state["enabled"] = mover->CompressorFlag;
        state["allowed"] = mover->CompressorAllow;
    };

} // namespace godot
