#include "godot-ecs/EntityRegistry.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

void EntityRegistry::_bind_methods() {
    ClassDB::bind_method(D_METHOD("register_entity", "entity"), &EntityRegistry::register_entity);
    ClassDB::bind_method(D_METHOD("unregister_entity", "entity_id"), &EntityRegistry::unregister_entity);
    ClassDB::bind_method(D_METHOD("get_entity", "entity_id"), &EntityRegistry::get_entity);
    ClassDB::bind_method(D_METHOD("list_entities"), &EntityRegistry::list_entities);
}

bool EntityRegistry::register_entity(const Ref<Entity> &p_entity) {
    if (p_entity.is_null()) {
        UtilityFunctions::push_warning("EntityRegistry::register_entity called with null entity");
        return false;
    }

    const StringName id = p_entity->get_entity_id();
    if (id.is_empty()) {
        UtilityFunctions::push_warning("Cannot register entity without id");
        return false;
    }

    entities[id] = p_entity;
    return true;
}

void EntityRegistry::unregister_entity(const StringName &p_entity_id) {
    entities.erase(p_entity_id);
}

Ref<Entity> EntityRegistry::get_entity(const StringName &p_entity_id) const {
    if (!entities.has(p_entity_id)) {
        return Ref<Entity>();
    }
    return entities[p_entity_id];
}

TypedArray<StringName> EntityRegistry::list_entities() const {
    TypedArray<StringName> out;
    for (const KeyValue<StringName, Ref<Entity>> &E : entities) {
        out.push_back(E.key);
    }
    return out;
}

} // namespace godot
