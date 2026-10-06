extends "settings_screen.gd"

## The settings as a panel beside the game, a third of the screen wide, so what a change does stays
## in sight: the sections first, and the settings of one in their place once it is opened; back -
## the arrow - shows the sections again. It is played with the mouse and leaves the game its keys:
## none of its sections takes the keyboard. Escape alone is its while it is open - it asks about the
## settings, as the close button does, instead of reaching the game (exit to menu).

## Another game directory, confirmed in a game - the game ends and the directory is set in the
## menu, once the scenery read from the old one is gone
signal game_dir_change_requested(path: String)

## The sections' own title, shown while no section is open
const SECTIONS_TITLE: String = "Settings"

## A section's settings stand in the place of the sections
var _page_open: bool = false
## The directory chosen on its page, waiting for the player to confirm the game ends
var _chosen_game_dir: String = ""


func focus_section_list() -> void:
    _page_open = false
    _pages[_page].visible = false
    %SectionList.visible = true
    %BackButton.visible = false
    %Title.text = SECTIONS_TITLE


func focus_page() -> void:
    _page_open = true
    %SectionList.visible = false
    _pages[_page].visible = true
    %BackButton.visible = true
    %Title.text = _titles[_page]


## F11 and the "Simulator" menu's entry: open, or - open already - closed as by its close button
func toggle() -> void:
    if visible:
        ask_save_or_discard()
        return
    open()


## Up from the buttons: back to whichever of the two is shown
func focus_content() -> void:
    if _page_open:
        focus_page()
        return
    focus_section_list()


## Before the game's own Escape - a menu shortcut, which comes after the input - sees the key
func _input(event: InputEvent) -> void:
    if visible and event.is_action_pressed("menu_back", false, true):
        get_viewport().set_input_as_handled()
        ask_save_or_discard()


## The section is only chosen here; it is shown when it is opened
func _on_section_list_item_selected(index: int) -> void:
    if index < 0:
        return
    _page = index


## In a game another directory ends it: the scenery was read from the old one. The player is asked.
func choose_game_dir(path: String, _row: SettingGameDir) -> void:
    _chosen_game_dir = path
    %GameDirQuestion.ask()


## The panel goes and the game ends; the directory is set in the menu (demo_scenery_loading.gd)
func change_game_dir() -> void:
    _close()
    game_dir_change_requested.emit(_chosen_game_dir)
