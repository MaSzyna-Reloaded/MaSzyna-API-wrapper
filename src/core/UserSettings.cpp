#include "UserSettings.hpp"

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>

namespace godot {

    namespace {
        constexpr const char *GRAPHICS_BACKEND_VULKAN = "vulkan";
        constexpr const char *GRAPHICS_BACKEND_OPENGL3 = "opengl3";
        constexpr const char *GRAPHICS_BACKEND_D3D12 = "d3d12";
        constexpr const char *GRAPHICS_BACKEND_METAL = "metal";
    } // namespace

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

        ClassDB::bind_method(D_METHOD("get_current_graphics_backend"), &UserSettings::get_current_graphics_backend);
        ClassDB::bind_method(D_METHOD("save_graphics_backend", "backend"), &UserSettings::save_graphics_backend);

        ADD_SIGNAL(MethodInfo("config_changed"));
        ADD_SIGNAL(MethodInfo(
                "setting_changed", PropertyInfo(Variant::STRING, "section"), PropertyInfo(Variant::STRING, "key")));
        ADD_SIGNAL(MethodInfo("game_dir_changed"));
        ADD_SIGNAL(MethodInfo("graphics_backend_changed"));
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

        const Error err = config->load(config_file_path);

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

        const Variant old_value = config->get_value(p_section, p_key, Variant());

        config->set_value(p_section, p_key, p_value);

        const Error err = config->save(config_file_path);
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
        const OS *os = OS::get_singleton();
        ERR_FAIL_NULL_V(os, ".");

        if (os->has_feature("release") && !os->has_feature("editor")) {
            return ".";
        }

        const Variant value = get_setting(MASZYNA_GAMEDIR_SECTION, MASZYNA_GAMEDIR_KEY, ".");

        String dir = String(value);
        if (dir.is_empty()) {
            return ".";
        }

        return dir;
    }

    void UserSettings::save_maszyna_game_dir(const String &p_path) {
        save_setting(MASZYNA_GAMEDIR_SECTION, MASZYNA_GAMEDIR_KEY, p_path);
    }

    String UserSettings::get_current_graphics_backend() const {
        if (RenderingServer::get_singleton()->get_rendering_device() == nullptr) {
            return GRAPHICS_BACKEND_OPENGL3;
        }

        return RenderingServer::get_singleton()->get_current_rendering_driver_name();
    }

    String UserSettings::_get_override_cfg_path() {
        const OS *os = OS::get_singleton();
        ERR_FAIL_NULL_V(os, "user://override.cfg");

        // Godot only reads override.cfg from beside the running executable (or the
        // project root when running from source/editor); it is not part of user://,
        // and it must exist before the renderer initializes at boot, well before
        // this singleton (or any script) runs.
        if (os->has_feature("editor")) {
            return ProjectSettings::get_singleton()->globalize_path("res://override.cfg");
        }

        return os->get_executable_path().get_base_dir().path_join("override.cfg");
    }

    void UserSettings::save_graphics_backend(const String &p_backend) {
        ERR_FAIL_COND_MSG(
                p_backend != GRAPHICS_BACKEND_VULKAN && p_backend != GRAPHICS_BACKEND_OPENGL3 &&
                        p_backend != GRAPHICS_BACKEND_D3D12 && p_backend != GRAPHICS_BACKEND_METAL,
                "Unknown graphics backend: " + p_backend);

        const String path = _get_override_cfg_path();

        Ref<ConfigFile> override_cfg;
        override_cfg.instantiate();
        // Ignore the load error: the file may not exist yet, in which case we
        // simply start from an empty config and create it.
        override_cfg->load(path);

        if (p_backend == GRAPHICS_BACKEND_OPENGL3) {
            override_cfg->set_value("rendering", "renderer/rendering_method", "gl_compatibility");
            if (override_cfg->has_section_key("rendering", "rendering_device/driver")) {
                override_cfg->erase_section_key("rendering", "rendering_device/driver");
            }
        } else {
            override_cfg->set_value("rendering", "renderer/rendering_method", "forward_plus");
            override_cfg->set_value("rendering", "rendering_device/driver", p_backend);
        }

        const Error err = override_cfg->save(path);
        ERR_FAIL_COND_MSG(err != OK, "Cannot save graphics backend override to '" + path + "'.");

        emit_signal("graphics_backend_changed");
    }

} // namespace godot
