#include "VehicleModel.hpp"

namespace godot {
    void VehicleModel::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_properties", "properties"), &VehicleModel::set_properties);
        ClassDB::bind_method(D_METHOD("get_properties"), &VehicleModel::get_properties);
        ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "properties"), "set_properties", "get_properties");

        ClassDB::bind_method(D_METHOD("set_components", "components"), &VehicleModel::set_components);
        ClassDB::bind_method(D_METHOD("get_components"), &VehicleModel::get_components);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::ARRAY, "components", PROPERTY_HINT_ARRAY_TYPE,
                        vformat("%s/%s:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, "VehicleComponentModel")),
                "set_components", "get_components");

        ClassDB::bind_static_method("VehicleModel", D_METHOD("capture", "object"), &VehicleModel::capture);
        ClassDB::bind_static_method("VehicleModel", D_METHOD("apply", "object", "properties"), &VehicleModel::apply);
        BIND_CONSTANT(FORMAT_VERSION);
    }

    void VehicleModel::set_properties(const Dictionary &p_properties) {
        properties = p_properties;
    }

    Dictionary VehicleModel::get_properties() const {
        return properties;
    }

    void VehicleModel::set_components(const TypedArray<VehicleComponentModel> &p_components) {
        components = p_components;
    }

    TypedArray<VehicleComponentModel> VehicleModel::get_components() const {
        return components;
    }

    Dictionary VehicleModel::capture(Object *p_object) {
        Dictionary captured;
        ERR_FAIL_NULL_V(p_object, captured);
        const TypedArray<Dictionary> property_list = p_object->get_property_list();
        for (int i = 0; i < property_list.size(); i++) {
            const Dictionary property = property_list[i];
            if ((static_cast<int>(property["usage"]) & PROPERTY_USAGE_STORAGE) == 0) {
                continue;
            }
            const String name = property["name"];
            if (name == "script") {
                continue;
            }
            captured[name] = p_object->get(name);
        }
        return captured;
    }

    void VehicleModel::apply(Object *p_object, const Dictionary &p_properties) {
        ERR_FAIL_NULL(p_object);
        const Array names = p_properties.keys();
        for (int i = 0; i < names.size(); i++) {
            p_object->set(names[i], p_properties[names[i]]);
        }
    }
} // namespace godot
