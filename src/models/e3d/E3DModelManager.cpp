#include "E3DModelManager.hpp"

#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

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
        ClassDB::bind_method(D_METHOD("clear_cache"), &E3DModelManager::clear_cache);
        ClassDB::bind_method(D_METHOD("load_model", "data_path", "file_name"), &E3DModelManager::load_model);
    }

    E3DModelManager::E3DModelManager() : user_settings_node(nullptr), loader(memnew(ResourceLoader)), saver(memnew(ResourceSaver)) {


        if (game_dir == "") {
            Ref<ConfigFile> _config = memnew(ConfigFile);
            _config.instantiate();
            _config->load("user://settings.cfg");
            game_dir = _config->get_value("maszyna", "game_dir");
        }
        // Will be set when entering the SceneTree
    }

    E3DModelManager::~E3DModelManager() {
        memdelete(loader);
        memdelete(saver);
    }

    void E3DModelManager::clear_cache() const {
        if (DirAccess::dir_exists_absolute(e3d_cache_path)) {
            if (const Error err = remove_dir_recursively(e3d_cache_path); err == OK) {
                UtilityFunctions::print("E3D/RES cache cleared");
                return;
            }

            UtilityFunctions::push_error("E3D/RES cache error: "); //err
        }
    }

    Ref<E3DModel> E3DModelManager::load_model(const String &data_path, const String &file_name) {
        Ref<E3DModel> model;
        const String path = game_dir + "/" + data_path + "/" + file_name + ".e3d";
        String cached_path = e3d_cache_path+"/"+data_path+"__"+file_name+".res";
        const String cached_meta_path = cached_path+".meta";
        if (FileAccess::file_exists(path)) {
            bool is_cache_valid = false;
            const String _hash = std::to_string(FileAccess::get_modified_time(path)).c_str();
            if (FileAccess::file_exists(cached_path) && FileAccess::file_exists(cached_meta_path)) {
                const String _cached_hash = FileAccess::get_file_as_string(cached_path);
                is_cache_valid = (_hash == _cached_hash);
            }

            if (is_cache_valid) {
                model = loader->load(cached_path);
            } else {
                if (const Ref<Resource> res = loader->load(path); res.is_valid()) {
                    model = res;
                    const String cached_dir = cached_path.get_base_dir();
                    DirAccess::make_dir_recursive_absolute(cached_dir);
                    saver->save(res, cached_path);
                    if (const Ref<FileAccess> meta = FileAccess::open(cached_meta_path, FileAccess::WRITE); meta.is_valid()) {
                        meta->store_string(_hash);
                        meta->close();
                    }
                }
            }
        } else {
            UtilityFunctions::push_error("File does not exist: " + path);
        }

        return model;
    }

    Error E3DModelManager::remove_dir_recursively(const String &p_path) {
        PackedStringArray subdirs = DirAccess::get_directories_at(p_path);

        for (const String& subdir: subdirs) {
            if (const Error err = remove_dir_recursively(p_path + String("/") + subdir); err != OK) {
                return OK;
            }
        }

        for (const String& file: DirAccess::get_files_at(p_path)) {
            if (const Error err = DirAccess::remove_absolute(p_path + String("/") + file); err != OK) {
                return err;
            }
        }

        return DirAccess::remove_absolute(p_path);
    }

} //namespace godot
