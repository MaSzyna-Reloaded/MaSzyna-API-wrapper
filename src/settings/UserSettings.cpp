#include "UserSettings.hpp"
#include <godot_cpp/classes/rendering_server.hpp>

namespace godot {
    const char *UserSettings::CONFIG_CHANGED_SIGNAL = "config_changed";
    const char *UserSettings::GAME_DIR_CHANGED_SIGNAL = "game_dir_changed";
    const char *UserSettings::SETTING_CHANGED_SIGNAL = "setting_changed";
    const char *UserSettings::CACHE_CLEAR_REQUESTED_SIGNAL = "cache_clear_requested";
    const char *UserSettings::MODELS_RELOAD_REQUESTED_SIGNAL = "models_reload_requested";
    const char *UserSettings::CACHE_CLEARED_SIGNAL = "cache_cleared";

    void UserSettings::_apply_defaults() {
        Array sections = _defaults.keys();
        for (int i = 0; i < sections.size(); i++) {
            String section = sections[i];
            Dictionary cfg_dict = _defaults[section];
            Array keys = cfg_dict.keys();
            for (int j = 0; j < keys.size(); j++) {
                String key = keys[j];
                Variant value = cfg_dict[key];
                cfg->set_value(section, key, value);
            }
        }
    }

    void UserSettings::_bind_methods() {
        ClassDB::bind_method(D_METHOD("load_config"), &UserSettings::load_config);
        ClassDB::bind_method(D_METHOD("get_maszyna_game_dir"), &UserSettings::get_maszyna_game_dir);
        ClassDB::bind_method(D_METHOD("save_setting", "section", "key", "value"), &UserSettings::save_setting);
        ClassDB::bind_method(D_METHOD("get_setting", "section", "key", "default_value"), &UserSettings::get_setting);
        ClassDB::bind_method(D_METHOD("save_maszyna_game_dir", "path"), &UserSettings::save_maszyna_game_dir);
        ADD_SIGNAL(MethodInfo(CONFIG_CHANGED_SIGNAL));
        ADD_SIGNAL(MethodInfo(GAME_DIR_CHANGED_SIGNAL));
        ADD_SIGNAL(MethodInfo(SETTING_CHANGED_SIGNAL, PropertyInfo(Variant::STRING, "section"), PropertyInfo(Variant::STRING, "key")));
        ADD_SIGNAL(MethodInfo(CACHE_CLEARED_SIGNAL));
        ADD_SIGNAL(MethodInfo(CACHE_CLEAR_REQUESTED_SIGNAL));
        ADD_SIGNAL(MethodInfo(MODELS_RELOAD_REQUESTED_SIGNAL));
    }

    UserSettings::UserSettings()
        {
        cfg = memnew(ConfigFile);
        os = OS::get_singleton();

        // Initialize defaults dictionary
        {
            Dictionary maszyna;
            maszyna["game_dir"] = ".";

            Dictionary render;
            render["msaa_3d"] = RenderingServer::VIEWPORT_MSAA_4X;
            render["anisotropic_filtering_level"] = RenderingServer::VIEWPORT_ANISOTROPY_4X;

            _defaults["maszyna"] = maszyna;
            _defaults["render"] = render;
        }
    }

    UserSettings::~UserSettings() {
        // 'os' is owned by the engine; do not delete.
        memdelete(cfg);
    }

    String UserSettings::get_maszyna_game_dir() const {
        // In exported builds outside the editor, always use the current directory
        if (os->has_feature("release") && !os->has_feature("editor")) {
            return ".";
        }
        if (String dir = cfg->get_value(_game_dir_section, _game_dir_key); dir.length() > 0) {
            return dir;
        }
        return ".";
    }

    void UserSettings::load_config() {
        _apply_defaults();
        if (cfg->load(config_file_path) != OK) {
            cfg->save(config_file_path);
        }
        emit_signal(CONFIG_CHANGED_SIGNAL);
    }

    void UserSettings::save_setting(const String &p_section, const String &p_key, const Variant &p_value) {
        const Variant old_value = cfg->get_value(p_section, p_key);
        cfg->set_value(p_section, p_key, old_value);
        cfg->save(config_file_path);
        if (p_value != old_value) {
            emit_signal(SETTING_CHANGED_SIGNAL, p_section, p_key);
            emit_signal(CONFIG_CHANGED_SIGNAL);

            if (p_section == _game_dir_section && p_key == _game_dir_key) {
                emit_signal(GAME_DIR_CHANGED_SIGNAL);
            }
        }
    }

    Variant UserSettings::get_setting(const String &p_section, const String &p_key, const Variant &p_default_value) const {
        return cfg->get_value(p_section, p_key, p_default_value);
    }

    void UserSettings::save_maszyna_game_dir(const String &p_path) {
        save_setting(_game_dir_section, _game_dir_key, p_path);
    }
}
