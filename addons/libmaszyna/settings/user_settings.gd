@tool
extends Node

signal config_changed
signal game_dir_changed
signal setting_changed(section, key)
signal cache_clear_requested  # probably should be somewhere else?
signal models_reload_requested  # probably should be somewhere else?
signal cache_cleared

var config_file_path = "user://settings.cfg"
var config = ConfigFile.new()
var DEFAULTS = {
    "maszyna": {
        "game_dir": ".",
    },
    "render": {
        "msaa_3d": RenderingServer.VIEWPORT_MSAA_4X,
        "anisotropic_filtering_level": RenderingServer.VIEWPORT_ANISOTROPY_4X
    }
}

const MASZYNA_GAMEDIR_SECTION = "maszyna"
const MASZYNA_GAMEDIR_KEY = "game_dir"

func _ready():
    load_config()

func _apply_defaults():
    for section in DEFAULTS:
        var cfg = DEFAULTS[section]
        for key in cfg:
            config.set_value(section, key, cfg[key])

func load_config():
    _apply_defaults()
    if config.load(config_file_path) != OK:
        config.save(config_file_path)
    config_changed.emit()

func save_setting(section: String, key: String, value):
    var old_value = config.get_value(section, key)
    config.set_value(section, key, value)
    config.save(config_file_path)
    if not old_value == value:
        setting_changed.emit(section, key)
        config_changed.emit()
        if section == MASZYNA_GAMEDIR_SECTION and key == MASZYNA_GAMEDIR_KEY:
            game_dir_changed.emit()

func get_setting(section: String, key: String, default_value = null):
    return config.get_value(section, key, default_value)

func get_maszyna_game_dir():
    if OS.has_feature("release") and not OS.has_feature("editor"):
        return "."
    else:
        var dir = config.get_value(MASZYNA_GAMEDIR_SECTION, MASZYNA_GAMEDIR_KEY)
        if not dir:
            return "."
        else:
            return dir

func save_maszyna_game_dir(path):
    save_setting(MASZYNA_GAMEDIR_SECTION, MASZYNA_GAMEDIR_KEY, path)
