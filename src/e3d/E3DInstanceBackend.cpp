#include "E3DInstanceBackend.hpp"
#include <godot_cpp/classes/base_material3d.hpp>

namespace godot {
    bool E3DInstanceBackend::_is_submodel_valid(const E3DSubModel *p_submodel, const Array &p_exclude_node_names) {
        if (p_submodel->get_skip_rendering()) {
            return false;
        }
        switch (p_submodel->get_submodel_type()) {
            case E3DSubModel::SUBMODEL_TRANSFORM:
            case E3DSubModel::SUBMODEL_FREE_SPOTLIGHT:
                return true;
            case E3DSubModel::SUBMODEL_GL_TRIANGLES:
                return !p_exclude_node_names.has(p_submodel->get_name());
            default:
                return false;
        }
    }

    Vector<E3DSubModel *> E3DInstanceBackend::_get_force_alpha_submodels(const E3DInstanceData &p_instance) {
        Vector<E3DSubModel *> result;
        for (int i = 0; i < p_instance.force_alpha_submodel_paths.size(); i++) {
            const Ref<E3DSubModel> submodel =
                    p_instance.model->get_node_or_null(p_instance.force_alpha_submodel_paths[i]);
            if (submodel.is_valid()) {
                result.push_back(submodel.ptr());
            }
        }
        return result;
    }

    bool E3DInstanceBackend::_is_force_alpha(
            const E3DInstanceData &p_instance, E3DSubModel *p_submodel,
            const Vector<E3DSubModel *> &p_force_alpha_submodels, const bool p_parent_force_alpha) {
        return p_parent_force_alpha || p_force_alpha_submodels.has(p_submodel) ||
               (p_instance.force_alpha && p_submodel->get_material_transparent());
    }

    bool E3DInstanceBackend::_requires_alpha_depth_prepass_sorting(const Ref<Material> &p_material) {
        const Ref<BaseMaterial3D> base_material = p_material;
        return base_material.is_valid() &&
               base_material->get_transparency() == BaseMaterial3D::TRANSPARENCY_ALPHA_DEPTH_PRE_PASS;
    }
} // namespace godot
