#pragma once

#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {
    /* One thing a vehicle can be asked about.
     *
     * A component declares its properties once, by name, and everything downstream addresses them
     * by the id this hands back. The name is resolved to that id once - at registration, where a
     * sound bank or a cab widget is built - and never again per frame. */
    struct VehiclePropertyDescriptor {
            StringName name;
            Variant::Type type = Variant::NIL;
            /* The component class that declared it. Two classes claiming one name is an error,
             * not a race decided by whichever processed last. */
            StringName owner;
            /* Inspector grouping path only; never part of the public name. */
            StringName group;
    };

    /* The names every vehicle shares, interned once for the whole process.
     *
     * Global rather than per vehicle because the names are: two vehicles that both have brakes
     * answer to the same `brake_pipe_pressure`, and a consumer resolves it once for all of them. */
    class VehiclePropertyRegistry {
        private:
            static HashMap<StringName, int> ids;
            static Vector<VehiclePropertyDescriptor> descriptors;

        public:
            /* Declares a property and returns its id. Declaring the same name from a second owner
             * fails loudly, naming both - the collision `power_source` used to be, silently. */
            static int declare(
                    const StringName &p_name, Variant::Type p_type, const StringName &p_owner,
                    const StringName &p_group = StringName());

            /* The id of a name, or -1 when nothing declares it. */
            static int get_id(const StringName &p_name);
            static const VehiclePropertyDescriptor &get_descriptor(int p_id);
            static bool has_id(int p_id);
    };
} // namespace godot
