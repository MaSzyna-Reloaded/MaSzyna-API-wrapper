// state_proxy.hpp
#ifndef STATE_PROXY_HPP
#define STATE_PROXY_HPP

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

    class StateProxy : public Object {
            GDCLASS(StateProxy, Object);

        private:
            HashMap<StringName, Object *> _objects;
            HashMap<StringName, StringName> _property_names;

        protected:
            static void _bind_methods();

        public:
            StateProxy() = default;
            ~StateProxy() = default;

            void register_property(Object *node, const StringName &property_name, const StringName &exposed_name = "");
            Variant get(const StringName &property, const Variant &default_value = Variant()) const;
            bool _get(const StringName &property, Variant &r_value) const; // Correct signature
            Variant _get(const StringName &property) const;                // For Godot property binding
            Dictionary get_all() const;
    };
} // namespace godot

#endif // STATE_PROXY_HPP
