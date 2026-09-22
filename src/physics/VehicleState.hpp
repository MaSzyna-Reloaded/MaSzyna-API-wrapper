#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_float64_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace godot {
    /* One vehicle's state, read by name or by id.
     *
     * It holds the vehicle's handle, never a pointer, so a script that keeps it past the vehicle's
     * life reads defaults and says so rather than dangling. Nothing is computed until something
     * asks: a property is a question put to the component that owns it.
     *
     * Three ways in, for three situations. `get_float_by_id()` after resolving the name once is
     * what a per-frame reader uses; `read_floats()` is one crossing for many values;
     * `_get()` by name is the readable one, for a console, an inspector or a one-off script. */
    class VehicleState : public RefCounted {
            GDCLASS(VehicleState, RefCounted)

        private:
            RID vehicle;

        protected:
            static void _bind_methods();
            bool _get(const StringName &p_name, Variant &p_ret) const;
            bool _set(const StringName &p_name, const Variant &p_value);
            void _get_property_list(List<PropertyInfo> *p_list) const;

        public:
            void set_vehicle(const RID &p_vehicle);
            RID get_vehicle() const;
            bool is_valid() const;

            /* The id of a name, resolved once and used from then on. -1 when nothing declares it. */
            static int resolve(const StringName &p_name);

            Variant get_by_id(int p_property_id) const;
            double get_float_by_id(int p_property_id) const;
            /* Many values, one crossing. */
            PackedFloat64Array read_floats(const PackedInt32Array &p_property_ids) const;
            /* Everything this vehicle publishes, by name - expensive on purpose, for diagnostics. */
            Dictionary snapshot() const;
    };
} // namespace godot
