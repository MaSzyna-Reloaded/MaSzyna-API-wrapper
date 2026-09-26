#include "SemaphoreSystemDelegate.hpp"

namespace godot {
    void SemaphoreSystemDelegate::_bind_methods() {
        GDVIRTUAL_BIND(_system_attached, "system");
        GDVIRTUAL_BIND(_system_detached, "system");
        GDVIRTUAL_BIND(_semaphore_added, "system", "semaphore");
        GDVIRTUAL_BIND(_semaphore_removed, "system", "semaphore");
        GDVIRTUAL_BIND(_source_added, "system", "source");
        GDVIRTUAL_BIND(_source_removed, "system", "source");
        GDVIRTUAL_BIND(_semaphore_aspect_changed, "system", "semaphore", "aspect");
        GDVIRTUAL_BIND(_handle_event, "system", "event", "arguments");
        GDVIRTUAL_BIND(_handle_source_event, "system", "source", "event", "arguments");
    }

    void SemaphoreSystemDelegate::system_attached(const RID &p_system) {
        GDVIRTUAL_CALL(_system_attached, p_system);
    }

    void SemaphoreSystemDelegate::system_detached(const RID &p_system) {
        GDVIRTUAL_CALL(_system_detached, p_system);
    }

    void SemaphoreSystemDelegate::semaphore_added(const RID &p_system, const RID &p_semaphore) {
        GDVIRTUAL_CALL(_semaphore_added, p_system, p_semaphore);
    }

    void SemaphoreSystemDelegate::semaphore_removed(const RID &p_system, const RID &p_semaphore) {
        GDVIRTUAL_CALL(_semaphore_removed, p_system, p_semaphore);
    }

    void SemaphoreSystemDelegate::source_added(const RID &p_system, const RID &p_source) {
        GDVIRTUAL_CALL(_source_added, p_system, p_source);
    }

    void SemaphoreSystemDelegate::source_removed(const RID &p_system, const RID &p_source) {
        GDVIRTUAL_CALL(_source_removed, p_system, p_source);
    }

    void SemaphoreSystemDelegate::semaphore_aspect_changed(
            const RID &p_system, const RID &p_semaphore, const StringName &p_aspect) {
        GDVIRTUAL_CALL(_semaphore_aspect_changed, p_system, p_semaphore, p_aspect);
    }

    void SemaphoreSystemDelegate::handle_event(
            const RID &p_system, const StringName &p_event, const Dictionary &p_arguments) {
        GDVIRTUAL_CALL(_handle_event, p_system, p_event, p_arguments);
    }

    void SemaphoreSystemDelegate::handle_source_event(
            const RID &p_system, const RID &p_source, const StringName &p_event, const Dictionary &p_arguments) {
        GDVIRTUAL_CALL(_handle_source_event, p_system, p_source, p_event, p_arguments);
    }
} // namespace godot
