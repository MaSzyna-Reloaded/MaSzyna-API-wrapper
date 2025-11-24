#include "godot-ecs/EntityNode.hpp"

namespace godot {

void EntityNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_entity_id", "id"), &EntityNode::set_entity_id);
    ClassDB::bind_method(D_METHOD("get_entity_id"), &EntityNode::get_entity_id);
    ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "entity_id"), "set_entity_id", "get_entity_id");

    ClassDB::bind_method(D_METHOD("set_components", "components"), &EntityNode::set_components);
    ClassDB::bind_method(D_METHOD("get_components"), &EntityNode::get_components);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "components", PROPERTY_HINT_TYPE_STRING, "Component"), "set_components",
                 "get_components");

    ClassDB::bind_method(D_METHOD("find_components", "capability"), &EntityNode::find_components);
    ClassDB::bind_method(D_METHOD("get_component_by_index", "capability", "index"),
                         &EntityNode::get_component_by_index, DEFVAL(0));
    ClassDB::bind_method(D_METHOD("get_component", "capability", "instance_id"), &EntityNode::get_component);

    ClassDB::bind_method(D_METHOD("get_property_value", "property", "default"), &EntityNode::get_property_value,
                         Variant());
    ClassDB::bind_method(D_METHOD("get_state"), &EntityNode::get_state);
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "state", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR),
                 "", "get_state");

    ClassDB::bind_method(D_METHOD("has_capability", "capability"), &EntityNode::has_capability);
    ClassDB::bind_method(D_METHOD("list_capabilities"), &EntityNode::list_capabilities);

    ClassDB::bind_method(D_METHOD("rebuild_property_index"), &EntityNode::rebuild_property_index);
}

void EntityNode::_notification(int p_what) {
    if (p_what == NOTIFICATION_READY) {
        ensure_entity();
    }
}

void EntityNode::ensure_entity() {
    if (entity.is_null()) {
        entity.instantiate();
    }
}

void EntityNode::set_entity_id(const StringName &p_id) {
    ensure_entity();
    entity->set_entity_id(p_id);
}

StringName EntityNode::get_entity_id() const {
    return entity.is_null() ? StringName() : entity->get_entity_id();
}

void EntityNode::set_components(const TypedArray<Ref<Component>> &p_components) {
    ensure_entity();
    entity->set_components(p_components);
}

TypedArray<Ref<Component>> EntityNode::get_components() const {
    return entity.is_null() ? TypedArray<Ref<Component>>() : entity->get_components();
}

TypedArray<Ref<Component>> EntityNode::find_components(const StringName &p_capability) const {
    return entity.is_null() ? TypedArray<Ref<Component>>() : entity->find_components(p_capability);
}

Ref<Component> EntityNode::get_component_by_index(const StringName &p_capability, int64_t p_index) const {
    return entity.is_null() ? Ref<Component>() : entity->get_component_by_index(p_capability, p_index);
}

Ref<Component> EntityNode::get_component(const StringName &p_capability, const StringName &p_instance_id) const {
    return entity.is_null() ? Ref<Component>() : entity->get_component(p_capability, p_instance_id);
}

Variant EntityNode::get_property_value(const StringName &p_property, const Variant &p_default) const {
    return entity.is_null() ? p_default : entity->get_property(p_property, p_default);
}

Dictionary EntityNode::get_state() const {
    return entity.is_null() ? Dictionary() : entity->get_state();
}

bool EntityNode::has_capability(const StringName &p_capability) const {
    return !entity.is_null() && entity->has_capability(p_capability);
}

TypedArray<StringName> EntityNode::list_capabilities() const {
    return entity.is_null() ? TypedArray<StringName>() : entity->list_capabilities();
}

void EntityNode::rebuild_property_index() {
    if (entity.is_null()) {
        return;
    }
    entity->rebuild_property_index();
}

} // namespace godot
