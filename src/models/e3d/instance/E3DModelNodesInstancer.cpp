#include "E3DModelNodesInstancer.hpp"
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/memory.hpp>
namespace godot {
    Ref<Material> E3DModelNodesInstancer::_colored_material;

    Node3D *E3DModelNodesInstancer::_create_submodel_instance(
            const E3DModelInstance &p_target_node, const E3DSubModel &submodel) {
        if (submodel.get_skip_rendering()) {
            return nullptr;
        }

        switch (submodel.get_submodel_type()) {
            case E3DSubModel::TRANSFORM: {
                Node3D *obj = memnew(Node3D);
                obj->set_name(StringName(submodel.get_name()));
                obj->set_visible(submodel.get_visible());
                return obj;
            }

            case E3DSubModel::GL_TRIANGLES: {
                if (const bool is_name_excluded = p_target_node.get_exclude_node_names().has(submodel.get_name()); !is_name_excluded) {
                    MeshInstance3D *mesh_instance = memnew(MeshInstance3D);
                    mesh_instance->set_name(StringName(submodel.get_name()));
                    mesh_instance->set_mesh(submodel.get_mesh());
                    mesh_instance->set_visibility_range_begin(submodel.get_visibility_range_begin());
                    mesh_instance->set_visibility_range_end(submodel.get_visibility_range_end());
                    mesh_instance->set_visible(submodel.get_visible());
                    return mesh_instance;
                }
            }
            default: ;
        }
        return nullptr;
    }

    void E3DModelNodesInstancer::_do_add_submodels(
            const E3DModelInstance &p_target_node, Node3D* parent, TypedArray<E3DSubModel> submodels, const bool editable) {
        if (parent == nullptr) {
            return;
        }
        for (int i = 0; i < submodels.size(); i++) {
            // Extract E3DSubModel from the array safely.
            Ref<E3DSubModel> submodel_ref = submodels[i];
            if (!submodel_ref.is_valid()) {
                continue;
            }
            const E3DSubModel *submodel = submodel_ref.ptr();
            if (submodel == nullptr) {
                continue;
            }

            Node3D *child = _create_submodel_instance(p_target_node, *submodel);
            if (child == nullptr) {
                continue;
            }

            _update_submodel_material(p_target_node, *child, *submodel);
            // Choose internal mode: editable -> disabled (regular child), non-editable -> internal back
            const InternalMode internal = editable ? Node::INTERNAL_MODE_DISABLED : Node::INTERNAL_MODE_BACK;
            parent->add_child(child, false, internal);

            // Apply transform AFTER adding to the tree (important especially on Windows)
            if (Node3D *child3d = cast_to<Node3D>(child); submodel_ref->has_method("get_transform") && child3d != nullptr) {
                // ReSharper disable once CppDFAUnreachableCode
                child3d->set_transform(submodel->get_transform());
            }

            // In editor, set owner depending on editability
            if (Engine::get_singleton() && Engine::get_singleton()->is_editor_hint()) {
                // Prefer using the same owner as the target node for editable; otherwise, no explicit owner
                if (Node *owner = editable ? p_target_node.get_owner() : nullptr; owner && child != nullptr) {
                    // ReSharper disable once CppDFAUnreachableCode
                    child->set_owner(owner);
                }
            }

            // Recurse into submodels
            if (TypedArray<E3DSubModel> children = submodel->get_submodels(); children.size() > 0) {
                _do_add_submodels(p_target_node, child, children, editable);
            }
        }
    }

    void E3DModelNodesInstancer::_update_submodel_material(
            const E3DModelInstance &p_target_node, Node3D subnode, const E3DSubModel &submodel) {
        String _model_path = String("/").join(p_target_node.get_data_path().split("/").slice(1));
        if (submodel.get_dynamic_material()) {
            if (p_target_node.get_skins().size() < submodel.get_dynamic_material_index() + 1) {
                UtilityFunctions::push_warning("Model " + p_target_node.get_name() + " has less skins than dynamic material index " + String::num_int64(submodel.get_dynamic_material_index()));
            } else {
                // MaterialManager.Transaprency
            }
        }
    }

    void E3DModelNodesInstancer::_bind_methods() {
        // Do not load GPU resources at bind time; this function can be called when the
        // RenderingServer is not fully initialized (e.g., doctool), which leads to
        // shutdown errors. Only bind methods/signals here if needed in the future.
    }

    Ref<Material> E3DModelNodesInstancer::get_colored_material() {
        // Lazy-load on demand when actually needed and when safe to do so.
        if (!_colored_material.is_valid()) {
            if (Engine::get_singleton() && !Engine::get_singleton()->is_editor_hint()) {
                const String path = "res://addons/libmaszyna/e3d/colored.material";
                if (const Ref<Material> res = ResourceLoader::get_singleton()->load(path, "Material", ResourceLoader::CACHE_MODE_REUSE); res.is_valid()) {
                    _colored_material = res;
                }
            }
        }
        return _colored_material;
    }

    void E3DModelNodesInstancer::instantiate(const E3DModel &p_model, E3DModelInstance p_target_node, const bool editable) {
        // Safely remove all children from the target node.
        // Iterate backwards to avoid index shifting issues and operate on real Node* pointers.
        const int32_t child_count = p_target_node.get_child_count(true);
        for (int32_t i = child_count - 1; i >= 0; --i) {
            Node *child = p_target_node.get_child(i, true);
            if (!child) {
                continue;
            }
            // Detach from the scene tree immediately and free safely.
            p_target_node.remove_child(child);
            child->queue_free();
        }

        _do_add_submodels(p_target_node, &p_target_node, p_model.get_submodels(), editable);
    }

} // namespace godot
