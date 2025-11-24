#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "godot-ecs/Entity.hpp"

namespace godot {

class EntityRegistry : public RefCounted {
        GDCLASS(EntityRegistry, RefCounted)

    public:
        EntityRegistry() = default;
        ~EntityRegistry() override = default;

        bool register_entity(const Ref<Entity> &p_entity);
        void unregister_entity(const StringName &p_entity_id);
        Ref<Entity> get_entity(const StringName &p_entity_id) const;
        TypedArray<StringName> list_entities() const;

    protected:
        static void _bind_methods();

    private:
        HashMap<StringName, Ref<Entity>> entities;
};

} // namespace godot
