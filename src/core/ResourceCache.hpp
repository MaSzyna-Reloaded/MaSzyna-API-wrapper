#pragma once

#include "macros.hpp"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

#ifndef RESOURCE_CACHE_DIR_PREFIX
#define RESOURCE_CACHE_DIR_PREFIX "user://cache/"
#endif


namespace godot {

    class ResourceCache : public RefCounted {
            GDCLASS(ResourceCache, RefCounted)

        protected:
            static void _bind_methods();
            String _get_cache_path(const String &p_path) const;
            void _clear_cache_dir(String p_path) const;

        public:
            static const char *cache_cleared_signal;
            static Ref<ResourceCache> create(const String &p_cache_dir);

            MAKE_MEMBER_GS(String, cache_dir, "");

            bool has(const String &p_path) const;
            Ref<Resource> get(const String &p_path) const;
            void set(const String &p_path, const Ref<Resource> &p_resource) const;
            void remove(const String &p_path) const;
            void clear();
            void emit_cache_cleared_signal();
    };
} // namespace godot
