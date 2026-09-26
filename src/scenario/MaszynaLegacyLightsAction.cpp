#include "../semaphores/MaszynaLegacySemaphoreDelegate.hpp"
#include "../semaphores/SemaphoreServer.hpp"
#include "MaszynaLegacyLightsAction.hpp"

namespace godot {
    void MaszynaLegacyLightsAction::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_semaphores", "semaphores"), &MaszynaLegacyLightsAction::set_semaphores);
        ClassDB::bind_method(D_METHOD("get_semaphores"), &MaszynaLegacyLightsAction::get_semaphores);
        ClassDB::bind_method(D_METHOD("set_aspects", "aspects"), &MaszynaLegacyLightsAction::set_aspects);
        ClassDB::bind_method(D_METHOD("get_aspects"), &MaszynaLegacyLightsAction::get_aspects);

        ADD_PROPERTY(
                PropertyInfo(Variant::ARRAY, "semaphores", PROPERTY_HINT_ARRAY_TYPE, "RID"), "set_semaphores",
                "get_semaphores");
        ADD_PROPERTY(
                PropertyInfo(Variant::ARRAY, "aspects", PROPERTY_HINT_ARRAY_TYPE, "StringName"), "set_aspects",
                "get_aspects");
    }

    void MaszynaLegacyLightsAction::run(const RID &p_event, const RID &p_activator) {
        ERR_FAIL_COND_MSG(!(semaphores.size() == aspects.size()), "Every semaphore needs its aspect.");
        SemaphoreServer *server = SemaphoreServer::get_instance();
        ERR_FAIL_NULL(server);
        for (int i = 0; i < semaphores.size(); i++) {
            const RID semaphore = semaphores[i];
            Dictionary arguments;
            arguments["semaphore"] = semaphore;
            arguments["aspect"] = aspects[i];
            server->system_send_event(
                    server->semaphore_get_system(semaphore), MaszynaLegacySemaphoreDelegate::LIGHTS_EVENT, arguments);
        }
    }

    void MaszynaLegacyLightsAction::set_semaphores(const TypedArray<RID> &p_semaphores) {
        semaphores = p_semaphores;
    }

    TypedArray<RID> MaszynaLegacyLightsAction::get_semaphores() const {
        return semaphores;
    }

    void MaszynaLegacyLightsAction::set_aspects(const TypedArray<StringName> &p_aspects) {
        aspects = p_aspects;
    }

    TypedArray<StringName> MaszynaLegacyLightsAction::get_aspects() const {
        return aspects;
    }
} // namespace godot
