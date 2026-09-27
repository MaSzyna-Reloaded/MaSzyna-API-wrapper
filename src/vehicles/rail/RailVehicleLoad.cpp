#include "RailVehicleLoad.hpp"

namespace godot {
    void RailVehicleLoad::_bind_methods() {
        BIND_PROPERTY_W_HINT_RES_ARRAY(RailVehicleLoad, Variant::ARRAY, load_list, PROPERTY_HINT_ARRAY_TYPE, "RailVehicleLoadListItem")
        BIND_PROPERTY_W_HINT(RailVehicleLoad, Variant::INT, load_unit, PROPERTY_HINT_ENUM, "Tons,Pieces");
        BIND_PROPERTY(RailVehicleLoad, Variant::FLOAT, overload_factor);
        BIND_PROPERTY(RailVehicleLoad, Variant::FLOAT, load_speed);
        BIND_PROPERTY(RailVehicleLoad, Variant::FLOAT, unload_speed);
        BIND_PROPERTY(RailVehicleLoad, Variant::FLOAT, max_load);
        BIND_PROPERTY_ARRAY(RailVehicleLoad, minimum_load_offsets);
        BIND_PROPERTY_ARRAY(RailVehicleLoad, accepted_loads);
        BIND_ENUM_CONSTANT(LOAD_UNIT_TONS);
        BIND_ENUM_CONSTANT(LOAD_UNIT_PIECES);
    }

    void RailVehicleLoad::_register_commands() {
        VehicleComponent::_register_commands();
    }

    void RailVehicleLoad::_unregister_commands() {
        VehicleComponent::_unregister_commands();
    }
} // namespace godot
