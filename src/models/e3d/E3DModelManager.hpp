#pragma once
#include "E3DModel.hpp"
#include "core/ResourceCache.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

namespace godot {
    class E3DModelManager : public Object {
            GDCLASS(E3DModelManager, Object)
        private:
            Ref<ResourceCache> resource_cache;

            static void _set_owner_recursive(Node *p_node, Node *p_new_owner);

        protected:
            static void _bind_methods();

        public:
            static E3DModelManager *get_instance() {
                return dynamic_cast<E3DModelManager *>(Engine::get_singleton()->get_singleton("E3DModelManager"));
            }

            E3DModelManager();
            Ref<E3DModel> load_model(const String &p_data_path, const String &p_file_name);
            void clear_cache();
    };
} // namespace godot
