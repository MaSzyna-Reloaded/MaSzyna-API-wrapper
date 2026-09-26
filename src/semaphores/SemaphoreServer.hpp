#pragma once
#include "SemaphoreKind.hpp"
#include "SemaphoreSystemDelegate.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {
    /// RID based registry of semaphores and of the systems that control them.
    ///
    /// A semaphore is the lights of one model instance; it lives exactly as long as that
    /// instance. Low level, its lights are switched one by one. High level, semaphores and event
    /// sources are grouped into systems, and a system's SemaphoreSystemDelegate decides what an
    /// event means. The server knows no aspects - "S1" or "Sz" is the vocabulary of a delegate,
    /// which announces it with system_publish_event().
    class SemaphoreServer : public Object {
            GDCLASS(SemaphoreServer, Object)

        public:
            enum LightState {
                LIGHT_STATE_OFF,
                LIGHT_STATE_ON,
                LIGHT_STATE_BLINKING,
            };

            /// iMaxNumLights, AnimModel.h:23
            static constexpr int MAX_LIGHTS = 8;

            static const char *semaphore_registered_signal;
            static const char *semaphore_unregistered_signal;
            static const char *semaphore_light_state_changed_signal;
            static const char *semaphore_aspect_changed_signal;
            static const char *semaphore_config_changed_signal;
            static const char *system_semaphore_added_signal;
            static const char *system_semaphore_removed_signal;
            static const char *system_source_added_signal;
            static const char *system_source_removed_signal;
            static const char *system_event_published_signal;

            /// The PROPERTY_HINT_ENUM string of LightState
            static String light_state_hint();

            static SemaphoreServer *get_instance() {
                return Object::cast_to<SemaphoreServer>(Engine::get_singleton()->get_singleton("SemaphoreServer"));
            }

        private:
            struct SemaphoreData {
                    RID instance;
                    StringName name;
                    RID system;
                    Ref<SemaphoreKind> kind;
                    int light_count = 0; // the model's, known once its instance is built
                    LightState light_states[MAX_LIGHTS] = {};
                    StringName aspect;
            };

            struct SourceData {
                    StringName name;
                    RID system;
            };

            struct SystemData {
                    StringName name;
                    Ref<SemaphoreSystemDelegate> delegate;
                    Vector<RID> semaphores;
                    Vector<RID> sources;
            };

            HashMap<RID, SemaphoreData> semaphores;
            HashMap<RID, SourceData> sources;
            HashMap<RID, SystemData> systems;
            HashMap<StringName, RID> semaphores_by_name;
            HashMap<StringName, RID> sources_by_name;
            HashMap<StringName, RID> systems_by_name;
            HashMap<RID, RID> semaphores_by_instance;

            void _on_instance_freed(const RID &p_instance);
            void _on_instance_built(const RID &p_instance);
            void _set_light_state(const RID &p_semaphore, SemaphoreData &p_data, int p_light, LightState p_state);
            /// The original's name tables let the newest entry win a duplicate name (Names.h:29)
            static void
            _rename(HashMap<StringName, RID> &p_names, const StringName &p_from, const StringName &p_to,
                    const RID &p_rid);

        protected:
            static void _bind_methods();

        public:
            SemaphoreServer();

            RID semaphore_create(const RID &p_instance);
            void semaphore_free(const RID &p_semaphore);
            void semaphore_set_name(const RID &p_semaphore, const StringName &p_name);
            StringName semaphore_get_name(const RID &p_semaphore) const;
            RID semaphore_get_rid_by_name(const StringName &p_name) const;
            RID semaphore_get_system(const RID &p_semaphore) const;
            void semaphore_light_enable(const RID &p_semaphore, int p_light);
            void semaphore_light_disable(const RID &p_semaphore, int p_light);
            /// On for p_on_time seconds, off for p_off_time, the cycle shifted by p_phase seconds
            void semaphore_light_blink(const RID &p_semaphore, int p_light, float p_on_time, float p_off_time, float p_phase);
            LightState semaphore_get_light_state(const RID &p_semaphore, int p_light) const;
            int semaphore_get_light_count(const RID &p_semaphore) const;
            void semaphore_set_kind(const RID &p_semaphore, const Ref<SemaphoreKind> &p_kind);
            Ref<SemaphoreKind> semaphore_get_kind(const RID &p_semaphore) const;
            /// The aspects the semaphore's kind can show, empty without a kind
            PackedStringArray semaphore_get_aspects(const RID &p_semaphore) const;
            /// Lights the aspect the way the semaphore's kind describes it
            void semaphore_set_aspect(const RID &p_semaphore, const StringName &p_aspect);
            StringName semaphore_get_aspect(const RID &p_semaphore) const;

            RID system_create();
            void system_free(const RID &p_system);
            void system_attach_delegate(const RID &p_system, const Ref<SemaphoreSystemDelegate> &p_delegate);
            Ref<SemaphoreSystemDelegate> system_get_delegate(const RID &p_system) const;
            void system_set_name(const RID &p_system, const StringName &p_name);
            StringName system_get_name(const RID &p_system) const;
            RID system_get_rid_by_name(const StringName &p_name) const;
            void system_add_semaphore(const RID &p_system, const RID &p_semaphore);
            void system_remove_semaphore(const RID &p_system, const RID &p_semaphore);
            TypedArray<RID> system_get_semaphores(const RID &p_system) const;
            void system_add_source(const RID &p_system, const RID &p_source);
            void system_remove_source(const RID &p_system, const RID &p_source);
            TypedArray<RID> system_get_sources(const RID &p_system) const;
            void system_send_event(const RID &p_system, const StringName &p_event, const Dictionary &p_arguments);
            void system_publish_event(const RID &p_system, const StringName &p_event, const Dictionary &p_arguments);

            RID source_create();
            void source_free(const RID &p_source);
            void source_set_name(const RID &p_source, const StringName &p_name);
            StringName source_get_name(const RID &p_source) const;
            RID source_get_rid_by_name(const StringName &p_name) const;
            RID source_get_system(const RID &p_source) const;
            void source_send_event(const RID &p_source, const StringName &p_event, const Dictionary &p_arguments);
    };
} // namespace godot

VARIANT_ENUM_CAST(SemaphoreServer::LightState)
