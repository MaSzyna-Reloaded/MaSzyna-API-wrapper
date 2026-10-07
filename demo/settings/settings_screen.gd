extends Control

## The settings, over the screen that opened it, in a window of the game (UIWindow): the sections
## on the left, the settings of the selected one on the right, both built from settings.json. A
## change is in effect at once; "Save" writes the changes to the file and closes; "Discard
## changes", once the player confirms it, puts back what the file holds - what there was when the
## screen opened - and closes; the close button and Escape ask whether to save or to discard, or
## close at once when nothing was changed. Tab
## and Shift+Tab walk the sidebar, the page and the buttons. The project's settings that a server
## reads at its init take effect on the next start (setting_row.gd). The window (UIWindow) keeps
## the keys and the mouse to itself.
##
## settings.json lists "sections", each with a "title" and its "settings". A setting has a "title",
## a "store" ("project": "key" is the full name of a project setting; "user": "section" and "key"
## of UserSettings), a "type" ("bool", "int", "float", "String" - a text typed into a field), a
## "default" and, for a number, a "hint"
## with its "hint_string" as a property hint of that kind has it: "range" "min,max,step,suffix:m",
## "enum" "Name:value,Name:value"; "hint": "resolution" is the window's size, its choices made from
## the screen, and "folder" a project setting naming a file, shown as its folder with a button that
## opens it in the file manager, and "game_dir" the game directory (a "user" String), changed in the
## GameDirWindow. Titles and choice names are msgids. "advanced": true puts a setting
## under the section's folded "Advanced". "excludes" names the keys of the section's settings that
## cannot be switched on while this one is on (TAA or MSAA and FXAA); "depends_on" the key of the
## one that has to be on for this one to be changed; "disabled_by" the key of the one that keeps it
## from being changed while it is on. "restart_required": true marks one that takes effect only
## after the game restarts - a change of it lights the note above the buttons, and Save asks
## whether to restart.

const DEFINITION: JSON = preload("settings.json")
const PAGE: PackedScene = preload("settings_page.tscn")
const TOGGLE: PackedScene = preload("setting_toggle.tscn")
const SLIDER: PackedScene = preload("setting_slider.tscn")
const OPTION: PackedScene = preload("setting_option.tscn")
const RESOLUTION: PackedScene = preload("setting_resolution.tscn")
const FOLDER: PackedScene = preload("setting_folder.tscn")
const GAME_DIR: PackedScene = preload("setting_game_dir.tscn")
const TEXT: PackedScene = preload("setting_text.tscn")
const STORES: Dictionary[String, SettingRow.Store] = {
    "project": SettingRow.Store.PROJECT,
    "user": SettingRow.Store.USER,
}

## Part of the screen the settings window takes
const WINDOW_RATIO: float = 0.76
## The note above the buttons, shown while a change of a setting that takes effect after a restart
## waits for one - every other setting takes effect at once

## The bank the screen and its pages play from
@export var sounds: SfxBank = null
## The window the settings stand in, a window of its own so the keys stay in it - none for a panel
## beside the game, which leaves the game its keys
@export var window: UIWindow = null

## One page per section, in the order of the sidebar, and the sections' titles in the same order
var _pages: Array[SettingsPage] = []
var _titles: PackedStringArray = []
## Page the sidebar has selected
var _page: int = 0
var _ui_sounds: SfxPlayer


func _ready() -> void:
    _ui_sounds = SfxPlayer.new()
    _ui_sounds.bank = sounds
    add_child(_ui_sounds)
    UserSettings.config_changed.connect(_apply_graphics)
    UserSettings.config_changed.connect(_update_restart_note)
    _apply_graphics()
    var notes: PackedStringArray = []
    for section: Dictionary in DEFINITION.data["sections"]:
        var settings: Array = section["settings"]
        var page: SettingsPage = PAGE.instantiate()
        page.sounds = sounds
        page.visible = false
        # a setting names the ones it excludes or depends on by their key; the rows are known once
        # all are built
        var rows: Dictionary[String, SettingRow] = {}
        for setting: Dictionary in settings:
            var row: SettingRow = (
                TOGGLE if setting["type"] == "bool"
                else SLIDER if setting.get("hint") == "range"
                else RESOLUTION if setting.get("hint") == "resolution"
                else FOLDER if setting.get("hint") == "folder"
                else GAME_DIR if setting.get("hint") == "game_dir"
                else TEXT if setting["type"] == "String"
                else OPTION
            ).instantiate()
            row.title = setting["title"]
            row.store = STORES[setting["store"]]
            row.section = setting.get("section", "")
            row.key = setting["key"]
            row.default_value = setting["default"]
            row.hint_string = setting.get("hint_string", "")
            row.restart_required = setting.get("restart_required", false)
            if setting.get("advanced", false):
                page.add_advanced_row(row)
            else:
                page.add_row(row)
            rows[row.key] = row
            if row is SettingGameDir:
                row.game_dir_chosen.connect(choose_game_dir.bind(row))
        for setting: Dictionary in settings:
            var row: SettingRow = rows[setting["key"]]
            for excluded: String in setting.get("excludes", []):
                rows[excluded].excluding_rows.append(row)
                row.affected_rows.append(rows[excluded])
            if setting.has("depends_on"):
                row.required_rows.append(rows[setting["depends_on"]])
                rows[setting["depends_on"]].affected_rows.append(row)
            if setting.has("disabled_by"):
                row.disabling_rows.append(rows[setting["disabled_by"]])
                rows[setting["disabled_by"]].affected_rows.append(row)
        page.cancelled.connect(ask_save_or_discard)
        page.navigate_down.connect(focus_actions)
        page.navigate_left.connect(focus_section_list)
        page.focus_requested.connect(focus_page)
        %Content.add_child(page)
        _pages.append(page)
        _titles.append(section["title"])
        notes.append(str(settings.filter(
            func(setting: Dictionary) -> bool: return not setting.get("advanced", false)
        ).size()))
    for page: SettingsPage in _pages:
        page.remember_values()
    # the list selects its first row and reports it back, so the first page shows from here on
    %SectionList.set_rows(_titles, notes)


## Shows what is in effect now
func open() -> void:
    _ui_sounds.play(&"list_item_click")
    for page: SettingsPage in _pages:
        page.revert()
        page.remember_values()
    _update_restart_note()
    visible = true
    if window:
        window.show_part(WINDOW_RATIO)
    focus_section_list()


## Saved; a change that takes effect after a restart asks whether to restart now
func save() -> void:
    _ui_sounds.play(&"load_scenery")
    UserSettings.save_config()
    if _pages.any(func(page: SettingsPage) -> bool: return page.needs_restart()):
        %RestartDialog.ask()
        return
    _close()


## The game starts again with what it was started with - the saved settings are read then
func restart() -> void:
    var arguments: PackedStringArray = OS.get_cmdline_args()
    var user_arguments: PackedStringArray = OS.get_cmdline_user_args()
    if user_arguments:
        arguments.append("--")
        arguments.append_array(user_arguments)
    OS.set_restart_on_exit(true, arguments)
    get_tree().quit()


## Every setting back to the default, in effect at once like any change - "Save" keeps it,
## "Discard changes" brings back what the file holds
func restore_defaults() -> void:
    _ui_sounds.play(&"list_item_click")
    for page: SettingsPage in _pages:
        page.restore_defaults()


## "Discard changes": the changes go once the player confirms it
func cancel() -> void:
    %DiscardDialog.ask()


## The close button and Escape, wherever the focus is (editing a value aside, which Escape ends):
## the player chooses whether the changes are saved or discarded - with nothing changed it closes
func ask_save_or_discard() -> void:
    if not UserSettings.is_config_changed():
        _ui_sounds.play(&"back_button")
        _close()
        return
    %CloseDialog.ask()


## Nothing is saved while the screen is open, so the file still holds what there was when it opened
func discard() -> void:
    _ui_sounds.play(&"back_button")
    UserSettings.load_config()
    for page: SettingsPage in _pages:
        page.revert()
    _close()


## In the menu the directory chosen on its page is set at once, like any other setting
func choose_game_dir(path: String, row: SettingGameDir) -> void:
    row.set_game_dir(path)


## The sidebar, the page and the buttons are exclusive; every way between them ends here
func focus_section_list() -> void:
    _pages[_page].release_section_focus()
    %ActionsSection.release_section_focus()
    %ActionButtons.release_button_focus()
    %SectionList.grab_section_focus()


func focus_page() -> void:
    %SectionList.release_section_focus()
    %ActionsSection.release_section_focus()
    %ActionButtons.release_button_focus()
    _pages[_page].grab_section_focus()


## The buttons take the keys themselves (UIActionButtons): left and right choose, Enter presses
func focus_actions() -> void:
    %SectionList.release_section_focus()
    _pages[_page].release_section_focus()
    %ActionsSection.grab_section_focus()
    %ActionButtons.grab_button_focus()


func _update_restart_note() -> void:
    %Note.visible = _pages.any(func(page: SettingsPage) -> bool: return page.needs_restart())


func _close() -> void:
    %SectionList.release_section_focus()
    _pages[_page].release_section_focus()
    %ActionsSection.release_section_focus()
    %ActionButtons.release_button_focus()
    if window:
        window.hide()
    visible = false


## Tab and Shift+Tab walk the sidebar, the page and the buttons, round; none of them takes Tab
## itself (the buttons' row has tab_walks_buttons off), so the key does nothing more there
func _on_settings_window_window_input(event: InputEvent) -> void:
    var step: int = 0
    if event.is_action_pressed("menu_next_section", true, true):
        step = 1
    elif event.is_action_pressed("menu_previous_section", true, true):
        step = -1
    else:
        return
    # an edited row takes Tab itself, between its control and its value field
    if _pages[_page].is_editing():
        return
    window.set_input_as_handled()
    # a value being typed is left - and taken - with the section
    window.gui_release_focus()
    var sections: Array[Callable] = [focus_section_list, focus_page, focus_actions]
    var at: int = 0
    if _pages[_page].focused:
        at = 1
    elif %ActionsSection.focused:
        at = 2
    sections[wrapi(at + step, 0, sections.size())].call()


func _on_section_list_item_selected(index: int) -> void:
    if index < 0:
        return
    _pages[_page].visible = false
    _page = index
    _pages[_page].visible = true


## The Graphics section's values that live on the viewport, not in a reader of their own (the
## window's are GameWindow's) - the environment takes its own from UserSettings (maszyna_environment_node.gd). The defaults
## are the project's own, so nothing changes until the player sets it.
func _apply_graphics() -> void:
    var viewport: Viewport = get_tree().root.get_viewport()
    viewport.msaa_3d = UserSettings.get_setting("render", "msaa_3d", Viewport.MSAA_DISABLED)
    viewport.screen_space_aa = UserSettings.get_setting(
        "render", "screen_space_aa", Viewport.SCREEN_SPACE_AA_DISABLED
    )
    viewport.use_taa = UserSettings.get_setting("render", "use_taa", true)
