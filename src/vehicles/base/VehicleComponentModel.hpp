#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    /* One component of a parsed vehicle: which implementation answers for it, and the
     * configuration it was authored with.
     *
     * The configuration is a property name -> value map on purpose. Those names are Godot's own
     * property system, which is the authoring source of truth; storing them is serialising, not
     * inventing a second description of the same thing. */
    class VehicleComponentModel : public Resource {
            GDCLASS(VehicleComponentModel, Resource)

        private:
            StringName implementation;
            Dictionary properties;

        protected:
            static void _bind_methods();

        public:
            /* The class that answers for this kind on the simulation the vehicle runs on */
            void set_implementation(const StringName &p_implementation);
            StringName get_implementation() const;

            void set_properties(const Dictionary &p_properties);
            Dictionary get_properties() const;
    };
} // namespace godot
