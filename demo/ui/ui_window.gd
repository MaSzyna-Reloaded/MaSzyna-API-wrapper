class_name UIWindow
extends Window

## A window of the game: a window of its own, so no key typed in it reaches what is under it,
## borderless and transparent, with the frame of the game's windows drawn inside (ui_theme.tres) -
## the title at the top with the close button, the content below in the Body, which the window's
## user fills: its scene instances this one with editable children and puts its own nodes under
## Body. The close button, and Escape when nothing in the window took it (a section stepping back),
## ask for the window to be closed (close_requested, as a titlebar's close would); whoever owns it
## decides what closing means. Shown as a part of the game's window (show_part()), the frame stays
## that part - its size and its place follow the game's window when that one changes (another
## resolution, fullscreen and back).
##
## The window itself covers the whole game's window, the Backdrop dimming what is under the frame
## (a user that wants the game seen as it is clears its colour): a click beside the frame lands in
## the window, which keeps the keys, and nothing under it is reached. It is not exclusive - the
## game's window ignores the window manager's close while it has an exclusive child (window.cpp,
## WINDOW_EVENT_CLOSE_REQUEST), and the game could not be closed while one was open.


func _ready() -> void:
    %Title.text = title
    get_parent().get_viewport().size_changed.connect(_fit)


## A window inside another window (a settings row's) outlives none of it: the outer one's size
## changes while it is freed, after this one's parent has left it
func _exit_tree() -> void:
    get_parent().get_viewport().size_changed.disconnect(_fit)


## The frame shown in the middle of the game's window, taking that part of it each way
func show_part(part: float) -> void:
    %Frame.anchor_left = (1.0 - part) / 2.0
    %Frame.anchor_top = (1.0 - part) / 2.0
    %Frame.anchor_right = (1.0 + part) / 2.0
    %Frame.anchor_bottom = (1.0 + part) / 2.0
    _fit()
    show()


## The whole game's window; the frame's anchors keep its part of it
func _fit() -> void:
    position = Vector2i.ZERO
    size = Vector2i(get_parent().get_viewport().get_visible_rect().size)


func _unhandled_input(event: InputEvent) -> void:
    if event.is_action_pressed("menu_back", false, true):
        set_input_as_handled()
        close_requested.emit()


func _on_close_button_pressed() -> void:
    close_requested.emit()
