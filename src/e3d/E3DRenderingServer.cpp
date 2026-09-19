#include "E3DRenderingServer.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void E3DRenderingServer::_bind_methods() {
        ClassDB::bind_method(D_METHOD("instance_create", "model", "instancer"), &E3DRenderingServer::instance_create);
        ClassDB::bind_method(D_METHOD("instance_free", "instance"), &E3DRenderingServer::instance_free);
        ClassDB::bind_method(D_METHOD("instance_build", "instance"), &E3DRenderingServer::instance_build);
        ClassDB::bind_method(
                D_METHOD(
                        "instance_set_options", "instance", "data_path", "skins", "exclude_node_names", "force_alpha",
                        "force_alpha_submodel_paths"),
                &E3DRenderingServer::instance_set_options);
        ClassDB::bind_method(
                D_METHOD("instance_attach_node", "instance", "node"), &E3DRenderingServer::instance_attach_node);
        ClassDB::bind_method(
                D_METHOD("instance_set_scenario", "instance", "scenario"), &E3DRenderingServer::instance_set_scenario);
        ClassDB::bind_method(
                D_METHOD("instance_set_transform", "instance", "transform"),
                &E3DRenderingServer::instance_set_transform);
        ClassDB::bind_method(
                D_METHOD("instance_set_visible", "instance", "visible"), &E3DRenderingServer::instance_set_visible);
        ClassDB::bind_method(
                D_METHOD("instance_set_layer_mask", "instance", "mask"), &E3DRenderingServer::instance_set_layer_mask);
        ClassDB::bind_method(
                D_METHOD("instance_set_visibility_range", "instance", "begin", "end"),
                &E3DRenderingServer::instance_set_visibility_range);
        ClassDB::bind_method(
                D_METHOD("instance_set_lights_state", "instance", "lights_state"),
                &E3DRenderingServer::instance_set_lights_state);
        ClassDB::bind_method(
                D_METHOD("set_material_resolver", "material_resolver"), &E3DRenderingServer::set_material_resolver);

        BIND_ENUM_CONSTANT(INSTANCER_OPTIMIZED);
        BIND_ENUM_CONSTANT(INSTANCER_NODES);
        BIND_ENUM_CONSTANT(INSTANCER_EDITABLE_NODES);
    }

    E3DRenderingServer::~E3DRenderingServer() {
        // Nodes built by the NODES backends belong to the scene tree, only RenderingServer RIDs are freed here
        for (KeyValue<RID, E3DInstanceData> &item: instances) {
            if (item.value.instancer == INSTANCER_OPTIMIZED) {
                optimized_backend.clear(item.value);
            }
        }
        instances.clear();
    }

    E3DInstanceBackend &E3DRenderingServer::_get_backend(const E3DInstanceData &p_instance) {
        switch (p_instance.instancer) {
            case INSTANCER_NODES:
                return nodes_backend;
            case INSTANCER_EDITABLE_NODES:
                return editable_nodes_backend;
            default:
                return optimized_backend;
        }
    }

    void E3DRenderingServer::_rebuild_if_built(E3DInstanceData &p_instance) {
        if (p_instance.built) {
            E3DInstanceBackend &backend = _get_backend(p_instance);
            backend.clear(p_instance);
            backend.build(p_instance, material_resolver);
        }
    }

    void E3DRenderingServer::_update_if_built(E3DInstanceData &p_instance) {
        if (p_instance.built) {
            _get_backend(p_instance).update(p_instance);
        }
    }

    /// Creates an empty instance; set it up with instance_set_*() and instance_attach_node(),
    /// then call instance_build().
    RID E3DRenderingServer::instance_create(const Ref<E3DModel> &p_model, const Instancer p_instancer) {
        ERR_FAIL_COND_V(p_model.is_null(), RID());
        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        E3DInstanceData &instance = instances[rid];
        instance.model = p_model;
        instance.instancer = p_instancer;
        return rid;
    }

    void E3DRenderingServer::instance_free(const RID &p_instance) {
        const HashMap<RID, E3DInstanceData>::Iterator item = instances.find(p_instance);
        ERR_FAIL_COND(item == instances.end());
        if (item->value.built) {
            _get_backend(item->value).clear(item->value);
        }
        instances.remove(item);
    }

    /// Builds (or rebuilds) the instance content. Later changes of options, attached node and
    /// scenario rebuild it again, other setters only update it.
    void E3DRenderingServer::instance_build(const RID &p_instance) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        E3DInstanceBackend &backend = _get_backend(*instance);
        if (instance->built) {
            backend.clear(*instance);
        }
        instance->built = true;
        backend.build(*instance, material_resolver);
    }

    void E3DRenderingServer::instance_set_options(
            const RID &p_instance, const String &p_data_path, const PackedStringArray &p_skins,
            const Array &p_exclude_node_names, const bool p_force_alpha,
            const TypedArray<NodePath> &p_force_alpha_submodel_paths) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        instance->data_path = p_data_path;
        instance->skins = p_skins;
        instance->exclude_node_names = p_exclude_node_names;
        instance->force_alpha = p_force_alpha;
        instance->force_alpha_submodel_paths = p_force_alpha_submodel_paths;
        _rebuild_if_built(*instance);
    }

    /// NODES/EDITABLE_NODES build their node tree under [param p_node]; OPTIMIZED instances
    /// report it as their owner (e.g. for picking in the editor).
    void E3DRenderingServer::instance_attach_node(const RID &p_instance, Node3D *p_node) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        instance->node_id = p_node != nullptr ? ObjectID(p_node->get_instance_id()) : ObjectID();
        _rebuild_if_built(*instance);
    }

    void E3DRenderingServer::instance_set_scenario(const RID &p_instance, const RID &p_scenario) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        instance->scenario = p_scenario;
        _rebuild_if_built(*instance);
    }

    /// Global transform of an OPTIMIZED instance (a NODES tree follows its attached node)
    void E3DRenderingServer::instance_set_transform(const RID &p_instance, const Transform3D &p_transform) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        instance->transform = p_transform;
        _update_if_built(*instance);
    }

    void E3DRenderingServer::instance_set_visible(const RID &p_instance, const bool p_visible) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        instance->visible = p_visible;
        _update_if_built(*instance);
    }

    void E3DRenderingServer::instance_set_layer_mask(const RID &p_instance, const uint32_t p_mask) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        instance->layer_mask = p_mask;
        _update_if_built(*instance);
    }

    /// Limits the visibility range of all submodels of an OPTIMIZED instance (0 - no limit)
    void
    E3DRenderingServer::instance_set_visibility_range(const RID &p_instance, const float p_begin, const float p_end) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        instance->visibility_range_begin = p_begin;
        instance->visibility_range_end = p_end;
        _rebuild_if_built(*instance);
    }

    /// Light name -> enabled; shows the "on" or "off" submodels of the model's lights
    void E3DRenderingServer::instance_set_lights_state(const RID &p_instance, const Dictionary &p_lights_state) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        instance->lights_state = p_lights_state.duplicate();
        _update_if_built(*instance);
    }

    /// `material_resolver(submodel: E3DSubModel, data_path: String, skins: PackedStringArray,
    /// force_alpha: bool) -> Material`, used by instance_build()
    void E3DRenderingServer::set_material_resolver(const Callable &p_material_resolver) {
        material_resolver.set_callable(p_material_resolver);
    }
} // namespace godot
