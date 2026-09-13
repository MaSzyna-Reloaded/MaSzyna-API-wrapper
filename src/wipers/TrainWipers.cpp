#include "TrainWipers.hpp"

namespace godot {
    void TrainWipers::_bind_methods() {
        BIND_PROPERTY(TrainWipers, Variant::FLOAT, angle);
        BIND_PROPERTY(TrainWipers, Variant::INT, default_position);
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                TrainWipers, Variant::ARRAY, positions, PROPERTY_HINT_TYPE_STRING, "WiperListItem");
    }

    void TrainWipers::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        // Not wired to the mover: see the class-level note in TrainWipers.hpp.
    }
} // namespace godot
