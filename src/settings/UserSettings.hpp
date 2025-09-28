#pragma once
#include <godot_cpp/classes/config_file.hpp>
#include "macros.hpp"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/os.hpp>

namespace godot {
    class UserSettings : public Node {
        GDCLASS(UserSettings, Node);
        private:
            OS *os;
            ConfigFile *cfg;
            String _game_dir_section = "maszyna";
            String _game_dir_key = "game_dir";
            Dictionary _defaults = {};
            void _apply_defaults();
        protected:
            static void _bind_methods();
        public:
            static const char* CONFIG_CHANGED_SIGNAL;
            static const char* GAME_DIR_CHANGED_SIGNAL;
            static const char* SETTING_CHANGED_SIGNAL;
            static const char* CACHE_CLEAR_REQUESTED_SIGNAL;
            static const char* MODELS_RELOAD_REQUESTED_SIGNAL;
            static const char* CACHE_CLEARED_SIGNAL;
            UserSettings();
            ~UserSettings() override;
            String get_maszyna_game_dir() const;
            void load_config();
            void save_setting(const String& p_section, const String& p_key, const Variant& p_value);
        Variant get_setting(const String& p_section, const String& p_key, const Variant& p_default_value) const;
            void save_maszyna_game_dir(const String &p_path);

            MAKE_MEMBER_GS(String, config_file_path, "user://settings.cfg")

    };
} //namespace godot
