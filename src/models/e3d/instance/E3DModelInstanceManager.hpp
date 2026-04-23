#pragma once

#include "models/e3d/E3DModelManager.hpp"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <vector>

namespace godot {
    class E3DModelInstance; // forward declaration
    class E3DModelInstanceManager : public Node {
            GDCLASS(E3DModelInstanceManager, Node)
        protected:
            static void _bind_methods();
            E3DModelManager *model_manager;
            std::vector<E3DModelInstance *> instances;

        public:
            E3DModelInstanceManager();
            ~E3DModelInstanceManager() override;
            static const char *instances_reloaded_signal;
            void reload_all() const;
            void register_instance(E3DModelInstance *p_instance);
            void unregister_instance(E3DModelInstance *p_instance);
            void reload_instance(E3DModelInstance *p_instance) const;
    };
} // namespace godot
