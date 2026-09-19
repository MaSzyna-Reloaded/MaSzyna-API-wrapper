#include "E3DNodesBackend.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    E3DNodesBackend::E3DNodesBackend(const bool p_editable) : editable(p_editable) {}

    void E3DNodesBackend::build(E3DInstanceData &p_instance, const Callable &p_material_resolver) {
        Node3D *target = Object::cast_to<Node3D>(ObjectDB::get_instance(p_instance.node_id));
        ERR_FAIL_NULL_MSG(target, "NODES instancer requires a node attached with instance_attach_node()");

        HashMap<E3DSubModel *, LightRole> light_roles;
        const TypedDictionary<String, E3DModelLightDefinition> lights = p_instance.model->get_lights();
        const Array light_names = lights.keys();
        for (int i = 0; i < light_names.size(); i++) {
            const String light_name = light_names[i];
            const Ref<E3DModelLightDefinition> light_info = lights[light_name];
            if (light_info.is_null()) {
                continue;
            }
            const Ref<E3DSubModel> on = p_instance.model->get_node_or_null(light_info->get_on_submodel_path());
            const Ref<E3DSubModel> off = p_instance.model->get_node_or_null(light_info->get_off_submodel_path());
            if (on.is_valid() && !light_roles.has(on.ptr())) {
                light_roles[on.ptr()] = {light_name, true};
            }
            if (off.is_valid() && !light_roles.has(off.ptr())) {
                light_roles[off.ptr()] = {light_name, false};
            }
        }

        _add_submodels(
                p_instance, target, target, p_instance.model->get_submodels(), light_roles, String(),
                _get_force_alpha_submodels(p_instance), false, p_material_resolver);
        update(p_instance);
    }

    void E3DNodesBackend::clear(E3DInstanceData &p_instance) {
        for (const ObjectID &node_id: p_instance.root_nodes) {
            Node *node = Object::cast_to<Node>(ObjectDB::get_instance(node_id));
            if (node == nullptr) {
                continue;
            }
            if (Node *parent = node->get_parent(); parent != nullptr) {
                parent->remove_child(node);
            }
            node->queue_free();
        }
        p_instance.root_nodes.clear();
        p_instance.light_nodes.clear();
    }

    void E3DNodesBackend::update(const E3DInstanceData &p_instance) {
        const Array light_names = p_instance.lights_state.keys();
        for (int i = 0; i < light_names.size(); i++) {
            const String light_name = light_names[i];
            const HashMap<String, E3DInstanceData::LightNodes>::ConstIterator light =
                    p_instance.light_nodes.find(light_name);
            if (light == p_instance.light_nodes.end()) {
                const Node *target = Object::cast_to<Node>(ObjectDB::get_instance(p_instance.node_id));
                UtilityFunctions::push_warning(
                        "[", target != nullptr ? String(target->get_name()) : String(),
                        "] LightInfo not found for light: ", light_name);
                continue;
            }
            const bool enabled = p_instance.lights_state[light_name];
            _set_node_visible(light->value.on, enabled);
            _set_node_visible(light->value.off, !enabled);
            _set_node_visible(light->value.spotlight, enabled);
        }
    }

    void E3DNodesBackend::_add_submodels(
            E3DInstanceData &p_instance, Node3D *p_target, Node3D *p_parent, const TypedArray<E3DSubModel> &p_submodels,
            const HashMap<E3DSubModel *, LightRole> &p_light_roles, const String &p_parent_light_name,
            const Vector<E3DSubModel *> &p_force_alpha_submodels, const bool p_force_alpha,
            const Callable &p_material_resolver) {
        const bool is_editor = Engine::get_singleton()->is_editor_hint();

        for (int i = 0; i < p_submodels.size(); i++) {
            const Ref<E3DSubModel> submodel = p_submodels[i];
            if (submodel.is_null() || !_is_submodel_valid(submodel.ptr(), p_instance.exclude_node_names)) {
                continue;
            }
            Node3D *child = _create_submodel_node(submodel.ptr());
            if (child == nullptr) {
                continue;
            }

            // A spotlight belongs to the light of its nearest "on" ancestor
            String light_name = p_parent_light_name;
            if (const HashMap<E3DSubModel *, LightRole>::ConstIterator role = p_light_roles.find(submodel.ptr());
                role != p_light_roles.end()) {
                E3DInstanceData::LightNodes &light_nodes = p_instance.light_nodes[role->value.light_name];
                if (light_nodes.submodel == nullptr) {
                    light_nodes.submodel = submodel.ptr();
                }
                if (role->value.on) {
                    light_nodes.on = ObjectID(child->get_instance_id());
                    light_name = role->value.light_name;
                } else {
                    light_nodes.off = ObjectID(child->get_instance_id());
                }
            }

            const bool force_alpha =
                    _is_force_alpha(p_instance, submodel.ptr(), p_force_alpha_submodels, p_force_alpha);
            if (GeometryInstance3D *geometry = Object::cast_to<GeometryInstance3D>(child); geometry != nullptr) {
                const Ref<Material> material =
                        _resolve_material(p_material_resolver, p_instance, submodel.ptr(), force_alpha);
                if (material.is_valid()) {
                    geometry->set_material_override(material);
                }
                if (submodel->get_material_colored()) {
                    geometry->set_instance_shader_parameter("albedo_color", submodel->get_diffuse_color());
                }
                if (Object::cast_to<MeshInstance3D>(child) != nullptr &&
                    _requires_alpha_depth_prepass_sorting(material)) {
                    geometry->set_sorting_offset(-1.0);
                }
            }

            p_parent->add_child(child, false, editable ? Node::INTERNAL_MODE_DISABLED : Node::INTERNAL_MODE_BACK);
            if (p_parent == p_target) {
                p_instance.root_nodes.push_back(ObjectID(child->get_instance_id()));
            }

            // IMPORTANT: applying transform **after** adding to the tree
            // Applying transform before adding may cause issues (especially on windows)
            if (SpotLight3D *spotlight = Object::cast_to<SpotLight3D>(child); spotlight != nullptr) {
                // Do not scale SpotLight3D to avoid configuration warnings
                spotlight->set_position(submodel->get_transform().origin);
                spotlight->set_basis(submodel->get_transform().basis.orthonormalized());
                if (!p_parent_light_name.is_empty()) {
                    E3DInstanceData::LightNodes &light_nodes = p_instance.light_nodes[p_parent_light_name];
                    light_nodes.spotlight = ObjectID(spotlight->get_instance_id());
                    _configure_spotlight(spotlight, p_parent_light_name, light_nodes.submodel);
                }
            } else {
                child->set_transform(submodel->get_transform());
            }

            if (is_editor) {
                child->set_owner(editable ? p_target->get_owner() : p_target);
            }

            _add_submodels(
                    p_instance, p_target, child, submodel->get_submodels(), p_light_roles, light_name,
                    p_force_alpha_submodels, force_alpha, p_material_resolver);
        }
    }

    Node3D *E3DNodesBackend::_create_submodel_node(E3DSubModel *p_submodel) {
        Node3D *node = nullptr;
        switch (p_submodel->get_submodel_type()) {
            case E3DSubModel::SUBMODEL_TRANSFORM:
                node = memnew(Node3D);
                break;
            case E3DSubModel::SUBMODEL_GL_TRIANGLES: {
                MeshInstance3D *mesh_instance = memnew(MeshInstance3D);
                mesh_instance->set_mesh(p_submodel->get_mesh());
                mesh_instance->set_visibility_range_begin(p_submodel->get_visibility_range_begin());
                mesh_instance->set_visibility_range_end(p_submodel->get_visibility_range_end());
                node = mesh_instance;
                break;
            }
            case E3DSubModel::SUBMODEL_FREE_SPOTLIGHT: {
                const ProjectSettings *settings = ProjectSettings::get_singleton();
                SpotLight3D *spotlight = memnew(SpotLight3D);
                Color color = p_submodel->get_diffuse_color();
                color.a = 1.0;
                spotlight->set_color(color);
                spotlight->set_param(
                        Light3D::PARAM_VOLUMETRIC_FOG_ENERGY,
                        settings->get_setting("maszyna/vehicle_lights_volumetric_fog_energy", 4.0));
                spotlight->set_shadow(true);
                spotlight->set_shadow_reverse_cull_face(
                        settings->get_setting("maszyna/rendering/lights_shadow_reverse_cull_face", true));
                spotlight->set_enable_distance_fade(true);
                spotlight->set_distance_fade_begin(150.0);
                spotlight->set_distance_fade_shadow(100.0);
                spotlight->set_distance_fade_length(200.0);
                spotlight->set_param(Light3D::PARAM_RANGE, p_submodel->get_light_range());
                spotlight->set_param(Light3D::PARAM_SPOT_ANGLE, p_submodel->get_light_angle());
                node = spotlight;
                break;
            }
            default:
                return nullptr;
        }
        node->set_name(p_submodel->get_name());
        node->set_visible(p_submodel->get_visible());
        return node;
    }

    void E3DNodesBackend::_configure_spotlight(
            SpotLight3D *p_spotlight, const String &p_light_name, E3DSubModel *p_submodel) {
        const bool is_end_light = p_light_name.begins_with("endsignal") || p_light_name.begins_with("endtab");

        float energy = DEFAULT_LIGHT_ENERGY;
        if (p_light_name.begins_with("headlamp")) {
            energy = DEFAULT_HEAD_LIGHT_ENERGY;
        } else if (p_light_name.begins_with("highbeam")) {
            energy = DEFAULT_HIGHBEAM_LIGHT_ENERGY;
        } else if (is_end_light) {
            energy = DEFAULT_END_LIGHT_ENERGY;
        } else if (p_submodel->get_light_energy() > 0.0) {
            energy = p_submodel->get_light_energy();
        }

        float range = DEFAULT_LIGHT_SPOT_RANGE;
        if (is_end_light) {
            range = FORCED_END_LIGHT_SPOT_RANGE;
        } else if (p_submodel->get_light_range() > 0.0) {
            range = p_submodel->get_light_range();
        }

        float attenuation = 1.0;
        switch (p_submodel->get_far_attenuation_decay()) {
            case 0:
                attenuation = 0.0;
                break;
            case 2:
                attenuation = 2.0;
                break;
            default:
                break;
        }

        const float inner_angle = Math::rad_to_deg(Math::acos(CLAMP(p_submodel->get_cos_hotspot_angle(), -1.0f, 1.0f)));
        const float outer_angle = MAX(p_submodel->get_light_angle(), 0.001f);
        const float penumbra_ratio = CLAMP((outer_angle - inner_angle) / outer_angle, 0.0f, 1.0f);

        p_spotlight->set_param(Light3D::PARAM_ENERGY, energy);
        p_spotlight->set_param(Light3D::PARAM_RANGE, range);
        p_spotlight->set_param(Light3D::PARAM_ATTENUATION, attenuation);
        p_spotlight->set_param(Light3D::PARAM_SPOT_ATTENUATION, Math::lerp(2.0f, 0.0f, penumbra_ratio));
    }

    void E3DNodesBackend::_set_node_visible(const ObjectID &p_node_id, const bool p_visible) {
        if (Node3D *node = Object::cast_to<Node3D>(ObjectDB::get_instance(p_node_id)); node != nullptr) {
            node->set_visible(p_visible);
        }
    }
} // namespace godot
