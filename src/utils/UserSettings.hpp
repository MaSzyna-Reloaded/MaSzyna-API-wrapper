#pragma once

#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

    class UserSettings : public Object {
            GDCLASS(UserSettings, Object)

        private:
            String config_file_path = "user://settings.cfg";
            Ref<ConfigFile> config;
            Dictionary defaults;

            static constexpr const char *MASZYNA_GAMEDIR_SECTION = "maszyna";
            static constexpr const char *MASZYNA_GAMEDIR_KEY = "game_dir";

            void _setup_defaults();
            void _apply_defaults();

        protected:
            static void _bind_methods();

        public:
            UserSettings();

            static UserSettings *get_instance() {
                return dynamic_cast<UserSettings *>(Engine::get_singleton()->get_singleton("UserSettings"));
            }

            void load_config();

            void save_setting(const String &p_section, const String &p_key, const Variant &p_value);
            Variant
            get_setting(const String &p_section, const String &p_key, const Variant &p_default_value = Variant()) const;

            String get_maszyna_game_dir() const;
            void save_maszyna_game_dir(const String &p_path);
    };

} // namespace godot
