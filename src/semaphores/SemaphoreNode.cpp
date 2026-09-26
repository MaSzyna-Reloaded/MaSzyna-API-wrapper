#include "SemaphoreNode.hpp"
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

namespace godot {
    const char *SemaphoreNode::light_state_changed_signal = "light_state_changed";
    const char *SemaphoreNode::aspect_changed_signal = "aspect_changed";
    const char *SemaphoreNode::semaphore_registered_signal = "semaphore_registered";
    const char *SemaphoreNode::semaphore_unregistered_signal = "semaphore_unregistered";

    /// E3DModelInstance is a GDScript class, so its signal and method are reached by name
    static constexpr const char *MODEL_INSTANCE_CREATED_SIGNAL = "e3d_instance_created";
    static constexpr const char *MODEL_GET_INSTANCE_METHOD = "get_e3d_instance";
    static constexpr const char *ASPECT_PROPERTY = "aspect";
    static constexpr const char *LIGHT_PROPERTY_PREFIX = "light_";
    static constexpr const char *LIGHT_STATE_PROPERTY_SUFFIX = "_state";

    void SemaphoreNode::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_semaphore_name", "name"), &SemaphoreNode::set_semaphore_name);
        ClassDB::bind_method(D_METHOD("get_semaphore_name"), &SemaphoreNode::get_semaphore_name);
        ClassDB::bind_method(D_METHOD("set_model", "model"), &SemaphoreNode::set_model);
        ClassDB::bind_method(D_METHOD("get_model"), &SemaphoreNode::get_model);
        ClassDB::bind_method(D_METHOD("set_kind", "kind"), &SemaphoreNode::set_kind);
        ClassDB::bind_method(D_METHOD("get_kind"), &SemaphoreNode::get_kind);
        ClassDB::bind_method(D_METHOD("set_aspect", "aspect"), &SemaphoreNode::set_aspect);
        ClassDB::bind_method(D_METHOD("get_aspect"), &SemaphoreNode::get_aspect);
        ClassDB::bind_method(D_METHOD("get_aspects"), &SemaphoreNode::get_aspects);
        ClassDB::bind_method(D_METHOD("get_light_count"), &SemaphoreNode::get_light_count);
        ClassDB::bind_method(D_METHOD("get_semaphore"), &SemaphoreNode::get_semaphore);
        ClassDB::bind_method(D_METHOD("enable_light", "light"), &SemaphoreNode::enable_light);
        ClassDB::bind_method(D_METHOD("disable_light", "light"), &SemaphoreNode::disable_light);
        ClassDB::bind_method(
                D_METHOD("blink_light", "light", "on_time", "off_time", "phase"), &SemaphoreNode::blink_light);
        ClassDB::bind_method(D_METHOD("get_light_state", "light"), &SemaphoreNode::get_light_state);

        ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "semaphore_name"), "set_semaphore_name", "get_semaphore_name");
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "model", PROPERTY_HINT_NODE_TYPE, "E3DModelInstance"), "set_model",
                "get_model");
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "kind", PROPERTY_HINT_RESOURCE_TYPE, "SemaphoreKind"), "set_kind",
                "get_kind");
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "light_count", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_light_count");

        ADD_SIGNAL(MethodInfo(
                light_state_changed_signal, PropertyInfo(Variant::INT, "light"),
                PropertyInfo(Variant::INT, "state", PROPERTY_HINT_ENUM, SemaphoreServer::light_state_hint())));
        ADD_SIGNAL(MethodInfo(aspect_changed_signal, PropertyInfo(Variant::STRING_NAME, "aspect")));
        ADD_SIGNAL(MethodInfo(semaphore_registered_signal));
        ADD_SIGNAL(MethodInfo(semaphore_unregistered_signal));
    }

    /// Runs in the editor too: the model is a @tool, so the semaphore - its light count, its
    /// aspects, what an aspect lights - is there to preview while the scene is edited
    void SemaphoreNode::_notification(const int p_what) {
        SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL(server);
        switch (p_what) {
            case NOTIFICATION_ENTER_TREE: {
                server->connect(
                        SemaphoreServer::semaphore_registered_signal,
                        callable_mp(this, &SemaphoreNode::_on_semaphore_registered));
                server->connect(
                        SemaphoreServer::semaphore_unregistered_signal,
                        callable_mp(this, &SemaphoreNode::_on_semaphore_unregistered));
                server->connect(
                        SemaphoreServer::semaphore_config_changed_signal,
                        callable_mp(this, &SemaphoreNode::_on_semaphore_config_changed));
                server->connect(
                        SemaphoreServer::semaphore_light_state_changed_signal,
                        callable_mp(this, &SemaphoreNode::_on_semaphore_light_state_changed));
                server->connect(
                        SemaphoreServer::semaphore_aspect_changed_signal,
                        callable_mp(this, &SemaphoreNode::_on_semaphore_aspect_changed));
                Node *model = get_model();
                if (model == nullptr) {
                    semaphore = server->semaphore_get_rid_by_name(semaphore_name);
                    break;
                }
                model->connect(
                        MODEL_INSTANCE_CREATED_SIGNAL, callable_mp(this, &SemaphoreNode::_on_model_instance_created));
                // a model that re-entered the tree before this node has its instance already
                if (const RID instance = model->call(MODEL_GET_INSTANCE_METHOD); instance.is_valid()) {
                    _on_model_instance_created(instance);
                }
            } break;
            case NOTIFICATION_EXIT_TREE: {
                server->disconnect(
                        SemaphoreServer::semaphore_registered_signal,
                        callable_mp(this, &SemaphoreNode::_on_semaphore_registered));
                server->disconnect(
                        SemaphoreServer::semaphore_unregistered_signal,
                        callable_mp(this, &SemaphoreNode::_on_semaphore_unregistered));
                server->disconnect(
                        SemaphoreServer::semaphore_config_changed_signal,
                        callable_mp(this, &SemaphoreNode::_on_semaphore_config_changed));
                server->disconnect(
                        SemaphoreServer::semaphore_light_state_changed_signal,
                        callable_mp(this, &SemaphoreNode::_on_semaphore_light_state_changed));
                server->disconnect(
                        SemaphoreServer::semaphore_aspect_changed_signal,
                        callable_mp(this, &SemaphoreNode::_on_semaphore_aspect_changed));
                if (Node *model = get_model(); model != nullptr) {
                    model->disconnect(
                            MODEL_INSTANCE_CREATED_SIGNAL,
                            callable_mp(this, &SemaphoreNode::_on_model_instance_created));
                }
                // a semaphore registered from the model goes with the model's instance
                semaphore = RID();
            } break;
            default:
                break;
        }
    }

    /// The model built a new instance (first load or a reload): its lights become the semaphore,
    /// of the kind and showing the aspect the scene sets
    void SemaphoreNode::_on_model_instance_created(const RID &p_instance) {
        SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL(server);
        semaphore = server->semaphore_create(p_instance);
        server->semaphore_set_kind(semaphore, kind);
        if (kind.is_valid() && kind->has_aspect(aspect)) {
            server->semaphore_set_aspect(semaphore, aspect);
        }
        server->semaphore_set_name(semaphore, semaphore_name);
        notify_property_list_changed();
    }

    void SemaphoreNode::_on_semaphore_registered(const RID &p_semaphore, const StringName &p_name) {
        if (!(p_name == semaphore_name)) {
            return;
        }
        semaphore = p_semaphore;
        notify_property_list_changed();
        emit_signal(semaphore_registered_signal);
    }

    void SemaphoreNode::_on_semaphore_unregistered(const RID &p_semaphore, const StringName &p_name) {
        if (!(p_semaphore == semaphore)) {
            return;
        }
        semaphore = RID();
        notify_property_list_changed();
        emit_signal(semaphore_unregistered_signal);
    }

    /// The model's lights became known, or the kind changed - the property list follows both
    void SemaphoreNode::_on_semaphore_config_changed(const RID &p_semaphore) {
        if (p_semaphore == semaphore) {
            notify_property_list_changed();
        }
    }

    void SemaphoreNode::_on_semaphore_light_state_changed(const RID &p_semaphore, const int p_light, const int p_state) {
        if (p_semaphore == semaphore) {
            emit_signal(light_state_changed_signal, p_light, p_state);
        }
    }

    void SemaphoreNode::_on_semaphore_aspect_changed(const RID &p_semaphore, const StringName &p_aspect) {
        if (p_semaphore == semaphore) {
            emit_signal(aspect_changed_signal, p_aspect);
        }
    }

    int SemaphoreNode::_parse_light_state_property(const StringName &p_name) {
        const String name = p_name;
        if (!name.begins_with(LIGHT_PROPERTY_PREFIX) || !name.ends_with(LIGHT_STATE_PROPERTY_SUFFIX)) {
            return -1;
        }
        const String index = name.trim_prefix(LIGHT_PROPERTY_PREFIX).trim_suffix(LIGHT_STATE_PROPERTY_SUFFIX);
        return index.is_valid_int() ? static_cast<int>(index.to_int()) : -1;
    }

    /// `aspect`, and light_<n>_state - a proxy of the light on the server, blinking with the
    /// default times
    bool SemaphoreNode::_set(const StringName &p_name, const Variant &p_value) {
        if (p_name == StringName(ASPECT_PROPERTY)) {
            set_aspect(p_value);
            return true;
        }
        const int light = _parse_light_state_property(p_name);
        if (light < 0) {
            return false;
        }
        switch (static_cast<int>(p_value)) {
            case SemaphoreServer::LIGHT_STATE_ON:
                enable_light(light);
                break;
            case SemaphoreServer::LIGHT_STATE_BLINKING:
                blink_light(light, DEFAULT_BLINK_TIME, DEFAULT_BLINK_TIME, 0.0);
                break;
            default:
                disable_light(light);
                break;
        }
        return true;
    }

    bool SemaphoreNode::_get(const StringName &p_name, Variant &p_value) const {
        if (p_name == StringName(ASPECT_PROPERTY)) {
            p_value = get_aspect();
            return true;
        }
        const int light = _parse_light_state_property(p_name);
        if (light < 0) {
            return false;
        }
        p_value = get_light_state(light);
        return true;
    }

    /// The aspect as an enum of the kind's aspects (kept in the scene), and one state per light of
    /// the model while the semaphore exists (not kept - it is the server's)
    void SemaphoreNode::_get_property_list(List<PropertyInfo> *p_list) const {
        p_list->push_back(PropertyInfo(
                Variant::STRING_NAME, ASPECT_PROPERTY, PROPERTY_HINT_ENUM, String(",").join(get_aspects())));
        const int count = get_light_count();
        for (int light = 0; light < count; light++) {
            p_list->push_back(PropertyInfo(
                    Variant::INT,
                    String(LIGHT_PROPERTY_PREFIX) + String::num_int64(light) + String(LIGHT_STATE_PROPERTY_SUFFIX),
                    PROPERTY_HINT_ENUM, SemaphoreServer::light_state_hint(), PROPERTY_USAGE_EDITOR));
        }
    }

    /// Renames the semaphore this node registered, or attaches to the one registered under the name
    void SemaphoreNode::set_semaphore_name(const StringName &p_name) {
        semaphore_name = p_name;
        if (!is_inside_tree()) {
            return;
        }
        SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL(server);
        if (!model_id.is_valid()) {
            semaphore = server->semaphore_get_rid_by_name(p_name);
            notify_property_list_changed();
        } else if (semaphore.is_valid()) {
            server->semaphore_set_name(semaphore, p_name);
        }
    }

    StringName SemaphoreNode::get_semaphore_name() const {
        return semaphore_name;
    }

    /// Taken when the node enters the tree
    void SemaphoreNode::set_model(Node *p_model) {
        model_id = p_model != nullptr ? ObjectID(p_model->get_instance_id()) : ObjectID();
    }

    Node *SemaphoreNode::get_model() const {
        return Object::cast_to<Node>(ObjectDB::get_instance(model_id));
    }

    void SemaphoreNode::set_kind(const Ref<SemaphoreKind> &p_kind) {
        kind = p_kind;
        if (semaphore.is_valid()) {
            SemaphoreServer *server = SemaphoreServer::get_instance();
            ERR_FAIL_NULL(server);
            server->semaphore_set_kind(semaphore, kind);
        }
        notify_property_list_changed();
    }

    Ref<SemaphoreKind> SemaphoreNode::get_kind() const {
        const SemaphoreServer *server = SemaphoreServer::get_instance();
        return semaphore.is_valid() && server != nullptr ? server->semaphore_get_kind(semaphore) : kind;
    }

    void SemaphoreNode::set_aspect(const StringName &p_aspect) {
        aspect = p_aspect;
        if (!semaphore.is_valid() || p_aspect.is_empty()) {
            return;
        }
        SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL(server);
        server->semaphore_set_aspect(semaphore, aspect);
    }

    StringName SemaphoreNode::get_aspect() const {
        const SemaphoreServer *server = SemaphoreServer::get_instance();
        return semaphore.is_valid() && server != nullptr ? server->semaphore_get_aspect(semaphore) : aspect;
    }

    /// The aspects the semaphore's kind can show
    PackedStringArray SemaphoreNode::get_aspects() const {
        const Ref<SemaphoreKind> current = get_kind();
        return current.is_valid() ? current->get_aspect_names() : PackedStringArray();
    }

    /// The lights of the semaphore's model; 0 until the semaphore exists and its model is loaded
    int SemaphoreNode::get_light_count() const {
        const SemaphoreServer *server = SemaphoreServer::get_instance();
        return semaphore.is_valid() && server != nullptr ? server->semaphore_get_light_count(semaphore) : 0;
    }

    RID SemaphoreNode::get_semaphore() const {
        return semaphore;
    }

    void SemaphoreNode::enable_light(const int p_light) {
        SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL(server);
        server->semaphore_light_enable(semaphore, p_light);
    }

    void SemaphoreNode::disable_light(const int p_light) {
        SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL(server);
        server->semaphore_light_disable(semaphore, p_light);
    }

    void SemaphoreNode::blink_light(const int p_light, const float p_on_time, const float p_off_time, const float p_phase) {
        SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL(server);
        server->semaphore_light_blink(semaphore, p_light, p_on_time, p_off_time, p_phase);
    }

    SemaphoreServer::LightState SemaphoreNode::get_light_state(const int p_light) const {
        const SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL_V(server, SemaphoreServer::LIGHT_STATE_OFF);
        return server->semaphore_get_light_state(semaphore, p_light);
    }
} // namespace godot
