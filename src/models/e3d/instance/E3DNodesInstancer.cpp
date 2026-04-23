#include "E3DNodesInstancer.hpp"
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "models/MaterialManager.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/memory.hpp>

namespace godot {
    Ref<Material> E3DNodesInstancer::colored_material;
    Node3D * E3DNodesInstancer::_create_submodel_instance(const E3DModelInstance &p_target_node, const E3DSubModel &p_submodel) {
        if (p_submodel.get_skip_rendering()) {
            return nullptr;
        }

        switch (p_submodel.get_submodel_type()) {
            case E3DSubModel::TRANSFORM: {
                Node3D *obj = memnew(Node3D);
                obj->set_name(StringName(p_submodel.get_name()));
                obj->set_visible(p_submodel.get_visible());
                return obj;
            }

            case E3DSubModel::GL_TRIANGLES: {
                if (const bool is_name_excluded = p_target_node.get_exclude_node_names().has(p_submodel.get_name()); !is_name_excluded) {
                    MeshInstance3D *mesh_instance = memnew(MeshInstance3D);
                    mesh_instance->set_name(StringName(p_submodel.get_name()));
                    mesh_instance->set_mesh(p_submodel.get_mesh());
                    mesh_instance->set_visibility_range_begin(p_submodel.get_visibility_range_begin());
                    mesh_instance->set_visibility_range_end(p_submodel.get_visibility_range_end());
                    mesh_instance->set_visible(p_submodel.get_visible());
                    _update_submodel_material(p_target_node, *mesh_instance, p_submodel);
                    return mesh_instance;
                }
            }
            default:;
        }
        return nullptr;
    }

    void E3DNodesInstancer::_do_add_submodels(
            const E3DModelInstance &p_target_node, Node3D *p_parent, const TypedArray<E3DSubModel> &p_submodels,
            const bool p_editable) {
        if (p_parent == nullptr) {
            return;
        }
        for (const Variant &i: p_submodels) {
            Ref<E3DSubModel> submodel_ref = i;

            const E3DSubModel *submodel = submodel_ref.ptr();
            if (submodel == nullptr) {
                continue;
            }

            Node3D *child = _create_submodel_instance(p_target_node, *submodel);
            if (child == nullptr) {
                continue;
            }

            const InternalMode internal = p_editable ? Node::INTERNAL_MODE_DISABLED : Node::INTERNAL_MODE_BACK;
            p_parent->add_child(child, false, internal);

            // Apply transform AFTER adding to the tree (important especially on Windows)
            child->set_transform(submodel->get_transform());

            if ((Engine::get_singleton() != nullptr) && Engine::get_singleton()->is_editor_hint()) {
                Node *owner = nullptr;
                if (p_editable) {
                    owner = p_target_node.get_owner();
                    if (owner == nullptr) {
                        owner = const_cast<E3DModelInstance *>(&p_target_node);
                    }
                }

                if (owner != nullptr) {
                    child->set_owner(owner);
                }
            }

            if (TypedArray<E3DSubModel> children = submodel->get_submodels(); children.size() > 0) {
                _do_add_submodels(p_target_node, child, children, p_editable);
            }
        }
    }

    void E3DNodesInstancer::_update_submodel_material(
            const E3DModelInstance &p_target_node, Node3D &p_subnode, const E3DSubModel &p_submodel) {
        const String unprefixed_model_path = String("/").join(p_target_node.get_data_path().split("/").slice(1));
        MeshInstance3D *mesh_instance = cast_to<MeshInstance3D>(&p_subnode);
        if (mesh_instance == nullptr) {
            return;
        }

        MaterialManager *mm = cast_to<MaterialManager>(Engine::get_singleton()->get_singleton("MaterialManager"));
        if (mm == nullptr) {
            return;
        }

        String material_name = p_submodel.get_material_name();
        if (p_submodel.get_dynamic_material()) {
            const Array skins = p_target_node.get_skins();
            if (const int idx = p_submodel.get_dynamic_material_index(); skins.size() > idx) {
                material_name = skins.get(idx);
            } else {
                UtilityFunctions::push_warning( "Model " + p_target_node.get_name() + " has less skins than dynamic material index " + String::num_int64(idx));
            }
        }

        MaterialManager::Transparency transparency = MaterialManager::DISABLED;
        if (p_submodel.get_material_transparent()) {
            transparency = MaterialManager::ALPHA;
        }

        Ref<Material> mat;
        if (p_submodel.get_material_colored()) {
            mat = get_colored_material();
            if (mat.is_valid()) {
                const Ref<StandardMaterial3D> sm = mat;
                sm->set_albedo(p_submodel.get_diffuse_color());
            }
        } else {
            mat = mm->get_material(unprefixed_model_path, material_name, transparency, false, p_submodel.get_diffuse_color());
        }

        if (mat.is_valid()) {
            mesh_instance->set_surface_override_material(0, mat);
        }
    }

    void E3DNodesInstancer::_bind_methods() {
        ClassDB::bind_static_method(
                get_class_static(), D_METHOD("instantiate", "model", "target_node", "editable"),
                &E3DNodesInstancer::instantiate);
    }

    void E3DNodesInstancer::cleanup() {
        colored_material.unref();
    }

    Ref<Material> E3DNodesInstancer::get_colored_material() {
        if (!colored_material.is_valid()) {
            if ((Engine::get_singleton() != nullptr) && !Engine::get_singleton()->is_editor_hint()) {
                const String path = "res://addons/libmaszyna/e3d/colored.material";
                if (const Ref<Material> res = ResourceLoader::get_singleton()->load(path, "Material", ResourceLoader::CACHE_MODE_REUSE); res.is_valid()) {
                    colored_material = res;
                }
            }
        }
        return colored_material;
    }

    void E3DNodesInstancer::instantiate(const Ref<E3DModel> &p_model, E3DModelInstance *p_target_node, const bool p_editable) {
        if (p_target_node == nullptr) {
            return;
        }

        const int32_t child_count = p_target_node->get_child_count(true);
        for (int32_t i = child_count - 1; i >= 0; --i) {
            Node *child = p_target_node->get_child(i, true);
            if (child == nullptr) {
                continue;
            }

            p_target_node->remove_child(child);
            child->queue_free();
        }

        if (p_model.is_valid()) {
            _do_add_submodels(*p_target_node, p_target_node, p_model->get_submodels(), p_editable);
        }
    }
} // namespace godot
