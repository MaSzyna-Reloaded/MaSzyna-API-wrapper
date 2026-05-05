#pragma once

#include "E3DModel.hpp"
#include "core/ResourceCache.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

    class E3DModelManager : public Object {
            GDCLASS(E3DModelManager, Object)

        private:
            Ref<ResourceCache> cache;

            String _make_cache_path(const String &p_data_path, const String &p_filename) const;
            String _make_cache_hash(const String &p_source_abs_path) const;

        protected:
            static void _bind_methods();

        public:
            E3DModelManager();

            static E3DModelManager *get_instance() {
                return dynamic_cast<E3DModelManager *>(Engine::get_singleton()->get_singleton("E3DModelManager"));
            }

            void clear_cache() const;
            Ref<E3DModel> load_model(const String &p_data_path, const String &p_filename) const;
    };

} // namespace godot
