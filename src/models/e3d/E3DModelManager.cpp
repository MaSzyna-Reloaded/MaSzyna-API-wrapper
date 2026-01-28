#include "E3DModelManager.hpp"
#include "core/ResourceCache.hpp"

#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

namespace godot {

    void E3DModelManager::_set_owner_recursive(Node *node, Node *new_owner) {
        if (node != new_owner) {
            node->set_owner(new_owner);
        }

        if (node->get_child_count() > 0) {
            for (int i = 0; i < node->get_child_count(); ++i) {
                _set_owner_recursive(node->get_child(i), node);
            }
        }
    }

    void E3DModelManager::_bind_methods() {
        ClassDB::bind_method(D_METHOD("load_model", "data_path", "file_name"), &E3DModelManager::load_model);
    }

    E3DModelManager::E3DModelManager() : user_settings_node(nullptr) {
        Ref<ConfigFile> _config;
        _config.instantiate();
        if (_config->load("user://settings.cfg") == OK) {
            if (OS::get_singleton()->has_feature("release") && !OS::get_singleton()->has_feature("editor")) {
                game_dir = ".";
            } else {
                game_dir = _config->get_value("maszyna", "game_dir", ".");
            }
        } else {
            game_dir = ".";
        }
        // Will be set when entering the SceneTree
    }

    Ref<E3DModel> E3DModelManager::load_model(const String &data_path, const String &file_name) {
        Ref<E3DModel> model;
        const String path = game_dir + "/" + data_path + "/" + file_name + ".e3d";
        
        Ref<Resource> cached_res = ResourceCache::get(path, ResourceCache::RESOURCE_CACHE_DIR_MODELS);
        if (cached_res.is_valid()) {
            return cached_res;
        }

        if (FileAccess::file_exists(path)) {
            if (const Ref<Resource> res = ResourceLoader::get_singleton()->load(path); res.is_valid()) {
                model = res;
                ResourceCache::set(path, model, ResourceCache::RESOURCE_CACHE_DIR_MODELS);
            }
        } else {
            UtilityFunctions::push_warning("File does not exist: " + path);
        }

        return model;
    }
} // namespace godot
