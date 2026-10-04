class_name GameDirWindow
extends Control

## The GameDirPanel in a window of the game (UIWindow) over the screen that opened it. It is a
## top-level control, so it covers the screen wherever it stands. What the choice means is its
## owner's - the selector saves it.

## A game directory was chosen - the window closes once the player acknowledges the change
signal game_dir_chosen(path: String)
## The window went, with a game directory chosen or without
signal closed

## The part of the game's window it takes, each way
const WINDOW_RATIO: float = 0.6


func open() -> void:
    visible = true
    %Window.show_part(WINDOW_RATIO)
    %GameDirPanel.focus_installation_list()


func close() -> void:
    %GameDirPanel.release_panel_focus()
    %Window.hide()
    visible = false
    closed.emit()


## The selector saves it as the signal is emitted, so the notice follows it
func _on_game_dir_panel_game_dir_chosen(path: String) -> void:
    game_dir_chosen.emit(path)
    %GameDirPanel.show_game_dir_changed()


## Tab and Shift+Tab: the other of the panel's two sections
func _on_window_window_input(event: InputEvent) -> void:
    if not (
        event.is_action_pressed("menu_next_section", true, true)
        or event.is_action_pressed("menu_previous_section", true, true)
    ):
        return
    %Window.set_input_as_handled()
    %GameDirPanel.focus_next_section()
