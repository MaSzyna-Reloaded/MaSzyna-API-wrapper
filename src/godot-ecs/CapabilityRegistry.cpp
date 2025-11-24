#include "godot-ecs/CapabilityRegistry.hpp"

namespace godot {

void CapabilityRegistry::_bind_methods() {
    ClassDB::bind_method(D_METHOD("register_capability", "capability"), &CapabilityRegistry::register_capability);
    ClassDB::bind_method(D_METHOD("get_capability_id", "capability"), &CapabilityRegistry::get_capability_id);
    ClassDB::bind_method(D_METHOD("get_capability_name", "id"), &CapabilityRegistry::get_capability_name);
    ClassDB::bind_method(D_METHOD("list_capabilities"), &CapabilityRegistry::list_capabilities);
}

int64_t CapabilityRegistry::register_capability(const StringName &p_capability) {
    if (p_capability.is_empty()) {
        return -1;
    }

    if (capability_to_id.has(p_capability)) {
        return capability_to_id[p_capability];
    }

    const int64_t id = id_to_capability.size();
    capability_to_id[p_capability] = id;
    id_to_capability.push_back(p_capability);
    return id;
}

int64_t CapabilityRegistry::get_capability_id(const StringName &p_capability) const {
    if (capability_to_id.has(p_capability)) {
        return capability_to_id[p_capability];
    }
    return -1;
}

StringName CapabilityRegistry::get_capability_name(int64_t p_id) const {
    if (p_id < 0 || p_id >= id_to_capability.size()) {
        return StringName();
    }
    return id_to_capability[p_id];
}

TypedArray<StringName> CapabilityRegistry::list_capabilities() const {
    TypedArray<StringName> out;
    for (const KeyValue<StringName, int64_t> &E : capability_to_id) {
        out.push_back(E.key);
    }
    return out;
}

} // namespace godot
