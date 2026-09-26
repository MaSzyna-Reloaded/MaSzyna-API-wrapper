#include "VehicleLoad.hpp"

namespace godot {
    void VehicleLoad::_bind_methods() {
        BIND_PROPERTY_W_HINT_RES_ARRAY(VehicleLoad, Variant::ARRAY, load_list, PROPERTY_HINT_ARRAY_TYPE, "LoadListItem")
        BIND_PROPERTY_W_HINT(VehicleLoad, Variant::INT, load_unit, PROPERTY_HINT_ENUM, "Tons,Pieces");
        BIND_PROPERTY(VehicleLoad, Variant::FLOAT, overload_factor);
        BIND_PROPERTY(VehicleLoad, Variant::FLOAT, load_speed);
        BIND_PROPERTY(VehicleLoad, Variant::FLOAT, unload_speed);
        BIND_PROPERTY(VehicleLoad, Variant::FLOAT, max_load);
        BIND_PROPERTY_ARRAY(VehicleLoad, minimum_load_offsets);
        BIND_PROPERTY_ARRAY(VehicleLoad, accepted_loads);
        BIND_ENUM_CONSTANT(LOAD_UNIT_TONS);
        BIND_ENUM_CONSTANT(LOAD_UNIT_PIECES);
    }

    void VehicleLoad::_register_commands() {
        VehicleComponent::_register_commands();
    }

    void VehicleLoad::_unregister_commands() {
        VehicleComponent::_unregister_commands();
    }
} // namespace godot
