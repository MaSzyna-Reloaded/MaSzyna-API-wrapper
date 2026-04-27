#include "E3DModelManager.hpp"
#include "core/ResourceCache.hpp"
#include "core/UserSettings.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

namespace godot {

    void E3DModelManager::_set_owner_recursive(Node *p_node, Node *p_new_owner) {
        if (p_node != p_new_owner) {
            p_node->set_owner(p_new_owner);
        }

        if (p_node->get_child_count() > 0) {
            for (int i = 0; i < p_node->get_child_count(); ++i) {
                _set_owner_recursive(p_node->get_child(i), p_node);
            }
        }
    }

    void E3DModelManager::_bind_methods() {
        ClassDB::bind_method(D_METHOD("load_model", "data_path", "file_name"), &E3DModelManager::load_model);
        ClassDB::bind_method(D_METHOD("clear_cache"), &E3DModelManager::clear_cache);
    }

    E3DModelManager::E3DModelManager() {
        resource_cache = ResourceCache::create("models");
    }

    Ref<E3DModel> E3DModelManager::load_model(const String &p_data_path, const String &p_file_name) {
        Ref<E3DModel> model;
        UserSettings *settings = UserSettings::get_instance();
        if (settings == nullptr) {
            return model;
        }
        String game_dir = settings->get_maszyna_game_dir();

        const String path = game_dir + "/" + p_data_path + "/" + p_file_name + ".e3d";

        Ref<Resource> cached_res = resource_cache->get(path);
        if (cached_res.is_valid()) {
            return cached_res;
        }

        if (FileAccess::file_exists(path)) {
            if (const Ref<Resource> res = ResourceLoader::get_singleton()->load(path); res.is_valid()) {
                model = res;
                resource_cache->set(path, model);
            }
        } else {
            UtilityFunctions::push_warning("File does not exist: " + path);
        }

        return model;
    }

    void E3DModelManager::clear_cache() {
        resource_cache->clear();
    }
} // namespace godot
