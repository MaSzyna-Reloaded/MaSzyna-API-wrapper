#include "SemaphoreSystemNode.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

namespace godot {
    const char *SemaphoreSystemNode::event_published_signal = "event_published";

    void SemaphoreSystemNode::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_delegate", "delegate"), &SemaphoreSystemNode::set_delegate);
        ClassDB::bind_method(D_METHOD("get_delegate"), &SemaphoreSystemNode::get_delegate);
        ClassDB::bind_method(D_METHOD("set_semaphore_names", "names"), &SemaphoreSystemNode::set_semaphore_names);
        ClassDB::bind_method(D_METHOD("get_semaphore_names"), &SemaphoreSystemNode::get_semaphore_names);
        ClassDB::bind_method(D_METHOD("get_system"), &SemaphoreSystemNode::get_system);
        ClassDB::bind_method(
                D_METHOD("send_event", "event", "arguments"), &SemaphoreSystemNode::send_event,
                DEFVAL(Dictionary()));

        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "delegate", PROPERTY_HINT_RESOURCE_TYPE, "SemaphoreSystemDelegate"),
                "set_delegate", "get_delegate");
        ADD_PROPERTY(
                PropertyInfo(Variant::PACKED_STRING_ARRAY, "semaphore_names"), "set_semaphore_names",
                "get_semaphore_names");

        ADD_SIGNAL(MethodInfo(
                event_published_signal, PropertyInfo(Variant::STRING_NAME, "event"),
                PropertyInfo(Variant::DICTIONARY, "arguments")));
    }

    void SemaphoreSystemNode::_notification(const int p_what) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL(server);
        switch (p_what) {
            case NOTIFICATION_ENTER_TREE: {
                system = server->system_create();
                server->system_attach_delegate(system, delegate);
                server->connect(
                        SemaphoreServer::semaphore_registered_signal,
                        callable_mp(this, &SemaphoreSystemNode::_on_semaphore_registered));
                server->connect(
                        SemaphoreServer::system_event_published_signal,
                        callable_mp(this, &SemaphoreSystemNode::_on_system_event_published));
                for (const String &name: semaphore_names) {
                    if (const RID semaphore = server->semaphore_get_rid_by_name(name); semaphore.is_valid()) {
                        server->system_add_semaphore(system, semaphore);
                    }
                }
            } break;
            case NOTIFICATION_EXIT_TREE: {
                server->disconnect(
                        SemaphoreServer::semaphore_registered_signal,
                        callable_mp(this, &SemaphoreSystemNode::_on_semaphore_registered));
                server->disconnect(
                        SemaphoreServer::system_event_published_signal,
                        callable_mp(this, &SemaphoreSystemNode::_on_system_event_published));
                server->system_free(system);
                system = RID();
            } break;
            default:
                break;
        }
    }

    /// A listed semaphore that shows up after the system was created - a model built later, or
    /// the same model after a reload
    void SemaphoreSystemNode::_on_semaphore_registered(const RID &p_semaphore, const StringName &p_name) {
        if (!semaphore_names.has(p_name)) {
            return;
        }
        SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL(server);
        if (server->semaphore_get_system(p_semaphore) == system) {
            return; // renamed within the list
        }
        server->system_add_semaphore(system, p_semaphore);
    }

    void SemaphoreSystemNode::_on_system_event_published(
            const RID &p_system, const StringName &p_event, const Dictionary &p_arguments) {
        if (p_system == system) {
            emit_signal(event_published_signal, p_event, p_arguments);
        }
    }

    void SemaphoreSystemNode::set_delegate(const Ref<SemaphoreSystemDelegate> &p_delegate) {
        delegate = p_delegate;
        if (!system.is_valid()) {
            return;
        }
        SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL(server);
        server->system_attach_delegate(system, delegate);
    }

    Ref<SemaphoreSystemDelegate> SemaphoreSystemNode::get_delegate() const {
        return delegate;
    }

    /// Taken when the node enters the tree, and for semaphores registered after that
    void SemaphoreSystemNode::set_semaphore_names(const PackedStringArray &p_names) {
        semaphore_names = p_names;
    }

    PackedStringArray SemaphoreSystemNode::get_semaphore_names() const {
        return semaphore_names;
    }

    RID SemaphoreSystemNode::get_system() const {
        return system;
    }

    void SemaphoreSystemNode::send_event(const StringName &p_event, const Dictionary &p_arguments) {
        SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL(server);
        server->system_send_event(system, p_event, p_arguments);
    }
} // namespace godot
