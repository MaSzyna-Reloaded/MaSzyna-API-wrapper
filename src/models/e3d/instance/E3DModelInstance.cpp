#include "E3DModelInstance.hpp"
#include "E3DModelInstanceManager.hpp"
#include "E3DNodesInstancer.hpp"
#include "E3DRenderingServerInstancer.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/world3d.hpp>

namespace godot {
    const char *E3DModelInstance::E3D_LOADED_SIGNAL = "e3d_loaded";

    static AABB _calculate_submodels_aabb(const TypedArray<E3DSubModel> &submodels, const Transform3D &parent_transform, bool &has_aabb) {
        AABB merged;

        for (const Variant &item: submodels) {
            const Ref<E3DSubModel> submodel_ref = item;
            const E3DSubModel *submodel = submodel_ref.ptr();
            if (submodel == nullptr) {
                continue;
            }

            const Transform3D current_transform = parent_transform * submodel->get_transform();

            if (const Ref<ArrayMesh> mesh = submodel->get_mesh(); mesh.is_valid()) {
                const AABB mesh_aabb = current_transform.xform(mesh->get_aabb());
                if (mesh_aabb.has_surface()) {
                    if (has_aabb) {
                        merged = merged.merge(mesh_aabb);
                    } else {
                        merged = mesh_aabb;
                        has_aabb = true;
                    }
                }
            }

            if (TypedArray<E3DSubModel> children = submodel->get_submodels(); children.size() > 0) {
                bool has_child_aabb = false;
                const AABB child_aabb = _calculate_submodels_aabb(children, current_transform, has_child_aabb);
                if (has_child_aabb) {
                    if (has_aabb) {
                        merged = merged.merge(child_aabb);
                    } else {
                        merged = child_aabb;
                        has_aabb = true;
                    }
                }
            }
        }

        return merged;
    }

    E3DModelInstance::E3DModelInstance()
        : data_path(String(), [this] { _reload(); })
        , model_filename(String(), [this] { _reload(); })
        , skins(Array(), [this] { _reload(); })
        , exclude_node_names(Array(), [this] { _reload(); })
        , instancer(INSTANCER_NODES, [this] { _reload(); })
        , submodels_aabb(AABB())
        , editable_in_editor(false, [this] { _reload(); })
        {
        _mutex.instantiate();
        set_notify_transform(true);
        set_notify_local_transform(true);
    }

    E3DModelInstance::~E3DModelInstance() {
        _clear_optimized_instances();
        _mutex.unref();
    }

    E3DModelInstanceManager *E3DModelInstance::_get_manager() const {
        return cast_to<E3DModelInstanceManager>(Engine::get_singleton()->get_singleton("E3DModelInstanceManager"));
    }

    void E3DModelInstance::_bind_methods() {
        ADD_SIGNAL(MethodInfo(E3D_LOADED_SIGNAL));
        BIND_PROPERTY(Variant::VECTOR3, "default_aabb_size", "default_aabb_size", &E3DModelInstance::set_default_aabb_size, &E3DModelInstance::get_default_aabb_size, "default_aabb_size");
        BIND_PROPERTY(Variant::STRING, "data_path", "data_path", &E3DModelInstance::set_data_path, &E3DModelInstance::get_data_path, "data_path");
        BIND_PROPERTY(Variant::STRING, "model_filename", "model_filename", &E3DModelInstance::set_model_filename, &E3DModelInstance::get_model_filename, "model_filename");
        BIND_PROPERTY_ARRAY("skins", "skins", &E3DModelInstance::set_skins, &E3DModelInstance::get_skins, "skins");
        BIND_PROPERTY_ARRAY("exclude_node_names", "exclude_node_names", &E3DModelInstance::set_exclude_node_names, &E3DModelInstance::get_exclude_node_names, "exclude_node_names");
        BIND_PROPERTY_W_HINT(Variant::INT, "instancer", "instancer", &E3DModelInstance::set_instancer, &E3DModelInstance::get_instancer, "instancer", PROPERTY_HINT_ENUM, "Optimized,Nodes,Editable nodes");
        BIND_PROPERTY(Variant::AABB, "submodels_aabb", "submodels_aabb", &E3DModelInstance::set_submodels_aabb, &E3DModelInstance::get_submodels_aabb, "submodels_aabb");
        BIND_PROPERTY(Variant::BOOL, "editable_in_editor", "editable_in_editor", &E3DModelInstance::set_editable_in_editor, &E3DModelInstance::get_editable_in_editor, "editable_in_editor");

        BIND_ENUM_CONSTANT(INSTANCER_NODES);
        BIND_ENUM_CONSTANT(INSTANCER_EDITABLE_NODES);
        BIND_ENUM_CONSTANT(INSTANCER_OPTIMIZED);

        ClassDB::bind_method(D_METHOD("_instantiate_children", "model"), &E3DModelInstance::_instantiate_children);
        ClassDB::bind_method(D_METHOD("_deferred_reload"), &E3DModelInstance::_deferred_reload);
        ClassDB::bind_method(D_METHOD("_flush_pending_model"), &E3DModelInstance::_flush_pending_model);
        ClassDB::bind_method(
                D_METHOD("set_submodel_material_override", "submodel_name", "material"),
                &E3DModelInstance::set_submodel_material_override);
        ClassDB::bind_method(
                D_METHOD("get_submodel_material_override", "submodel_name"),
                &E3DModelInstance::get_submodel_material_override);
        ClassDB::bind_method(
                D_METHOD("clear_submodel_material_override", "submodel_name"),
                &E3DModelInstance::clear_submodel_material_override);
    }

    void E3DModelInstance::_notification(const int p_what) {
        switch (p_what) {
            case NOTIFICATION_ENTER_TREE: {
                UtilityFunctions::print_verbose("[E3DModelInstance] Entering tree");
                _reload();
                if (E3DModelInstanceManager *manager = _get_manager()) {
                    manager->register_instance(this);
                }
            } break;

            case NOTIFICATION_ENTER_WORLD:
            case NOTIFICATION_TRANSFORM_CHANGED:
            case NOTIFICATION_LOCAL_TRANSFORM_CHANGED:
            case NOTIFICATION_VISIBILITY_CHANGED: {
                _sync_optimized_instances();
            } break;

            case NOTIFICATION_EXIT_TREE: {
                UtilityFunctions::print_verbose("[E3DModelInstance] Exiting tree");
                if (E3DModelInstanceManager *manager = _get_manager()) {
                    manager->unregister_instance(this);
                }
                _clear_optimized_instances();
            } break;
            default: ;
        }
    }

    void E3DModelInstance::_reload() {
        if (_is_dirty) {
            return;
        }
        _is_dirty = true;
        call_deferred("_deferred_reload");
    }

    void E3DModelInstance::_deferred_reload() {
        _is_dirty = false;
        if (const E3DModelInstanceManager *manager = _get_manager()) {
            manager->reload_instance(this);
        }
    }

    void E3DModelInstance::_instantiate_children(const Ref<E3DModel> &p_model) {
        _mutex->lock();
        const bool is_scheduled = _pending_model_scheduled;
        _pending_model = p_model;
        _pending_model_scheduled = true;
        _mutex->unlock();

        if (is_scheduled) {
            return;
        }

        call_deferred("_flush_pending_model");
    }

    void E3DModelInstance::_flush_pending_model() {
        _mutex->lock();
        const Ref<E3DModel> model = _pending_model;
        _pending_model_scheduled = false;
        _mutex->unlock();
        _loaded_model = model;

        bool has_model_aabb = false;
        const AABB model_aabb = model.is_valid()
                ? _calculate_submodels_aabb(model->get_submodels(), Transform3D(), has_model_aabb)
                : AABB();
        set_submodels_aabb(has_model_aabb ? model_aabb : AABB());

        _clear_optimized_instances();
        switch (get_instancer()) {
            case INSTANCER_OPTIMIZED: {
                E3DNodesInstancer::clear_children(this);
                const std::vector<E3DRenderingServerInstancer::InstanceRecord> instances =
                        E3DRenderingServerInstancer::instantiate(model, *this, RID());
                _optimized_instances.clear();
                _optimized_instances.reserve(instances.size());
                for (const E3DRenderingServerInstancer::InstanceRecord &record: instances) {
                    _optimized_instances.push_back({record.instance_rid, record.local_transform, record.visible, record.name});
                }
                if (is_inside_tree()) {
                    _sync_optimized_instances();
                }
            } break;
            case INSTANCER_EDITABLE_NODES:
                E3DNodesInstancer::instantiate(model, this, true);
                break;
            case INSTANCER_NODES:
            default:
                E3DNodesInstancer::instantiate(model, this, get_editable_in_editor());
                _apply_material_overrides_to_node(this);
                break;
        }
        emit_signal(E3D_LOADED_SIGNAL);
    }

    RID E3DModelInstance::_get_current_scenario() const {
        if (!is_inside_tree()) {
            return RID();
        }
        const Ref<World3D> world = get_world_3d();
        if (world.is_null()) {
            return RID();
        }
        return world->get_scenario();
    }

    void E3DModelInstance::_clear_optimized_instances() {
        RenderingServer *rs = RenderingServer::get_singleton();
        if (rs != nullptr) {
            for (const OptimizedInstanceRecord &record: _optimized_instances) {
                if (record.instance_rid.is_valid()) {
                    rs->free_rid(record.instance_rid);
                }
            }
        }
        _optimized_instances.clear();
    }

    void E3DModelInstance::_sync_optimized_instances() {
        if (get_instancer() != INSTANCER_OPTIMIZED || _optimized_instances.empty()) {
            return;
        }

        RenderingServer *rs = RenderingServer::get_singleton();
        if (rs == nullptr) {
            return;
        }

        const RID scenario = _get_current_scenario();
        if (!scenario.is_valid() || !is_inside_tree()) {
            return;
        }
        const Transform3D global_transform = get_global_transform();
        const bool visible = is_visible_in_tree();
        const uint32_t layer_mask = get_layer_mask();

        for (const OptimizedInstanceRecord &record: _optimized_instances) {
            if (!record.instance_rid.is_valid()) {
                continue;
            }

            rs->instance_set_scenario(record.instance_rid, scenario);
            rs->instance_set_layer_mask(record.instance_rid, layer_mask);
            rs->instance_set_transform(record.instance_rid, global_transform * record.local_transform);
            rs->instance_set_visible(record.instance_rid, visible && record.visible);

            const Variant override_value = _submodel_material_overrides.get(record.name, Variant());
            const Ref<Material> override_material = override_value;
            if (override_material.is_valid()) {
                rs->instance_geometry_set_material_override(record.instance_rid, override_material->get_rid());
            }
        }
    }

    void E3DModelInstance::_apply_material_overrides_to_node(Node *node) const {
        if (node == nullptr) {
            return;
        }

        if (MeshInstance3D *mesh_instance = cast_to<MeshInstance3D>(node)) {
            const Variant override_value = _submodel_material_overrides.get(String(mesh_instance->get_name()), Variant());
            const Ref<Material> override_material = override_value;
            if (override_material.is_valid()) {
                mesh_instance->set_material_override(override_material);
            }
        }

        const int child_count = node->get_child_count();
        for (int i = 0; i < child_count; ++i) {
            _apply_material_overrides_to_node(node->get_child(i));
        }
    }

    AABB E3DModelInstance::_get_aabb() const {
        const AABB stored_aabb = get_submodels_aabb();
        if (stored_aabb.has_surface()) {
            return stored_aabb;
        }

        if (_loaded_model.is_valid()) {
            bool has_model_aabb = false;
            const AABB model_aabb = _calculate_submodels_aabb(_loaded_model->get_submodels(), Transform3D(), has_model_aabb);
            if (has_model_aabb) {
                return model_aabb;
            }
        }

        const Vector3 default_size = get_default_aabb_size();
        return AABB(-default_size / 2.0, default_size);
    }

    void E3DModelInstance::set_submodel_material_override(const String &submodel_name, const Ref<Material> &material) {
        if (submodel_name.is_empty()) {
            return;
        }
        _submodel_material_overrides.set(submodel_name, material);

        if (get_instancer() == INSTANCER_OPTIMIZED) {
            _sync_optimized_instances();
        } else {
            _apply_material_overrides_to_node(this);
        }
    }

    Ref<Material> E3DModelInstance::get_submodel_material_override(const String &submodel_name) const {
        const Variant override_value = _submodel_material_overrides.get(submodel_name, Variant());
        const Ref<Material> override_material = override_value;
        return override_material;
    }

    void E3DModelInstance::clear_submodel_material_override(const String &submodel_name) {
        if (!_submodel_material_overrides.has(submodel_name)) {
            return;
        }
        _submodel_material_overrides.erase(submodel_name);
        _reload();
    }
}//namespace godot
