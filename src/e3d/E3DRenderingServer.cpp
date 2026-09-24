#include "../scenery/SceneryStreamingServer.hpp"
#include "E3DRenderingServer.hpp"
#include <godot_cpp/classes/gpu_particles3d.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/mutex_lock.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void E3DRenderingServer::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("instance_create", "model", "instancer", "instance_kind"),
                &E3DRenderingServer::instance_create);
        ClassDB::bind_method(
                D_METHOD(
                        "instance_register", "data_path", "model_filename", "skins", "transform", "range_begin",
                        "range_end", "scenario"),
                &E3DRenderingServer::instance_register);
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
                D_METHOD("instance_set_lights_modes", "instance", "modes"),
                &E3DRenderingServer::instance_set_lights_modes);
        ClassDB::bind_method(
                D_METHOD("instance_set_lights_colors", "instance", "colors"),
                &E3DRenderingServer::instance_set_lights_colors);
        ClassDB::bind_method(
                D_METHOD("emission_light_create", "instance", "light_name"),
                &E3DRenderingServer::emission_light_create);
        ClassDB::bind_method(
                D_METHOD("spot_light_create", "instance", "light_name", "submodel_path"),
                &E3DRenderingServer::spot_light_create);
        ClassDB::bind_method(
                D_METHOD("omni_light_create", "instance", "light_name", "submodel_path"),
                &E3DRenderingServer::omni_light_create);
        ClassDB::bind_method(D_METHOD("light_free", "light"), &E3DRenderingServer::light_free);
        ClassDB::bind_method(D_METHOD("light_enable", "light"), &E3DRenderingServer::light_enable);
        ClassDB::bind_method(D_METHOD("light_disable", "light"), &E3DRenderingServer::light_disable);
        ClassDB::bind_method(D_METHOD("get_light_statistics"), &E3DRenderingServer::get_light_statistics);
        ClassDB::bind_method(
                D_METHOD("instance_set_smoke_intensity", "instance", "intensity"),
                &E3DRenderingServer::instance_set_smoke_intensity);
        ClassDB::bind_method(D_METHOD("get_smoke_statistics"), &E3DRenderingServer::get_smoke_statistics);
        ClassDB::bind_method(D_METHOD("set_current_time", "hours"), &E3DRenderingServer::set_current_time);
        ClassDB::bind_method(D_METHOD("set_light_level", "level"), &E3DRenderingServer::set_light_level);
        ClassDB::bind_method(D_METHOD("set_wind", "strength", "direction"), &E3DRenderingServer::set_wind);
        ClassDB::bind_method(D_METHOD("set_wind_strength", "strength"), &E3DRenderingServer::set_wind_strength);
        ClassDB::bind_method(D_METHOD("set_wind_direction", "direction"), &E3DRenderingServer::set_wind_direction);
        ClassDB::bind_method(
                D_METHOD("set_material_resolver", "material_resolver"), &E3DRenderingServer::set_material_resolver);
        ClassDB::bind_method(D_METHOD("set_model_loader", "model_loader"), &E3DRenderingServer::set_model_loader);
        ClassDB::bind_method(
                D_METHOD("set_smoke_source_resolver", "smoke_source_resolver"),
                &E3DRenderingServer::set_smoke_source_resolver);

        BIND_ENUM_CONSTANT(INSTANCER_OPTIMIZED);
        BIND_ENUM_CONSTANT(INSTANCER_NODES);
        BIND_ENUM_CONSTANT(INSTANCER_EDITABLE_NODES);

        BIND_ENUM_CONSTANT(INSTANCE_KIND_STATIC);
        BIND_ENUM_CONSTANT(INSTANCE_KIND_DYNAMIC);

        BIND_ENUM_CONSTANT(LIGHT_MODE_OFF);
        BIND_ENUM_CONSTANT(LIGHT_MODE_ON);
        BIND_ENUM_CONSTANT(LIGHT_MODE_BLINK);
        BIND_ENUM_CONSTANT(LIGHT_MODE_DARK);
        BIND_ENUM_CONSTANT(LIGHT_MODE_HOME);
    }

    E3DRenderingServer::E3DRenderingServer() {
        models_mutex.instantiate();
    }

    E3DRenderingServer::~E3DRenderingServer() {
        // Nodes built by the NODES backends belong to the scene tree, only RenderingServer RIDs are freed here
        for (KeyValue<RID, LightObject> &light: lights) {
            _light_clear(light.key);
        }
        lights.clear();
        for (KeyValue<RID, SmokeObject> &smoke: smoke_objects) {
            _smoke_clear(smoke.key);
        }
        smoke_objects.clear();
        smoke_order.clear();
        _set_smoke_processing(false);
        for (KeyValue<RID, E3DInstanceData> &item: instances) {
            item.value.light_objects.clear();
            item.value.smoke_objects.clear();
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
    ///
    /// The kind is given here rather than through a setter because it cannot be changed once the
    /// instance is built: the smoke density it selects is baked into every emitter - its process
    /// material, its particle budget and its spawn rate. A client that has to change it frees the
    /// instance and creates it again, which is what E3DModelInstance's own _dirty does.
    RID E3DRenderingServer::instance_create(
            const Ref<E3DModel> &p_model, const Instancer p_instancer, const InstanceKind p_instance_kind) {
        ERR_FAIL_COND_V(p_model.is_null(), RID());
        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        E3DInstanceData &instance = instances[rid];
        instance.model = p_model;
        instance.instancer = p_instancer;
        instance.instance_kind = p_instance_kind;
        return rid;
    }

    void E3DRenderingServer::instance_free(const RID &p_instance) {
        E3DInstanceData *found = instances.getptr(p_instance);
        ERR_FAIL_NULL(found);
        // Taken out of the registry before anything else runs: freeing a stream, a light or a
        // smoke source re-enters this server, and an instance created or freed in between
        // rehashes `instances` - which would leave this holding a dead entry. Cold caches make
        // that overlap routine, because a vehicle is still building models while a scenery is
        // being torn down.
        const E3DInstanceData data = *found;
        instances.erase(p_instance);

        if (data.stream_rid.is_valid()) {
            if (SceneryStreamingServer *streaming = SceneryStreamingServer::get_instance();
                streaming != nullptr) {
                streaming->stream_free(data.stream_rid);
            }
            MutexLock lock(**models_mutex);
            stream_models.erase(p_instance);
        }
        E3DInstanceData clearing = data;
        _clear_instance_lights(clearing);
        _clear_instance_smoke_sources(clearing);
        if (clearing.built) {
            _get_backend(clearing).clear(clearing);
        }
    }

    /// Builds (or rebuilds) the instance content. Later changes of options, attached node and
    /// scenario rebuild it again, other setters only update it.
    void E3DRenderingServer::instance_build(const RID &p_instance) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        E3DInstanceBackend &backend = _get_backend(*instance);
        if (instance->built) {
            _clear_instance_lights(*instance);
            _clear_instance_smoke_sources(*instance);
            backend.clear(*instance);
            instance->built = false;
        }
        // Identifying the model's lights is neither instancer's job - both only render what this
        // lists. Resolved before the build too, because the backend applies lights_state as it
        // builds.
        instance->model_lights = E3DLightFactory::discover(instance->model, instance->model_filename);
        _resolve_lights(*instance);
        instance->built = true;
        backend.build(*instance, material_resolver);
        _build_instance_lights(p_instance, *instance);
        _build_instance_smoke_sources(p_instance, *instance);
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
        if (instance->built) {
            _get_backend(*instance).apply_transform(*instance);
        }
        _update_instance_smoke(*instance);
    }

    void E3DRenderingServer::instance_set_visible(const RID &p_instance, const bool p_visible) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        instance->visible = p_visible;
        _update_if_built(*instance);
        _update_instance_smoke(*instance);
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

    /// Light name -> enabled; shows the "on" or "off" submodels of the model's lights. A value set
    /// here is a manual override and wins over the mode the scenery node declared.
    void E3DRenderingServer::instance_set_lights_state(const RID &p_instance, const Dictionary &p_lights_state) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        instance->lights_override = p_lights_state.duplicate();
        _resolve_lights(*instance);
        _update_if_built(*instance);
    }

    /// The `lights` list of a scenery model node, by light index: `lights 3` means light 0 is
    /// LIGHT_MODE_DARK. The index maps to the name the E3D parser gave the light_onNN submodel
    /// pair, exactly as the original binds Light_On00..07 by slot (AnimModel.cpp:303-317).
    void E3DRenderingServer::instance_set_lights_modes(const RID &p_instance, const PackedFloat32Array &p_modes) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        for (int i = 0; i < p_modes.size(); i++) {
            instance->light_declarations[_light_name_for_index(i)].mode = p_modes[i];
        }
        _resolve_lights(*instance);
        _update_if_built(*instance);
    }

    /// The `lightcolors` list of a scenery model node, in the same order. It overrides the colour
    /// the light submodel carries (SetDiffuseOverride(), AnimModel.cpp:625).
    void E3DRenderingServer::instance_set_lights_colors(const RID &p_instance, const PackedColorArray &p_colors) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        for (int i = 0; i < p_colors.size(); i++) {
            E3DInstanceData::LightDeclaration &declaration = instance->light_declarations[_light_name_for_index(i)];
            declaration.color = p_colors[i];
            declaration.has_color = p_colors[i].r >= 0.0; // a negative colour is the data's "-1"
        }
        _rebuild_if_built(*instance); // the colour is applied when the light is created
    }

    /// `material_resolver(submodel: E3DSubModel, data_path: String, skins: PackedStringArray,
    /// force_alpha: bool) -> Material`, used by instance_build()
    void E3DRenderingServer::set_material_resolver(const Callable &p_material_resolver) {
        material_resolver.set_callable(p_material_resolver);
    }

    /// Registers a scenery model placement for streaming: nothing is loaded or built until the
    /// streaming camera comes within its visibility range of the chunk it falls into. A range of
    /// 0 (a scenery node that declares none) means the global draw distance - see
    /// SceneryStreamingServer.
    RID E3DRenderingServer::instance_register(
            const String &p_data_path, const String &p_model_filename, const PackedStringArray &p_skins,
            const Transform3D &p_transform, const float p_range_begin, const float p_range_end, const RID &p_scenario) {
        SceneryStreamingServer *streaming = SceneryStreamingServer::get_instance();
        ERR_FAIL_NULL_V(streaming, RID());
        if (stream_owner < 0) {
            stream_owner = streaming->owner_create(
                    callable_mp(this, &E3DRenderingServer::_stream_preload),
                    callable_mp(this, &E3DRenderingServer::_stream_build),
                    callable_mp(this, &E3DRenderingServer::_stream_clear));
        }

        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        E3DInstanceData &instance = instances[rid];
        instance.instancer = INSTANCER_OPTIMIZED;
        instance.model_filename = p_model_filename;
        instance.data_path = p_data_path;
        instance.skins = p_skins;
        instance.transform = p_transform;
        instance.visibility_range_begin = p_range_begin;
        instance.visibility_range_end = p_range_end;
        instance.scenario = p_scenario;
        {
            MutexLock lock(**models_mutex);
            StreamModel stream_model;
            stream_model.data_path = p_data_path;
            stream_model.model_filename = p_model_filename;
            stream_models[rid] = stream_model;
        }
        instance.stream_rid = streaming->stream_register(stream_owner, rid, p_transform.origin, p_range_end);
        return rid;
    }

    /// `model_loader(data_path: String, filename: String) -> E3DModel`, called on the streaming
    /// worker thread for registered instances entering the camera's range
    void E3DRenderingServer::set_model_loader(const Callable &p_model_loader) {
        MutexLock lock(**models_mutex);
        model_loader = p_model_loader;
    }

    /// `smoke_source_resolver(template_name: String, kind: InstanceKind) -> Dictionary` with the keys
    /// process_material/mesh/amount/lifetime/aabb, used by _smoke_build(). The template files live
    /// under the game's data/ directory, which is GDScript's business, not this server's.
    void E3DRenderingServer::set_smoke_source_resolver(const Callable &p_smoke_source_resolver) {
        smoke_source_resolver = p_smoke_source_resolver;
    }

    /// Memoized: a scenery places the same few hundred models thousands of times. Two threads
    /// loading the same model at once only duplicate work the loader itself caches.
    Ref<E3DModel> E3DRenderingServer::_load_model(const String &p_data_path, const String &p_model_filename) {
        const String key = p_data_path.path_join(p_model_filename);
        Callable loader;
        {
            MutexLock lock(**models_mutex);
            const Ref<E3DModel> *cached = models.getptr(key);
            if (cached != nullptr) {
                return *cached;
            }
            loader = model_loader;
        }
        const Ref<E3DModel> model =
                loader.is_valid() ? Ref<E3DModel>(loader.call(p_data_path, p_model_filename)) : Ref<E3DModel>();
        MutexLock lock(**models_mutex);
        models[key] = model;
        return model;
    }

    /// Streaming worker thread - the instances map belongs to the main thread, so the model path
    /// is read from the copy made by instance_register()
    Variant E3DRenderingServer::_stream_preload(const RID &p_instance) {
        StreamModel stream_model;
        {
            MutexLock lock(**models_mutex);
            const StreamModel *found = stream_models.getptr(p_instance);
            if (found == nullptr) {
                return Variant();
            }
            stream_model = *found;
        }
        return _load_model(stream_model.data_path, stream_model.model_filename);
    }

    void E3DRenderingServer::_stream_build(const RID &p_instance, const Variant &p_preloaded) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        if (instance == nullptr) {
            return;
        }
        const Ref<E3DModel> model = p_preloaded;
        if (model.is_null()) {
            return; // the loader already reported why
        }
        instance->model = model;
        instance_build(p_instance);
    }

    void E3DRenderingServer::_stream_clear(const RID &p_instance) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        if (instance == nullptr || !instance->built) {
            return;
        }
        _clear_instance_lights(*instance);
        _clear_instance_smoke_sources(*instance);
        _get_backend(*instance).clear(*instance);
        instance->built = false;
        instance->model.unref();
    }

    RID E3DRenderingServer::_light_create(
            const RID &p_instance, const String &p_light_name, const LightKind p_kind,
            const E3DLightParams &p_params, const bool p_synthesized) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL_V(instance, RID());

        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        LightObject &light = lights[rid];
        light.owner = p_instance;
        light.kind = p_kind;
        light.light_name = p_light_name;
        light.params = p_params;
        light.synthesized = p_synthesized;
        light.enabled = instance->lights_state.get(p_light_name, false);
        instance->light_objects.push_back(rid);

        if (p_kind == LIGHT_KIND_EMISSION) {
            return rid; // the backends switch the on/off submodels from lights_state
        }

        // A scenery light is streamed with a range of its own, far shorter than the model's: a
        // street lamp is visible from half a kilometre and lights fifteen metres. Anything built
        // directly (a node, a vehicle) gets its RenderingServer light right away instead - it is
        // not part of the streamed scenery and may well be moving.
        SceneryStreamingServer *streaming = SceneryStreamingServer::get_instance();
        if (instance->stream_rid.is_valid() && streaming != nullptr) {
            if (light_stream_owner < 0) {
                light_stream_owner = streaming->owner_create(
                        Callable(), callable_mp(this, &E3DRenderingServer::_light_stream_build),
                        callable_mp(this, &E3DRenderingServer::_light_clear));
            }
            const ProjectSettings *settings = ProjectSettings::get_singleton();
            const float distance =
                    settings->get_setting(SCENERY_LIGHT_DISTANCE_SETTING, DEFAULT_SCENERY_LIGHT_DISTANCE);
            const Vector3 position = (instance->transform * p_params.transform).origin;
            light.stream_rid = streaming->stream_register(light_stream_owner, rid, position, distance);
        } else {
            _light_build(rid);
        }
        return rid;
    }

    /// Creates the RenderingServer light of a spot/omni light object
    void E3DRenderingServer::_light_build(const RID &p_light) {
        LightObject *light = lights.getptr(p_light);
        if (light == nullptr || light->kind == LIGHT_KIND_EMISSION || light->light.is_valid()) {
            return;
        }
        const E3DInstanceData *instance = instances.getptr(light->owner);
        if (instance == nullptr) {
            return;
        }
        RenderingServer *rs = RenderingServer::get_singleton();
        ERR_FAIL_NULL(rs);

        const ProjectSettings *settings = ProjectSettings::get_singleton();
        const float energy_scale = settings->get_setting(SCENERY_LIGHT_ENERGY_SETTING, DEFAULT_SCENERY_LIGHT_ENERGY);
        const bool shadows = settings->get_setting(SCENERY_LIGHT_SHADOWS_SETTING, DEFAULT_SCENERY_LIGHT_SHADOWS);
        const float tint = settings->get_setting(SCENERY_LIGHT_TINT_SETTING, DEFAULT_SCENERY_LIGHT_TINT);
        const float fog_energy = settings->get_setting(
                SCENERY_LIGHT_VOLUMETRIC_FOG_ENERGY_SETTING, DEFAULT_SCENERY_LIGHT_VOLUMETRIC_FOG_ENERGY);

        light->light = light->kind == LIGHT_KIND_OMNI ? rs->omni_light_create() : rs->spot_light_create();
        rs->light_set_color(light->light, Color(1.0, 1.0, 1.0).lerp(light->params.color, tint));
        rs->light_set_param(light->light, RenderingServer::LIGHT_PARAM_ENERGY, light->params.energy * energy_scale);
        rs->light_set_param(light->light, RenderingServer::LIGHT_PARAM_RANGE, light->params.range);
        rs->light_set_param(light->light, RenderingServer::LIGHT_PARAM_ATTENUATION, light->params.attenuation);
        rs->light_set_param(light->light, RenderingServer::LIGHT_PARAM_SIZE, light->params.size);
        if (light->kind == LIGHT_KIND_SPOT) {
            rs->light_set_param(light->light, RenderingServer::LIGHT_PARAM_SPOT_ANGLE, light->params.spot_angle);
            rs->light_set_param(
                    light->light, RenderingServer::LIGHT_PARAM_SPOT_ATTENUATION, light->params.spot_attenuation);
        }
        rs->light_set_param(light->light, RenderingServer::LIGHT_PARAM_VOLUMETRIC_FOG_ENERGY, fog_energy);
        rs->light_set_shadow(light->light, shadows);
        if (shadows) {
            rs->light_set_shadow_caster_mask(light->light, ~SCENERY_LIGHT_OWNER_LAYER);
            rs->light_set_param(
                    light->light, RenderingServer::LIGHT_PARAM_SHADOW_BIAS,
                    light->kind == LIGHT_KIND_OMNI ? OMNI_LIGHT_SHADOW_BIAS : SPOT_LIGHT_SHADOW_BIAS);
            rs->light_set_param(
                    light->light, RenderingServer::LIGHT_PARAM_SHADOW_NORMAL_BIAS, LIGHT_SHADOW_NORMAL_BIAS);
            // Must be set explicitly, like the biases above: a RenderingServer light does not get
            // it from Light3D's constructor and starts with it on, which stripes the ground with
            // shadow acne. The setting carries the project's own quirk (the original renders
            // shadow maps with front faces culled, opengl33renderer.cpp:1634).
            rs->light_set_reverse_cull_face_mode(
                    light->light, settings->get_setting(LIGHTS_SHADOW_REVERSE_CULL_FACE_SETTING, true));
            // The light keeps reaching as far as it is streamed; only its shadow map stops early,
            // because a scenery puts 152 of these within 300 m
            const float distance =
                    settings->get_setting(SCENERY_LIGHT_DISTANCE_SETTING, DEFAULT_SCENERY_LIGHT_DISTANCE);
            rs->light_set_distance_fade(
                    light->light, true, distance, SCENERY_LIGHT_SHADOW_FADE_DISTANCE, distance * 0.25f);
        }

        light->light_instance = rs->instance_create();
        rs->instance_set_base(light->light_instance, light->light);
        rs->instance_set_scenario(light->light_instance, instance->scenario);
        rs->instance_set_transform(light->light_instance, instance->transform * light->params.transform);
        rs->instance_set_visible(light->light_instance, light->enabled);
        light->streamed_in = true;
    }

    /// The build callback of the light stream; separate from _light_build() only because
    /// SceneryStreamingServer passes the preloaded value along
    void E3DRenderingServer::_light_stream_build(const RID &p_light, const Variant &p_preloaded) {
        _light_build(p_light);
    }

    void E3DRenderingServer::_light_clear(const RID &p_light) {
        LightObject *light = lights.getptr(p_light);
        if (light == nullptr) {
            return;
        }
        light->streamed_in = false;
        RenderingServer *rs = RenderingServer::get_singleton();
        if (rs == nullptr) {
            return;
        }
        if (light->light_instance.is_valid()) {
            rs->free_rid(light->light_instance);
            light->light_instance = RID();
        }
        if (light->light.is_valid()) {
            rs->free_rid(light->light);
            light->light = RID();
        }
    }

    void E3DRenderingServer::_light_apply_enabled(LightObject &p_light) {
        if (!p_light.light_instance.is_valid()) {
            return;
        }
        RenderingServer *rs = RenderingServer::get_singleton();
        ERR_FAIL_NULL(rs);
        rs->instance_set_visible(p_light.light_instance, p_light.enabled);
    }

    /// Creates the light objects of a freshly built instance out of the lights E3DLightFactory
    /// found in the model
    void E3DRenderingServer::_build_instance_lights(const RID &p_instance, E3DInstanceData &p_instance_data) {
        // The NODES backends build SpotLight3D nodes of their own. A vehicle far enough away to
        // have switched to OPTIMIZED is past the distance where those were faded out anyway
        // (maszyna/vehicles/detail_distance), so only the streamed scenery gets lights
        // here - the ones a caller asks for by hand still go through *_light_create().
        if (p_instance_data.instancer != INSTANCER_OPTIMIZED || !p_instance_data.stream_rid.is_valid()) {
            return;
        }

        for (const E3DModelLightPlacement &placement: p_instance_data.model_lights.placements) {
            E3DLightParams params = placement.params;
            _apply_declared_color(p_instance_data, placement.light_name, params);
            _light_create(
                    p_instance, placement.light_name, params.omni ? LIGHT_KIND_OMNI : LIGHT_KIND_SPOT, params,
                    placement.synthesized);
        }
        // the model now owns a light, so keep its own geometry out of every scenery shadow map
        if (!p_instance_data.light_objects.is_empty() &&
            (p_instance_data.layer_mask & SCENERY_LIGHT_OWNER_LAYER) == 0) {
            p_instance_data.layer_mask |= SCENERY_LIGHT_OWNER_LAYER;
            _update_if_built(p_instance_data);
        }
    }

    /// `lightcolors` of the scenery node overrides the colour the model carries
    void E3DRenderingServer::_apply_declared_color(
            const E3DInstanceData &p_instance_data, const String &p_light_name, E3DLightParams &p_params) {
        const E3DInstanceData::LightDeclaration *declaration = p_instance_data.light_declarations.getptr(p_light_name);
        if (declaration != nullptr && declaration->has_color) {
            p_params.color = declaration->color;
            p_params.color.a = 1.0;
        }
    }

    void E3DRenderingServer::_clear_instance_lights(E3DInstanceData &p_instance_data) {
        SceneryStreamingServer *streaming = SceneryStreamingServer::get_instance();
        for (const RID &light_rid: p_instance_data.light_objects) {
            LightObject *light = lights.getptr(light_rid);
            if (light == nullptr) {
                continue;
            }
            if (light->stream_rid.is_valid() && streaming != nullptr) {
                streaming->stream_free(light->stream_rid);
            }
            _light_clear(light_rid);
            lights.erase(light_rid);
        }
        p_instance_data.light_objects.clear();
    }

    /// Creates the emitters of a freshly built instance out of what E3DSmokeSourceFactory found
    /// in the model. Unlike the lights this runs for every instancer: a distant vehicle has no
    /// node tree left, and the OPTIMIZED backend renders no emitter of its own.
    void E3DRenderingServer::_build_instance_smoke_sources(const RID &p_instance, E3DInstanceData &p_instance_data) {
        const ProjectSettings *settings = ProjectSettings::get_singleton();
        if (!settings->get_setting(SMOKE_ENABLED_SETTING, DEFAULT_SMOKE_ENABLED)) {
            return;
        }

        const Vector<E3DSmokeSourcePlacement> placements = E3DSmokeSourceFactory::discover(p_instance_data.model);
        if (placements.is_empty()) {
            return;
        }

        SceneryStreamingServer *streaming = SceneryStreamingServer::get_instance();
        // a locomotive's plume and a chimney's are seen from different distances, and they are
        // streamed by the same range, so each kind carries its own
        const bool dynamic_instance = p_instance_data.instance_kind == INSTANCE_KIND_DYNAMIC;
        const float distance = settings->get_setting(
                dynamic_instance ? SMOKE_DYNAMIC_DISTANCE_SETTING : SMOKE_STATIC_DISTANCE_SETTING,
                DEFAULT_SMOKE_DISTANCE);

        for (const E3DSmokeSourcePlacement &placement: placements) {
            const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
            SmokeObject &smoke = smoke_objects[rid];
            if (dynamic_instance) {
                smoke_order.push_back(rid);
            }
            smoke.owner = p_instance;
            smoke.template_name = placement.template_name;
            smoke.offset = placement.offset;
            p_instance_data.smoke_objects.push_back(rid);

            // A scenery emitter streams with a range of its own, the way a scenery light does.
            // Anything built directly (a vehicle, an editor model) gets its particles right away -
            // it is not part of the streamed scenery and is usually moving.
            if (p_instance_data.stream_rid.is_valid() && streaming != nullptr) {
                if (smoke_stream_owner < 0) {
                    smoke_stream_owner = streaming->owner_create(
                            Callable(), callable_mp(this, &E3DRenderingServer::_smoke_stream_build),
                            callable_mp(this, &E3DRenderingServer::_smoke_clear));
                }
                const Vector3 position = p_instance_data.transform.xform(placement.offset);
                smoke.stream_rid = streaming->stream_register(smoke_stream_owner, rid, position, distance);
            } else {
                _smoke_build(rid);
            }
        }
        if (!smoke_order.is_empty()) {
            _set_smoke_processing(true);
        }
    }

    /// Where the emitter spawns: the model root's own basis (the original launches the particles
    /// along the owner's up vector, particles.cpp:63/300) over the submodel's offset
    Transform3D
    E3DRenderingServer::_smoke_transform(const E3DInstanceData &p_instance_data, const SmokeObject &p_smoke) {
        return p_instance_data.transform * Transform3D(Basis(), p_smoke.offset);
    }

    /// Where the emitter spawns. The transform goes on the RenderingServer instance, not straight
    /// to particles_set_emission_transform(): the scene cull pushes an instance's transform into
    /// the emission transform on every update of its own, so anything set directly is overwritten
    /// with the instance's - which is how a GPUParticles3D node is driven too. The custom AABB
    /// stays in the emitter's local space and is transformed along with it.
    void E3DRenderingServer::_apply_smoke_placement(const E3DInstanceData &p_instance_data, SmokeObject &p_smoke) {
        RenderingServer *rs = RenderingServer::get_singleton();
        ERR_FAIL_NULL(rs);
        p_smoke.transform = _smoke_transform(p_instance_data, p_smoke);
        p_smoke.visible = p_instance_data.visible;
        rs->instance_set_transform(p_smoke.particles_instance, p_smoke.transform);
    }

    void E3DRenderingServer::_apply_smoke_wind(const SmokeObject &p_smoke) const {
        if (p_smoke.process_material.is_null()) {
            return;
        }
        p_smoke.process_material->set_gravity(wind * SMOKE_WIND_ACCELERATION);
    }

    /// Creates the RenderingServer particles of an emitter
    void E3DRenderingServer::_smoke_build(const RID &p_smoke) {
        SmokeObject *smoke = smoke_objects.getptr(p_smoke);
        if (smoke == nullptr || smoke->particles.is_valid()) {
            return;
        }
        const E3DInstanceData *instance = instances.getptr(smoke->owner);
        if (instance == nullptr || !smoke_source_resolver.is_valid()) {
            return;
        }
        RenderingServer *rs = RenderingServer::get_singleton();
        ERR_FAIL_NULL(rs);

        const Dictionary source = smoke_source_resolver.call(smoke->template_name, instance->instance_kind);
        Ref<ParticleProcessMaterial> process_material = source.get("process_material", Variant());
        const Ref<Mesh> mesh = source.get("mesh", Variant());
        if (process_material.is_null() || mesh.is_null()) {
            return; // the library already reported why
        }
        // Shared with every other emitter of the same template on purpose: nothing per instance
        // writes into it. The only thing that does is the wind, which is the whole world's.
        smoke->process_material = process_material;
        _apply_smoke_wind(*smoke);

        smoke->amount = source.get("amount", 1);
        smoke->spawn_rate = source.get("spawn_rate", 0.0);
        const float lifetime = source.get("lifetime", 1.0);
        smoke->local_aabb = source.get("aabb", AABB());

        smoke->particles = rs->particles_create();
        rs->particles_set_mode(smoke->particles, RenderingServer::PARTICLES_MODE_3D);
        // world space: the plume is left behind, it does not follow the vehicle
        rs->particles_set_use_local_coordinates(smoke->particles, false);
        rs->particles_set_amount(smoke->particles, smoke->amount);
        rs->particles_set_lifetime(smoke->particles, lifetime);
        rs->particles_set_process_material(smoke->particles, process_material->get_rid());
        rs->particles_set_draw_passes(smoke->particles, 1);
        rs->particles_set_draw_pass_mesh(smoke->particles, 0, mesh->get_rid());
        rs->particles_set_draw_order(smoke->particles, RenderingServer::PARTICLES_DRAW_ORDER_VIEW_DEPTH);
        rs->particles_set_custom_aabb(smoke->particles, smoke->local_aabb);
        if (instance->instance_kind == INSTANCE_KIND_DYNAMIC) {
            // the rate follows the engine state, so process_smoke() spawns by hand and the
            // automatic emitter has to stay out of it
            rs->particles_set_emitting(smoke->particles, false);
        } else {
            // A static emitter spawns at one rate for ever - amount over lifetime is exactly the
            // template's own - so the engine emits for it and it is not ticked at all. That also
            // buys the pre-process: the plume is already in the air when a chimney streams in,
            // instead of building up from nothing in front of the player.
            rs->particles_set_pre_process_time(smoke->particles, source.get("preprocess", 0.0));
            rs->particles_set_emitting(smoke->particles, instance->visible);
        }

        smoke->particles_instance = rs->instance_create();
        rs->instance_set_base(smoke->particles_instance, smoke->particles);
        rs->instance_set_scenario(smoke->particles_instance, instance->scenario);
        rs->instance_set_visible(smoke->particles_instance, instance->visible);
        _apply_smoke_placement(*instance, *smoke);
        smoke->last_spawn_usec = Time::get_singleton()->get_ticks_usec();
        smoke->streamed_in = true;
    }

    /// The build callback of the smoke stream; separate from _smoke_build() only because
    /// SceneryStreamingServer passes the preloaded value along
    void E3DRenderingServer::_smoke_stream_build(const RID &p_smoke, const Variant &p_preloaded) {
        _smoke_build(p_smoke);
    }

    void E3DRenderingServer::_smoke_clear(const RID &p_smoke) {
        SmokeObject *smoke = smoke_objects.getptr(p_smoke);
        if (smoke == nullptr) {
            return;
        }
        smoke->streamed_in = false;
        smoke->process_material.unref();
        RenderingServer *rs = RenderingServer::get_singleton();
        if (rs == nullptr) {
            return;
        }
        if (smoke->particles_instance.is_valid()) {
            rs->free_rid(smoke->particles_instance);
            smoke->particles_instance = RID();
        }
        if (smoke->particles.is_valid()) {
            rs->free_rid(smoke->particles);
            smoke->particles = RID();
        }
    }

    void E3DRenderingServer::_clear_instance_smoke_sources(E3DInstanceData &p_instance_data) {
        SceneryStreamingServer *streaming = SceneryStreamingServer::get_instance();
        for (const RID &smoke_rid: p_instance_data.smoke_objects) {
            SmokeObject *smoke = smoke_objects.getptr(smoke_rid);
            if (smoke == nullptr) {
                continue;
            }
            if (smoke->stream_rid.is_valid() && streaming != nullptr) {
                streaming->stream_free(smoke->stream_rid);
            }
            _smoke_clear(smoke_rid);
            smoke_objects.erase(smoke_rid);
            smoke_order.erase(smoke_rid);
        }
        p_instance_data.smoke_objects.clear();
        if (smoke_order.is_empty()) {
            _set_smoke_processing(false);
        }
    }

    /// Follows the instance: a moving vehicle spawns from where it is now, an invisible one
    /// stops spawning
    void E3DRenderingServer::_update_instance_smoke(const E3DInstanceData &p_instance_data) {
        RenderingServer *rs = RenderingServer::get_singleton();
        ERR_FAIL_NULL(rs);
        for (const RID &smoke_rid: p_instance_data.smoke_objects) {
            SmokeObject *smoke = smoke_objects.getptr(smoke_rid);
            if (smoke == nullptr || !smoke->particles.is_valid()) {
                continue;
            }
            _apply_smoke_placement(p_instance_data, *smoke);
            if (p_instance_data.instance_kind == INSTANCE_KIND_STATIC) {
                rs->particles_set_emitting(smoke->particles, p_instance_data.visible);
            }
            rs->instance_set_visible(smoke->particles_instance, p_instance_data.visible);
        }
    }

    void E3DRenderingServer::instance_set_smoke_intensity(const RID &p_instance, const float p_intensity) {
        E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL(instance);
        for (const RID &smoke_rid: instance->smoke_objects) {
            if (SmokeObject *smoke = smoke_objects.getptr(smoke_rid); smoke != nullptr) {
                smoke->intensity = p_intensity;
            }
        }
    }

    /// Spawns the particles every emitter owes this frame. The emitters emit by hand rather than
    /// through particles_set_emitting(): the rate follows the engine state, and the only knob
    /// Godot offers for that - amount_ratio - deactivates live particles instead of slowing the
    /// spawning, which cut a whole plume off in one frame. This is the original's own model
    /// (m_spawncount, particles.cpp:157-212).
    void E3DRenderingServer::_process_smoke() {
        const int size = smoke_order.size();
        if (size == 0) {
            return;
        }
        const uint64_t now = Time::get_singleton()->get_ticks_usec();
        const int visited = MIN(size, MAX_SMOKE_SOURCES_PER_FRAME);
        for (int i = 0; i < visited; i++) {
            if (smoke_cursor >= size) {
                smoke_cursor = 0;
            }
            if (SmokeObject *smoke = smoke_objects.getptr(smoke_order[smoke_cursor]); smoke != nullptr) {
                _process_smoke_source(*smoke, now);
            }
            smoke_cursor++;
        }
    }

    /// The tick runs only while the world holds an emitter
    void E3DRenderingServer::_set_smoke_processing(const bool p_processing) {
        if (smoke_processing == p_processing) {
            return;
        }
        SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
        if (tree == nullptr) {
            return;
        }
        smoke_processing = p_processing;
        if (p_processing) {
            tree->connect("process_frame", callable_mp(this, &E3DRenderingServer::_process_smoke));
            return;
        }
        tree->disconnect("process_frame", callable_mp(this, &E3DRenderingServer::_process_smoke));
    }

    /// One emitter's share of a frame. Fractional particles are carried over, so a rate below one
    /// per second still spawns - the original accumulates the same way (particles.cpp:162).
    void E3DRenderingServer::_process_smoke_source(SmokeObject &p_smoke, const uint64_t p_now) {
        if (!p_smoke.visible || !p_smoke.particles.is_valid()) {
            return;
        }
        const double delta = static_cast<double>(p_now - p_smoke.last_spawn_usec) / 1000000.0;
        p_smoke.last_spawn_usec = p_now;
        p_smoke.spawn_backlog += p_smoke.spawn_rate * p_smoke.intensity * delta;
        const int count = MIN(static_cast<int>(p_smoke.spawn_backlog), p_smoke.amount);
        if (count < 1) {
            return;
        }
        p_smoke.spawn_backlog -= static_cast<float>(count);

        RenderingServer *rs = RenderingServer::get_singleton();
        ERR_FAIL_NULL(rs);
        // Only the spawn point is dictated; velocity, size, roll and colour stay with the process
        // material, which randomizes them the way the template asks for
        for (int i = 0; i < count; i++) {
            rs->particles_emit(
                    p_smoke.particles, p_smoke.transform, Vector3(), Color(), Color(),
                    GPUParticles3D::EMIT_FLAG_POSITION);
        }
    }

    /// Addressable handle for the model's own light_onNN/light_offNN submodel pair. The submodels
    /// themselves are switched by the backend from lights_state, so this only gives a caller
    /// something to enable and disable uniformly with the real lights.
    RID E3DRenderingServer::emission_light_create(const RID &p_instance, const String &p_light_name) {
        return _light_create(p_instance, p_light_name, LIGHT_KIND_EMISSION, E3DLightParams());
    }

    RID E3DRenderingServer::spot_light_create(
            const RID &p_instance, const String &p_light_name, const NodePath &p_submodel_path) {
        const E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL_V(instance, RID());
        ERR_FAIL_COND_V(instance->model.is_null(), RID());
        const Ref<E3DSubModel> submodel = instance->model->get_node_or_null(p_submodel_path);
        ERR_FAIL_COND_V(submodel.is_null(), RID());
        const E3DLightParams params = E3DLightFactory::from_submodel(submodel.ptr(), p_light_name);
        return _light_create(p_instance, p_light_name, LIGHT_KIND_SPOT, params);
    }

    RID E3DRenderingServer::omni_light_create(
            const RID &p_instance, const String &p_light_name, const NodePath &p_submodel_path) {
        const E3DInstanceData *instance = instances.getptr(p_instance);
        ERR_FAIL_NULL_V(instance, RID());
        ERR_FAIL_COND_V(instance->model.is_null(), RID());
        const Ref<E3DSubModel> submodel = instance->model->get_node_or_null(p_submodel_path);
        ERR_FAIL_COND_V(submodel.is_null(), RID());
        const E3DLightParams params = E3DLightFactory::from_submodel(submodel.ptr(), p_light_name);
        return _light_create(p_instance, p_light_name, LIGHT_KIND_OMNI, params);
    }

    void E3DRenderingServer::light_free(const RID &p_light) {
        const HashMap<RID, LightObject>::Iterator item = lights.find(p_light);
        ERR_FAIL_COND(item == lights.end());
        if (E3DInstanceData *instance = instances.getptr(item->value.owner); instance != nullptr) {
            instance->light_objects.erase(p_light);
        }
        if (SceneryStreamingServer *streaming = SceneryStreamingServer::get_instance();
            item->value.stream_rid.is_valid() && streaming != nullptr) {
            streaming->stream_free(item->value.stream_rid);
        }
        _light_clear(p_light);
        lights.remove(item);
    }

    void E3DRenderingServer::light_enable(const RID &p_light) {
        LightObject *light = lights.getptr(p_light);
        ERR_FAIL_NULL(light);
        light->enabled = true;
        _light_apply_enabled(*light);
        if (E3DInstanceData *instance = instances.getptr(light->owner); instance != nullptr) {
            instance->lights_override[light->light_name] = true;
            _resolve_lights(*instance);
            _update_if_built(*instance);
        }
    }

    void E3DRenderingServer::light_disable(const RID &p_light) {
        LightObject *light = lights.getptr(p_light);
        ERR_FAIL_NULL(light);
        light->enabled = false;
        _light_apply_enabled(*light);
        if (E3DInstanceData *instance = instances.getptr(light->owner); instance != nullptr) {
            instance->lights_override[light->light_name] = false;
            _resolve_lights(*instance);
            _update_if_built(*instance);
        }
    }

    /// Light 0 of a scenery node is the "00" the E3D parser derived from "light_on00"
    String E3DRenderingServer::_light_name_for_index(const int p_index) {
        return String::num_int64(p_index).pad_zeros(2);
    }

    /// TAnimModel::RaPrepare(), AnimModel.cpp:578-627. ls_Blink is not ported - it is used 15
    /// times in the whole data set and needs a per-frame timer (see TODO.md), so it follows
    /// ls_Dark for now.
    bool E3DRenderingServer::_is_light_mode_on(const float p_mode) const {
        const double mode = Math::abs(static_cast<double>(p_mode));
        const double mode_integral = Math::floor(mode);
        switch (static_cast<int>(mode_integral)) {
            case LIGHT_MODE_OFF:
                return false;
            case LIGHT_MODE_ON:
                return true;
            case LIGHT_MODE_HOME: {
                // like dark, but forced off late at night
                if (current_time >= HOME_LIGHTS_OFF_FROM_HOUR && current_time < HOME_LIGHTS_OFF_TO_HOUR) {
                    return false;
                }
                [[fallthrough]];
            }
            case LIGHT_MODE_BLINK:
            case LIGHT_MODE_DARK:
            default: {
                // the fraction carries the light's own threshold, e.g. `lights 3.4` means 0.4
                const double fraction = mode - mode_integral;
                const double threshold = fraction < 0.01 ? DEFAULT_DARK_THRESHOLD : fraction;
                return light_level <= threshold;
            }
        }
    }

    void E3DRenderingServer::_resolve_lights(E3DInstanceData &p_instance) {
        Dictionary state;
        for (const KeyValue<String, E3DInstanceData::LightDeclaration> &declaration: p_instance.light_declarations) {
            state[declaration.key] = _is_light_mode_on(declaration.value.mode);
        }
        state.merge(p_instance.lights_override, true);
        p_instance.lights_state = state;

        for (const RID &light_rid: p_instance.light_objects) {
            LightObject *light = lights.getptr(light_rid);
            if (light == nullptr) {
                continue;
            }
            const bool enabled = state.get(light->light_name, false);
            if (light->enabled == enabled) {
                continue;
            }
            light->enabled = enabled;
            _light_apply_enabled(*light);
        }
    }

    void E3DRenderingServer::_resolve_all_lights() {
        for (KeyValue<RID, E3DInstanceData> &item: instances) {
            if (item.value.light_declarations.is_empty()) {
                continue; // nothing automatic to decide
            }
            const Dictionary previous = item.value.lights_state;
            _resolve_lights(item.value);
            if (!(previous == item.value.lights_state)) {
                _update_if_built(item.value);
            }
        }
    }

    void E3DRenderingServer::set_current_time(const double p_hours) {
        if (Math::is_equal_approx(current_time, p_hours)) {
            return;
        }
        current_time = p_hours;
        _resolve_all_lights();
    }

    void E3DRenderingServer::set_light_level(const double p_level) {
        if (Math::is_equal_approx(light_level, p_level)) {
            return;
        }
        light_level = p_level;
        _resolve_all_lights();
    }

    Dictionary E3DRenderingServer::get_light_statistics() const {
        int lit = 0;
        int spot = 0;
        int omni = 0;
        int synthesized = 0;
        for (const KeyValue<RID, LightObject> &item: lights) {
            if (item.value.kind == LIGHT_KIND_EMISSION) {
                continue;
            }
            if (item.value.enabled) {
                lit++;
            }
            if (item.value.kind == LIGHT_KIND_OMNI) {
                omni++;
            } else {
                spot++;
            }
            if (item.value.synthesized) {
                synthesized++;
            }
        }
        Dictionary statistics;
        statistics["total"] = static_cast<int>(lights.size());
        statistics["lit"] = lit;
        statistics["spot"] = spot;
        statistics["omni"] = omni;
        statistics["synthesized"] = synthesized;
        return statistics;
    }

    /// The whole simulation shares one wind (simulationenvironment.cpp:255), so it reaches every
    /// emitter - including the template materials the streamed scenery emitters share. Strength
    /// (m/s) and direction are separate so that the direction can grow a vertical component
    /// without the signature changing.
    void E3DRenderingServer::set_wind(const float p_strength, const Vector3 &p_direction) {
        wind_strength = p_strength;
        wind_direction = p_direction;
        _update_wind();
    }

    void E3DRenderingServer::set_wind_strength(const float p_strength) {
        wind_strength = p_strength;
        _update_wind();
    }

    void E3DRenderingServer::set_wind_direction(const Vector3 &p_direction) {
        wind_direction = p_direction;
        _update_wind();
    }

    void E3DRenderingServer::_update_wind() {
        const Vector3 new_wind = wind_direction.normalized() * wind_strength;
        if (wind.is_equal_approx(new_wind)) {
            return;
        }
        wind = new_wind;
        for (const KeyValue<RID, SmokeObject> &item: smoke_objects) {
            _apply_smoke_wind(item.value);
        }
    }

    Dictionary E3DRenderingServer::get_smoke_statistics() const {
        int built = 0;
        for (const KeyValue<RID, SmokeObject> &item: smoke_objects) {
            if (item.value.streamed_in) {
                built++;
            }
        }
        Dictionary statistics;
        statistics["total"] = static_cast<int>(smoke_objects.size());
        statistics["built"] = built;
        return statistics;
    }
} // namespace godot
