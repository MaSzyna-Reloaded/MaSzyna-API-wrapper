#pragma once
#include "SemaphoreSystemDelegate.hpp"

namespace godot {
    /// The original's semaphores: there is no state machine, the scenery's own events drive the
    /// lights (a `lights` event calls TAnimModel::LightSet(), Event.cpp:1741-1803). Each such event
    /// is an aspect of the semaphore's kind (MaszynaLegacySemaphoreKindFactory); this delegate shows
    /// it. It keeps no state, the aspect is the server's.
    class MaszynaLegacySemaphoreDelegate : public SemaphoreSystemDelegate {
            GDCLASS(MaszynaLegacySemaphoreDelegate, SemaphoreSystemDelegate)

        public:
            /// `lights` event: {"semaphore": RID, "aspect": StringName}
            static constexpr const char *LIGHTS_EVENT = "lights";

        protected:
            static void _bind_methods();

            void handle_event(const RID &p_system, const StringName &p_event, const Dictionary &p_arguments) override;
    };
} // namespace godot
