#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
class Entity;

class Component : public Resource {
        GDCLASS(Component, Resource)

    public:
        Component() = default;
        ~Component() override = default;

        void set_capability(const StringName &p_capability);
        StringName get_capability() const;

        void set_type_id(const StringName &p_type_id);
        StringName get_type_id() const;

        void set_instance_id(const StringName &p_instance_id);
        StringName get_instance_id() const;

        void set_entity(Entity *p_entity);
        Entity *get_entity() const;

        virtual TypedArray<StringName> list_properties() const;
        virtual Variant get_property_value(const StringName &p_property) const;
        Dictionary get_state() const;
        // Not exposed to Godot; used by Entity
        virtual void append_state(Dictionary &p_out, int64_t p_fallback_index = -1) const;
        virtual void on_added_to_entity(Entity *p_entity);

    protected:
        static void _bind_methods();

    private:
        StringName capability;
        StringName type_id;
        StringName instance_id;
        Entity *entity{nullptr};
};
} // namespace godot
