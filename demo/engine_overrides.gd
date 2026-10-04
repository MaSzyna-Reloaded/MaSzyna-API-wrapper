extends Node

## The engine's settings the player sets that the engine reads only at its own start, before the game
## can set anything - the file logging: kept in override.cfg, which Godot reads then (beside
## project.godot, beside the executable of a release; application/config/
## disable_project_settings_override is off). The only writer of that file. The player's value lives
## in UserSettings like any other, so Save and Discard changes hold for it; the file holds what
## differs from the project's own and is gone when nothing is left in it.

## The player's setting (settings.json, Debug)
const SETTINGS_SECTION: String = "debug"
const FILE_LOGGING_KEY: String = "file_logging"
## Logging is the project's own (project.godot): to the file, the prints and the errors of a release
## with it - which the file has nothing of otherwise - each written out at once, a line whole as
## it comes (flush_stdout_on_print). Switched off, all of it goes quiet.
const FILE_LOGGING_DEFAULT: bool = true
const LOGGING_OFF: Dictionary[String, bool] = {
    "debug/file_logging/enable_file_logging": false,
    "application/run/disable_stdout.release": true,
    "application/run/disable_stderr.release": true,
}
const OVERRIDE_FILE: String = "override.cfg"


func _ready() -> void:
    UserSettings.config_changed.connect(_write_overrides)
    _write_overrides()


func _write_overrides() -> void:
    var logging: bool = bool(
        UserSettings.get_setting(SETTINGS_SECTION, FILE_LOGGING_KEY, FILE_LOGGING_DEFAULT)
    )
    # beside the executable of a release, beside project.godot otherwise
    var directory: String = (
        OS.get_executable_path().get_base_dir() if OS.has_feature("template")
        else ProjectSettings.globalize_path("res://")
    )
    var path: String = directory.path_join(OVERRIDE_FILE)
    var overrides: ConfigFile = ConfigFile.new()
    overrides.load(path)
    var changed: bool = false
    for setting: String in LOGGING_OFF:
        var section: String = setting.get_slice("/", 0)
        var key: String = setting.trim_prefix(section + "/")
        var stored: bool = overrides.has_section_key(section, key)
        if logging and stored:
            overrides.erase_section_key(section, key)
            changed = true
        elif not logging and not (stored and overrides.get_value(section, key) == LOGGING_OFF[setting]):
            overrides.set_value(section, key, LOGGING_OFF[setting])
            changed = true
    if not changed:
        return
    if overrides.get_sections():
        overrides.save(path)
    else:
        DirAccess.remove_absolute(path)
