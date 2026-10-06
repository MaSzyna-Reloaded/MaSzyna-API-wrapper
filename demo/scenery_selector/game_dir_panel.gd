class_name GameDirPanel
extends VBoxContainer

## Where the game's data is: the directory in use, the installations of the original found on this
## computer (UserSettings.find_maszyna_game_dirs()) - looked for again whenever the panel is shown
## and by "Scan" - one of which is chosen as the game directory, or a folder chosen by hand, and
## below them how Reloaded is installed. The list and the buttons are its two sections. What the
## choice means is its owner's: the selector saves it, the settings set it until "Save". It stands
## in the GameDirWindow and on the settings' "Game directory" page.

## A game directory was chosen
signal game_dir_chosen(path: String)
## The notice that the game directory changed - and its data was read again - was acknowledged
signal game_dir_change_acknowledged
## Escape in one of its sections - the owner decides what back means
signal cancelled

## The bank the panel plays from
@export var sounds: SfxBank = null

## Installations on the list, in its order
var _dirs: PackedStringArray = []
var _ui_sounds: SfxPlayer


func _ready() -> void:
    _ui_sounds = SfxPlayer.new()
    _ui_sounds.bank = sounds
    add_child(_ui_sounds)


func _notification(what: int) -> void:
    if what == NOTIFICATION_VISIBILITY_CHANGED and is_visible_in_tree():
        scan()


## The installations looked for again, and the directory in use shown
func scan() -> void:
    _dirs = UserSettings.find_maszyna_game_dirs()
    var notes: PackedStringArray = []
    notes.resize(_dirs.size())
    notes.fill("")
    # the directory in use stands selected, when the search found it
    %InstallationList.set_rows(_dirs, notes, PackedStringArray(), _dirs.find(UserSettings.get_maszyna_game_dir()))
    %NoneFound.visible = not _dirs
    %InstallationList.visible = not %NoneFound.visible
    %SetButton.disabled = %NoneFound.visible
    %Status.text = ""
    show_game_dir()


## The directory in use, and the problem with it when it holds no game data
func show_game_dir() -> void:
    %Problem.visible = not UserSettings.is_maszyna_game_dir_valid()
    %CurrentDir.text = tr("Current game directory: %s") % UserSettings.get_maszyna_game_dir()


## The owner decides what the choice means - applies it at once, or asks first (in a game)
func choose_game_dir(path: String) -> void:
    game_dir_chosen.emit(path)


## Called by the owner once it applied the directory - the game's data was read again at once
func show_game_dir_changed() -> void:
    show_game_dir()
    %ChangedNotice.message = tr("The game's data is now read from %s.") % UserSettings.get_maszyna_game_dir()
    %ChangedNotice.ask()


## The notice's OK, or Escape
func acknowledge_game_dir_change() -> void:
    game_dir_change_acknowledged.emit()


## "Set game directory" - a click or Enter on the list only selects the installation and hands the
## keyboard to the buttons, "Set game directory" chosen
func choose_selected_game_dir() -> void:
    var index: int = %InstallationList.get_selected()
    if index < 0:
        return
    choose_game_dir(_dirs[index])


## The system's own folder dialog - a folder the search did not find
func choose_folder() -> void:
    _ui_sounds.play(&"list_item_click")
    %FolderDialog.popup_centered()


## "Scan" - an installation added since the panel was shown
func rescan() -> void:
    _ui_sounds.play(&"list_item_click")
    scan()


## The list and the buttons are exclusive; every way between them ends here
func focus_installation_list() -> void:
    if not _dirs:
        focus_actions()
        return
    %ActionsSection.release_section_focus()
    %ActionButtons.release_button_focus()
    %InstallationList.grab_section_focus()


## The buttons take the keys themselves (UIActionButtons): left and right choose, Enter presses
func focus_actions() -> void:
    %InstallationList.release_section_focus()
    %ActionsSection.grab_section_focus()
    %ActionButtons.grab_button_focus()


## Tab in a window: the other of the two sections
func focus_next_section() -> void:
    if %InstallationList.focused:
        focus_actions()
    else:
        focus_installation_list()


## Escape: the panel lets the keyboard go and its owner decides where it lands
func cancel() -> void:
    release_panel_focus()
    cancelled.emit()


func release_panel_focus() -> void:
    %InstallationList.release_section_focus()
    %ActionsSection.release_section_focus()
    %ActionButtons.release_button_focus()


func _on_folder_dialog_dir_selected(dir: String) -> void:
    if UserSettings.is_maszyna_game_dir(dir):
        choose_game_dir(dir)
        return
    %Status.text = tr("%s is not a MaSzyna game directory - it has no scenery, dynamic and textures folders.") % dir
