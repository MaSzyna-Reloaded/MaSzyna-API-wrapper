#include "E3DLightFactory.hpp"
#include "E3DNodesBackend.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    E3DNodesBackend::E3DNodesBackend(const bool p_editable) : editable(p_editable) {}

    void E3DNodesBackend::build(E3DInstanceData &p_instance, E3DMaterialResolver &p_material_resolver) {
        Node3D *target = Object::cast_to<Node3D>(ObjectDB::get_instance(p_instance.node_id));
        ERR_FAIL_NULL_MSG(target, "NODES instancer requires a node attached with instance_attach_node()");

        // which submodels belong to which light was found by E3DLightFactory, not here
        HashMap<E3DSubModel *, LightRole> light_roles;
        for (const E3DModelLight &light: p_instance.model_lights.lights) {
            if (light.on != nullptr && !light_roles.has(light.on)) {
                light_roles[light.on] = {light.name, true};
            }
            if (light.off != nullptr && !light_roles.has(light.off)) {
                light_roles[light.off] = {light.name, false};
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
        p_instance.submodel_nodes.clear();
    }

    void E3DNodesBackend::apply_poses(E3DInstanceData &p_instance) {
        for (const KeyValue<E3DSubModel *, Transform3D> &pose: p_instance.submodel_poses) {
            const ObjectID *node_id = p_instance.submodel_nodes.getptr(pose.key);
            if (node_id == nullptr) {
                continue;
            }
            if (Node3D *node = Object::cast_to<Node3D>(ObjectDB::get_instance(*node_id)); node != nullptr) {
                node->set_transform(pose.key->get_transform() * pose.value);
            }
        }
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
            E3DMaterialResolver &p_material_resolver) {
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
                const Ref<Material> material = p_material_resolver.resolve(p_instance, submodel.ptr(), force_alpha);
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
            p_instance.submodel_nodes[submodel.ptr()] = ObjectID(child->get_instance_id());
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
                        settings->get_setting("maszyna/vehicles/lights_volumetric_fog_energy", 4.0));
                spotlight->set_shadow(true);
                spotlight->set_shadow_reverse_cull_face(
                        settings->get_setting("maszyna/lights/reverse_cull_face", true));
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
        const E3DLightParams params = E3DLightFactory::from_submodel(p_submodel, p_light_name);
        p_spotlight->set_param(Light3D::PARAM_ENERGY, params.energy);
        p_spotlight->set_param(Light3D::PARAM_RANGE, params.range);
        p_spotlight->set_param(Light3D::PARAM_ATTENUATION, params.attenuation);
        p_spotlight->set_param(Light3D::PARAM_SPOT_ATTENUATION, params.spot_attenuation);
    }

    void E3DNodesBackend::_set_node_visible(const ObjectID &p_node_id, const bool p_visible) {
        if (Node3D *node = Object::cast_to<Node3D>(ObjectDB::get_instance(p_node_id)); node != nullptr) {
            node->set_visible(p_visible);
        }
    }
} // namespace godot
