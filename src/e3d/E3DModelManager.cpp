#include "E3DModelManager.hpp"

#include "core/UserSettings.hpp"
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    void E3DModelManager::_bind_methods() {
        ClassDB::bind_method(D_METHOD("clear_cache"), &E3DModelManager::clear_cache);
        ClassDB::bind_method(D_METHOD("load_model", "data_path", "filename"), &E3DModelManager::load_model);
    }

    E3DModelManager::E3DModelManager() {
        cache = ResourceCache::create("e3d");
    }

    void E3DModelManager::clear_cache() const {
        if (cache.is_valid()) {
            cache->clear();
        }
    }

    String E3DModelManager::_make_cache_path(const String &p_data_path, const String &p_filename) const {
        return p_data_path.path_join(p_filename + String(".e3d.res"));
    }

    String E3DModelManager::_make_cache_hash(const String &p_source_abs_path) const {
        const String hash_source = String::num_int64(FileAccess::get_modified_time(p_source_abs_path)) + ":" +
                                   String::num_int64(E3DModel::FORMAT_VERSION) + ":" + p_source_abs_path;
        return hash_source.md5_text();
    }

    Ref<E3DModel> E3DModelManager::load_model(const String &p_data_path, const String &p_filename) const {
        UserSettings *user_settings = UserSettings::get_instance();
        if (user_settings == nullptr) {
            return nullptr;
        }

        const String relative_path = p_data_path.path_join(p_filename + String(".e3d"));
        const String source_path = user_settings->get_maszyna_game_dir().path_join(relative_path);
        const String cached_path = _make_cache_path(p_data_path, p_filename);
        const String cache_hash = _make_cache_hash(source_path);

        Ref<E3DModel> output = cache->get(cached_path, cache_hash);
        if (output.is_valid()) {
            return output;
        }

        if (!FileAccess::file_exists(source_path)) {
            UtilityFunctions::push_error("File does not exist: " + source_path);
            return nullptr;
        }

        ResourceLoader *resource_loader = ResourceLoader::get_singleton();
        if (resource_loader == nullptr) {
            return nullptr;
        }

        output = resource_loader->load(source_path, "E3DModel");
        if (output.is_null()) {
            UtilityFunctions::push_warning("File is not an E3DModel: " + source_path);
            return nullptr;
        }

        cache->set(cached_path, output, cache_hash);
        return cache->get(cached_path);
    }

} // namespace godot
