#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

class CapabilityRegistry : public RefCounted {
        GDCLASS(CapabilityRegistry, RefCounted)

    public:
        CapabilityRegistry() = default;
        ~CapabilityRegistry() override = default;

        int64_t register_capability(const StringName &p_capability);
        int64_t get_capability_id(const StringName &p_capability) const;
        StringName get_capability_name(int64_t p_id) const;
        TypedArray<StringName> list_capabilities() const;

    protected:
        static void _bind_methods();

    private:
        HashMap<StringName, int64_t> capability_to_id;
        Vector<StringName> id_to_capability;
};

} // namespace godot
