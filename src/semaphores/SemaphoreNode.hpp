#pragma once
#include "SemaphoreServer.hpp"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/object_id.hpp>

namespace godot {
    /// One semaphore of SemaphoreServer, driven freely - no state machine. With a model set, the
    /// node registers that model's lights as the semaphore itself; without one, it attaches to
    /// the semaphore already registered under its name (a scenery's).
    ///
    /// The properties are proxies of the semaphore on the server: they read and change it there,
    /// so a change made by a system shows here as well. Only the kind and the aspect a registered
    /// semaphore starts with are kept on the node, for the scene to set. A semaphore that belongs
    /// to a system is still changed from here, and the system overwrites it with its next event.
    class SemaphoreNode : public Node {
            GDCLASS(SemaphoreNode, Node)

        public:
            static const char *light_state_changed_signal;
            static const char *aspect_changed_signal;
            static const char *semaphore_registered_signal;
            static const char *semaphore_unregistered_signal;

            /// The original's default blinking (fOnTime/fOffTime, AnimModel.h:208-209)
            static constexpr float DEFAULT_BLINK_TIME = 0.5;

        private:
            StringName semaphore_name;
            ObjectID model_id; // the model may be freed before this node
            RID semaphore;
            Ref<SemaphoreKind> kind;
            StringName aspect;

            void _on_model_instance_created(const RID &p_instance);
            void _on_semaphore_registered(const RID &p_semaphore, const StringName &p_name);
            void _on_semaphore_unregistered(const RID &p_semaphore, const StringName &p_name);
            void _on_semaphore_config_changed(const RID &p_semaphore);
            void _on_semaphore_light_state_changed(const RID &p_semaphore, int p_light, int p_state);
            void _on_semaphore_aspect_changed(const RID &p_semaphore, const StringName &p_aspect);
            /// "light_<n>_state" -> n; -1 for any other name
            static int _parse_light_state_property(const StringName &p_name);

        protected:
            static void _bind_methods();
            void _notification(int p_what);
            bool _set(const StringName &p_name, const Variant &p_value);
            bool _get(const StringName &p_name, Variant &p_value) const;
            void _get_property_list(List<PropertyInfo> *p_list) const;

        public:
            void set_semaphore_name(const StringName &p_name);
            StringName get_semaphore_name() const;
            void set_model(Node *p_model);
            Node *get_model() const;
            void set_kind(const Ref<SemaphoreKind> &p_kind);
            Ref<SemaphoreKind> get_kind() const;
            void set_aspect(const StringName &p_aspect);
            StringName get_aspect() const;
            PackedStringArray get_aspects() const;
            int get_light_count() const;

            RID get_semaphore() const;
            void enable_light(int p_light);
            void disable_light(int p_light);
            void blink_light(int p_light, float p_on_time, float p_off_time, float p_phase);
            SemaphoreServer::LightState get_light_state(int p_light) const;
    };
} // namespace godot
