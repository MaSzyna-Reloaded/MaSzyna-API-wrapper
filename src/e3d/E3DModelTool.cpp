#include "E3DModelTool.hpp"

#include "macros.hpp"

namespace godot {

    void E3DModelTool::_bind_methods() {
        ClassDB::bind_method(D_METHOD("find_first_mesh", "model"), &E3DModelTool::find_first_mesh);
        ClassDB::bind_method(D_METHOD("get_aabb", "model"), &E3DModelTool::get_aabb);
    }

    Ref<ArrayMesh> E3DModelTool::find_first_mesh(const Ref<E3DModel> &p_model) const {
        if (p_model.is_null()) {
            return nullptr;
        }

        return _find_first_mesh_in_submodels(p_model->get_submodels());
    }

    Ref<ArrayMesh> E3DModelTool::_find_first_mesh_in_submodels(const Array &p_submodels) const {
        for (int i = 0; i < p_submodels.size(); i++) {
            Ref<E3DSubModel> submodel = p_submodels[i];
            if (submodel.is_null()) {
                continue;
            }

            Ref<ArrayMesh> mesh = submodel->get_mesh();
            if (mesh.is_valid()) {
                return mesh;
            }

            mesh = _find_first_mesh_in_submodels(submodel->get_submodels());
            if (mesh.is_valid()) {
                return mesh;
            }
        }

        return nullptr;
    }

    AABB E3DModelTool::get_aabb(const Ref<E3DModel> &p_model) const {
        if (p_model.is_null()) {
            return AABB();
        }

        AABB result;
        bool has_aabb = false;
        Array submodels = p_model->get_submodels();
        for (int i = 0; i < submodels.size(); i++) {
            Ref<E3DSubModel> submodel = submodels[i];
            if (submodel.is_valid()) {
                _get_aabb_from_submodel(submodel, Transform3D(), result, has_aabb);
            }
        }

        return has_aabb ? result : AABB();
    }

    bool E3DModelTool::_get_aabb_from_submodel(
            const Ref<E3DSubModel> &p_submodel, const Transform3D &p_parent_transform, AABB &r_aabb,
            bool &r_has_aabb) const {
        const Transform3D local_transform = p_parent_transform * p_submodel->get_transform();

        Ref<ArrayMesh> mesh = p_submodel->get_mesh();
        if (mesh.is_valid()) {
            AABB local_aabb = local_transform.xform(mesh->get_aabb());
            if (r_has_aabb) {
                r_aabb = r_aabb.merge(local_aabb);
            } else {
                r_aabb = local_aabb;
                r_has_aabb = true;
            }
        }

        Array children = p_submodel->get_submodels();
        for (int i = 0; i < children.size(); i++) {
            Ref<E3DSubModel> child = children[i];
            if (child.is_valid()) {
                _get_aabb_from_submodel(child, local_transform, r_aabb, r_has_aabb);
            }
        }

        return r_has_aabb;
    }

} // namespace godot
