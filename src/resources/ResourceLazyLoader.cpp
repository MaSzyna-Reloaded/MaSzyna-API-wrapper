#include "ResourceLazyLoader.hpp"

#include "game_data/GameDataServer.hpp"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    ResourceLazyLoader *ResourceLazyLoader::singleton = nullptr;

    ResourceLazyLoader *ResourceLazyLoader::get_instance() {
        return singleton;
    }

    /// What is read from the game's data is let go when the data is (GameDataServer)
    ResourceLazyLoader::ResourceLazyLoader() {
        singleton = this;
        GameDataServer *game_data = GameDataServer::get_instance();
        ERR_FAIL_NULL(game_data);
        game_data->connect(
                GameDataServer::data_unload_requested_signal,
                callable_mp(this, &ResourceLazyLoader::_on_data_unload_requested));
    }

    ResourceLazyLoader::~ResourceLazyLoader() {
        singleton = nullptr;
    }

    void ResourceLazyLoader::_bind_methods() {
        ClassDB::bind_method(D_METHOD("resource_register", "key", "loader"), &ResourceLazyLoader::resource_register);
        ClassDB::bind_method(D_METHOD("resource_free", "resource"), &ResourceLazyLoader::resource_free);
        ClassDB::bind_method(D_METHOD("resource_load", "resource"), &ResourceLazyLoader::resource_load);
        ClassDB::bind_method(D_METHOD("resource_fetch", "resource"), &ResourceLazyLoader::resource_fetch);
        ClassDB::bind_method(D_METHOD("resource_release", "resource"), &ResourceLazyLoader::resource_release);
        ClassDB::bind_method(D_METHOD("resource_is_resident", "resource"), &ResourceLazyLoader::resource_is_resident);
        ClassDB::bind_method(D_METHOD("resource_get_statistics"), &ResourceLazyLoader::resource_get_statistics);
    }

    /// The held resource, or the one loaded last if anything still keeps it alive. Under the mutex.
    Ref<Resource> ResourceLazyLoader::_get_loaded(const Entry &p_entry) const {
        if (p_entry.resource.is_valid()) {
            return p_entry.resource;
        }
        if (!p_entry.loaded.is_valid()) {
            return {};
        }
        // a resource whose last reference is being dropped right now is not taken again: the Ref
        // stays empty when its reference count has already reached zero
        return Ref<Resource>(Object::cast_to<Resource>(ObjectDB::get_instance(p_entry.loaded)));
    }

    /// The new data is loaded anew; whoever holds a resource keeps holding it and gets the new one
    /// with its next fetch
    void ResourceLazyLoader::_on_data_unload_requested() {
        Vector<Ref<Resource>> dropped; // freed once the mutex is let go
        MutexLock lock(mutex);
        for (KeyValue<RID, Entry> &entry: entries) {
            dropped.push_back(entry.value.resource);
            entry.value.resource.unref();
            entry.value.loaded = ObjectID();
        }
    }

    RID ResourceLazyLoader::resource_register(const String &p_key, const Callable &p_loader) {
        MutexLock lock(mutex);
        if (const RID *found = keys.getptr(p_key); found != nullptr) {
            entries[*found].registrations++;
            return *found;
        }
        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        Entry &entry = entries[rid];
        entry.key = p_key;
        entry.loader = p_loader;
        entry.registrations = 1;
        keys[p_key] = rid;
        return rid;
    }

    void ResourceLazyLoader::resource_free(const RID &p_resource) {
        Ref<Resource> dropped; // freed once the mutex is let go
        MutexLock lock(mutex);
        Entry *entry = entries.getptr(p_resource);
        ERR_FAIL_NULL(entry);
        entry->registrations--;
        if (entry->registrations > 0) {
            return;
        }
        dropped = entry->resource;
        keys.erase(entry->key);
        entries.erase(p_resource);
    }

    /// Two threads loading one resource at once both call the loader; the first result is kept, so
    /// both hand out the same resource
    Ref<Resource> ResourceLazyLoader::resource_load(const RID &p_resource) {
        Callable loader;
        {
            MutexLock lock(mutex);
            const Entry *entry = entries.getptr(p_resource);
            ERR_FAIL_NULL_V(entry, Ref<Resource>());
            if (const Ref<Resource> loaded = _get_loaded(*entry); loaded.is_valid()) {
                return loaded;
            }
            loader = entry->loader;
        }
        Ref<Resource> resource = loader.call();
        MutexLock lock(mutex);
        Entry *entry = entries.getptr(p_resource);
        if (entry == nullptr) {
            return resource; // freed while it loaded
        }
        if (const Ref<Resource> loaded = _get_loaded(*entry); loaded.is_valid()) {
            return loaded;
        }
        load_count++;
        if (resource.is_valid()) {
            entry->loaded = ObjectID(resource->get_instance_id());
            if (entry->holders > 0) {
                entry->resource = resource;
            }
        }
        return resource;
    }

    /// Nothing is held when the resource cannot be loaded
    Ref<Resource> ResourceLazyLoader::resource_fetch(const RID &p_resource) {
        Ref<Resource> resource = resource_load(p_resource);
        if (resource.is_null()) {
            return resource;
        }
        MutexLock lock(mutex);
        Entry *entry = entries.getptr(p_resource);
        ERR_FAIL_NULL_V(entry, resource);
        entry->holders++;
        entry->resource = resource;
        return resource;
    }

    void ResourceLazyLoader::resource_release(const RID &p_resource) {
        Ref<Resource> dropped; // freed once the mutex is let go
        MutexLock lock(mutex);
        Entry *entry = entries.getptr(p_resource);
        ERR_FAIL_NULL(entry);
        ERR_FAIL_COND_MSG(entry->holders <= 0, "Released more times than fetched: " + entry->key);
        entry->holders--;
        if (entry->holders == 0) {
            dropped = entry->resource;
            entry->resource.unref();
        }
    }

    bool ResourceLazyLoader::resource_is_resident(const RID &p_resource) const {
        MutexLock lock(mutex);
        const Entry *entry = entries.getptr(p_resource);
        return entry != nullptr && entry->resource.is_valid();
    }

    Dictionary ResourceLazyLoader::resource_get_statistics() const {
        MutexLock lock(mutex);
        int resident = 0;
        for (const KeyValue<RID, Entry> &entry: entries) {
            resident += entry.value.resource.is_valid() ? 1 : 0;
        }
        Dictionary statistics;
        statistics["registered"] = entries.size();
        statistics["resident"] = resident;
        statistics["loads"] = load_count;
        return statistics;
    }
} // namespace godot
