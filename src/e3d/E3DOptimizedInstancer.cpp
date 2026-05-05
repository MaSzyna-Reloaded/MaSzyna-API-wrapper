#include "E3DOptimizedInstancer.hpp"

#include "E3DModelInstance.hpp"
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/world3d.hpp>

namespace godot {

    void E3DOptimizedInstancer::_bind_methods() {}

    void
    E3DOptimizedInstancer::instantiate(E3DModelInstance *p_target_node, const Ref<E3DModel> &p_model, bool p_editable) {
        if (p_target_node == nullptr || p_model.is_null() || p_target_node->get_world_3d().is_null()) {
            return;
        }

        InstancerEntry entry;
        const RID scenario = p_target_node->get_world_3d()->get_scenario();
        _add_submodels(entry, p_target_node, p_model->get_submodels(), Transform3D(), scenario);
        instances_by_target[p_target_node->get_instance_id()] = entry;
        sync(p_target_node);
    }

    void E3DOptimizedInstancer::clear(E3DModelInstance *p_target_node) {
        if (p_target_node == nullptr) {
            return;
        }

        auto entry_it = instances_by_target.find(p_target_node->get_instance_id());
        if (entry_it == instances_by_target.end()) {
            return;
        }

        RenderingServer *rendering_server = RenderingServer::get_singleton();
        for (const InstanceData &instance: entry_it->second.instances) {
            rendering_server->free_rid(instance.rid);
        }

        instances_by_target.erase(entry_it);
    }

    void E3DOptimizedInstancer::sync(E3DModelInstance *p_target_node) {
        if (p_target_node == nullptr) {
            return;
        }

        auto entry_it = instances_by_target.find(p_target_node->get_instance_id());
        if (entry_it == instances_by_target.end()) {
            return;
        }

        RenderingServer *rendering_server = RenderingServer::get_singleton();
        for (const InstanceData &instance: entry_it->second.instances) {
            rendering_server->instance_set_transform(
                    instance.rid, p_target_node->get_global_transform() * instance.local_transform);
        }
    }

    void E3DOptimizedInstancer::_add_submodels(
            InstancerEntry &r_entry, E3DModelInstance *p_target_node, const Array &p_submodels,
            const Transform3D &p_parent_transform, const RID &p_scenario) {
        for (int i = 0; i < p_submodels.size(); i++) {
            Ref<E3DSubModel> submodel = p_submodels[i];
            if (!_is_submodel_valid(p_target_node, submodel)) {
                continue;
            }

            if (submodel->get_submodel_type() != E3DSubModel::SUBMODEL_TRANSFORM && submodel->get_mesh().is_null()) {
                continue;
            }

            const Transform3D local_transform = p_parent_transform * submodel->get_transform();

            if (submodel->get_submodel_type() == E3DSubModel::SUBMODEL_GL_TRIANGLES) {
                _add_submodel(r_entry, p_target_node, submodel, local_transform, p_scenario);
            }

            if (submodel->get_submodels().size() > 0) {
                _add_submodels(r_entry, p_target_node, submodel->get_submodels(), local_transform, p_scenario);
            }
        }
    }

    void E3DOptimizedInstancer::_add_submodel(
            InstancerEntry &r_entry, E3DModelInstance *p_target_node, const Ref<E3DSubModel> &p_submodel,
            const Transform3D &p_local_transform, const RID &p_scenario) {
        RenderingServer *rendering_server = RenderingServer::get_singleton();
        const RID instance = rendering_server->instance_create();
        rendering_server->instance_attach_object_instance_id(instance, p_target_node->get_instance_id());
        rendering_server->instance_set_base(instance, p_submodel->get_mesh()->get_rid());
        rendering_server->instance_set_scenario(instance, p_scenario);
        rendering_server->instance_geometry_set_visibility_range(
                instance, p_submodel->get_visibility_range_begin(), p_submodel->get_visibility_range_end(), 0.0f, 0.0f,
                RenderingServer::VISIBILITY_RANGE_FADE_DISABLED);

        Ref<Material> material = _get_material_override(p_target_node, p_submodel);
        if (material.is_valid()) {
            r_entry.materials.push_back(material);
            rendering_server->instance_geometry_set_material_override(instance, material->get_rid());
            if (_requires_alpha_depth_prepass_sorting(material)) {
                rendering_server->instance_set_pivot_data(instance, -1.0f, false);
            }
        }

        rendering_server->instance_set_visible(instance, p_submodel->get_visible());
        r_entry.instances.push_back({instance, p_local_transform});
    }

} // namespace godot
