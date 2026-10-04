extends MaszynaGutTest

## The game's window (game_window.gd): it opens fullscreen on the primary screen, takes the mode of
## the player's Display settings, Alt+Enter and nothing else switches fullscreen and windowed - by
## changing that setting - and Alt+Enter is never taken for Enter by the menus. Headless, the root
## window has no screen and its mode never changes, so the switching is run on an embedded Window,
## which keeps its own mode. The player's settings are set here and loaded back after each test,
## never saved.

const GAME_WINDOW: GDScript = preload("res://game_window.gd")
const FOCUS_SECTION: GDScript = preload("res://ui/focus_section.gd")
## project.godot, [display]: fullscreen, on the primary screen - not the screen with the mouse,
## which opened the game on whatever monitor the cursor was on
const DISPLAY_MODE_SETTING: String = "display/window/size/mode"
const INITIAL_POSITION_SETTING: String = "display/window/size/initial_position_type"
## ProjectSettings' "Center of Primary Screen"
const INITIAL_POSITION_PRIMARY_SCREEN: int = 1
## game_window.gd's settings and its Mode values
const SETTINGS_SECTION: String = "window"
const MODE_WINDOWED: int = 0
const MODE_FULLSCREEN: int = 1

var _window: Window = null
var _game_window: Node = null


func before_each() -> void:
    UserSettings.set_setting(SETTINGS_SECTION, "mode", MODE_FULLSCREEN)
    get_tree().root.gui_embed_subwindows = true
    _window = Window.new()
    _window.mode = Window.MODE_FULLSCREEN
    _game_window = GAME_WINDOW.new()
    _window.add_child(_game_window)
    add_child_autofree(_window)


## What the player has saved, back - nothing a test set stays
func after_each() -> void:
    UserSettings.load_config()


func test_the_game_opens_fullscreen_on_the_primary_screen() -> void:
    assert_eq(ProjectSettings.get_setting(DISPLAY_MODE_SETTING), Window.MODE_FULLSCREEN)
    assert_eq(ProjectSettings.get_setting(INITIAL_POSITION_SETTING), INITIAL_POSITION_PRIMARY_SCREEN)


func test_toggle_fullscreen_is_alt_enter_alone() -> void:
    var events: Array[InputEvent] = InputMap.action_get_events("toggle_fullscreen")
    assert_eq(events.size(), 1)
    var key: InputEventKey = events[0] as InputEventKey
    assert_not_null(key)
    assert_eq(key.keycode, KEY_ENTER)
    assert_true(key.alt_pressed)
    assert_false(key.ctrl_pressed or key.shift_pressed or key.meta_pressed)


func test_alt_enter_is_not_enter_and_enter_is_not_alt_enter() -> void:
    assert_true(_key(KEY_ENTER, true).is_action_pressed("toggle_fullscreen", false, true))
    assert_false(_key(KEY_ENTER, true).is_action_pressed("menu_activate", false, true))
    assert_true(_key(KEY_ENTER, false).is_action_pressed("menu_activate", false, true))
    assert_false(_key(KEY_ENTER, false).is_action_pressed("toggle_fullscreen", false, true))


func test_alt_enter_switches_fullscreen_and_windowed() -> void:
    _window.push_input(_key(KEY_ENTER, true))
    assert_eq(_window.mode, Window.MODE_WINDOWED)
    assert_eq(UserSettings.get_setting(SETTINGS_SECTION, "mode", -1), MODE_WINDOWED)
    _window.push_input(_key(KEY_ENTER, true))
    assert_eq(_window.mode, Window.MODE_FULLSCREEN)
    assert_eq(UserSettings.get_setting(SETTINGS_SECTION, "mode", -1), MODE_FULLSCREEN)


func test_the_window_takes_the_mode_setting() -> void:
    UserSettings.set_setting(SETTINGS_SECTION, "mode", MODE_WINDOWED)
    assert_eq(_window.mode, Window.MODE_WINDOWED)
    UserSettings.set_setting(SETTINGS_SECTION, "mode", MODE_FULLSCREEN)
    assert_eq(_window.mode, Window.MODE_FULLSCREEN)


func test_enter_leaves_the_window_mode() -> void:
    _window.push_input(_key(KEY_ENTER, false))
    assert_eq(_window.mode, Window.MODE_FULLSCREEN)


## The starter's sections took Enter loosely and would have loaded a scenery on Alt+Enter
func test_a_section_activates_on_enter_but_not_on_alt_enter() -> void:
    var section: Node = FOCUS_SECTION.new()
    _window.add_child(section)
    section.grab_section_focus()
    watch_signals(section)
    _window.push_input(_key(KEY_ENTER, true))
    assert_signal_not_emitted(section, "activated")
    _window.push_input(_key(KEY_ENTER, false))
    assert_signal_emitted(section, "activated")


## A move that stays on its screen keeps the window as it is - only a move to another screen takes
## fullscreen again there (headless has no second screen to move to)
func test_a_move_on_the_same_screen_keeps_the_mode() -> void:
    _window.propagate_notification(Node.NOTIFICATION_WM_POSITION_CHANGED)
    assert_eq(_window.mode, Window.MODE_FULLSCREEN)
    _window.mode = Window.MODE_WINDOWED
    _window.propagate_notification(Node.NOTIFICATION_WM_POSITION_CHANGED)
    assert_eq(_window.mode, Window.MODE_WINDOWED)


func _key(keycode: Key, alt: bool) -> InputEventKey:
    var event := InputEventKey.new()
    event.keycode = keycode
    event.alt_pressed = alt
    event.pressed = true
    return event
