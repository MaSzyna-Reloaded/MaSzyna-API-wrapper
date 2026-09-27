#include "UserSettings.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    UserSettings::UserSettings() {
        config.instantiate();
        _setup_defaults();
        load_config();
    }

    void UserSettings::_bind_methods() {
        ClassDB::bind_method(D_METHOD("load_config"), &UserSettings::load_config);

        ClassDB::bind_method(D_METHOD("save_setting", "section", "key", "value"), &UserSettings::save_setting);

        ClassDB::bind_method(
                D_METHOD("get_setting", "section", "key", "default_value"), &UserSettings::get_setting,
                DEFVAL(Variant()));

        ClassDB::bind_method(D_METHOD("get_maszyna_game_dir"), &UserSettings::get_maszyna_game_dir);

        ClassDB::bind_method(D_METHOD("save_maszyna_game_dir", "path"), &UserSettings::save_maszyna_game_dir);

        ADD_SIGNAL(MethodInfo("config_changed"));
        ADD_SIGNAL(MethodInfo(
                "setting_changed", PropertyInfo(Variant::STRING, "section"), PropertyInfo(Variant::STRING, "key")));
        ADD_SIGNAL(MethodInfo("game_dir_changed"));
    }

    void UserSettings::_setup_defaults() {
        Dictionary e3d;
        e3d["auto_generate_normal"] = false;
        e3d["auto_generate_metallic"] = false;
        e3d["auto_generate_height"] = false;

        Dictionary maszyna;
        maszyna["game_dir"] = ".";

        Dictionary render;
        render["msaa_3d"] = RenderingServer::VIEWPORT_MSAA_DISABLED;
        render["anisotropic_filtering_level"] = RenderingServer::VIEWPORT_ANISOTROPY_DISABLED;
        render["use_taa"] = true;

        defaults["e3d"] = e3d;
        defaults["maszyna"] = maszyna;
        defaults["render"] = render;
    }

    void UserSettings::_apply_defaults() {
        Array sections = defaults.keys();

        for (int i = 0; i < sections.size(); i++) {
            String section = sections[i];
            Dictionary section_defaults = defaults[section];
            Array keys = section_defaults.keys();

            for (int j = 0; j < keys.size(); j++) {
                String key = keys[j];

                if (!config->has_section_key(section, key)) {
                    config->set_value(section, key, section_defaults[key]);
                }
            }
        }
    }

    void UserSettings::load_config() {
        ERR_FAIL_COND_MSG(config.is_null(), "UserSettings config is null.");

        _apply_defaults();

        Error err = config->load(config_file_path);

        if (err != OK) {
            Error save_err = config->save(config_file_path);
            ERR_FAIL_COND_MSG(save_err != OK, "Cannot save default user settings.");
        } else {
            // Po load() plik nadpisuje wartości w ConfigFile.
            // Defaults trzeba dołożyć drugi raz, ale tylko brakujące klucze.
            _apply_defaults();
        }

        emit_signal("config_changed");
    }

    void UserSettings::save_setting(const String &p_section, const String &p_key, const Variant &p_value) {
        ERR_FAIL_COND_MSG(config.is_null(), "UserSettings config is null.");

        Variant old_value = config->get_value(p_section, p_key, Variant());

        config->set_value(p_section, p_key, p_value);

        Error err = config->save(config_file_path);
        ERR_FAIL_COND_MSG(err != OK, "Cannot save user settings.");

        if (old_value != p_value) {
            emit_signal("setting_changed", p_section, p_key);
            emit_signal("config_changed");

            if (p_section == MASZYNA_GAMEDIR_SECTION && p_key == MASZYNA_GAMEDIR_KEY) {
                emit_signal("game_dir_changed");
            }
        }
    }

    Variant
    UserSettings::get_setting(const String &p_section, const String &p_key, const Variant &p_default_value) const {
        ERR_FAIL_COND_V_MSG(config.is_null(), p_default_value, "UserSettings config is null.");
        return config->get_value(p_section, p_key, p_default_value);
    }

    String UserSettings::get_maszyna_game_dir() const {
        OS *os = OS::get_singleton();
        ERR_FAIL_NULL_V(os, ".");

        const bool is_shipped = os->has_feature("release") && !os->has_feature("editor");
        String dir = is_shipped ? String(".") : String(get_setting(MASZYNA_GAMEDIR_SECTION, MASZYNA_GAMEDIR_KEY, "."));
        if (dir.is_empty()) {
            dir = ".";
        }

        /* Made absolute here, once, against the directory the game itself sits in - so a build
         * dropped anywhere still finds the data next to it, and "." keeps meaning what it meant.
         * It must not be left relative: a path with no prefix is read as res://, which in a
         * shipped build is the packed data and holds no game files, so every FileAccess check
         * over such a path answers "missing" (see FINDINGS.md, 2026-09-23). */
        if (dir.is_relative_path()) {
            dir = os->get_executable_path().get_base_dir().path_join(dir);
        }

        return dir.simplify_path();
    }

    void UserSettings::save_maszyna_game_dir(const String &p_path) {
        save_setting(MASZYNA_GAMEDIR_SECTION, MASZYNA_GAMEDIR_KEY, p_path);
    }

} // namespace godot
