#include "E3DNodesInstancer.hpp"
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "models/MaterialManager.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/memory.hpp>
namespace godot {
    Ref<Material> E3DNodesInstancer::_colored_material;
    Node3D *
    E3DNodesInstancer::_create_submodel_instance(const E3DModelInstance &p_target_node, const E3DSubModel &submodel) {
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
                if (const bool is_name_excluded = p_target_node.get_exclude_node_names().has(submodel.get_name());
                    !is_name_excluded) {
                    MeshInstance3D *mesh_instance = memnew(MeshInstance3D);
                    mesh_instance->set_name(StringName(submodel.get_name()));
                    mesh_instance->set_mesh(submodel.get_mesh());
                    mesh_instance->set_visibility_range_begin(submodel.get_visibility_range_begin());
                    mesh_instance->set_visibility_range_end(submodel.get_visibility_range_end());
                    mesh_instance->set_visible(submodel.get_visible());
                    _update_submodel_material(p_target_node, *mesh_instance, submodel);
                    return mesh_instance;
                }
            }
            default:;
        }
        return nullptr;
    }

    void E3DNodesInstancer::_do_add_submodels(
            const E3DModelInstance &p_target_node, Node3D *parent, const TypedArray<E3DSubModel> &submodels,
            const bool editable) {
        if (parent == nullptr) {
            return;
        }
        for (const auto &i: submodels) {
            // Extract E3DSubModel from the array safely.
            Ref<E3DSubModel> submodel_ref = i;

            // FIXME
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

            // Choose internal mode: editable -> disabled (regular child), non-editable -> internal back
            const InternalMode internal = editable ? Node::INTERNAL_MODE_DISABLED : Node::INTERNAL_MODE_BACK;
            parent->add_child(child, false, internal);

            // Apply transform AFTER adding to the tree (important especially on Windows)
            if (Node3D *child3d = cast_to<Node3D>(child); child3d != nullptr) { // submodel_ref->has_method("get_transform") was removed; It caused the if
                                      // condition not to be met
                child3d->set_transform(submodel->get_transform());
            }

            // In editor, set owner depending on editability
            if ((Engine::get_singleton() != nullptr) && Engine::get_singleton()->is_editor_hint()) {
                // Prefer using the same owner as the target node for editable; otherwise, no explicit owner
                if (Node *owner = editable ? p_target_node.get_owner() : nullptr;
                    (owner != nullptr) && child != nullptr) {
                    child->set_owner(owner);
                }
            }

            // Recurse into submodels
            if (TypedArray<E3DSubModel> children = submodel->get_submodels(); children.size() > 0) {
                _do_add_submodels(p_target_node, child, children, editable);
            }
        }
    }

    void E3DNodesInstancer::_update_submodel_material(
            const E3DModelInstance &p_target_node, Node3D &subnode, const E3DSubModel &submodel) {
        MeshInstance3D *mesh_instance = cast_to<MeshInstance3D>(&subnode);
        if (mesh_instance == nullptr) {
            return;
        }

        MaterialManager *mm = cast_to<MaterialManager>(Engine::get_singleton()->get_singleton("MaterialManager"));
        if (mm == nullptr) {
            return;
        }

        String material_name = submodel.get_material_name();
        if (submodel.get_dynamic_material()) {
            const Array skins = p_target_node.get_skins();
            if (const int idx = submodel.get_dynamic_material_index(); skins.size() > idx) {
                material_name = skins.get(idx);
            } else {
                UtilityFunctions::push_warning(
                        "Model " + p_target_node.get_name() + " has less skins than dynamic material index " +
                        String::num_int64(idx));
            }
        }

        MaterialManager::Transparency transparency = MaterialManager::Disabled;
        if (submodel.get_material_transparent()) {
            transparency = MaterialManager::Alpha;
        }

        Ref<Material> mat;
        if (submodel.get_material_colored()) {
            mat = get_colored_material();
            if (mat.is_valid()) {
                // Since it's a shared material, we might need to duplicate it if we want per-instance colors,
                // but usually vertex colors are used.
                if (const Ref<StandardMaterial3D> sm = mat; sm.is_valid()) {
                    sm->set_albedo(submodel.get_diffuse_color());
                }
            }
        } else {
            mat = mm->get_material(
                    p_target_node.get_data_path(), material_name, transparency, false, submodel.get_diffuse_color());
        }

        if (mat.is_valid()) {
            mesh_instance->set_surface_override_material(0, mat);
        }
    }

    void E3DNodesInstancer::_bind_methods() {
        ClassDB::bind_static_method(
                "E3DNodesInstancer", D_METHOD("instantiate", "model", "target_node", "editable"),
                &E3DNodesInstancer::instantiate);
    }

    Ref<Material> E3DNodesInstancer::get_colored_material() {
        if (!_colored_material.is_valid()) {
            if ((Engine::get_singleton() != nullptr) && !Engine::get_singleton()->is_editor_hint()) {
                const String path = "res://addons/libmaszyna/e3d/colored.material";
                if (const Ref<Material> res =
                            ResourceLoader::get_singleton()->load(path, "Material", ResourceLoader::CACHE_MODE_REUSE);
                    res.is_valid()) {
                    _colored_material = res;
                }
            }
        }
        return _colored_material;
    }

    void
    E3DNodesInstancer::instantiate(const Ref<E3DModel> &p_model, E3DModelInstance *p_target_node, const bool editable) {
        if (p_target_node == nullptr) {
            return;
        }
        const int32_t child_count = p_target_node->get_child_count(true);
        for (int32_t i = child_count - 1; i >= 0; --i) {
            Node *child = p_target_node->get_child(i, true);
            if (child == nullptr) {
                continue;
            }
            // Detach from the scene tree immediately and free safely.
            p_target_node->remove_child(child);
            child->queue_free();
        }

        if (p_model.is_valid()) {
            _do_add_submodels(*p_target_node, p_target_node, p_model->get_submodels(), editable);
        }
    }

} // namespace godot
