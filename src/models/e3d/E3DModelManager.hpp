#pragma once
#include "E3DModel.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/resource_loader.hpp>

#include <godot_cpp/classes/node.hpp>

namespace godot {
    class E3DModelManager: public Node {
        GDCLASS(E3DModelManager, Node)
        private:
            String game_dir;
            Object *user_settings_node;

            static void _set_owner_recursive(Node *node, Node *new_owner);
        protected:
            static void _bind_methods();
        public:
            E3DModelManager();
            void reload_settings();
            Ref<E3DModel> load_model(const String &data_path, const String &file_name);
    };
} //namespace godot
