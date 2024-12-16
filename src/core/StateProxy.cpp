#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include "StateProxy.hpp"

using namespace godot;

void StateProxy::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("register_property", "node", "property_name", "exposed_name"), &StateProxy::register_property);
    ClassDB::bind_method(D_METHOD("get", "property", "default_value"), &StateProxy::get);
    ClassDB::bind_method(D_METHOD("get_all"), &StateProxy::get_all);
}

void StateProxy::register_property(Object *node, const StringName &property_name, const StringName &exposed_name) {
    StringName key = exposed_name.is_empty() ? property_name : exposed_name;
    if (!_objects.has(key)) {
        _objects[key] = node;
        _property_names[key] = property_name;
    }
}

Variant StateProxy::get(const StringName &property, const Variant &default_value) const {
    if (_objects.has(property)) {
        Object *obj = _objects[property];
        if (obj) {
            Variant ret = obj->get(_property_names[property]);
            if (ret.get_type() != Variant::Type::NIL) {
                return ret;
            }
        }
    }
    return default_value;
}

bool StateProxy::_get(const StringName &property, Variant &r_value) const {
    if (_objects.has(property)) {
        Object *obj = _objects[property];
        if (obj) {
            r_value = obj->get(_property_names[property]);
            return true;
        }
    }
    return false;
}

Variant StateProxy::_get(const StringName &property) const {
    Variant value;
    if (_get(property, value)) {
        return value;
    }
    return Variant(); // Return nil if property doesn't exist
}

Dictionary StateProxy::get_all() const {
    Dictionary out;
    for (const KeyValue<StringName, Object *> &entry: _objects) {
        Object *obj = entry.value;
        StringName prop_name = _property_names[entry.key];
        out[entry.key] = obj->get(prop_name);
    }
    return out;
}
