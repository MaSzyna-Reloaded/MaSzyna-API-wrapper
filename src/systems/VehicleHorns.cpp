#include "VehicleHorns.hpp"
#include "maszyna/utilities.h"

namespace godot {
    void VehicleHorns::_bind_methods() {
        BIND_PROPERTY(VehicleHorns, Variant::BOOL, low_horn_enabled);
        BIND_PROPERTY(VehicleHorns, Variant::BOOL, high_horn_enabled);
        BIND_PROPERTY(VehicleHorns, Variant::BOOL, whistle_enabled);
        ClassDB::bind_method(D_METHOD("set_horn_low", "state"), &VehicleHorns::set_horn_low);
        ClassDB::bind_method(D_METHOD("set_horn_high", "state"), &VehicleHorns::set_horn_high);
        ClassDB::bind_method(D_METHOD("set_whistle", "state"), &VehicleHorns::set_whistle);
        ClassDB::bind_method(D_METHOD("set_horn", "position"), &VehicleHorns::set_horn);

        ClassDB::bind_method(D_METHOD("get_low_pressed"), &VehicleHorns::get_low_pressed);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "low_pressed", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_low_pressed");
        ClassDB::bind_method(D_METHOD("get_high_pressed"), &VehicleHorns::get_high_pressed);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "high_pressed", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_high_pressed");
        ClassDB::bind_method(D_METHOD("get_whistle_pressed"), &VehicleHorns::get_whistle_pressed);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "whistle_pressed", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_whistle_pressed");
        ClassDB::bind_method(D_METHOD("get_combined_signal"), &VehicleHorns::get_combined_signal);
        ClassDB::bind_method(D_METHOD("get_low_active"), &VehicleHorns::get_low_active);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "low_active", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_low_active");
        ClassDB::bind_method(D_METHOD("get_high_active"), &VehicleHorns::get_high_active);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "high_active", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_high_active");
        ClassDB::bind_method(D_METHOD("get_whistle_active"), &VehicleHorns::get_whistle_active);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "whistle_active", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_whistle_active");
        ClassDB::bind_method(D_METHOD("get_horn"), &VehicleHorns::get_horn);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "horn", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_horn");
    }

    void VehicleHorns::_register_commands() {
        register_command("horn_low", Callable(this, "set_horn_low"));
        register_command("horn_high", Callable(this, "set_horn_high"));
        register_command("whistle", Callable(this, "set_whistle"));
        register_command("horn", Callable(this, "set_horn"));
    }

    void VehicleHorns::_unregister_commands() {
        unregister_command("horn_low", Callable(this, "set_horn_low"));
        unregister_command("horn_high", Callable(this, "set_horn_high"));
        unregister_command("whistle", Callable(this, "set_whistle"));
        unregister_command("horn", Callable(this, "set_horn"));
    }
} // namespace godot
