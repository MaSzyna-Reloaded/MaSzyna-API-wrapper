extends Node

## The game's window, for as long as the game runs, and the only one that changes it: it takes the
## player's Display settings - the mode, the windowed size, V-Sync, HDR output - whenever they
## change; Alt+Enter switches between fullscreen and a window by changing the mode setting, so the
## settings and the key never tell two stories; and a fullscreen window moved to another screen
## fills that screen. Windows has no fullscreen state of its own - Win+Shift+arrow moves the
## fullscreen game as an ordinary window and leaves it the size of the screen it came from.

## The Display section's window mode (settings.json "mode")
enum Mode {
    WINDOWED,
    FULLSCREEN,
}

## UserSettings section of the Display settings
const SETTINGS_SECTION: String = "window"

## Screen the window stood on when it last moved
var _screen: int = -1
## The windowed size last given to the window: it is given again only when the setting changes, so
## a window the player resized stays so while other settings change
var _windowed_size: Vector2i = Vector2i.ZERO


func _ready() -> void:
    _screen = get_window().current_screen
    UserSettings.config_changed.connect(_apply_settings)
    _apply_settings()


## In _input, before the GUI: a search field would take Enter for itself
func _input(event: InputEvent) -> void:
    if not event.is_action_pressed("toggle_fullscreen", false, true):
        return
    var mode: Mode = UserSettings.get_setting(SETTINGS_SECTION, "mode", Mode.FULLSCREEN)
    UserSettings.set_setting(
        SETTINGS_SECTION, "mode", Mode.WINDOWED if mode == Mode.FULLSCREEN else Mode.FULLSCREEN
    )
    get_viewport().set_input_as_handled()


## The window tells every node in it that it moved (window.cpp, _rect_changed_callback)
func _notification(what: int) -> void:
    if not what == NOTIFICATION_WM_POSITION_CHANGED:
        return
    var window: Window = get_window()
    if window.current_screen == _screen:
        return
    _screen = window.current_screen
    if not window.mode == Window.MODE_FULLSCREEN:
        return
    # fullscreen is taken again where the window is now, at the size of that screen
    window.mode = Window.MODE_WINDOWED
    window.mode = Window.MODE_FULLSCREEN


## The Display settings onto the window - only what differs from what it has, so a change of
## another setting leaves the window be
func _apply_settings() -> void:
    var window: Window = get_window()
    var mode: Mode = UserSettings.get_setting(SETTINGS_SECTION, "mode", Mode.FULLSCREEN)
    if mode == Mode.FULLSCREEN:
        if not window.mode == Window.MODE_FULLSCREEN:
            window.mode = Window.MODE_FULLSCREEN
        # back in a window, the window takes its size again
        _windowed_size = Vector2i.ZERO
    else:
        if not window.mode == Window.MODE_WINDOWED:
            window.mode = Window.MODE_WINDOWED
        # no size stored (the default, ZERO): the screen's own
        var size: Variant = UserSettings.get_setting(SETTINGS_SECTION, "resolution", Vector2i.ZERO)
        var windowed: Vector2i = (
            size if size is Vector2i and not size == Vector2i.ZERO
            else DisplayServer.screen_get_size(window.current_screen)
        )
        if not windowed == _windowed_size:
            _windowed_size = windowed
            window.size = windowed
            window.move_to_center()
    DisplayServer.window_set_vsync_mode(
        DisplayServer.VSYNC_ENABLED
        if UserSettings.get_setting(SETTINGS_SECTION, "vsync_enabled", true)
        else DisplayServer.VSYNC_DISABLED
    )
    # asked of a display that has it - the engine warns of a request it cannot meet
    var hdr: bool = (
        bool(UserSettings.get_setting(SETTINGS_SECTION, "hdr_output", false))
        and DisplayServer.window_is_hdr_output_supported(window.get_window_id())
    )
    if not window.hdr_output_requested == hdr:
        window.hdr_output_requested = hdr
        # what reads the screen behind it (the HUD tiles' contrast) reads it linear then
        RenderingServer.global_shader_parameter_set(&"maszyna_hdr_output", hdr)
