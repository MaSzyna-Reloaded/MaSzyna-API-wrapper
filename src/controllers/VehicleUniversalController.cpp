#include "VehicleUniversalController.hpp"
#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void VehicleUniversalController::_bind_methods() {
        BIND_PROPERTY(VehicleUniversalController, Variant::BOOL, integrated_brake_pn);
        BIND_PROPERTY(VehicleUniversalController, Variant::BOOL, integrated_brake);
        BIND_PROPERTY(VehicleUniversalController, Variant::INT, selector_position);
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleUniversalController, Variant::ARRAY, positions, PROPERTY_HINT_TYPE_STRING,
                "UniversalControllerListItem");
    }
} // namespace godot
