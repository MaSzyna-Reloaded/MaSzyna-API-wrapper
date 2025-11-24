#include "godot-ecs/Component.hpp"

#include "godot-ecs/Entity.hpp"

#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/variant/array.hpp>

namespace godot {

void Component::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_capability"), &Component::get_capability);
    ClassDB::bind_method(D_METHOD("set_capability", "capability"), &Component::set_capability);
    ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "capability"), "set_capability", "get_capability");

    ClassDB::bind_method(D_METHOD("get_type_id"), &Component::get_type_id);
    ClassDB::bind_method(D_METHOD("set_type_id", "type_id"), &Component::set_type_id);
    ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "type_id"), "set_type_id", "get_type_id");

    ClassDB::bind_method(D_METHOD("get_instance_id"), &Component::get_instance_id);
    ClassDB::bind_method(D_METHOD("set_instance_id", "instance_id"), &Component::set_instance_id);
    ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "instance_id"), "set_instance_id", "get_instance_id");

    ClassDB::bind_method(D_METHOD("get_entity"), &Component::get_entity);

    ClassDB::bind_method(D_METHOD("list_properties"), &Component::list_properties);
    ClassDB::bind_method(D_METHOD("get_property_value", "property"), &Component::get_property_value);
    ClassDB::bind_method(D_METHOD("get_state"), &Component::get_state);
    // append_state is internal; no binding
    ClassDB::bind_method(D_METHOD("on_added_to_entity", "entity"), &Component::on_added_to_entity);
}

void Component::set_capability(const StringName &p_capability) {
    capability = p_capability;
}

StringName Component::get_capability() const {
    return capability;
}

void Component::set_type_id(const StringName &p_type_id) {
    type_id = p_type_id;
}

StringName Component::get_type_id() const {
    return type_id;
}

void Component::set_instance_id(const StringName &p_instance_id) {
    instance_id = p_instance_id;
}

StringName Component::get_instance_id() const {
    return instance_id;
}

void Component::set_entity(Entity *p_entity) {
    entity = p_entity;
}

Entity *Component::get_entity() const {
    return entity;
}

TypedArray<StringName> Component::list_properties() const {
    TypedArray<StringName> out;
    const Array props = get_property_list();

    static const HashSet<StringName> skip = {
        StringName("capability"),
        StringName("type_id"),
        StringName("instance_id"),
        StringName("script"),
        StringName("resource_path"),
        StringName("resource_local_to_scene"),
        StringName("resource_name"),
        StringName("resource_scene_unique_id"),
    };

    for (int i = 0; i < props.size(); i++) {
        const Dictionary d = props[i];
        if (!d.has("name")) {
            continue;
        }
        const StringName name = d["name"];
        if (skip.has(name)) {
            continue;
        }

        // Skip metadata/internal entries like "metadata/_custom_type_script"
        if (String(name).contains("/")) {
            continue;
        }

        const int usage = d.get("usage", PROPERTY_USAGE_DEFAULT);
        if ((usage & PROPERTY_USAGE_STORAGE) == 0 && (usage & PROPERTY_USAGE_SCRIPT_VARIABLE) == 0) {
            continue;
        }

        out.push_back(name);
    }
    return out;
}

Variant Component::get_property_value(const StringName &p_property) const {
    return get(p_property);
}

Dictionary Component::get_state() const {
    Dictionary out;
    const TypedArray<StringName> props = list_properties();
    for (int i = 0; i < props.size(); i++) {
        const StringName prop = props[i];
        out[prop] = get_property_value(prop);
    }
    return out;
}

void Component::append_state(Dictionary &p_out, const int64_t p_fallback_index) const {
    const TypedArray<StringName> props = list_properties();
    const String cap = String(get_capability());
    const String inst = get_instance_id().is_empty() && p_fallback_index >= 0 ? String::num_int64(p_fallback_index)
                                                                             : String(get_instance_id());
    const String prefix = cap + String("_") + inst + String("_"); // <capability>_<instance>_

    for (int i = 0; i < props.size(); i++) {
        const StringName prop = props[i];
        const String key = prefix + String(prop);
        p_out[StringName(key)] = get_property_value(prop);
    }
}

void Component::on_added_to_entity(Entity *) {
}

} // namespace godot
