#include "godot-ecs/Entity.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

void Entity::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_entity_id", "id"), &Entity::set_entity_id);
    ClassDB::bind_method(D_METHOD("get_entity_id"), &Entity::get_entity_id);
    ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "entity_id"), "set_entity_id", "get_entity_id");

    ClassDB::bind_method(D_METHOD("set_components", "components"), &Entity::set_components);
    ClassDB::bind_method(D_METHOD("get_components"), &Entity::get_components);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "components", PROPERTY_HINT_TYPE_STRING, "Component"), "set_components",
                 "get_components");

    ClassDB::bind_method(D_METHOD("add_component", "component"), &Entity::add_component);
    ClassDB::bind_method(D_METHOD("remove_component", "component"), &Entity::remove_component);
    ClassDB::bind_method(D_METHOD("find_components", "capability"), &Entity::find_components);
    ClassDB::bind_method(D_METHOD("get_component_by_index", "capability", "index"), &Entity::get_component_by_index,
                         DEFVAL(0));
    ClassDB::bind_method(D_METHOD("get_component", "capability", "instance_id"), &Entity::get_component);

    ClassDB::bind_method(D_METHOD("get_property_value", "property", "default"), &Entity::get_property, Variant());
    ClassDB::bind_method(D_METHOD("get_state"), &Entity::get_state);

    ClassDB::bind_method(D_METHOD("has_capability", "capability"), &Entity::has_capability);
    ClassDB::bind_method(D_METHOD("list_capabilities"), &Entity::list_capabilities);

    ClassDB::bind_method(D_METHOD("rebuild_property_index"), &Entity::rebuild_property_index);
}

void Entity::set_entity_id(const StringName &p_id) {
    entity_id = p_id;
}

StringName Entity::get_entity_id() const {
    return entity_id;
}

void Entity::set_components(const TypedArray<Ref<Component>> &p_components) {
    components_storage = p_components;
    components_by_capability.clear();
    property_index.clear();

    for (int i = 0; i < p_components.size(); i++) {
        const Ref<Component> comp = p_components[i];
        if (comp.is_null()) {
            continue;
        }

        comp->set_entity(this);
        const StringName capability = comp->get_capability();
        if (capability.is_empty()) {
            // allow empty capability here (editor placeholder), but skip indexing
            continue;
        }

        TypedArray<Ref<Component>> bucket;
        if (components_by_capability.has(capability)) {
            bucket = components_by_capability[capability];
        }
        const int64_t idx = bucket.size();
        bucket.push_back(comp);
        components_by_capability[capability] = bucket;
        index_component_properties(comp, capability, idx);
        comp->on_added_to_entity(this);
    }
}

TypedArray<Ref<Component>> Entity::get_components() const {
    return components_storage;
}

bool Entity::add_component(const Ref<Component> &p_component) {
    if (p_component.is_null()) {
        UtilityFunctions::push_warning("Entity::add_component called with null component");
        return false;
    }

    const StringName capability = p_component->get_capability();
    if (capability.is_empty()) {
        UtilityFunctions::push_warning("Component without capability cannot be added to Entity");
        return false;
    }

    components_storage.push_back(p_component);
    p_component->set_entity(this);
    if (!capability.is_empty()) {
        TypedArray<Ref<Component>> bucket;
        if (components_by_capability.has(capability)) {
            bucket = components_by_capability[capability];
        }
        bucket.push_back(p_component);
        components_by_capability[capability] = bucket;
        index_component_properties(p_component, capability, bucket.size() - 1);
    }
    p_component->on_added_to_entity(this);
    return true;
}

void Entity::remove_component(const Ref<Component> &p_component) {
    if (p_component.is_null()) {
        return;
    }

    const StringName capability = p_component->get_capability();
    if (!components_by_capability.has(capability)) {
        return;
    }

    TypedArray<Ref<Component>> bucket = components_by_capability[capability];
    TypedArray<Ref<Component>> filtered;
    for (int i = 0; i < bucket.size(); i++) {
        if (bucket[i] != p_component) {
            filtered.push_back(bucket[i]);
        }
    }
    components_by_capability[capability] = filtered;
    rebuild_property_index();
}

TypedArray<Ref<Component>> Entity::find_components(const StringName &p_capability) const {
    if (components_by_capability.has(p_capability)) {
        return components_by_capability[p_capability];
    }
    return TypedArray<Ref<Component>>();
}

Ref<Component> Entity::get_component_by_index(const StringName &p_capability, int64_t p_index) const {
    if (!components_by_capability.has(p_capability)) {
        return Ref<Component>();
    }
    const TypedArray<Ref<Component>> bucket = components_by_capability[p_capability];
    if (p_index < 0 || p_index >= bucket.size()) {
        return Ref<Component>();
    }
    return bucket[p_index];
}

Ref<Component> Entity::get_component(const StringName &p_capability, const StringName &p_instance_id) const {
    if (!components_by_capability.has(p_capability)) {
        return Ref<Component>();
    }
    const TypedArray<Ref<Component>> bucket = components_by_capability[p_capability];
    for (int i = 0; i < bucket.size(); i++) {
        const Ref<Component> comp = bucket[i];
        if (comp.is_null()) {
            continue;
        }
        if (comp->get_instance_id() == p_instance_id) {
            return comp;
        }
    }
    return Ref<Component>();
}

Variant Entity::get_property(const StringName &p_property, const Variant &p_default) const {
    if (!property_index.has(p_property)) {
        return p_default;
    }
    const Ref<Component> comp = property_index[p_property];
    if (comp.is_null()) {
        return p_default;
    }

    // key format: <capability>_<instance>_<prop>, so split to get prop name
    const PackedStringArray parts = String(p_property).split("_", false, 3);
    if (parts.size() < 3) {
        return p_default;
    }
    const StringName prop(parts[2]);
    return comp->get_property_value(prop);
}

Dictionary Entity::get_state() const {
    Dictionary out;
    HashMap<StringName, int64_t> capability_counters;
    for (int i = 0; i < components_storage.size(); i++) {
        const Ref<Component> comp = components_storage[i];
        if (comp.is_null()) {
            continue;
        }

        const StringName cap = comp->get_capability();
        int64_t idx = 0;
        if (capability_counters.has(cap)) {
            idx = capability_counters[cap];
        }

        comp->append_state(out, idx);
        capability_counters[cap] = idx + 1;
    }
    return out;
}

bool Entity::has_capability(const StringName &p_capability) const {
    return components_by_capability.has(p_capability) && components_by_capability[p_capability].size() > 0;
}

TypedArray<StringName> Entity::list_capabilities() const {
    TypedArray<StringName> out;
    for (const KeyValue<StringName, TypedArray<Ref<Component>>> &E : components_by_capability) {
        out.push_back(E.key);
    }
    return out;
}

void Entity::rebuild_property_index() {
    property_index.clear();
    for (const KeyValue<StringName, TypedArray<Ref<Component>>> &E : components_by_capability) {
        const StringName capability = E.key;
        const TypedArray<Ref<Component>> bucket = E.value;
        for (int i = 0; i < bucket.size(); i++) {
            index_component_properties(bucket[i], capability, i);
        }
    }
}

void Entity::index_component_properties(const Ref<Component> &p_component, const StringName &p_capability, int64_t p_idx) {
    if (p_component.is_null()) {
        return;
    }

    const StringName instance_id = p_component->get_instance_id();
    const String instance_suffix = instance_id.is_empty() ? String::num_int64(p_idx) : String(instance_id);

    const TypedArray<StringName> props = p_component->list_properties();
    for (int i = 0; i < props.size(); i++) {
        const StringName prop = props[i];
        const String key = String(p_capability) + "_" + instance_suffix + "_" + String(prop);
        property_index[StringName(key)] = p_component;
    }
}

} // namespace godot
