#include "MaszynaLegacySemaphoreDelegate.hpp"
#include "SemaphoreServer.hpp"

namespace godot {
    void MaszynaLegacySemaphoreDelegate::_bind_methods() {}

    void MaszynaLegacySemaphoreDelegate::handle_event(
            const RID &p_system, const StringName &p_event, const Dictionary &p_arguments) {
        if (!(p_event == StringName(LIGHTS_EVENT))) {
            return;
        }
        SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL(server);
        server->semaphore_set_aspect(p_arguments.get("semaphore", RID()), p_arguments.get("aspect", StringName()));
    }
} // namespace godot
