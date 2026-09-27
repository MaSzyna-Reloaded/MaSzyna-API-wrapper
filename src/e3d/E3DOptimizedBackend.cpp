#include "E3DOptimizedBackend.hpp"
#include <godot_cpp/classes/rendering_server.hpp>

namespace godot {
    void E3DOptimizedBackend::build(E3DInstanceData &p_instance, E3DMaterialResolver &p_material_resolver) {
        // which submodels belong to which light was found by E3DLightFactory, not here
        for (const E3DModelLight &light: p_instance.model_lights.lights) {
            E3DInstanceData::LightSubmodels light_submodels;
            light_submodels.on = light.on;
            light_submodels.off = light.off;
            p_instance.light_submodels[light.name] = light_submodels;
        }

        _add_submodels(
                p_instance, p_instance.model->get_submodels(), Transform3D(), Vector<E3DSubModel *>(),
                _get_force_alpha_submodels(p_instance), false, p_material_resolver);
        update(p_instance);
    }

    void E3DOptimizedBackend::clear(E3DInstanceData &p_instance) {
        RenderingServer *rs = RenderingServer::get_singleton();
        if (rs != nullptr) {
            for (const RID &rid: p_instance.rids) {
                rs->free_rid(rid);
            }
        }
        p_instance.rids.clear();
        p_instance.local_transforms.clear();
        p_instance.chains.clear();
        p_instance.materials.clear();
        p_instance.light_submodels.clear();
    }

    void E3DOptimizedBackend::update(const E3DInstanceData &p_instance) {
        // Submodel visibility overrides from the light state, same as E3DNodesBackend::update()
        HashMap<E3DSubModel *, bool> overrides;
        for (const KeyValue<String, E3DInstanceData::LightSubmodels> &light: p_instance.light_submodels) {
            if (!p_instance.lights_state.has(light.key)) {
                continue;
            }
            const bool enabled = p_instance.lights_state[light.key];
            if (light.value.on != nullptr) {
                overrides[light.value.on] = enabled;
            }
            if (light.value.off != nullptr) {
                overrides[light.value.off] = !enabled;
            }
        }

        RenderingServer *rs = RenderingServer::get_singleton();
        for (int i = 0; i < p_instance.rids.size(); i++) {
            // A submodel is rendered only if all its ancestors are visible (node hierarchy semantics)
            bool visible = p_instance.visible;
            for (E3DSubModel *submodel: p_instance.chains[i]) {
                const HashMap<E3DSubModel *, bool>::ConstIterator override = overrides.find(submodel);
                visible = visible && (override == overrides.end() ? submodel->get_visible() : override->value);
            }
            rs->instance_set_transform(p_instance.rids[i], p_instance.transform * p_instance.local_transforms[i]);
            rs->instance_set_visible(p_instance.rids[i], visible);
            rs->instance_set_layer_mask(p_instance.rids[i], p_instance.layer_mask);
        }
    }

    void E3DOptimizedBackend::apply_transform(const E3DInstanceData &p_instance) {
        RenderingServer *rs = RenderingServer::get_singleton();
        ERR_FAIL_NULL(rs);
        for (int i = 0; i < p_instance.rids.size(); i++) {
            rs->instance_set_transform(p_instance.rids[i], p_instance.transform * p_instance.local_transforms[i]);
        }
    }

    /// The chain's transforms again, each submodel with its pose on top (TSubModel::RaAnimation(),
    /// Model3d.cpp:1130-1160)
    void E3DOptimizedBackend::apply_poses(E3DInstanceData &p_instance) {
        RenderingServer *rs = RenderingServer::get_singleton();
        ERR_FAIL_NULL(rs);
        for (int i = 0; i < p_instance.rids.size(); i++) {
            Transform3D local_transform;
            for (E3DSubModel *submodel: p_instance.chains[i]) {
                local_transform = local_transform * submodel->get_transform();
                if (const Transform3D *pose = p_instance.submodel_poses.getptr(submodel); pose != nullptr) {
                    local_transform = local_transform * *pose;
                }
            }
            p_instance.local_transforms.write[i] = local_transform;
            rs->instance_set_transform(p_instance.rids[i], p_instance.transform * local_transform);
        }
    }

    void E3DOptimizedBackend::_add_submodels(
            E3DInstanceData &p_instance, const TypedArray<E3DSubModel> &p_submodels,
            const Transform3D &p_parent_transform, const Vector<E3DSubModel *> &p_parent_chain,
            const Vector<E3DSubModel *> &p_force_alpha_submodels, const bool p_force_alpha,
            E3DMaterialResolver &p_material_resolver) {
        for (int i = 0; i < p_submodels.size(); i++) {
            const Ref<E3DSubModel> submodel = p_submodels[i];
            if (submodel.is_null() || !_is_submodel_valid(submodel.ptr(), p_instance.exclude_node_names)) {
                continue;
            }

            const Transform3D local_transform = p_parent_transform * submodel->get_transform();
            Vector<E3DSubModel *> chain = p_parent_chain;
            chain.push_back(submodel.ptr());
            const bool force_alpha =
                    _is_force_alpha(p_instance, submodel.ptr(), p_force_alpha_submodels, p_force_alpha);

            // SUBMODEL_FREE_SPOTLIGHT draws nothing; E3DRenderingServer owns the light RIDs and
            // streams them with a range of their own, far shorter than the model's
            if (submodel->get_submodel_type() == E3DSubModel::SUBMODEL_GL_TRIANGLES &&
                submodel->get_mesh().is_valid()) {
                _add_submodel(p_instance, submodel.ptr(), local_transform, chain, force_alpha, p_material_resolver);
            }

            _add_submodels(
                    p_instance, submodel->get_submodels(), local_transform, chain, p_force_alpha_submodels, force_alpha,
                    p_material_resolver);
        }
    }

    void E3DOptimizedBackend::_add_submodel(
            E3DInstanceData &p_instance, E3DSubModel *p_submodel, const Transform3D &p_local_transform,
            const Vector<E3DSubModel *> &p_chain, const bool p_force_alpha, E3DMaterialResolver &p_material_resolver) {
        RenderingServer *rs = RenderingServer::get_singleton();
        const RID rid = rs->instance_create();
        if (p_instance.node_id.is_valid()) {
            rs->instance_attach_object_instance_id(rid, p_instance.node_id);
        }
        rs->instance_set_base(rid, p_submodel->get_mesh()->get_rid());
        rs->instance_set_scenario(rid, p_instance.scenario);

        // the instance range (scenery node range_min/range_max) limits the submodel's own range
        float range_begin = MAX(p_submodel->get_visibility_range_begin(), p_instance.visibility_range_begin);
        float range_end = p_submodel->get_visibility_range_end();
        if (p_instance.visibility_range_end > 0.0 &&
            (range_end <= 0.0 || p_instance.visibility_range_end < range_end)) {
            range_end = p_instance.visibility_range_end;
        }
        rs->instance_geometry_set_visibility_range(
                rid, range_begin, range_end, 0.0, 0.0, RenderingServer::VISIBILITY_RANGE_FADE_DISABLED);

        const Ref<Material> material = p_material_resolver.resolve(p_instance, p_submodel, p_force_alpha);
        if (material.is_valid()) {
            p_instance.materials.push_back(material);
            rs->instance_geometry_set_material_override(rid, material->get_rid());
            // same as MeshInstance3D.sorting_offset = -1 in E3DNodesBackend
            if (_requires_alpha_depth_prepass_sorting(material)) {
                rs->instance_set_pivot_data(rid, -1.0, false);
            }
        }
        if (p_submodel->get_material_colored()) {
            rs->instance_geometry_set_shader_parameter(rid, "albedo_color", p_submodel->get_diffuse_color());
        }

        p_instance.rids.push_back(rid);
        p_instance.local_transforms.push_back(p_local_transform);
        p_instance.chains.push_back(p_chain);
    }
} // namespace godot
