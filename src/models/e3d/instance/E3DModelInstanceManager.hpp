#pragma once

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/core/object.hpp>

#include <vector>

namespace godot {

    class E3DModelInstance;

    class E3DModelInstanceManager : public Object {
            GDCLASS(E3DModelInstanceManager, Object)

        protected:
            static void _bind_methods();

        private:
            std::vector<ObjectID> instances;
            bool extension_reload_in_progress = false;
            E3DModelInstance *_get_instance(const ObjectID &p_instance_id) const;
            void _compact_instances();

        public:
            E3DModelInstanceManager();

            static E3DModelInstanceManager *get_instance() {
                return Object::cast_to<E3DModelInstanceManager>(
                        Engine::get_singleton()->get_singleton("E3DModelInstanceManager"));
            }

            static const char *instances_reloaded_signal;

            void reload_all();
            void register_instance(E3DModelInstance *p_instance);
            void unregister_instance(E3DModelInstance *p_instance);
            void reload_instance(E3DModelInstance *p_instance);
            void teardown_all_for_extension_reload();
            void cleanup();
    };

} // namespace godot
