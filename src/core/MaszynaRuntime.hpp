#pragma once

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

    /// Runtime-wide services shared by every cache: the single "throw the caches away" entry point
    /// and the build stamp those caches are keyed against.
    class MaszynaRuntime : public Object {
            GDCLASS(MaszynaRuntime, Object)

        private:
            static constexpr const char *BUILD_NUMBER_PATH = "res://build_number.txt";
            static constexpr const char *BUILD_SECTION = "app";
            static constexpr const char *BUILD_NUMBER_KEY = "build_number";

            String build_number;
            bool build_number_read = false;
            bool build_version_checked = false;

        protected:
            static void _bind_methods();

        public:
            static const char *cache_clear_requested_signal;

            static MaszynaRuntime *get_instance() {
                return dynamic_cast<MaszynaRuntime *>(Engine::get_singleton()->get_singleton("MaszynaRuntime"));
            }

            void clear_cache();
            String get_build_number();
            bool check_build_version();
    };

} // namespace godot
