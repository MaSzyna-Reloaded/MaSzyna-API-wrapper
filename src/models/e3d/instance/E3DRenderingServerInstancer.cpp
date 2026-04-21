#include "E3DRenderingServerInstancer.hpp"

#include "E3DNodesInstancer.hpp"

#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

namespace godot {
    void E3DRenderingServerInstancer::_append_submodels(
            const E3DModelInstance &p_target_node,
            const TypedArray<E3DSubModel> &submodels,
            const Transform3D &parent_transform,
            const bool parent_visible,
            std::vector<InstanceRecord> &instances) {
        RenderingServer *rs = RenderingServer::get_singleton();
        if (rs == nullptr) {
            return;
        }

        for (const Variant &i: submodels) {
            const Ref<E3DSubModel> submodel_ref = i;
            const E3DSubModel *submodel = submodel_ref.ptr();
            if (submodel == nullptr || submodel->get_skip_rendering()) {
                continue;
            }

            const Transform3D current_transform = parent_transform * submodel->get_transform();
            const bool current_visible = parent_visible && submodel->get_visible();

            if (TypedArray<E3DSubModel> children = submodel->get_submodels(); children.size() > 0) {
                _append_submodels(p_target_node, children, current_transform, current_visible, instances);
            }

            if (submodel->get_submodel_type() != E3DSubModel::GL_TRIANGLES) {
                continue;
            }

            if (p_target_node.get_exclude_node_names().has(submodel->get_name())) {
                continue;
            }

            const Ref<ArrayMesh> mesh = submodel->get_mesh();
            if (mesh.is_null()) {
                continue;
            }

            const RID instance_rid = rs->instance_create2(mesh->get_rid(), RID());
            if (!instance_rid.is_valid()) {
                continue;
            }

            rs->instance_geometry_set_visibility_range(
                    instance_rid,
                    submodel->get_visibility_range_begin(),
                    submodel->get_visibility_range_end(),
                    0.0f,
                    0.0f,
                    RenderingServer::VISIBILITY_RANGE_FADE_DISABLED);
            rs->instance_geometry_set_cast_shadows_setting(
                    instance_rid,
                    RenderingServer::SHADOW_CASTING_SETTING_ON);
            rs->instance_set_visible(instance_rid, current_visible);

            if (const Ref<Material> material = E3DNodesInstancer::resolve_submodel_material(p_target_node, *submodel);
                material.is_valid()) {
                rs->instance_geometry_set_material_override(instance_rid, material->get_rid());
            }

            instances.push_back({instance_rid, current_transform, current_visible, submodel->get_name()});
        }
    }

    std::vector<E3DRenderingServerInstancer::InstanceRecord> E3DRenderingServerInstancer::instantiate(
            const Ref<E3DModel> &p_model, const E3DModelInstance &p_target_node, const RID &scenario) {
        std::vector<InstanceRecord> instances;
        if (!p_model.is_valid()) {
            return instances;
        }

        _append_submodels(p_target_node, p_model->get_submodels(), Transform3D(), true, instances);

        RenderingServer *rs = RenderingServer::get_singleton();
        if (rs == nullptr) {
            return instances;
        }

        for (const InstanceRecord &record: instances) {
            if (scenario.is_valid()) {
                rs->instance_set_scenario(record.instance_rid, scenario);
            }
        }

        return instances;
    }
} // namespace godot
