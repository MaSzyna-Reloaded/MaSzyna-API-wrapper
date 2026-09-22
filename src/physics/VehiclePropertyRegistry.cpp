#include "VehiclePropertyRegistry.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    HashMap<StringName, int> VehiclePropertyRegistry::ids;
    Vector<VehiclePropertyDescriptor> VehiclePropertyRegistry::descriptors;

    int VehiclePropertyRegistry::declare(
            const StringName &p_name, const Variant::Type p_type, const StringName &p_owner,
            const StringName &p_group) {
        if (const int *existing = ids.getptr(p_name); existing != nullptr) {
            const VehiclePropertyDescriptor &descriptor = descriptors[*existing];
            if (descriptor.owner == p_owner) {
                return *existing;
            }
            ERR_FAIL_V_MSG(
                    -1,
                    vformat("Vehicle property '%s' is declared by both %s and %s - one name, one owner.", p_name,
                            descriptor.owner, p_owner));
        }
        VehiclePropertyDescriptor descriptor;
        descriptor.name = p_name;
        descriptor.type = p_type;
        descriptor.owner = p_owner;
        descriptor.group = p_group;
        const int id = static_cast<int>(descriptors.size());
        descriptors.push_back(descriptor);
        ids.insert(p_name, id);
        return id;
    }

    int VehiclePropertyRegistry::get_id(const StringName &p_name) {
        const int *id = ids.getptr(p_name);
        return id != nullptr ? *id : -1;
    }


    const VehiclePropertyDescriptor &VehiclePropertyRegistry::get_descriptor(const int p_id) {
        static const VehiclePropertyDescriptor unknown;
        if (!has_id(p_id)) {
            return unknown;
        }
        return descriptors[p_id];
    }

    bool VehiclePropertyRegistry::has_id(const int p_id) {
        return p_id >= 0 && p_id < static_cast<int>(descriptors.size());
    }
} // namespace godot
