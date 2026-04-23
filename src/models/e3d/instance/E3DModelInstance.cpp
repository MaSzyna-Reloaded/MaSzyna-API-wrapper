#include "E3DModelInstance.hpp"
#include "E3DModelInstanceManager.hpp"
#include "E3DNodesInstancer.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/world3d.hpp>

namespace godot {
    const char *E3DModelInstance::e3d_loaded_signal = "e3d_loaded";

    E3DModelInstance::E3DModelInstance() {
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
        ADD_SIGNAL(MethodInfo(e3d_loaded_signal));
        BIND_PROPERTY(
                Variant::VECTOR3, "default_aabb_size", "default_aabb_size", &E3DModelInstance::set_default_aabb_size,
                &E3DModelInstance::get_default_aabb_size, "default_aabb_size");
        BIND_PROPERTY(
                Variant::STRING, "data_path", "data_path", &E3DModelInstance::set_data_path,
                &E3DModelInstance::get_data_path, "data_path");
        BIND_PROPERTY(
                Variant::STRING, "model_filename", "model_filename", &E3DModelInstance::set_model_filename,
                &E3DModelInstance::get_model_filename, "model_filename");
        BIND_PROPERTY_ARRAY("skins", "skins", &E3DModelInstance::set_skins, &E3DModelInstance::get_skins, "skins");
        BIND_PROPERTY_ARRAY(
                "exclude_node_names", "exclude_node_names", &E3DModelInstance::set_exclude_node_names,
                &E3DModelInstance::get_exclude_node_names, "exclude_node_names");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "instancer", "instancer", &E3DModelInstance::set_instancer,
                &E3DModelInstance::get_instancer, "instancer", PROPERTY_HINT_ENUM, "Optimized,Nodes,Editable nodes");
        BIND_PROPERTY(
                Variant::AABB, "submodels_aabb", "submodels_aabb", &E3DModelInstance::set_submodels_aabb,
                &E3DModelInstance::get_submodels_aabb, "submodels_aabb");
        BIND_PROPERTY(
                Variant::BOOL, "editable_in_editor", "editable_in_editor", &E3DModelInstance::set_editable_in_editor,
                &E3DModelInstance::get_editable_in_editor, "editable_in_editor");

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
                _reload();
                if (E3DModelInstanceManager *manager = _get_manager()) {
                    manager->register_instance(this);
                }
            } break;

            case NOTIFICATION_ENTER_WORLD:
            case NOTIFICATION_VISIBILITY_CHANGED:
            case NOTIFICATION_TRANSFORM_CHANGED:
            case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
                if (instancer == INSTANCER_OPTIMIZED) {
                    _sync_optimized_instances();
                }
            } break;

            case NOTIFICATION_EXIT_TREE: {
                if (E3DModelInstanceManager *manager = _get_manager()) {
                    manager->unregister_instance(this);
                }
                _clear_optimized_instances();
            } break;
            default:;
        }
    }

    void E3DModelInstance::_request_reload() {
        if (!is_inside_tree()) {
            return;
        }

        _reload();
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

    void E3DModelInstance::_collect_optimized_submodels(
            const TypedArray<E3DSubModel> &p_submodels, const Transform3D &p_parent_transform, bool p_parent_visible) {

        RenderingServer *rs = RenderingServer::get_singleton();
        for (int i = 0; i < p_submodels.size(); ++i) {
            const Ref<E3DSubModel> submodel = p_submodels[i];
            if (submodel.is_null() || submodel->get_skip_rendering()) {
                continue;
            }

            const Transform3D current_transform = p_parent_transform * submodel->get_transform();
            const bool current_visible = p_parent_visible && submodel->get_visible();

            if (submodel->get_submodels().size() > 0) {
                _collect_optimized_submodels(submodel->get_submodels(), current_transform, current_visible);
            }

            if (submodel->get_submodel_type() == E3DSubModel::GL_TRIANGLES &&
                !exclude_node_names.has(submodel->get_name())) {
                const Ref<ArrayMesh> mesh = submodel->get_mesh();
                if (mesh.is_valid()) {
                    const RID instance = rs->instance_create2(mesh->get_rid(), RID());
                    rs->instance_geometry_set_visibility_range(
                            instance, submodel->get_visibility_range_begin(), submodel->get_visibility_range_end(), 0,
                            0, RenderingServer::VISIBILITY_RANGE_FADE_DISABLED);

                    if (const Ref<Material> mat = E3DNodesInstancer::resolve_submodel_material(*this, *submodel.ptr());
                        mat.is_valid()) {
                        rs->instance_geometry_set_material_override(instance, mat->get_rid());
                    }

                    _optimized_instances.push_back(
                            {instance, current_transform, current_visible, submodel->get_name()});
                }
            }
        }
    }

    void E3DModelInstance::_flush_pending_model() {
        _mutex->lock();
        const Ref<E3DModel> model = _pending_model;
        _pending_model_scheduled = false;
        _mutex->unlock();
        _loaded_model = model;

        set_submodels_aabb(model.is_valid() ? model->get_aabb() : AABB());
        _clear_optimized_instances();

        if (instancer == INSTANCER_OPTIMIZED) {
            E3DNodesInstancer::clear_children(this);
            if (model.is_valid()) {
                _collect_optimized_submodels(model->get_submodels(), Transform3D(), true);
                _sync_optimized_instances();
            }
        } else {
            E3DNodesInstancer::instantiate(
                    model, this,
                    instancer == INSTANCER_EDITABLE_NODES || (instancer == INSTANCER_NODES && editable_in_editor));
        }

        emit_signal(e3d_loaded_signal);
    }

    void E3DModelInstance::_clear_optimized_instances() {
        RenderingServer *rs = RenderingServer::get_singleton();
        for (const auto &record: _optimized_instances) {
            if (record.instance_rid.is_valid()) {
                rs->free_rid(record.instance_rid);
            }
        }
        _optimized_instances.clear();
    }

    void E3DModelInstance::_sync_optimized_instances() {
        if (!is_inside_tree() || _optimized_instances.empty()) {
            return;
        }
        RenderingServer *rs = RenderingServer::get_singleton();
        const RID scenario = get_world_3d()->get_scenario();
        const Transform3D gt = get_global_transform();
        const bool tree_visible = is_visible_in_tree();
        const uint32_t layer_mask = get_layer_mask();

        for (const auto &record: _optimized_instances) {
            rs->instance_set_scenario(record.instance_rid, scenario);
            rs->instance_set_transform(record.instance_rid, gt * record.local_transform);
            rs->instance_set_visible(record.instance_rid, tree_visible && record.visible);
            rs->instance_set_layer_mask(record.instance_rid, layer_mask);

            const Ref<Material> mat = _submodel_material_overrides.get(record.name, Variant());
            if (mat.is_valid()) {
                rs->instance_geometry_set_material_override(record.instance_rid, mat->get_rid());
            }
        }
    }

    AABB E3DModelInstance::_get_aabb() const {
        if (submodels_aabb.has_surface()) {
            return submodels_aabb;
        }
        if (_loaded_model.is_valid()) {
            return _loaded_model->get_aabb();
        }
        return AABB(-default_aabb_size / 2.0, default_aabb_size);
    }

    void
    E3DModelInstance::set_submodel_material_override(const String &p_submodel_name, const Ref<Material> &p_material) {
        if (p_submodel_name.is_empty()) {
            return;
        }
        _submodel_material_overrides.set(p_submodel_name, p_material);
        if (instancer == INSTANCER_OPTIMIZED) {
            _sync_optimized_instances();
        } else {
            _reload();
        }
    }

    Ref<Material> E3DModelInstance::get_submodel_material_override(const String &p_submodel_name) const {
        return _submodel_material_overrides.get(p_submodel_name, Variant());
    }

    void E3DModelInstance::clear_submodel_material_override(const String &p_submodel_name) {
        _submodel_material_overrides.erase(p_submodel_name);
        _reload();
    }

    String E3DModelInstance::get_data_path() const {
        return data_path;
    }

    void E3DModelInstance::set_data_path(const String &p_value) {
        if (data_path == p_value) {
            return;
        }

        data_path = p_value;
        _request_reload();
    }

    String E3DModelInstance::get_model_filename() const {
        return model_filename;
    }

    void E3DModelInstance::set_model_filename(const String &p_value) {
        if (model_filename == p_value) {
            return;
        }

        model_filename = p_value;
        _request_reload();
    }

    Array E3DModelInstance::get_skins() const {
        return skins;
    }

    void E3DModelInstance::set_skins(const Array &p_value) {
        if (skins == p_value) {
            return;
        }

        skins = p_value;
        _request_reload();
    }

    Array E3DModelInstance::get_exclude_node_names() const {
        return exclude_node_names;
    }

    void E3DModelInstance::set_exclude_node_names(const Array &p_value) {
        if (exclude_node_names == p_value) {
            return;
        }

        exclude_node_names = p_value;
        _request_reload();
    }

    E3DModelInstance::Instancer E3DModelInstance::get_instancer() const {
        return instancer;
    }

    void E3DModelInstance::set_instancer(const Instancer p_value) {
        if (instancer == p_value) {
            return;
        }

        instancer = p_value;
        _request_reload();
    }

    AABB E3DModelInstance::get_submodels_aabb() const {
        return submodels_aabb;
    }

    void E3DModelInstance::set_submodels_aabb(const AABB &p_value) {
        if (submodels_aabb == p_value) {
            return;
        }

        submodels_aabb = p_value;
    }

    bool E3DModelInstance::get_editable_in_editor() const {
        return editable_in_editor;
    }

    void E3DModelInstance::set_editable_in_editor(const bool &p_value) {
        if (editable_in_editor == p_value) {
            return;
        }

        editable_in_editor = p_value;
        _request_reload();
    }
} // namespace godot
