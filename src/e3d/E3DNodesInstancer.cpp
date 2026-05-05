#include "E3DNodesInstancer.hpp"

#include "E3DModelInstance.hpp"
#include <godot_cpp/classes/geometry_instance3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>

namespace godot {

    void E3DNodesInstancer::_bind_methods() {}

    void
    E3DNodesInstancer::instantiate(E3DModelInstance *p_target_node, const Ref<E3DModel> &p_model, bool p_editable) {
        if (p_target_node == nullptr || p_model.is_null()) {
            return;
        }

        _do_add_submodels(p_target_node, p_target_node, p_model->get_submodels(), p_editable);
    }

    void E3DNodesInstancer::clear(E3DModelInstance *p_target_node) {
        if (p_target_node == nullptr) {
            return;
        }

        while (p_target_node->get_child_count(true) > 0) {
            Node *child = p_target_node->get_child(0, true);
            p_target_node->remove_child(child);
            child->queue_free();
        }
    }

    void E3DNodesInstancer::sync(E3DModelInstance *p_target_node) {}

    void E3DNodesInstancer::_do_add_submodels(
            E3DModelInstance *p_target_node, Node *p_parent, const Array &p_submodels, bool p_editable) {
        for (int i = 0; i < p_submodels.size(); i++) {
            Ref<E3DSubModel> submodel = p_submodels[i];
            if (!_is_submodel_valid(p_target_node, submodel)) {
                continue;
            }

            Node *child = _create_submodel_instance(p_target_node, submodel);
            if (child == nullptr) {
                continue;
            }

            _update_submodel_material(p_target_node, child, submodel);
            const Node::InternalMode internal_mode =
                    p_editable ? Node::INTERNAL_MODE_DISABLED : Node::INTERNAL_MODE_BACK;
            p_parent->add_child(child, false, internal_mode);

            Node3D *child_node_3d = Object::cast_to<Node3D>(child);
            if (child_node_3d != nullptr) {
                child_node_3d->set_transform(submodel->get_transform());
            }

            if (Engine::get_singleton()->is_editor_hint()) {
                child->set_owner(p_editable ? p_target_node->get_owner() : p_target_node);
            }

            if (submodel->get_submodels().size() > 0) {
                _do_add_submodels(p_target_node, child, submodel->get_submodels(), p_editable);
            }
        }
    }

    Node *
    E3DNodesInstancer::_create_submodel_instance(E3DModelInstance *p_target_node, const Ref<E3DSubModel> &p_submodel) {
        if (p_submodel->get_submodel_type() == E3DSubModel::SUBMODEL_TRANSFORM) {
            Node3D *node = memnew(Node3D);
            node->set_name(p_submodel->get_name());
            node->set_visible(p_submodel->get_visible());
            return node;
        }

        if (p_submodel->get_submodel_type() == E3DSubModel::SUBMODEL_GL_TRIANGLES) {
            MeshInstance3D *mesh_instance = memnew(MeshInstance3D);
            mesh_instance->set_name(p_submodel->get_name());
            mesh_instance->set_mesh(p_submodel->get_mesh());
            mesh_instance->set_visibility_range_begin(p_submodel->get_visibility_range_begin());
            mesh_instance->set_visibility_range_end(p_submodel->get_visibility_range_end());
            mesh_instance->set_visible(p_submodel->get_visible());
            return mesh_instance;
        }

        return nullptr;
    }

    void E3DNodesInstancer::_update_submodel_material(
            E3DModelInstance *p_target_node, Node *p_subnode, const Ref<E3DSubModel> &p_submodel) {
        GeometryInstance3D *geometry_instance = Object::cast_to<GeometryInstance3D>(p_subnode);
        if (geometry_instance == nullptr) {
            return;
        }

        Ref<Material> material = _get_material_override(p_target_node, p_submodel);
        if (material.is_valid()) {
            geometry_instance->set_material_override(material);
        }

        if (p_submodel->get_material_colored()) {
            geometry_instance->set_instance_shader_parameter("albedo_color", p_submodel->get_diffuse_color());
        }

        MeshInstance3D *mesh_instance = Object::cast_to<MeshInstance3D>(p_subnode);
        if (mesh_instance != nullptr && _requires_alpha_depth_prepass_sorting(material)) {
            mesh_instance->set_sorting_offset(-1.0f);
        }
    }

} // namespace godot
