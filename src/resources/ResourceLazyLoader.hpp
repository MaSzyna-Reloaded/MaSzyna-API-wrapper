#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/mutex.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {
    /// Resources needed only while something near the camera uses them - a scenery's models, its
    /// terrain - loaded when they are wanted and let go when nobody wants them any more, so their
    /// memory, RAM and the VRAM of a mesh or a texture, goes back.
    ///
    /// A resource is registered under a key with the Callable that loads it; one key is one
    /// resource, shared by everybody who registers it. resource_fetch() holds it, resource_release()
    /// lets it go, and once nobody holds it the loader keeps no reference of its own. A copy still
    /// alive elsewhere (a resource loaded on a worker on its way to be built) is handed out again
    /// rather than loaded twice.
    ///
    /// Thread-safe: the streaming loads on its worker thread (resource_load()) and builds on the
    /// main thread (resource_fetch()).
    class ResourceLazyLoader : public Object {
            GDCLASS(ResourceLazyLoader, Object)

        private:
            struct Entry {
                    String key;
                    Callable loader; // () -> Resource
                    int registrations = 0;
                    int holders = 0;
                    Ref<Resource> resource; // while held
                    ObjectID loaded;        // the last one loaded, while anything keeps it alive
            };

            static ResourceLazyLoader *singleton;

            mutable Mutex mutex;
            HashMap<RID, Entry> entries;
            HashMap<String, RID> keys;
            int load_count = 0;

            Ref<Resource> _get_loaded(const Entry &p_entry) const;
            void _on_data_unload_requested();

        protected:
            static void _bind_methods();

        public:
            static ResourceLazyLoader *get_instance();

            ResourceLazyLoader();
            ~ResourceLazyLoader() override;

            /* The same key gives the same RID; each registration is freed by its own resource_free() */
            RID resource_register(const String &p_key, const Callable &p_loader);
            void resource_free(const RID &p_resource);
            /* Loads the resource unless it is in memory, without holding it */
            Ref<Resource> resource_load(const RID &p_resource);
            /* resource_load(), and holds the resource until the matching resource_release() */
            Ref<Resource> resource_fetch(const RID &p_resource);
            void resource_release(const RID &p_resource);
            /* Somebody holds the resource */
            bool resource_is_resident(const RID &p_resource) const;
            /* registered, resident, loads - for a debug view */
            Dictionary resource_get_statistics() const;
    };
} // namespace godot
