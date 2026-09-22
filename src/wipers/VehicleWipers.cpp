#include "VehicleWipers.hpp"
#include <algorithm>

namespace godot {
    void VehicleWipers::_bind_methods() {
        ClassDB::bind_method(D_METHOD("switch_increase"), &VehicleWipers::switch_increase);
        ClassDB::bind_method(D_METHOD("switch_decrease"), &VehicleWipers::switch_decrease);
        BIND_PROPERTY(VehicleWipers, Variant::FLOAT, angle);
        BIND_PROPERTY(VehicleWipers, Variant::INT, default_position);
        BIND_PROPERTY(VehicleWipers, Variant::INT, wiper_count);
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleWipers, Variant::ARRAY, positions, PROPERTY_HINT_TYPE_STRING, "WiperListItem");

        ClassDB::bind_method(D_METHOD("get_switch_position"), &VehicleWipers::get_switch_position);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "switch_position", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_switch_position");
        ClassDB::bind_method(D_METHOD("get_sweep_positions"), &VehicleWipers::get_sweep_positions);
        ADD_PROPERTY(
                PropertyInfo(Variant::PACKED_FLOAT64_ARRAY, "sweep_positions", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_sweep_positions");
    }

    void VehicleWipers::_register_commands() {
        register_command("wipers_switch_increase", Callable(this, "switch_increase"));
        register_command("wipers_switch_decrease", Callable(this, "switch_decrease"));
    }

    void VehicleWipers::_unregister_commands() {
        unregister_command("wipers_switch_increase", Callable(this, "switch_increase"));
        unregister_command("wipers_switch_decrease", Callable(this, "switch_decrease"));
    }
} // namespace godot
