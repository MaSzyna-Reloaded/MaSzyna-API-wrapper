class_name SettingGameDir
extends SettingRow

## The game directory (UserSettings maszyna/game_dir), the whole of its page: its name, marked when
## the directory in use holds no game data, and under it the GameDirPanel - the directory in use,
## the installations found, "Scan", "Choose folder..." and "Set game directory". The mouse works
## the panel directly; Enter on the row hands the keyboard to it and Escape takes it back. The
## directory chosen there is set like any other setting: in effect at once, saved by "Save" and
## dropped by "Discard changes" - in the menu; in a game the settings ask first (settings_panel.gd).

## A directory was chosen in the panel - the settings decide what it means
signal game_dir_chosen(path: String)


func is_edited_by_steps() -> bool:
    return false


## The page's keyboard left the row - so did the panel's
func set_selected(p_selected: bool) -> void:
    super(p_selected)
    if not p_selected:
        %GameDirPanel.release_panel_focus()


## The directory in use, not the stored value - the game's own directory takes over from one that
## holds no game data
func _show(_value: Variant) -> void:
    %WarningIcon.visible = not UserSettings.is_maszyna_game_dir_valid()
    %GameDirPanel.show_game_dir()


func _activate() -> void:
    %GameDirPanel.focus_installation_list()


## Set like any other setting, and the player told the data was read again
func set_game_dir(path: String) -> void:
    _store(path)
    revert()
    %GameDirPanel.show_game_dir_changed()


func _on_game_dir_panel_game_dir_chosen(path: String) -> void:
    game_dir_chosen.emit(path)
