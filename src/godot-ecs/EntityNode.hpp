#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "godot-ecs/Entity.hpp"

namespace godot {

class EntityNode : public Node {
        GDCLASS(EntityNode, Node)

    public:
        EntityNode() = default;
        ~EntityNode() override = default;

        void set_entity_id(const StringName &p_id);
        StringName get_entity_id() const;

        void set_components(const TypedArray<Ref<Component>> &p_components);
        TypedArray<Ref<Component>> get_components() const;

        bool add_component(const Ref<Component> &p_component);

        TypedArray<Ref<Component>> find_components(const StringName &p_capability) const;
        Ref<Component> get_component_by_index(const StringName &p_capability, int64_t p_index = 0) const;
        Ref<Component> get_component(const StringName &p_capability, const StringName &p_instance_id) const;

        Variant get_property_value(const StringName &p_property, const Variant &p_default = Variant()) const;
        Dictionary get_state() const;

        bool has_capability(const StringName &p_capability) const;
        TypedArray<StringName> list_capabilities() const;
        void rebuild_property_index();

    protected:
        static void _bind_methods();
        void _notification(int p_what);

    private:
        Ref<Entity> entity;

        void ensure_entity();
};

} // namespace godot
