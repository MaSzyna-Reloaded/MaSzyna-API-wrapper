#include "ResourceCache.hpp"

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    const char *ResourceCache::cache_cleared_signal = "cache_cleared";

    void ResourceCache::_bind_methods() {
        ClassDB::bind_method(D_METHOD("has", "path"), &ResourceCache::has);
        ClassDB::bind_method(D_METHOD("get", "path"), &ResourceCache::get);
        ClassDB::bind_method(D_METHOD("set", "path", "resource"), &ResourceCache::set);
        ClassDB::bind_method(D_METHOD("remove", "path"), &ResourceCache::remove);
        ClassDB::bind_method(D_METHOD("clear"), &ResourceCache::clear);
    }

    Ref<ResourceCache> ResourceCache::create(const String &p_cache_dir) {
        Ref<ResourceCache> cache;
        cache.instantiate();
        cache->set_cache_dir(p_cache_dir);
        return cache;
    }

    String ResourceCache::_get_cache_path(const String &p_path) const {
        return String(RESOURCE_CACHE_DIR_PREFIX).path_join(cache_dir).path_join(p_path.md5_text() + ".res");
    }

    bool ResourceCache::has(const String &p_path) const {
        return FileAccess::file_exists(_get_cache_path(p_path));
    }

    Ref<Resource> ResourceCache::get(const String &p_path) const {
        const String cache_path = _get_cache_path(p_path);

        if (FileAccess::file_exists(cache_path)) {
            UtilityFunctions::print_verbose(
                    "[ResourceCache] Loading from disk: " + cache_path + " (original: " + p_path + ")");
            Ref<Resource> res = ResourceLoader::get_singleton()->load(cache_path);
            if (res.is_valid()) {
                return res;
            }
        }
        UtilityFunctions::print_verbose("[ResourceCache] Cache miss: " + cache_path + " (original: " + p_path + ")");
        return nullptr;
    }

    void ResourceCache::set(const String &p_path, const Ref<Resource> &p_resource) const {
        if (p_resource.is_null()) {
            return;
        }

        const String cache_path = _get_cache_path(p_path);
        const String cache_dir = cache_path.get_base_dir();

        if (!DirAccess::dir_exists_absolute(cache_dir)) {
            DirAccess::make_dir_recursive_absolute(cache_dir);
        }

        const Error err = ResourceSaver::get_singleton()->save(p_resource, cache_path);
        if (err != OK) {
            UtilityFunctions::push_error("[ResourceCache] Failed to save cache: " + cache_path);
        } else {
            UtilityFunctions::print_verbose("[ResourceCache] Saved to disk: " + cache_path);
        }
    }

    void ResourceCache::remove(const String &p_path) const {
        const String cache_path = _get_cache_path(p_path);
        if (FileAccess::file_exists(cache_path)) {
            DirAccess::remove_absolute(cache_path);
        }
    }

    void ResourceCache::_clear_cache_dir(String p_path) const {
        if (DirAccess::dir_exists_absolute(p_path)) {
            const Ref<DirAccess> dir = DirAccess::open(p_path);
            if (dir.is_valid()) {
                UtilityFunctions::print_verbose("[ResourceCache] Clearing cache for " + p_path + "...");
                dir->list_dir_begin();
                String file_name = dir->get_next();
                while (file_name != "") {
                    if (!dir->current_is_dir()) {
                        dir->remove(file_name);
                    } else {
                        _clear_cache_dir(file_name);
                    }
                    file_name = dir->get_next();
                }
            }
        }
    }

    void ResourceCache::clear() {
        _clear_cache_dir(String(RESOURCE_CACHE_DIR_PREFIX).path_join(cache_dir));
        emit_cache_cleared_signal();
    }

    void ResourceCache::emit_cache_cleared_signal() {
        emit_signal(cache_cleared_signal);
    }

} // namespace godot
