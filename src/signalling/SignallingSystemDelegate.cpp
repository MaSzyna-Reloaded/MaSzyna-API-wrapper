#include "SignallingSystemDelegate.hpp"

namespace godot {
    void SignallingSystemDelegate::_bind_methods() {
        GDVIRTUAL_BIND(_system_attached, "system");
        GDVIRTUAL_BIND(_system_detached, "system");
        GDVIRTUAL_BIND(_signal_head_added, "system", "signal_head");
        GDVIRTUAL_BIND(_signal_head_removed, "system", "signal_head");
        GDVIRTUAL_BIND(_source_added, "system", "source");
        GDVIRTUAL_BIND(_source_removed, "system", "source");
        GDVIRTUAL_BIND(_signal_head_aspect_changed, "system", "signal_head", "aspect");
        GDVIRTUAL_BIND(_handle_event, "system", "event", "arguments");
        GDVIRTUAL_BIND(_handle_source_event, "system", "source", "event", "arguments");
    }

    void SignallingSystemDelegate::system_attached(const RID &p_system) {
        GDVIRTUAL_CALL(_system_attached, p_system);
    }

    void SignallingSystemDelegate::system_detached(const RID &p_system) {
        GDVIRTUAL_CALL(_system_detached, p_system);
    }

    void SignallingSystemDelegate::signal_head_added(const RID &p_system, const RID &p_signal_head) {
        GDVIRTUAL_CALL(_signal_head_added, p_system, p_signal_head);
    }

    void SignallingSystemDelegate::signal_head_removed(const RID &p_system, const RID &p_signal_head) {
        GDVIRTUAL_CALL(_signal_head_removed, p_system, p_signal_head);
    }

    void SignallingSystemDelegate::source_added(const RID &p_system, const RID &p_source) {
        GDVIRTUAL_CALL(_source_added, p_system, p_source);
    }

    void SignallingSystemDelegate::source_removed(const RID &p_system, const RID &p_source) {
        GDVIRTUAL_CALL(_source_removed, p_system, p_source);
    }

    void SignallingSystemDelegate::signal_head_aspect_changed(
            const RID &p_system, const RID &p_signal_head, const StringName &p_aspect) {
        GDVIRTUAL_CALL(_signal_head_aspect_changed, p_system, p_signal_head, p_aspect);
    }

    void SignallingSystemDelegate::handle_event(
            const RID &p_system, const StringName &p_event, const Dictionary &p_arguments) {
        GDVIRTUAL_CALL(_handle_event, p_system, p_event, p_arguments);
    }

    void SignallingSystemDelegate::handle_source_event(
            const RID &p_system, const RID &p_source, const StringName &p_event, const Dictionary &p_arguments) {
        GDVIRTUAL_CALL(_handle_source_event, p_system, p_source, p_event, p_arguments);
    }
} // namespace godot
