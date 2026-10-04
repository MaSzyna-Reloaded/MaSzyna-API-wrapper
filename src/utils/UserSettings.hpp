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
            /// The project's own value of each project setting the player's value replaced
            Dictionary project_values;

            static constexpr const char *MASZYNA_GAMEDIR_SECTION = "maszyna";
            static constexpr const char *MASZYNA_GAMEDIR_KEY = "game_dir";
            /// Section of values set onto ProjectSettings, each under its full name
            /// ("maszyna/scenery/draw_distance")
            static constexpr const char *PROJECT_SETTINGS_SECTION = "project_settings";

            void _apply_defaults();
            void _set_project_setting(const String &p_name, const Variant &p_value);

        protected:
            static void _bind_methods();

        public:
            UserSettings();

            static UserSettings *get_instance() {
                return dynamic_cast<UserSettings *>(Engine::get_singleton()->get_singleton("UserSettings"));
            }

            void load_config();

            /// Set in memory and in effect at once; saved by save_config(), dropped by load_config()
            void set_setting(const String &p_section, const String &p_key, const Variant &p_value);
            void save_config();
            /// The player's value goes, in memory like set_setting(): a key with a default takes the
            /// default, a project setting the project's own value
            void erase_setting(const String &p_section, const String &p_key);
            void save_setting(const String &p_section, const String &p_key, const Variant &p_value);
            Variant
            get_setting(const String &p_section, const String &p_key, const Variant &p_default_value = Variant()) const;

            String get_maszyna_game_dir() const;
            void save_maszyna_game_dir(const String &p_path);
    };

} // namespace godot
