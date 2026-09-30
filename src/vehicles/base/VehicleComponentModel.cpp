#include "VehicleComponentModel.hpp"

namespace godot {
    void VehicleComponentModel::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("set_implementation", "implementation"), &VehicleComponentModel::set_implementation);
        ClassDB::bind_method(D_METHOD("get_implementation"), &VehicleComponentModel::get_implementation);
        ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "implementation"), "set_implementation", "get_implementation");

        ClassDB::bind_method(D_METHOD("set_properties", "properties"), &VehicleComponentModel::set_properties);
        ClassDB::bind_method(D_METHOD("get_properties"), &VehicleComponentModel::get_properties);
        ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "properties"), "set_properties", "get_properties");
    }

    void VehicleComponentModel::set_implementation(const StringName &p_implementation) {
        implementation = p_implementation;
    }

    StringName VehicleComponentModel::get_implementation() const {
        return implementation;
    }

    void VehicleComponentModel::set_properties(const Dictionary &p_properties) {
        properties = p_properties;
    }

    Dictionary VehicleComponentModel::get_properties() const {
        return properties;
    }
} // namespace godot
