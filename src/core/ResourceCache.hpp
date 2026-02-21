#pragma once

#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class ResourceCache : public Object {
    GDCLASS(ResourceCache, Object)

public:
    enum ResourceCacheDir { 
        RESOURCE_CACHE_DIR_MODELS, 
        RESOURCE_CACHE_DIR_TEXTURES, 
        RESOURCE_CACHE_DIR_SOUNDS 
    };

private:
    static ResourceCache *singleton;
    Dictionary _cache;
    Ref<Mutex> _mutex;

    static String _get_dir_name(ResourceCacheDir p_dir);
    static String _get_cache_path(const String &p_path, ResourceCacheDir p_dir);

protected:
    static void _bind_methods();

public:
    ResourceCache();
    ~ResourceCache() override;

    static bool has(const String &p_path, ResourceCacheDir p_dir);
    static Ref<Resource> get(const String &p_path, ResourceCacheDir p_dir);
    static void set(const String &p_path, const Ref<Resource> &p_resource, ResourceCacheDir p_dir);
    static void remove(const String &p_path, ResourceCacheDir p_dir);
    static void clear(ResourceCacheDir p_dir);
    static void clear_all();

    static ResourceCache *get_singleton();
};

} // namespace godot

VARIANT_ENUM_CAST(ResourceCache::ResourceCacheDir);
