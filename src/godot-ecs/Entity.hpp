#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "godot-ecs/Component.hpp"

namespace godot {

class Entity : public RefCounted {
        GDCLASS(Entity, RefCounted)

    public:
        Entity() = default;
        ~Entity() override = default;

        void set_entity_id(const StringName &p_id);
        StringName get_entity_id() const;

        void set_components(const TypedArray<Ref<Component>> &p_components);
        TypedArray<Ref<Component>> get_components() const;

        bool add_component(const Ref<Component> &p_component);
        void remove_component(const Ref<Component> &p_component);

        TypedArray<Ref<Component>> find_components(const StringName &p_capability) const;
        Ref<Component> get_component_by_index(const StringName &p_capability, int64_t p_index = 0) const;
        Ref<Component> get_component(const StringName &p_capability, const StringName &p_instance_id) const;

        Variant get_property(const StringName &p_property, const Variant &p_default = Variant()) const;
        Dictionary get_state() const;

        bool has_capability(const StringName &p_capability) const;
        TypedArray<StringName> list_capabilities() const;
        void rebuild_property_index();

    protected:
        static void _bind_methods();

    private:
        StringName entity_id;
        TypedArray<Ref<Component>> components_storage;
        // capability -> list of components
        HashMap<StringName, TypedArray<Ref<Component>>> components_by_capability;
        // fully qualified property key -> component
        HashMap<StringName, Ref<Component>> property_index;

        void index_component_properties(const Ref<Component> &p_component, const StringName &p_capability, int64_t p_idx);
};

} // namespace godot
