#include "E3DMaterialResolver.hpp"
#include "E3DInstanceBackend.hpp"
#include <godot_cpp/core/object.hpp>

namespace godot {
    /// `p_callable(submodel, data_path, skins, force_alpha) -> Material`,
    /// see MaterialManager.get_submodel_material()
    void E3DMaterialResolver::set_callable(const Callable &p_callable) {
        callable = p_callable;
        materials.clear();
    }

    Ref<Material> E3DMaterialResolver::resolve(
            const E3DInstanceData &p_instance, E3DSubModel *p_submodel, const bool p_force_alpha) {
        if (!callable.is_valid()) {
            return {};
        }
        const String key = String::num_uint64(p_submodel->get_instance_id()) + "|" + p_instance.data_path + "|" +
                           String("|").join(p_instance.skins) + (p_force_alpha ? "|1" : "|0");
        const HashMap<String, ObjectID>::ConstIterator cached = materials.find(key);
        if (cached != materials.end()) {
            Material *material = Object::cast_to<Material>(ObjectDB::get_instance(cached->value));
            if (material != nullptr) {
                return Ref(material);
            }
        }
        const Ref<Material> material = callable.call(Ref(p_submodel), p_instance.data_path, p_instance.skins, p_force_alpha);
        if (material.is_valid()) {
            materials[key] = material->get_instance_id();
        }
        return material;
    }
} // namespace godot
