extends Node

## The game's window, for as long as the game runs: Alt+Enter switches between fullscreen and a
## window, and a fullscreen window moved to another screen fills that screen. Windows has no
## fullscreen state of its own - Win+Shift+arrow moves the fullscreen game as an ordinary window
## and leaves it the size of the screen it came from.

## Screen the window stood on when it last moved
var _screen: int = -1


func _ready() -> void:
    _screen = get_window().current_screen


## In _input, before the GUI: a search field would take Enter for itself
func _input(event: InputEvent) -> void:
    if not event.is_action_pressed("toggle_fullscreen", false, true):
        return
    var window: Window = get_window()
    window.mode = Window.MODE_WINDOWED if window.mode == Window.MODE_FULLSCREEN else Window.MODE_FULLSCREEN
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
