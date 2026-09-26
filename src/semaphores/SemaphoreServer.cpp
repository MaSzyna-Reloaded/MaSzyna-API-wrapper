#include "SemaphoreServer.hpp"
#include "../e3d/E3DRenderingServer.hpp"
#include "../macros.hpp"
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    const char *SemaphoreServer::semaphore_registered_signal = "semaphore_registered";
    const char *SemaphoreServer::semaphore_unregistered_signal = "semaphore_unregistered";
    const char *SemaphoreServer::semaphore_light_state_changed_signal = "semaphore_light_state_changed";
    const char *SemaphoreServer::semaphore_aspect_changed_signal = "semaphore_aspect_changed";
    const char *SemaphoreServer::semaphore_config_changed_signal = "semaphore_config_changed";
    const char *SemaphoreServer::system_semaphore_added_signal = "system_semaphore_added";
    const char *SemaphoreServer::system_semaphore_removed_signal = "system_semaphore_removed";
    const char *SemaphoreServer::system_source_added_signal = "system_source_added";
    const char *SemaphoreServer::system_source_removed_signal = "system_source_removed";
    const char *SemaphoreServer::system_event_published_signal = "system_event_published";

    void SemaphoreServer::_bind_methods() {
        ClassDB::bind_method(D_METHOD("semaphore_create", "instance"), &SemaphoreServer::semaphore_create);
        ClassDB::bind_method(D_METHOD("semaphore_free", "semaphore"), &SemaphoreServer::semaphore_free);
        ClassDB::bind_method(
                D_METHOD("semaphore_set_name", "semaphore", "name"), &SemaphoreServer::semaphore_set_name);
        ClassDB::bind_method(D_METHOD("semaphore_get_name", "semaphore"), &SemaphoreServer::semaphore_get_name);
        ClassDB::bind_method(
                D_METHOD("semaphore_get_rid_by_name", "name"), &SemaphoreServer::semaphore_get_rid_by_name);
        ClassDB::bind_method(D_METHOD("semaphore_get_system", "semaphore"), &SemaphoreServer::semaphore_get_system);
        ClassDB::bind_method(
                D_METHOD("semaphore_light_enable", "semaphore", "light"), &SemaphoreServer::semaphore_light_enable);
        ClassDB::bind_method(
                D_METHOD("semaphore_light_disable", "semaphore", "light"), &SemaphoreServer::semaphore_light_disable);
        ClassDB::bind_method(
                D_METHOD("semaphore_light_blink", "semaphore", "light", "on_time", "off_time", "phase"),
                &SemaphoreServer::semaphore_light_blink);
        ClassDB::bind_method(
                D_METHOD("semaphore_get_light_state", "semaphore", "light"),
                &SemaphoreServer::semaphore_get_light_state);

        ClassDB::bind_method(
                D_METHOD("semaphore_get_light_count", "semaphore"), &SemaphoreServer::semaphore_get_light_count);
        ClassDB::bind_method(D_METHOD("semaphore_set_kind", "semaphore", "kind"), &SemaphoreServer::semaphore_set_kind);
        ClassDB::bind_method(D_METHOD("semaphore_get_kind", "semaphore"), &SemaphoreServer::semaphore_get_kind);
        ClassDB::bind_method(D_METHOD("semaphore_get_aspects", "semaphore"), &SemaphoreServer::semaphore_get_aspects);
        ClassDB::bind_method(
                D_METHOD("semaphore_set_aspect", "semaphore", "aspect"), &SemaphoreServer::semaphore_set_aspect);
        ClassDB::bind_method(D_METHOD("semaphore_get_aspect", "semaphore"), &SemaphoreServer::semaphore_get_aspect);

        ClassDB::bind_method(D_METHOD("system_create"), &SemaphoreServer::system_create);
        ClassDB::bind_method(D_METHOD("system_free", "system"), &SemaphoreServer::system_free);
        ClassDB::bind_method(
                D_METHOD("system_attach_delegate", "system", "delegate"), &SemaphoreServer::system_attach_delegate);
        ClassDB::bind_method(D_METHOD("system_get_delegate", "system"), &SemaphoreServer::system_get_delegate);
        ClassDB::bind_method(D_METHOD("system_set_name", "system", "name"), &SemaphoreServer::system_set_name);
        ClassDB::bind_method(D_METHOD("system_get_name", "system"), &SemaphoreServer::system_get_name);
        ClassDB::bind_method(D_METHOD("system_get_rid_by_name", "name"), &SemaphoreServer::system_get_rid_by_name);
        ClassDB::bind_method(
                D_METHOD("system_add_semaphore", "system", "semaphore"), &SemaphoreServer::system_add_semaphore);
        ClassDB::bind_method(
                D_METHOD("system_remove_semaphore", "system", "semaphore"), &SemaphoreServer::system_remove_semaphore);
        ClassDB::bind_method(D_METHOD("system_get_semaphores", "system"), &SemaphoreServer::system_get_semaphores);
        ClassDB::bind_method(D_METHOD("system_add_source", "system", "source"), &SemaphoreServer::system_add_source);
        ClassDB::bind_method(
                D_METHOD("system_remove_source", "system", "source"), &SemaphoreServer::system_remove_source);
        ClassDB::bind_method(D_METHOD("system_get_sources", "system"), &SemaphoreServer::system_get_sources);
        ClassDB::bind_method(
                D_METHOD("system_send_event", "system", "event", "arguments"), &SemaphoreServer::system_send_event,
                DEFVAL(Dictionary()));
        ClassDB::bind_method(
                D_METHOD("system_publish_event", "system", "event", "arguments"),
                &SemaphoreServer::system_publish_event, DEFVAL(Dictionary()));

        ClassDB::bind_method(D_METHOD("source_create"), &SemaphoreServer::source_create);
        ClassDB::bind_method(D_METHOD("source_free", "source"), &SemaphoreServer::source_free);
        ClassDB::bind_method(D_METHOD("source_set_name", "source", "name"), &SemaphoreServer::source_set_name);
        ClassDB::bind_method(D_METHOD("source_get_name", "source"), &SemaphoreServer::source_get_name);
        ClassDB::bind_method(D_METHOD("source_get_rid_by_name", "name"), &SemaphoreServer::source_get_rid_by_name);
        ClassDB::bind_method(D_METHOD("source_get_system", "source"), &SemaphoreServer::source_get_system);
        ClassDB::bind_method(
                D_METHOD("source_send_event", "source", "event", "arguments"), &SemaphoreServer::source_send_event,
                DEFVAL(Dictionary()));

        BIND_ENUM_CONSTANT(LIGHT_STATE_OFF);
        BIND_ENUM_CONSTANT(LIGHT_STATE_ON);
        BIND_ENUM_CONSTANT(LIGHT_STATE_BLINKING);
        BIND_CONSTANT(MAX_LIGHTS);

        ADD_SIGNAL(MethodInfo(
                semaphore_registered_signal, PropertyInfo(Variant::RID, "semaphore"),
                PropertyInfo(Variant::STRING_NAME, "name")));
        ADD_SIGNAL(MethodInfo(
                semaphore_unregistered_signal, PropertyInfo(Variant::RID, "semaphore"),
                PropertyInfo(Variant::STRING_NAME, "name")));
        ADD_SIGNAL(MethodInfo(
                semaphore_light_state_changed_signal, PropertyInfo(Variant::RID, "semaphore"),
                PropertyInfo(Variant::INT, "light"),
                PropertyInfo(Variant::INT, "state", PROPERTY_HINT_ENUM, light_state_hint())));
        ADD_SIGNAL(MethodInfo(semaphore_config_changed_signal, PropertyInfo(Variant::RID, "semaphore")));
        ADD_SIGNAL(MethodInfo(
                semaphore_aspect_changed_signal, PropertyInfo(Variant::RID, "semaphore"),
                PropertyInfo(Variant::STRING_NAME, "aspect")));
        ADD_SIGNAL(MethodInfo(
                system_semaphore_added_signal, PropertyInfo(Variant::RID, "system"),
                PropertyInfo(Variant::RID, "semaphore")));
        ADD_SIGNAL(MethodInfo(
                system_semaphore_removed_signal, PropertyInfo(Variant::RID, "system"),
                PropertyInfo(Variant::RID, "semaphore")));
        ADD_SIGNAL(MethodInfo(
                system_source_added_signal, PropertyInfo(Variant::RID, "system"), PropertyInfo(Variant::RID, "source")));
        ADD_SIGNAL(MethodInfo(
                system_source_removed_signal, PropertyInfo(Variant::RID, "system"),
                PropertyInfo(Variant::RID, "source")));
        ADD_SIGNAL(MethodInfo(
                system_event_published_signal, PropertyInfo(Variant::RID, "system"),
                PropertyInfo(Variant::STRING_NAME, "event"), PropertyInfo(Variant::DICTIONARY, "arguments")));
    }

    String SemaphoreServer::light_state_hint() {
        return enum_hint({{"Off", LIGHT_STATE_OFF}, {"On", LIGHT_STATE_ON}, {"Blinking", LIGHT_STATE_BLINKING}});
    }

    /// A semaphore is the lights of an instance, so it goes when the instance does
    SemaphoreServer::SemaphoreServer() {
        E3DRenderingServer *rendering = E3DRenderingServer::get_instance();
        ERR_FAIL_NULL(rendering);
        rendering->connect(
                E3DRenderingServer::instance_freed_signal, callable_mp(this, &SemaphoreServer::_on_instance_freed));
        rendering->connect(
                E3DRenderingServer::instance_built_signal, callable_mp(this, &SemaphoreServer::_on_instance_built));
    }

    /// The model is known once the instance is built: its lights are the semaphore's
    void SemaphoreServer::_on_instance_built(const RID &p_instance) {
        const RID *semaphore = semaphores_by_instance.getptr(p_instance);
        if (semaphore == nullptr) {
            return;
        }
        E3DRenderingServer *rendering = E3DRenderingServer::get_instance();
        ERR_FAIL_NULL(rendering);
        SemaphoreData *data = semaphores.getptr(*semaphore);
        ERR_FAIL_NULL(data);
        const int light_count = MIN(rendering->instance_get_light_count(p_instance), MAX_LIGHTS);
        if (data->light_count == light_count) {
            return;
        }
        data->light_count = light_count;
        emit_signal(semaphore_config_changed_signal, *semaphore);
    }

    void SemaphoreServer::_on_instance_freed(const RID &p_instance) {
        const RID *semaphore = semaphores_by_instance.getptr(p_instance);
        if (semaphore == nullptr) {
            return;
        }
        semaphore_free(*semaphore);
    }

    void SemaphoreServer::_rename(
            HashMap<StringName, RID> &p_names, const StringName &p_from, const StringName &p_to, const RID &p_rid) {
        if (const RID *named = p_names.getptr(p_from); named != nullptr && *named == p_rid) {
            p_names.erase(p_from);
        }
        if (p_to.is_empty()) {
            return;
        }
        if (p_names.has(p_to)) {
            UtilityFunctions::push_warning("Duplicate name, the last one wins: " + String(p_to));
        }
        p_names.insert(p_to, p_rid);
    }

    // --- semaphore ---

    RID SemaphoreServer::semaphore_create(const RID &p_instance) {
        ERR_FAIL_COND_V_MSG(
                semaphores_by_instance.has(p_instance), RID(), "The instance already has a semaphore.");
        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        E3DRenderingServer *rendering = E3DRenderingServer::get_instance();
        ERR_FAIL_NULL_V(rendering, RID());
        SemaphoreData data;
        data.instance = p_instance;
        data.light_count = MIN(rendering->instance_get_light_count(p_instance), MAX_LIGHTS);
        semaphores.insert(rid, data);
        semaphores_by_instance.insert(p_instance, rid);
        return rid;
    }

    void SemaphoreServer::semaphore_free(const RID &p_semaphore) {
        const SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL(data);
        if (data->system.is_valid()) {
            system_remove_semaphore(data->system, p_semaphore);
        }
        const SemaphoreData freed = *semaphores.getptr(p_semaphore);
        semaphores.erase(p_semaphore);
        semaphores_by_instance.erase(freed.instance);
        if (!freed.name.is_empty()) {
            _rename(semaphores_by_name, freed.name, StringName(), p_semaphore);
            emit_signal(semaphore_unregistered_signal, p_semaphore, freed.name);
        }
    }

    /// Registers the semaphore under the name - what a SemaphoreNode, a script or a scenery event
    /// finds it by. An empty name unregisters it.
    void SemaphoreServer::semaphore_set_name(const RID &p_semaphore, const StringName &p_name) {
        SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL(data);
        const StringName previous = data->name;
        if (previous == p_name) {
            return;
        }
        data->name = p_name;
        _rename(semaphores_by_name, previous, p_name, p_semaphore);
        if (!previous.is_empty()) {
            emit_signal(semaphore_unregistered_signal, p_semaphore, previous);
        }
        if (!p_name.is_empty()) {
            emit_signal(semaphore_registered_signal, p_semaphore, p_name);
        }
    }

    StringName SemaphoreServer::semaphore_get_name(const RID &p_semaphore) const {
        const SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL_V(data, StringName());
        return data->name;
    }

    RID SemaphoreServer::semaphore_get_rid_by_name(const StringName &p_name) const {
        const RID *rid = semaphores_by_name.getptr(p_name);
        return rid != nullptr ? *rid : RID();
    }

    RID SemaphoreServer::semaphore_get_system(const RID &p_semaphore) const {
        const SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL_V(data, RID());
        return data->system;
    }

    void SemaphoreServer::_set_light_state(
            const RID &p_semaphore, SemaphoreData &p_data, const int p_light, const LightState p_state) {
        const LightState previous = p_data.light_states[p_light];
        p_data.light_states[p_light] = p_state;
        if (!(previous == p_state)) {
            emit_signal(semaphore_light_state_changed_signal, p_semaphore, p_light, p_state);
        }
    }

    void SemaphoreServer::semaphore_light_enable(const RID &p_semaphore, const int p_light) {
        SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL(data);
        ERR_FAIL_INDEX(p_light, MAX_LIGHTS);
        E3DRenderingServer *rendering = E3DRenderingServer::get_instance();
        ERR_FAIL_NULL(rendering);
        rendering->instance_set_light_mode(data->instance, p_light, E3DRenderingServer::LIGHT_MODE_ON);
        _set_light_state(p_semaphore, *data, p_light, LIGHT_STATE_ON);
    }

    void SemaphoreServer::semaphore_light_disable(const RID &p_semaphore, const int p_light) {
        SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL(data);
        ERR_FAIL_INDEX(p_light, MAX_LIGHTS);
        E3DRenderingServer *rendering = E3DRenderingServer::get_instance();
        ERR_FAIL_NULL(rendering);
        rendering->instance_set_light_mode(data->instance, p_light, E3DRenderingServer::LIGHT_MODE_OFF);
        _set_light_state(p_semaphore, *data, p_light, LIGHT_STATE_OFF);
    }

    void SemaphoreServer::semaphore_light_blink(
            const RID &p_semaphore, const int p_light, const float p_on_time, const float p_off_time,
            const float p_phase) {
        SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL(data);
        ERR_FAIL_INDEX(p_light, MAX_LIGHTS);
        E3DRenderingServer *rendering = E3DRenderingServer::get_instance();
        ERR_FAIL_NULL(rendering);
        rendering->instance_set_light_blink(data->instance, p_light, p_on_time, p_off_time, p_phase);
        _set_light_state(p_semaphore, *data, p_light, LIGHT_STATE_BLINKING);
    }

    SemaphoreServer::LightState SemaphoreServer::semaphore_get_light_state(const RID &p_semaphore, const int p_light) const {
        const SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL_V(data, LIGHT_STATE_OFF);
        ERR_FAIL_INDEX_V(p_light, MAX_LIGHTS, LIGHT_STATE_OFF);
        return data->light_states[p_light];
    }

    /// How many lights the semaphore's model has (TAnimModel::iNumLights); 0 until the model is
    /// loaded. A light past it may still be set, as LightSet() allows (AnimModel.cpp:664-670).
    int SemaphoreServer::semaphore_get_light_count(const RID &p_semaphore) const {
        const SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL_V(data, 0);
        return data->light_count;
    }

    /// What the semaphore can show. A new kind leaves the lights as they are and no aspect set.
    void SemaphoreServer::semaphore_set_kind(const RID &p_semaphore, const Ref<SemaphoreKind> &p_kind) {
        SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL(data);
        data->kind = p_kind;
        data->aspect = StringName();
        emit_signal(semaphore_config_changed_signal, p_semaphore);
    }

    Ref<SemaphoreKind> SemaphoreServer::semaphore_get_kind(const RID &p_semaphore) const {
        const SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL_V(data, Ref<SemaphoreKind>());
        return data->kind;
    }

    PackedStringArray SemaphoreServer::semaphore_get_aspects(const RID &p_semaphore) const {
        const SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL_V(data, PackedStringArray());
        return data->kind.is_valid() ? data->kind->get_aspect_names() : PackedStringArray();
    }

    /// Lights every light the aspect describes and tells the semaphore's system, whose delegate
    /// may react (an automatic block changes the semaphore behind)
    void SemaphoreServer::semaphore_set_aspect(const RID &p_semaphore, const StringName &p_aspect) {
        SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL(data);
        ERR_FAIL_COND_MSG(data->kind.is_null(), "The semaphore has no kind.");
        ERR_FAIL_COND_MSG(!data->kind->has_aspect(p_aspect), "The semaphore's kind has no aspect " + String(p_aspect) + ".");
        const Ref<SemaphoreKind> kind = data->kind;
        const Ref<SemaphoreAspect> shown = kind->get_aspect(p_aspect);
        ERR_FAIL_COND(shown.is_null());
        const PackedInt32Array lights = shown->get_lights();
        const PackedFloat32Array on_times = shown->get_on_times();
        const PackedFloat32Array off_times = shown->get_off_times();
        const PackedFloat32Array phases = shown->get_phases();
        const int count = MIN(static_cast<int>(lights.size()), MAX_LIGHTS);
        for (int light = 0; light < count; light++) {
            switch (lights[light]) {
                case SemaphoreAspect::LIGHT_OFF:
                    semaphore_light_disable(p_semaphore, light);
                    break;
                case SemaphoreAspect::LIGHT_ON:
                    semaphore_light_enable(p_semaphore, light);
                    break;
                case SemaphoreAspect::LIGHT_BLINK:
                    semaphore_light_blink(
                            p_semaphore, light, light < on_times.size() ? on_times[light] : kind->get_blink_on_time(),
                            light < off_times.size() ? off_times[light] : kind->get_blink_off_time(),
                            light < phases.size() ? phases[light] : 0.0f);
                    break;
                default:
                    break; // LIGHT_KEEP
            }
        }
        data = semaphores.getptr(p_semaphore); // the light signals may have rehashed the table
        ERR_FAIL_NULL(data);
        data->aspect = p_aspect;
        const RID system_rid = data->system;
        emit_signal(semaphore_aspect_changed_signal, p_semaphore, p_aspect);
        const SystemData *system = systems.getptr(system_rid);
        if (system != nullptr && system->delegate.is_valid()) {
            const Ref<SemaphoreSystemDelegate> delegate = system->delegate;
            delegate->semaphore_aspect_changed(system_rid, p_semaphore, p_aspect);
        }
    }

    StringName SemaphoreServer::semaphore_get_aspect(const RID &p_semaphore) const {
        const SemaphoreData *data = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL_V(data, StringName());
        return data->aspect;
    }

    // --- system ---

    RID SemaphoreServer::system_create() {
        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        systems.insert(rid, SystemData());
        return rid;
    }

    /// Frees the system; its semaphores and sources stay, belonging to no system
    void SemaphoreServer::system_free(const RID &p_system) {
        ERR_FAIL_COND(!systems.has(p_system));
        system_attach_delegate(p_system, Ref<SemaphoreSystemDelegate>());
        const SystemData freed = *systems.getptr(p_system);
        systems.erase(p_system);
        _rename(systems_by_name, freed.name, StringName(), p_system);
        for (const RID &semaphore_rid: freed.semaphores) {
            if (SemaphoreData *semaphore = semaphores.getptr(semaphore_rid); semaphore != nullptr) {
                semaphore->system = RID();
            }
        }
        for (const RID &source_rid: freed.sources) {
            if (SourceData *source = sources.getptr(source_rid); source != nullptr) {
                source->system = RID();
            }
        }
    }

    /// Replaces the system's delegate (null detaches it). The new delegate is told about every
    /// semaphore and source the system already holds, so the order of setting up does not matter.
    void SemaphoreServer::system_attach_delegate(const RID &p_system, const Ref<SemaphoreSystemDelegate> &p_delegate) {
        SystemData *system = systems.getptr(p_system);
        ERR_FAIL_NULL(system);
        const Ref<SemaphoreSystemDelegate> previous = system->delegate;
        system->delegate = p_delegate;
        // copied: a delegate may call back into this server and rehash `systems`
        const Vector<RID> held_semaphores = system->semaphores;
        const Vector<RID> held_sources = system->sources;
        if (previous.is_valid()) {
            previous->system_detached(p_system);
        }
        if (p_delegate.is_null()) {
            return;
        }
        p_delegate->system_attached(p_system);
        for (const RID &semaphore: held_semaphores) {
            p_delegate->semaphore_added(p_system, semaphore);
        }
        for (const RID &source: held_sources) {
            p_delegate->source_added(p_system, source);
        }
    }

    Ref<SemaphoreSystemDelegate> SemaphoreServer::system_get_delegate(const RID &p_system) const {
        const SystemData *system = systems.getptr(p_system);
        ERR_FAIL_NULL_V(system, Ref<SemaphoreSystemDelegate>());
        return system->delegate;
    }

    void SemaphoreServer::system_set_name(const RID &p_system, const StringName &p_name) {
        SystemData *system = systems.getptr(p_system);
        ERR_FAIL_NULL(system);
        const StringName previous = system->name;
        system->name = p_name;
        _rename(systems_by_name, previous, p_name, p_system);
    }

    StringName SemaphoreServer::system_get_name(const RID &p_system) const {
        const SystemData *system = systems.getptr(p_system);
        ERR_FAIL_NULL_V(system, StringName());
        return system->name;
    }

    RID SemaphoreServer::system_get_rid_by_name(const StringName &p_name) const {
        const RID *rid = systems_by_name.getptr(p_name);
        return rid != nullptr ? *rid : RID();
    }

    /// A semaphore belongs to one system at most - the one that decides its aspects
    void SemaphoreServer::system_add_semaphore(const RID &p_system, const RID &p_semaphore) {
        SystemData *system = systems.getptr(p_system);
        ERR_FAIL_NULL(system);
        SemaphoreData *semaphore = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL(semaphore);
        ERR_FAIL_COND_MSG(semaphore->system.is_valid(), "The semaphore already belongs to a system.");
        semaphore->system = p_system;
        system->semaphores.push_back(p_semaphore);
        const Ref<SemaphoreSystemDelegate> delegate = system->delegate;
        emit_signal(system_semaphore_added_signal, p_system, p_semaphore);
        if (delegate.is_valid()) {
            delegate->semaphore_added(p_system, p_semaphore);
        }
    }

    void SemaphoreServer::system_remove_semaphore(const RID &p_system, const RID &p_semaphore) {
        SystemData *system = systems.getptr(p_system);
        ERR_FAIL_NULL(system);
        SemaphoreData *semaphore = semaphores.getptr(p_semaphore);
        ERR_FAIL_NULL(semaphore);
        ERR_FAIL_COND_MSG(!(semaphore->system == p_system), "The semaphore does not belong to this system.");
        semaphore->system = RID();
        system->semaphores.erase(p_semaphore);
        const Ref<SemaphoreSystemDelegate> delegate = system->delegate;
        emit_signal(system_semaphore_removed_signal, p_system, p_semaphore);
        if (delegate.is_valid()) {
            delegate->semaphore_removed(p_system, p_semaphore);
        }
    }

    TypedArray<RID> SemaphoreServer::system_get_semaphores(const RID &p_system) const {
        TypedArray<RID> result;
        const SystemData *system = systems.getptr(p_system);
        ERR_FAIL_NULL_V(system, result);
        for (const RID &semaphore: system->semaphores) {
            result.push_back(semaphore);
        }
        return result;
    }

    /// A source belongs to one system at most - the one its events go to
    void SemaphoreServer::system_add_source(const RID &p_system, const RID &p_source) {
        SystemData *system = systems.getptr(p_system);
        ERR_FAIL_NULL(system);
        SourceData *source = sources.getptr(p_source);
        ERR_FAIL_NULL(source);
        ERR_FAIL_COND_MSG(source->system.is_valid(), "The source already belongs to a system.");
        source->system = p_system;
        system->sources.push_back(p_source);
        const Ref<SemaphoreSystemDelegate> delegate = system->delegate;
        emit_signal(system_source_added_signal, p_system, p_source);
        if (delegate.is_valid()) {
            delegate->source_added(p_system, p_source);
        }
    }

    void SemaphoreServer::system_remove_source(const RID &p_system, const RID &p_source) {
        SystemData *system = systems.getptr(p_system);
        ERR_FAIL_NULL(system);
        SourceData *source = sources.getptr(p_source);
        ERR_FAIL_NULL(source);
        ERR_FAIL_COND_MSG(!(source->system == p_system), "The source does not belong to this system.");
        source->system = RID();
        system->sources.erase(p_source);
        const Ref<SemaphoreSystemDelegate> delegate = system->delegate;
        emit_signal(system_source_removed_signal, p_system, p_source);
        if (delegate.is_valid()) {
            delegate->source_removed(p_system, p_source);
        }
    }

    TypedArray<RID> SemaphoreServer::system_get_sources(const RID &p_system) const {
        TypedArray<RID> result;
        const SystemData *system = systems.getptr(p_system);
        ERR_FAIL_NULL_V(system, result);
        for (const RID &source: system->sources) {
            result.push_back(source);
        }
        return result;
    }

    /// A command to the system as a whole, handed to its delegate
    void SemaphoreServer::system_send_event(
            const RID &p_system, const StringName &p_event, const Dictionary &p_arguments) {
        const SystemData *system = systems.getptr(p_system);
        ERR_FAIL_NULL(system);
        const Ref<SemaphoreSystemDelegate> delegate = system->delegate;
        ERR_FAIL_COND_MSG(delegate.is_null(), "The system has no delegate.");
        delegate->handle_event(p_system, p_event, p_arguments);
    }

    /// What the delegate announces - an aspect, a speed, anything its vocabulary has
    void SemaphoreServer::system_publish_event(
            const RID &p_system, const StringName &p_event, const Dictionary &p_arguments) {
        ERR_FAIL_COND(!systems.has(p_system));
        emit_signal(system_event_published_signal, p_system, p_event, p_arguments);
    }

    // --- source ---

    RID SemaphoreServer::source_create() {
        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        sources.insert(rid, SourceData());
        return rid;
    }

    void SemaphoreServer::source_free(const RID &p_source) {
        const SourceData *source = sources.getptr(p_source);
        ERR_FAIL_NULL(source);
        if (source->system.is_valid()) {
            system_remove_source(source->system, p_source);
        }
        const StringName name = sources.getptr(p_source)->name;
        sources.erase(p_source);
        _rename(sources_by_name, name, StringName(), p_source);
    }

    void SemaphoreServer::source_set_name(const RID &p_source, const StringName &p_name) {
        SourceData *source = sources.getptr(p_source);
        ERR_FAIL_NULL(source);
        const StringName previous = source->name;
        source->name = p_name;
        _rename(sources_by_name, previous, p_name, p_source);
    }

    StringName SemaphoreServer::source_get_name(const RID &p_source) const {
        const SourceData *source = sources.getptr(p_source);
        ERR_FAIL_NULL_V(source, StringName());
        return source->name;
    }

    RID SemaphoreServer::source_get_rid_by_name(const StringName &p_name) const {
        const RID *rid = sources_by_name.getptr(p_name);
        return rid != nullptr ? *rid : RID();
    }

    RID SemaphoreServer::source_get_system(const RID &p_source) const {
        const SourceData *source = sources.getptr(p_source);
        ERR_FAIL_NULL_V(source, RID());
        return source->system;
    }

    /// A report of the source, handed to the delegate of the system it belongs to
    void SemaphoreServer::source_send_event(
            const RID &p_source, const StringName &p_event, const Dictionary &p_arguments) {
        const SourceData *source = sources.getptr(p_source);
        ERR_FAIL_NULL(source);
        const SystemData *system = systems.getptr(source->system);
        ERR_FAIL_NULL_MSG(system, "The source belongs to no system.");
        const Ref<SemaphoreSystemDelegate> delegate = system->delegate;
        ERR_FAIL_COND_MSG(delegate.is_null(), "The system has no delegate.");
        delegate->handle_source_event(source->system, p_source, p_event, p_arguments);
    }
} // namespace godot
