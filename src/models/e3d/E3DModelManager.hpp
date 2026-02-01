#pragma once
#include "E3DModel.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

#include <godot_cpp/classes/node.hpp>

namespace godot {
    class E3DModelManager: public Node {
        GDCLASS(E3DModelManager, Node)
        private:
            String game_dir;
            Object *user_settings_node;
            ResourceLoader *loader;
            ResourceSaver *saver;
            static void _set_owner_recursive(Node *node, Node *new_owner);
        protected:
            static void _bind_methods();
        public:
            MAKE_MEMBER_GS(String, e3d_cache_path, "user://cache/e3d/res")
            E3DModelManager();
            ~E3DModelManager() override;
            void clear_cache() const;
            Ref<E3DModel> load_model(const String &data_path, const String &file_name);
            static Error remove_dir_recursively(const String &p_path);
    };
} //namespace godot