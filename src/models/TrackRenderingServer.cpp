#include "TrackRenderingServer.hpp"
#include "models/MaterialManager.hpp"
#include "models/e3d/E3DModel.hpp"
#include "models/e3d/E3DSubModel.hpp"

#include <cmath>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <vector>

namespace godot {

    void TrackRenderingServer::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("_on_material_manager_cache_cleared"), &TrackRenderingServer::_on_material_manager_cache_cleared);
        ClassDB::bind_method(D_METHOD("create_track"), &TrackRenderingServer::create_track);
        ClassDB::bind_method(D_METHOD("free_track", "track_id"), &TrackRenderingServer::free_track);
        ClassDB::bind_method(
                D_METHOD("set_track_transform", "track_id", "transform"), &TrackRenderingServer::set_track_transform);
        ClassDB::bind_method(D_METHOD("get_track_transform", "track_id"), &TrackRenderingServer::get_track_transform);
        ClassDB::bind_method(
                D_METHOD("set_track_visible", "track_id", "visible"), &TrackRenderingServer::set_track_visible);
        ClassDB::bind_method(D_METHOD("is_track_visible", "track_id"), &TrackRenderingServer::is_track_visible);
        ClassDB::bind_method(
                D_METHOD("set_track_scenario", "track_id", "scenario"), &TrackRenderingServer::set_track_scenario);
        ClassDB::bind_method(D_METHOD("get_track_scenario", "track_id"), &TrackRenderingServer::get_track_scenario);
        ClassDB::bind_method(
                D_METHOD("update_track_data", "track_id", "points", "quats", "length"),
                &TrackRenderingServer::update_track_data);
        ClassDB::bind_method(
                D_METHOD("update_rail_mesh", "track_id", "length", "rail_spacing", "curve_precision"),
                &TrackRenderingServer::update_rail_mesh);
        ClassDB::bind_method(
                D_METHOD("update_ballast_mesh", "track_id", "length", "curve_precision"),
                &TrackRenderingServer::update_ballast_mesh);
        ClassDB::bind_method(
                D_METHOD("set_sleeper_model_name", "track_id", "sleeper_model_name"),
                &TrackRenderingServer::set_sleeper_model_name);
        ClassDB::bind_method(
                D_METHOD("set_sleeper_model_skin", "track_id", "sleeper_model_skin"),
                &TrackRenderingServer::set_sleeper_model_skin);
        ClassDB::bind_method(D_METHOD("set_sleeper_mesh", "track_id", "mesh"), &TrackRenderingServer::set_sleeper_mesh);
        ClassDB::bind_method(D_METHOD("get_sleeper_mesh", "track_id"), &TrackRenderingServer::get_sleeper_mesh);
        ClassDB::bind_method(
                D_METHOD("set_ballast_texture", "track_id", "path"), &TrackRenderingServer::set_ballast_texture);

        ClassDB::bind_method(
                D_METHOD("set_sleeper_height", "track_id", "height"), &TrackRenderingServer::set_sleeper_height);
        ClassDB::bind_method(D_METHOD("get_sleeper_height", "track_id"), &TrackRenderingServer::get_sleeper_height);
        ClassDB::bind_method(
                D_METHOD("set_sleeper_spacing", "track_id", "spacing"), &TrackRenderingServer::set_sleeper_spacing);
        ClassDB::bind_method(D_METHOD("get_sleeper_spacing", "track_id"), &TrackRenderingServer::get_sleeper_spacing);
        ClassDB::bind_method(
                D_METHOD("set_rail_spacing", "track_id", "spacing"), &TrackRenderingServer::set_rail_spacing);
        ClassDB::bind_method(D_METHOD("get_rail_spacing", "track_id"), &TrackRenderingServer::get_rail_spacing);
        ClassDB::bind_method(
                D_METHOD("set_ballast_height", "track_id", "height"), &TrackRenderingServer::set_ballast_height);
        ClassDB::bind_method(D_METHOD("get_ballast_height", "track_id"), &TrackRenderingServer::get_ballast_height);
        ClassDB::bind_method(
                D_METHOD("set_ballast_offset", "track_id", "offset"), &TrackRenderingServer::set_ballast_offset);
        ClassDB::bind_method(D_METHOD("get_ballast_offset", "track_id"), &TrackRenderingServer::get_ballast_offset);
        ClassDB::bind_method(
                D_METHOD("set_ballast_uv_scale", "track_id", "scale"), &TrackRenderingServer::set_ballast_uv_scale);
        ClassDB::bind_method(D_METHOD("get_ballast_uv_scale", "track_id"), &TrackRenderingServer::get_ballast_uv_scale);
        ClassDB::bind_method(
                D_METHOD("set_ballast_width_tiling", "track_id", "tiling"),
                &TrackRenderingServer::set_ballast_width_tiling);
        ClassDB::bind_method(
                D_METHOD("get_ballast_width_tiling", "track_id"), &TrackRenderingServer::get_ballast_width_tiling);
        ClassDB::bind_method(
                D_METHOD("set_ballast_length_tiling", "track_id", "tiling"),
                &TrackRenderingServer::set_ballast_length_tiling);
        ClassDB::bind_method(
                D_METHOD("get_ballast_length_tiling", "track_id"), &TrackRenderingServer::get_ballast_length_tiling);
        ClassDB::bind_method(
                D_METHOD("set_ballast_enabled", "track_id", "enabled"), &TrackRenderingServer::set_ballast_enabled);
        ClassDB::bind_method(D_METHOD("get_ballast_enabled", "track_id"), &TrackRenderingServer::get_ballast_enabled);

        ClassDB::bind_method(D_METHOD("get_instance_rid", "track_id"), &TrackRenderingServer::get_instance_rid);
        ClassDB::bind_method(
                D_METHOD("get_rail_mesh_instance", "track_id"), &TrackRenderingServer::get_rail_mesh_instance);
        ClassDB::bind_method(
                D_METHOD("get_sleeper_multimesh_instance", "track_id"),
                &TrackRenderingServer::get_sleeper_multimesh_instance);
        ClassDB::bind_method(
                D_METHOD("get_ballast_mesh_instance", "track_id"), &TrackRenderingServer::get_ballast_mesh_instance);
    }

    TrackRenderingServer::TrackRenderingServer() {}

    TrackRenderingServer::~TrackRenderingServer() {
        _disconnect_material_manager_signals_if_needed();
        for (KeyValue<int64_t, TrackState> &entry: tracks) {
            _free_track_rids(entry.value);
        }
        tracks.clear();
    }

    TrackRenderingServer::TrackState *TrackRenderingServer::get_track_state(const int64_t p_track_id) {
        if (!tracks.has(p_track_id)) {
            UtilityFunctions::push_warning(vformat("[TrackRenderingServer] Unknown track id: %d", p_track_id));
            return nullptr;
        }
        return &tracks[p_track_id];
    }

    const TrackRenderingServer::TrackState *TrackRenderingServer::get_track_state(const int64_t p_track_id) const {
        const TrackState *state = tracks.getptr(p_track_id);
        if (state == nullptr) {
            UtilityFunctions::push_warning(vformat("[TrackRenderingServer] Unknown track id: %d", p_track_id));
        }
        return state;
    }

    void
    TrackRenderingServer::_apply_material_override(const RID &p_instance, const Ref<ShaderMaterial> &p_material) const {
        RenderingServer::get_singleton()->instance_geometry_set_material_override(
                p_instance, p_material.is_valid() ? p_material->get_rid() : RID());
    }

    Ref<ShaderMaterial> TrackRenderingServer::_create_shader_material(const String &p_shader_path) const {
        Ref<ShaderMaterial> material;
        material.instantiate();
        if (ResourceLoader *loader = ResourceLoader::get_singleton()) {
            material->set_shader(loader->load(p_shader_path));
        }
        return material;
    }

    void TrackRenderingServer::_clear_sleeper_material(TrackState &p_state) {
        if (p_state.sleeper_material.is_null()) {
            return;
        }

        p_state.sleeper_material->set_shader_parameter("albedo_tex", Variant());
        p_state.sleeper_material->set_shader_parameter("albedo", Color(1, 1, 1, 1));
    }

    void TrackRenderingServer::_clear_ballast_material(TrackState &p_state) {
        if (p_state.ballast_material.is_null()) {
            return;
        }

        p_state.ballast_material->set_shader_parameter("albedo_tex", Variant());
        p_state.ballast_material->set_shader_parameter("albedo", Color(1, 1, 1, 1));
    }

    void TrackRenderingServer::_apply_ballast_material(TrackState &p_state) {
        if (p_state.ballast_material.is_null()) {
            return;
        }

        const String texture_path = p_state.ballast_texture_path.strip_edges();
        if (texture_path.is_empty()) {
            _clear_ballast_material(p_state);
            return;
        }

        MaterialManager *mm = MaterialManager::get_instance();
        if (mm == nullptr) {
            _clear_ballast_material(p_state);
            return;
        }

        const Ref<ImageTexture> tex = mm->get_texture(texture_path);
        if (!tex.is_valid()) {
            UtilityFunctions::push_warning("[TrackRenderingServer] Ballast texture is not supported");
            _clear_ballast_material(p_state);
            return;
        }

        p_state.ballast_material->set_shader_parameter("albedo_tex", tex);
        p_state.ballast_material->set_shader_parameter("albedo", Color(1, 1, 1, 1));
    }

    void TrackRenderingServer::_refresh_track_materials(const int64_t p_track_id, TrackState &p_state) {
        _reload_sleeper_model(p_track_id, p_state);
        _apply_ballast_material(p_state);
    }

    void TrackRenderingServer::_on_material_manager_cache_cleared() {
        for (KeyValue<int64_t, TrackState> &entry: tracks) {
            _refresh_track_materials(entry.key, entry.value);
        }
    }

    void TrackRenderingServer::_connect_material_manager_signals_if_needed() {
        if (material_manager_connected) {
            return;
        }

        MaterialManager *mm = MaterialManager::get_instance();
        if (mm == nullptr) {
            return;
        }

        const Callable callable(this, "_on_material_manager_cache_cleared");
        if (!mm->is_connected(MaterialManager::cache_cleared_signal, callable)) {
            const Error err = mm->connect(MaterialManager::cache_cleared_signal, callable);
            if (err != OK) {
                UtilityFunctions::push_warning("[TrackRenderingServer] Failed to connect MaterialManager.cache_cleared");
                return;
            }
        }

        material_manager_connected = true;
    }

    void TrackRenderingServer::_disconnect_material_manager_signals_if_needed() {
        if (!material_manager_connected) {
            return;
        }

        MaterialManager *mm = MaterialManager::get_instance();
        if (mm != nullptr) {
            const Callable callable(this, "_on_material_manager_cache_cleared");
            if (mm->is_connected(MaterialManager::cache_cleared_signal, callable)) {
                mm->disconnect(MaterialManager::cache_cleared_signal, callable);
            }
        }

        material_manager_connected = false;
    }

    void TrackRenderingServer::_free_track_rids(TrackState &p_state) {
        RenderingServer *rs = RenderingServer::get_singleton();
        if (rs == nullptr) {
            return;
        }

        if (p_state.rail_mesh_instance.is_valid()) {
            rs->free_rid(p_state.rail_mesh_instance);
            p_state.rail_mesh_instance = RID();
        }

        if (p_state.ballast_mesh_instance.is_valid()) {
            rs->free_rid(p_state.ballast_mesh_instance);
            p_state.ballast_mesh_instance = RID();
        }

        if (p_state.sleeper_multimesh_instance.is_valid()) {
            rs->free_rid(p_state.sleeper_multimesh_instance);
            p_state.sleeper_multimesh_instance = RID();
        }

        if (p_state.sleeper_multimesh.is_valid()) {
            rs->free_rid(p_state.sleeper_multimesh);
            p_state.sleeper_multimesh = RID();
        }

        p_state.rail_mesh = RID();
        p_state.ballast_mesh = RID();
        p_state.sleeper_mesh.unref();
        p_state.internal_rail_mesh.unref();
        p_state.internal_ballast_mesh.unref();
    }
    void TrackRenderingServer::_update_render_instances(TrackState &p_state) {
        RenderingServer *rs = RenderingServer::get_singleton();
        const RID active_scenario = p_state.visible ? p_state.scenario : RID();

        rs->instance_set_scenario(p_state.rail_mesh_instance, active_scenario);
        rs->instance_set_scenario(p_state.sleeper_multimesh_instance, active_scenario);
        rs->instance_set_scenario(
                p_state.ballast_mesh_instance, (p_state.visible && p_state.ballast_enabled) ? p_state.scenario : RID());

        rs->instance_set_transform(p_state.rail_mesh_instance, p_state.transform);
        rs->instance_set_transform(p_state.sleeper_multimesh_instance, p_state.transform);
        rs->instance_set_transform(p_state.ballast_mesh_instance, p_state.transform);
    }

    int64_t TrackRenderingServer::create_track() {
        _connect_material_manager_signals_if_needed();
        RenderingServer *rs = RenderingServer::get_singleton();

        TrackState state;
        state.rail_mesh_instance = rs->instance_create();
        state.ballast_mesh_instance = rs->instance_create();
        state.sleeper_multimesh_instance = rs->instance_create();
        state.sleeper_multimesh = rs->multimesh_create();
        rs->instance_set_base(state.sleeper_multimesh_instance, state.sleeper_multimesh);
        state.rail_material = _create_shader_material("res://addons/libmaszyna/materials/track_path_deform.gdshader");
        _apply_material_override(state.rail_mesh_instance, state.rail_material);
        state.sleeper_material =
                _create_shader_material("res://addons/libmaszyna/materials/sleeper_path_deform.gdshader");
        _apply_material_override(state.sleeper_multimesh_instance, state.sleeper_material);
        state.ballast_material =
                _create_shader_material("res://addons/libmaszyna/materials/ballast_path_deform.gdshader");
        _apply_material_override(state.ballast_mesh_instance, state.ballast_material);

        const int64_t track_id = next_track_id++;
        tracks.insert(track_id, state);
        return track_id;
    }

    void TrackRenderingServer::free_track(const int64_t p_track_id) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);

        _free_track_rids(*state);
        tracks.erase(p_track_id);
    }

    void TrackRenderingServer::set_track_transform(const int64_t p_track_id, const Transform3D &p_transform) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);
        state->transform = p_transform;
        _update_render_instances(*state);
    }

    Transform3D TrackRenderingServer::get_track_transform(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, Transform3D());
        return state->transform;
    }

    void TrackRenderingServer::set_track_visible(const int64_t p_track_id, const bool p_visible) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);
        state->visible = p_visible;
        _update_render_instances(*state);
    }

    bool TrackRenderingServer::is_track_visible(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, false);
        return state->visible;
    }

    void TrackRenderingServer::set_track_scenario(const int64_t p_track_id, const RID p_scenario) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);
        state->scenario = p_scenario;
        _update_render_instances(*state);
    }

    RID TrackRenderingServer::get_track_scenario(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, RID());
        return state->scenario;
    }

    void TrackRenderingServer::update_track_data(
            const int64_t p_track_id, const PackedVector4Array &p_points, const PackedVector4Array &p_quats,
            const float p_length) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);

        state->path_points = p_points;
        state->path_quats = p_quats;
        state->path_length = p_length;

        RenderingServer *rs = RenderingServer::get_singleton();
        const float safe_length = p_length > 0.001f ? p_length : 0.001f;

        if (p_points.size() > 0) {
            AABB aabb(Vector3(p_points[0].x, p_points[0].y, p_points[0].z), Vector3(0, 0, 0));
            for (int i = 1; i < p_points.size(); i++) {
                aabb.expand_to(Vector3(p_points[i].x, p_points[i].y, p_points[i].z));
            }
            aabb.grow_by(5.0f);
            rs->instance_set_custom_aabb(state->rail_mesh_instance, aabb);
            rs->instance_set_custom_aabb(state->sleeper_multimesh_instance, aabb);
            rs->instance_set_custom_aabb(state->ballast_mesh_instance, aabb);
        }

        rs->instance_geometry_set_shader_parameter(state->rail_mesh_instance, "path_length", safe_length);
        rs->instance_geometry_set_shader_parameter(state->rail_mesh_instance, "path_point_count", p_points.size());
        if (state->rail_material.is_valid()) {
            state->rail_material->set_shader_parameter("path_points", p_points);
            state->rail_material->set_shader_parameter("path_quats", p_quats);
        }

        rs->instance_geometry_set_shader_parameter(state->ballast_mesh_instance, "path_length", safe_length);
        rs->instance_geometry_set_shader_parameter(state->ballast_mesh_instance, "path_point_count", p_points.size());
        if (state->ballast_material.is_valid()) {
            state->ballast_material->set_shader_parameter("path_points", p_points);
            state->ballast_material->set_shader_parameter("path_quats", p_quats);
            state->ballast_material->set_shader_parameter("uv_scale", state->ballast_uv_scale);
        }

        rs->instance_geometry_set_shader_parameter(state->sleeper_multimesh_instance, "path_length", safe_length);
        rs->instance_geometry_set_shader_parameter(
                state->sleeper_multimesh_instance, "path_point_count", p_points.size());
        rs->instance_geometry_set_shader_parameter(
                state->sleeper_multimesh_instance, "sleeper_height", state->sleeper_height);
        const float safe_spacing = state->sleeper_spacing > 0.001f ? state->sleeper_spacing : 0.001f;
        rs->instance_geometry_set_shader_parameter(state->sleeper_multimesh_instance, "sleeper_spacing", safe_spacing);
        if (state->sleeper_material.is_valid()) {
            state->sleeper_material->set_shader_parameter("path_points", p_points);
            state->sleeper_material->set_shader_parameter("path_quats", p_quats);
        }

        if (state->sleeper_multimesh.is_valid() && state->sleeper_mesh.is_valid()) {
            const int count = static_cast<int>(std::floor(safe_length / safe_spacing));
            if (count > 0) {
                rs->multimesh_allocate_data(state->sleeper_multimesh, count, RenderingServer::MULTIMESH_TRANSFORM_3D);
                rs->multimesh_set_visible_instances(state->sleeper_multimesh, -1);
                for (int i = 0; i < count; i++) {
                    rs->multimesh_instance_set_transform(state->sleeper_multimesh, i, Transform3D());
                }
            } else {
                rs->multimesh_allocate_data(state->sleeper_multimesh, 0, RenderingServer::MULTIMESH_TRANSFORM_3D);
            }
        }

        _update_render_instances(*state);
    }

    void TrackRenderingServer::set_sleeper_model_name(const int64_t p_track_id, const String &p_sleeper_model_name) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);

        if (state->sleeper_model_name == p_sleeper_model_name) {
            return;
        }

        state->sleeper_model_name = p_sleeper_model_name;
        _reload_sleeper_model(p_track_id, *state);
    }

    void TrackRenderingServer::set_sleeper_model_skin(const int64_t p_track_id, const String &p_sleeper_model_skin) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);

        if (state->sleeper_model_skin == p_sleeper_model_skin) {
            return;
        }

        state->sleeper_model_skin = p_sleeper_model_skin;
        _reload_sleeper_model(p_track_id, *state);
    }

    void TrackRenderingServer::_reload_sleeper_model(const int64_t p_track_id, TrackState &p_state) {
        String model_path = p_state.sleeper_model_name.strip_edges();
        if (model_path.begins_with("/")) {
            model_path = model_path.trim_prefix("/");
        }

        if (model_path.is_empty()) {
            set_sleeper_mesh(p_track_id, Ref<Mesh>());
            _clear_sleeper_material(p_state);
            return;
        }

        E3DModelManager *model_manager = E3DModelManager::get_instance();
        if (model_manager == nullptr) {
            return;
        }

        const int split_index = model_path.rfind("/");
        const String data_path = split_index >= 0 ? model_path.substr(0, split_index) : String();
        const String model_filename = split_index >= 0 ? model_path.substr(split_index + 1) : model_path;
        if (model_filename.is_empty()) {
            set_sleeper_mesh(p_track_id, Ref<Mesh>());
            _clear_sleeper_material(p_state);
            return;
        }

        const Ref<E3DModel> model = model_manager->load_model(data_path, model_filename);
        if (model.is_null()) {
            set_sleeper_mesh(p_track_id, Ref<Mesh>());
            _clear_sleeper_material(p_state);
            return;
        }

        TypedArray<E3DSubModel> submodels = model->get_submodels();
        if (submodels.is_empty()) {
            set_sleeper_mesh(p_track_id, Ref<Mesh>());
            _clear_sleeper_material(p_state);
            return;
        }

        Ref<Mesh> found_mesh;
        String found_material_name;
        Color found_diffuse = Color(1, 1, 1);
        bool found_colored = false;
        std::vector<Ref<E3DSubModel>> stack;
        stack.reserve(submodels.size());
        for (const Variant &submodel: submodels) {
            stack.emplace_back(submodel);
        }

        while (!stack.empty()) {
            const Ref<E3DSubModel> sm = stack.back();
            stack.pop_back();

            if (sm->get_mesh().is_valid()) {
                found_mesh = sm->get_mesh();
                found_material_name = sm->get_material_name();
                found_diffuse = sm->get_diffuse_color();
                found_colored = sm->get_material_colored();
                break;
            }

            TypedArray<E3DSubModel> children = sm->get_submodels();
            for (const Variant &child: children) {
                stack.emplace_back(child);
            }
        }

        if (!found_mesh.is_valid()) {
            set_sleeper_mesh(p_track_id, Ref<Mesh>());
            _clear_sleeper_material(p_state);
            return;
        }

        set_sleeper_mesh(p_track_id, found_mesh);

        MaterialManager *mm = MaterialManager::get_instance();
        if (mm == nullptr) {
            _clear_sleeper_material(p_state);
            return;
        }

        const String unprefixed_model_path = data_path;
        const String sleeper_model_skin = p_state.sleeper_model_skin.strip_edges();
        if (!sleeper_model_skin.is_empty()) {
            found_material_name = sleeper_model_skin;
        }

        if (found_colored) {
            p_state.sleeper_material->set_shader_parameter("albedo_tex", Variant());
            p_state.sleeper_material->set_shader_parameter(
                    "albedo", Color(found_diffuse.r, found_diffuse.g, found_diffuse.b, found_diffuse.a));
            return;
        }

        const Ref<MaszynaMaterial> material = mm->load_material(unprefixed_model_path, found_material_name);
        if (material.is_null()) {
            _clear_sleeper_material(p_state);
            return;
        }

        const String albedo_path = material->get_albedo_texture_path();
        if (albedo_path.is_empty()) {
            p_state.sleeper_material->set_shader_parameter("albedo_tex", Variant());
            p_state.sleeper_material->set_shader_parameter(
                    "albedo", Color(found_diffuse.r, found_diffuse.g, found_diffuse.b, found_diffuse.a));
            return;
        }

        const String texture_name = albedo_path.split(":").get(0);
        const Ref<ImageTexture> tex = mm->load_texture(unprefixed_model_path, texture_name);
        if (tex.is_valid()) {
            p_state.sleeper_material->set_shader_parameter("albedo_tex", tex);
            p_state.sleeper_material->set_shader_parameter("albedo", Color(1, 1, 1, 1));
        } else {
            _clear_sleeper_material(p_state);
        }
    }

    void TrackRenderingServer::set_sleeper_mesh(const int64_t p_track_id, const Ref<Mesh> &p_mesh) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);

        state->sleeper_mesh = p_mesh;
        RenderingServer *rs = RenderingServer::get_singleton();
        if (state->sleeper_multimesh.is_valid()) {
            rs->multimesh_set_mesh(state->sleeper_multimesh, p_mesh.is_valid() ? p_mesh->get_rid() : RID());
            rs->multimesh_set_visible_instances(state->sleeper_multimesh, -1);
        }
        update_track_data(p_track_id, state->path_points, state->path_quats, state->path_length);
    }

    Ref<Mesh> TrackRenderingServer::get_sleeper_mesh(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, Ref<Mesh>());
        return state->sleeper_mesh;
    }

    void TrackRenderingServer::set_ballast_uv_scale(const int64_t p_track_id, const float p_scale) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);

        state->ballast_uv_scale = p_scale;
        if (state->ballast_material.is_valid()) {
            state->ballast_material->set_shader_parameter("uv_scale", state->ballast_uv_scale);
        }
    }

    float TrackRenderingServer::get_ballast_uv_scale(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, 0.0f);
        return state->ballast_uv_scale;
    }

    void TrackRenderingServer::set_ballast_texture(const int64_t p_track_id, const String &p_path) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);
        const String texture_path = p_path.strip_edges();
        if (state->ballast_texture_path == texture_path) {
            return;
        }
        state->ballast_texture_path = texture_path;
        _apply_ballast_material(*state);
    }

    void TrackRenderingServer::update_ballast_mesh(
            const int64_t p_track_id, const float p_length, const float p_curve_precision) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);

        if (p_length < 0.1f) {
            return;
        }

        float old_length = 0.0f;
        float old_height = 0.0f;
        float old_offset = 0.0f;
        float old_w_tiling = 0.0f;
        float old_l_tiling = 0.0f;
        if (state->internal_ballast_mesh.is_valid()) {
            old_length = state->internal_ballast_mesh->get_meta("length", 0.0f);
            old_height = state->internal_ballast_mesh->get_meta("ballast_height", 0.0f);
            old_offset = state->internal_ballast_mesh->get_meta("ballast_offset", 0.0f);
            old_w_tiling = state->internal_ballast_mesh->get_meta("ballast_width_tiling", 0.0f);
            old_l_tiling = state->internal_ballast_mesh->get_meta("ballast_length_tiling", 0.0f);
        }

        bool should_generate = state->internal_ballast_mesh.is_null();
        if (!should_generate) {
            if (std::abs(old_length - p_length) > 1.0f || std::abs(old_height - state->ballast_height) > 0.001f ||
                std::abs(old_offset - state->ballast_offset) > 0.001f ||
                std::abs(old_w_tiling - state->ballast_width_tiling) > 0.001f ||
                std::abs(old_l_tiling - state->ballast_length_tiling) > 0.001f) {
                should_generate = true;
            }
        }

        if (!should_generate) {
            return;
        }

        const int steps = std::max(2, static_cast<int>(std::floor(p_length / p_curve_precision)));

        PackedVector2Array ballast_poly;
        ballast_poly.push_back(Vector2(-1.25f, 0.0f));
        ballast_poly.push_back(Vector2(1.25f, 0.0f));
        ballast_poly.push_back(Vector2(2.0f, -state->ballast_height));
        ballast_poly.push_back(Vector2(-2.0f, -state->ballast_height));

        PackedVector3Array vertices;
        PackedVector3Array normals;
        PackedVector2Array uvs;
        PackedInt32Array indices;

        const int poly_size = static_cast<int>(ballast_poly.size());

        for (int i = 0; i <= steps; i++) {
            const float z = (static_cast<float>(i) / static_cast<float>(steps)) * p_length;
            const float v_u = (static_cast<float>(i) / static_cast<float>(steps)) * p_length;

            for (int j = 0; j < poly_size; j++) {
                const Vector2 p = ballast_poly[j];
                vertices.push_back(Vector3(p.x, p.y + state->ballast_offset, z));

                Vector3 n(0, 1, 0);
                if (j == 1 || j == 2) {
                    n = Vector3(0.47f, 0.88f, 0.0f);
                } else if (j == 3 || j == 0) {
                    n = Vector3(-0.47f, 0.88f, 0.0f);
                }
                normals.push_back(n);
                uvs.push_back(Vector2(p.x * state->ballast_width_tiling, v_u * state->ballast_length_tiling));
            }
        }

        for (int i = 0; i < steps; i++) {
            for (int j = 0; j < poly_size; j++) {
                const int next_j = (j + 1) % poly_size;
                const int curr = (i * poly_size) + j;
                const int next = (i * poly_size) + next_j;
                const int step_curr = ((i + 1) * poly_size) + j;
                const int step_next = ((i + 1) * poly_size) + next_j;

                indices.push_back(curr);
                indices.push_back(step_curr);
                indices.push_back(next);
                indices.push_back(next);
                indices.push_back(step_curr);
                indices.push_back(step_next);
            }
        }

        Array arrays;
        arrays.resize(Mesh::ARRAY_MAX);
        arrays.set(Mesh::ARRAY_VERTEX, vertices);
        arrays.set(Mesh::ARRAY_NORMAL, normals);
        arrays.set(Mesh::ARRAY_TEX_UV, uvs);
        arrays.set(Mesh::ARRAY_INDEX, indices);

        Ref<ArrayMesh> new_mesh;
        new_mesh.instantiate();
        new_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

        state->internal_ballast_mesh = new_mesh;
        state->internal_ballast_mesh->set_meta("length", p_length);
        state->internal_ballast_mesh->set_meta("ballast_height", state->ballast_height);
        state->internal_ballast_mesh->set_meta("ballast_offset", state->ballast_offset);
        state->internal_ballast_mesh->set_meta("ballast_width_tiling", state->ballast_width_tiling);
        state->internal_ballast_mesh->set_meta("ballast_length_tiling", state->ballast_length_tiling);
        state->ballast_mesh = state->internal_ballast_mesh->get_rid();
        RenderingServer::get_singleton()->instance_set_base(state->ballast_mesh_instance, state->ballast_mesh);
    }

    void TrackRenderingServer::update_rail_mesh(
            const int64_t p_track_id, const float p_length, const float p_rail_spacing, const float p_curve_precision) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);

        if (p_length < 0.1f) {
            return;
        }

        float old_length = 0.0f;
        float old_spacing = 0.0f;
        if (state->internal_rail_mesh.is_valid()) {
            old_length = state->internal_rail_mesh->get_meta("length", 0.0f);
            old_spacing = state->internal_rail_mesh->get_meta("rail_spacing", 0.0f);
        }

        bool should_generate = state->internal_rail_mesh.is_null();
        if (!should_generate) {
            if (std::abs(old_length - p_length) > 1.0f || std::abs(old_spacing - p_rail_spacing) > 0.001f) {
                should_generate = true;
            }
        }

        if (!should_generate) {
            return;
        }

        const int steps = std::max(2, static_cast<int>(std::floor(p_length / p_curve_precision)));

        PackedVector2Array rail_poly;
        rail_poly.push_back(Vector2(-0.075f, 0.0f));
        rail_poly.push_back(Vector2(0.075f, 0.0f));
        rail_poly.push_back(Vector2(0.075f, 0.015f));
        rail_poly.push_back(Vector2(0.02f, 0.03f));
        rail_poly.push_back(Vector2(0.02f, 0.12f));
        rail_poly.push_back(Vector2(0.04f, 0.13f));
        rail_poly.push_back(Vector2(0.04f, 0.16f));
        rail_poly.push_back(Vector2(-0.04f, 0.16f));
        rail_poly.push_back(Vector2(-0.04f, 0.13f));
        rail_poly.push_back(Vector2(-0.02f, 0.12f));
        rail_poly.push_back(Vector2(-0.02f, 0.03f));
        rail_poly.push_back(Vector2(-0.075f, 0.015f));

        PackedVector3Array vertices;
        PackedVector3Array normals;
        PackedVector2Array uvs;
        PackedInt32Array indices;

        const float offset = p_rail_spacing * 0.5f;
        const int poly_size = static_cast<int>(rail_poly.size());

        for (int i = 0; i <= steps; i++) {
            const float z = (static_cast<float>(i) / static_cast<float>(steps)) * p_length;
            const float v_u = (static_cast<float>(i) / static_cast<float>(steps)) * p_length;

            for (int s = 0; s < 2; s++) {
                constexpr int SIDES[2] = {-1, 1};
                const int side = SIDES[s];
                for (int j = 0; j < poly_size; j++) {
                    const Vector2 p = rail_poly[j];
                    const float x_pos = (static_cast<float>(side) * offset) + (p.x * static_cast<float>(side));
                    vertices.push_back(Vector3(x_pos, p.y, z));
                    normals.push_back(Vector3(p.x, p.y - 0.08f, 0.0f).normalized());
                    uvs.push_back(Vector2(static_cast<float>(j) / static_cast<float>(poly_size - 1), v_u));
                }
            }
        }

        const int v_per_step = poly_size * 2;
        for (int i = 0; i < steps; i++) {
            for (int side_idx = 0; side_idx < 2; side_idx++) {
                const int side_off = side_idx * poly_size;
                for (int j = 0; j < poly_size; j++) {
                    const int next_j = (j + 1) % poly_size;
                    const int curr = (i * v_per_step) + side_off + j;
                    const int next = (i * v_per_step) + side_off + next_j;
                    const int step_curr = ((i + 1) * v_per_step) + side_off + j;
                    const int step_next = ((i + 1) * v_per_step) + side_off + next_j;

                    if (side_idx == 1) {
                        indices.push_back(curr);
                        indices.push_back(next);
                        indices.push_back(step_curr);
                        indices.push_back(next);
                        indices.push_back(step_next);
                        indices.push_back(step_curr);
                    } else {
                        indices.push_back(curr);
                        indices.push_back(step_curr);
                        indices.push_back(next);
                        indices.push_back(next);
                        indices.push_back(step_curr);
                        indices.push_back(step_next);
                    }
                }
            }
        }

        Array arrays;
        arrays.resize(Mesh::ARRAY_MAX);
        arrays.set(Mesh::ARRAY_VERTEX, vertices);
        arrays.set(Mesh::ARRAY_NORMAL, normals);
        arrays.set(Mesh::ARRAY_TEX_UV, uvs);
        arrays.set(Mesh::ARRAY_INDEX, indices);

        Ref<ArrayMesh> new_mesh;
        new_mesh.instantiate();
        new_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

        state->internal_rail_mesh = new_mesh;
        state->internal_rail_mesh->set_meta("length", p_length);
        state->internal_rail_mesh->set_meta("rail_spacing", p_rail_spacing);
        state->rail_mesh = state->internal_rail_mesh->get_rid();
        RenderingServer::get_singleton()->instance_set_base(state->rail_mesh_instance, state->rail_mesh);
    }

    void TrackRenderingServer::set_sleeper_height(const int64_t p_track_id, const float p_height) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);
        state->sleeper_height = p_height;
        update_track_data(p_track_id, state->path_points, state->path_quats, state->path_length);
    }

    float TrackRenderingServer::get_sleeper_height(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, 0.0f);
        return state->sleeper_height;
    }

    void TrackRenderingServer::set_sleeper_spacing(const int64_t p_track_id, const float p_spacing) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);
        state->sleeper_spacing = p_spacing;
        update_track_data(p_track_id, state->path_points, state->path_quats, state->path_length);
    }

    float TrackRenderingServer::get_sleeper_spacing(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, 0.0f);
        return state->sleeper_spacing;
    }

    void TrackRenderingServer::set_rail_spacing(const int64_t p_track_id, const float p_spacing) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);
        state->rail_spacing = p_spacing;
        update_track_data(p_track_id, state->path_points, state->path_quats, state->path_length);
    }

    float TrackRenderingServer::get_rail_spacing(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, 0.0f);
        return state->rail_spacing;
    }

    void TrackRenderingServer::set_ballast_height(const int64_t p_track_id, const float p_height) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);
        state->ballast_height = p_height;
        update_ballast_mesh(p_track_id, state->path_length, 0.5f);
    }

    float TrackRenderingServer::get_ballast_height(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, 0.0f);
        return state->ballast_height;
    }

    void TrackRenderingServer::set_ballast_offset(const int64_t p_track_id, const float p_offset) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);
        state->ballast_offset = p_offset;
        update_ballast_mesh(p_track_id, state->path_length, 0.5f);
    }

    float TrackRenderingServer::get_ballast_offset(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, 0.0f);
        return state->ballast_offset;
    }

    void TrackRenderingServer::set_ballast_width_tiling(const int64_t p_track_id, const float p_tiling) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);
        state->ballast_width_tiling = p_tiling;
        update_ballast_mesh(p_track_id, state->path_length, 0.5f);
    }

    float TrackRenderingServer::get_ballast_width_tiling(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, 0.0f);
        return state->ballast_width_tiling;
    }

    void TrackRenderingServer::set_ballast_length_tiling(const int64_t p_track_id, const float p_tiling) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);
        state->ballast_length_tiling = p_tiling;
        update_ballast_mesh(p_track_id, state->path_length, 0.5f);
    }

    float TrackRenderingServer::get_ballast_length_tiling(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, 0.0f);
        return state->ballast_length_tiling;
    }

    void TrackRenderingServer::set_ballast_enabled(const int64_t p_track_id, const bool p_enabled) {
        TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL(state);
        state->ballast_enabled = p_enabled;
        _update_render_instances(*state);
    }

    bool TrackRenderingServer::get_ballast_enabled(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, false);
        return state->ballast_enabled;
    }

    RID TrackRenderingServer::get_instance_rid(const int64_t p_track_id) const {
        return get_rail_mesh_instance(p_track_id);
    }

    RID TrackRenderingServer::get_rail_mesh_instance(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, RID());
        return state->rail_mesh_instance;
    }

    RID TrackRenderingServer::get_sleeper_multimesh_instance(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, RID());
        return state->sleeper_multimesh_instance;
    }

    RID TrackRenderingServer::get_ballast_mesh_instance(const int64_t p_track_id) const {
        const TrackState *state = get_track_state(p_track_id);
        ERR_FAIL_NULL_V(state, RID());
        return state->ballast_mesh_instance;
    }

} // namespace godot
