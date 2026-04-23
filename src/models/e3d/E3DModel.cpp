#include "E3DModel.hpp"

namespace godot {
    E3DModel::~E3DModel() {
        clear();
    }

    void E3DModel::clear() {
        for (int i = 0; i < submodels.size(); i++) {
            Ref<E3DSubModel> sm = submodels.get(i);
            if (sm.is_valid()) {
                sm->clear();
            }
        }
        submodels.clear();
    }

    void E3DModel::_bind_methods() {
        BIND_PROPERTY(Variant::STRING, "name", "name", &E3DModel::set_name, &E3DModel::get_name, "p_name");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                Variant::ARRAY, "submodels", "submodels", &E3DModel::set_submodels, &E3DModel::get_submodels,
                "p_submodels", PROPERTY_HINT_ARRAY_TYPE, "E3DSubModel");
    }

    void E3DModel::add_child(const Ref<E3DSubModel> &p_sub_model) {
        submodels.append(p_sub_model);
    }

    static void _accumulate_aabb(
            const TypedArray<E3DSubModel> &p_submodels, const Transform3D &p_current_transform, AABB &p_r_merged,
            bool &p_r_has_aabb) {
        for (int i = 0; i < p_submodels.size(); ++i) {
            const Ref<E3DSubModel> submodel = p_submodels[i];
            if (submodel.is_null()) {
                continue;
            }

            const Transform3D local_transform = p_current_transform * submodel->get_transform();

            if (const Ref<ArrayMesh> mesh = submodel->get_mesh(); mesh.is_valid()) {
                const AABB mesh_aabb = local_transform.xform(mesh->get_aabb());
                if (mesh_aabb.has_surface()) {
                    if (p_r_has_aabb) {
                        p_r_merged = p_r_merged.merge(mesh_aabb);
                    } else {
                        p_r_merged = mesh_aabb;
                        p_r_has_aabb = true;
                    }
                }
            }

            if (const TypedArray<E3DSubModel> children = submodel->get_submodels(); children.size() > 0) {
                _accumulate_aabb(children, local_transform, p_r_merged, p_r_has_aabb);
            }
        }
    }

    AABB E3DModel::get_aabb() const {
        AABB merged;
        bool has_aabb = false;
        _accumulate_aabb(get_submodels(), Transform3D(), merged, has_aabb);
        return merged;
    }
} // namespace godot
