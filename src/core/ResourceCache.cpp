#include "ResourceCache.hpp"
#include "models/e3d/E3DModel.hpp"

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    ResourceCache *ResourceCache::singleton = nullptr;

    void ResourceCache::_bind_methods() {
        ClassDB::bind_static_method(get_class_static(), D_METHOD("has", "path", "dir"), &ResourceCache::has);
        ClassDB::bind_static_method(get_class_static(), D_METHOD("get", "path", "dir"), &ResourceCache::get);
        ClassDB::bind_static_method(get_class_static(), D_METHOD("set", "path", "resource", "dir"), &ResourceCache::set);
        ClassDB::bind_static_method(get_class_static(), D_METHOD("remove", "path", "dir"), &ResourceCache::remove);
        ClassDB::bind_static_method(get_class_static(), D_METHOD("clear", "dir"), &ResourceCache::clear);
        ClassDB::bind_static_method(get_class_static(), D_METHOD("clear_all"), &ResourceCache::clear_all);

        BIND_ENUM_CONSTANT(RESOURCE_CACHE_DIR_MODELS);
        BIND_ENUM_CONSTANT(RESOURCE_CACHE_DIR_TEXTURES);
        BIND_ENUM_CONSTANT(RESOURCE_CACHE_DIR_SOUNDS);
    }

    ResourceCache::ResourceCache() {
        singleton = this;
        _mutex.instantiate();
    }

    ResourceCache::~ResourceCache() {
        if (singleton == this) {
            singleton = nullptr;
        }
        Array keys = _cache.keys();
        for (int i = 0; i < keys.size(); i++) {
            Ref<Resource> res = _cache[keys.get(i)];
            if (res.is_valid()) {
                if (E3DModel *model = cast_to<E3DModel>(res.ptr())) {
                    model->clear();
                }
            }
        }
        _cache.clear();
        _mutex.unref();
    }

    String ResourceCache::_get_dir_name(const ResourceCacheDir p_dir) {
        switch (p_dir) {
            case RESOURCE_CACHE_DIR_MODELS:
                return "models";
            case RESOURCE_CACHE_DIR_TEXTURES:
                return "textures";
            case RESOURCE_CACHE_DIR_SOUNDS:
                return "sounds";
            default:
                return "misc";
        }
    }

    String ResourceCache::_get_cache_path(const String &p_path, const ResourceCacheDir p_dir) {
        String cache_dir = "user://cache/" + _get_dir_name(p_dir);
        return cache_dir + "/" + p_path.md5_text() + ".res";
    }

    bool ResourceCache::has(const String &p_path, const ResourceCacheDir p_dir) {
        if (singleton == nullptr) {
            return false;
        }

        singleton->_mutex->lock();
        if (singleton->_cache.has(p_path)) {
            singleton->_mutex->unlock();
            return true;
        }
        singleton->_mutex->unlock();

        return FileAccess::file_exists(_get_cache_path(p_path, p_dir));
    }

    Ref<Resource> ResourceCache::get(const String &p_path, const ResourceCacheDir p_dir) {
        if (singleton == nullptr) {
            return Ref<Resource>();
        }

        singleton->_mutex->lock();
        if (singleton->_cache.has(p_path)) {
            Ref<Resource> res = singleton->_cache.get(p_path, Ref<Resource>());
            singleton->_mutex->unlock();
            return res;
        }
        singleton->_mutex->unlock();

        const String cache_path = _get_cache_path(p_path, p_dir);
        if (FileAccess::file_exists(cache_path)) {
            UtilityFunctions::print_verbose(
                    "[ResourceCache] Loading from disk: " + cache_path + " (original: " + p_path + ")");
            Ref<Resource> res = ResourceLoader::get_singleton()->load(cache_path);
            if (res.is_valid()) {
                singleton->_mutex->lock();
                singleton->_cache.set(p_path, res);
                singleton->_mutex->unlock();
                return res;
            }
        }

        return Ref<Resource>();
    }

    void ResourceCache::set(const String &p_path, const Ref<Resource> &p_resource, const ResourceCacheDir p_dir) {
        if (singleton == nullptr || p_resource.is_null()) {
            return;
        }

        singleton->_mutex->lock();
        singleton->_cache.set(p_path, p_resource);
        singleton->_mutex->unlock();

        const String cache_path = _get_cache_path(p_path, p_dir);
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

    void ResourceCache::remove(const String &p_path, const ResourceCacheDir p_dir) {
        if (singleton == nullptr) {
            return;
        }

        singleton->_mutex->lock();
        singleton->_cache.erase(p_path);
        singleton->_mutex->unlock();

        const String cache_path = _get_cache_path(p_path, p_dir);
        if (FileAccess::file_exists(cache_path)) {
            DirAccess::remove_absolute(cache_path);
        }
    }

    void ResourceCache::clear(const ResourceCacheDir p_dir) {
        if (singleton == nullptr) {
            return;
        }

        singleton->_mutex->lock();
        singleton->_cache.clear();
        singleton->_mutex->unlock();

        const String _dir_name = _get_dir_name(p_dir);
        const String cache_dir = "user://cache/" + _dir_name;
        if (DirAccess::dir_exists_absolute(cache_dir)) {
            const Ref<DirAccess> dir = DirAccess::open(cache_dir);
            if (dir.is_valid()) {
                UtilityFunctions::print_verbose("[ResourceCache] Clearing cache for " + _dir_name + "...");
                dir->list_dir_begin();
                String file_name = dir->get_next();
                while (file_name != "") {
                    if (!dir->current_is_dir()) {
                        dir->remove(file_name);
                    }
                    file_name = dir->get_next();
                }
            }
        }
    }

    void ResourceCache::clear_all() {
        if (singleton == nullptr) {
            return;
        }

        singleton->_mutex->lock();
        singleton->_cache.clear();
        singleton->_mutex->unlock();

        if (DirAccess::dir_exists_absolute("user://cache")) {
            clear(RESOURCE_CACHE_DIR_MODELS);
            clear(RESOURCE_CACHE_DIR_TEXTURES);
            clear(RESOURCE_CACHE_DIR_SOUNDS);
        }
    }

    ResourceCache *ResourceCache::get_singleton() {
        return singleton;
    }

} // namespace godot
