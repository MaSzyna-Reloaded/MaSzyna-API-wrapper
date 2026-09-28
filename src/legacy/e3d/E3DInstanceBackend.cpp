#include "E3DInstanceBackend.hpp"
#include <godot_cpp/classes/base_material3d.hpp>
#include <godot_cpp/core/math.hpp>

namespace godot {
    E3DInstanceBackend::E3DInstanceBackend() {
        // a mesh resource, because RenderingServer is not reachable yet while the servers register
        PackedVector3Array vertices;
        vertices.push_back(Vector3(-1.0, -1.0, 0.0));
        vertices.push_back(Vector3(1.0, -1.0, 0.0));
        vertices.push_back(Vector3(1.0, 1.0, 0.0));
        vertices.push_back(Vector3(-1.0, 1.0, 0.0));
        Array arrays;
        arrays.resize(Mesh::ARRAY_MAX);
        arrays[Mesh::ARRAY_VERTEX] = vertices;
        arrays[Mesh::ARRAY_INDEX] = PackedInt32Array({0, 2, 1, 0, 3, 2});
        point_mesh.instantiate();
        point_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
    }

    /// The colour is the submodel's, or the scenery node's `lightcolors` for its light
    /// (DiffuseOverride, opengl33renderer.cpp:4415); the cone and the range are the submodel's
    Dictionary E3DInstanceBackend::_free_spotlight_parameters(
            const E3DInstanceData &p_instance, const E3DSubModel *p_submodel, const String &p_light_name) {
        Color color = p_submodel->get_diffuse_color();
        const E3DInstanceData::LightDeclaration *declaration = p_instance.light_declarations.getptr(p_light_name);
        if (declaration != nullptr && declaration->has_color) {
            color = declaration->color;
        }
        Dictionary parameters;
        parameters["light_color"] = color;
        parameters["cos_hotspot_angle"] = p_submodel->get_cos_hotspot_angle();
        parameters["cos_falloff_angle"] = Math::cos(Math::deg_to_rad(p_submodel->get_light_angle()));
        parameters["max_distance"] = p_submodel->get_visibility_range_end();
        parameters["lights_on_threshold"] = p_submodel->get_lights_on_threshold();
        return parameters;
    }

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
