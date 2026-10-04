class_name SettingRow
extends PanelContainer

## One setting on a settings page: its name on the left, its control on the right. The screen sets
## its fields from the settings definition before it enters the tree. What the player sets is in
## effect at once (UserSettings.set_setting()); the screen saves it or reverts all of it.
##
## Every value is the player's and is saved to UserSettings. A project setting ("maszyna/...") goes
## into the section UserSettings sets onto ProjectSettings when it loads, before any server reads
## it - so what a server reads only at its init takes the new value on the next start of the game.

enum Store {
    ## A project setting: read from ProjectSettings, saved under its full name
    PROJECT,
    ## A setting of UserSettings' own, in its own section
    USER,
}

## UserSettings.hpp PROJECT_SETTINGS_SECTION
const PROJECT_SETTINGS_SECTION: String = "project_settings"

## Look of the row, by how lit it is - selector_theme.tres
const ITEM_IDLE: StringName = &"SettingItem"
const ITEM_HOVERED: StringName = &"SettingItemHovered"
const ITEM_SELECTED: StringName = &"SettingItemSelected"
const ITEM_EDITING: StringName = &"SettingItemEditing"
## ... and of an advanced row: fainter, a little less padding
const ADVANCED_ITEM_IDLE: StringName = &"SettingItemAdvanced"
const ADVANCED_ITEM_HOVERED: StringName = &"SettingItemAdvancedHovered"
const ADVANCED_ITEM_SELECTED: StringName = &"SettingItemAdvancedSelected"
const ADVANCED_ITEM_EDITING: StringName = &"SettingItemAdvancedEditing"
## The name of an advanced row, smaller than a main one's
const ADVANCED_TITLE_FONT_SIZE: int = 14
## Opacity of the name and the control of a row that cannot be changed now
const UNAVAILABLE_ALPHA: float = 0.4

## The setting's name, as a msgid - the label translates it
var title: String = ""
var store: Store = Store.PROJECT
## UserSettings section of a USER setting; a PROJECT setting has none of its own
var section: String = ""
## Full name of a PROJECT setting ("maszyna/scenery/draw_distance"), the key of a USER one
var key: String = ""
## What the game uses when the setting was never set - the value its reader falls back to
var default_value: Variant = null
## The control's range or choices, in the format of a property hint of that kind
var hint_string: String = ""
## The row stands under "Advanced": it and its control are drawn smaller and fainter
var advanced: bool = false
## A change takes effect only after the game restarts (settings.json "restart_required")
var restart_required: bool = false
## Rows that have to be on for this one to be changed
var required_rows: Array[SettingRow] = []
## Rows that, while one of them is on, keep this one from being switched on (TAA or MSAA and FXAA)
var excluding_rows: Array[SettingRow] = []
## Rows that, while one of them is on, keep this one from being changed at all (the window's size
## while the window is fullscreen)
var disabling_rows: Array[SettingRow] = []
## Rows whose availability follows this one's value - the ones requiring it or excluded by it
var affected_rows: Array[SettingRow] = []
## The control takes a change - otherwise it is disabled
var available: bool = true

var _selected: bool = false
var _editing: bool = false
## The value the settings were opened with - a restart is wanted while it differs
var _opened_value: Variant = null
var _hovered: bool = false
## The stored value is being put on the control: the control rounds and clamps it and reports that
## as a change, which is not the player's
var _showing: bool = false


func _ready() -> void:
    %Title.text = title
    if advanced:
        # one line: a name too long is cut, and shown whole under the mouse
        %Title.add_theme_font_size_override(&"font_size", ADVANCED_TITLE_FONT_SIZE)
        %Title.autowrap_mode = TextServer.AUTOWRAP_OFF
        %Title.text_overrun_behavior = TextServer.OVERRUN_TRIM_ELLIPSIS
        %Title.clip_text = true
        %Title.mouse_filter = Control.MOUSE_FILTER_PASS
        _make_compact()
        # after the control was made compact - a slider row moves the name onto its own line - and
        # for the size it has already
        %Title.resized.connect(_on_title_resized)
        _on_title_resized()
    _update_look()
    _showing = true
    _setup()
    _showing = false
    revert()
    update_available()


## The page's keyboard is on this row - the page shows it only while it has the focus itself
func set_selected(p_selected: bool) -> void:
    _selected = p_selected
    _update_look()


## Its value is being changed from the keyboard - the page's left and right step it
func set_editing(p_editing: bool) -> void:
    _editing = p_editing
    _update_look()


## Tab while the row is edited: from its control to the field its value is typed into and back -
## for a row that has one (a slider)
func toggle_value_typing() -> void:
    pass


## Whether Enter puts the row into editing (a slider, a choice) or does its own thing (a switch)
func is_edited_by_steps() -> bool:
    return true


## Puts the stored value back on the control - after a revert, and whenever the screen opens
func revert() -> void:
    _showing = true
    _show(_read())
    _showing = false
    _update_affected_rows()


## What the setting has as the settings are opened - what a change is told from
func remember_value() -> void:
    _opened_value = _read()


## A change of a setting that takes effect after a restart, made since the settings were opened
func needs_restart() -> bool:
    return restart_required and not _read() == _opened_value


## Whether the setting is on - true, or a choice or a number other than 0
func is_on() -> bool:
    var value: Variant = _read()
    if value is bool:
        return value
    if value is int or value is float:
        return not is_zero_approx(float(value))
    # a value that is not a switch or a number (a size) is never on
    return false


## Available while every required row is on and no excluding one is - unless this one is on itself,
## so a setting on together with an excluding one can still be switched off
func update_available() -> void:
    available = (
        required_rows.all(func(row: SettingRow) -> bool: return row.is_on())
        and not disabling_rows.any(func(row: SettingRow) -> bool: return row.is_on())
        and (is_on() or not excluding_rows.any(func(row: SettingRow) -> bool: return row.is_on()))
    )
    %Title.modulate.a = 1.0 if available else UNAVAILABLE_ALPHA
    _enable_control(available)


## The player's value goes - the project's own, or UserSettings' default, is in effect again
func restore_default() -> void:
    if store == Store.PROJECT:
        UserSettings.erase_setting(PROJECT_SETTINGS_SECTION, key)
    else:
        UserSettings.erase_setting(section, key)
    revert()


## Right and left on the selected row, and Enter on it - what each means is the control's own;
## a row that is not available takes none of them
func step_up() -> void:
    if available:
        _step_up()


func step_down() -> void:
    if available:
        _step_down()


func activate() -> void:
    if available:
        _activate()


func _step_up() -> void:
    pass


func _step_down() -> void:
    pass


func _activate() -> void:
    pass


## The control made from hint_string, before it is shown its first value
func _setup() -> void:
    pass


func _enable_control(_enabled: bool) -> void:
    pass


## The control drawn smaller, for an advanced row
func _make_compact() -> void:
    pass


func _show(_value: Variant) -> void:
    pass


## What the player set on the control, in effect at once. Nothing reaches here while the row is
## built or shown a value - is_node_ready() is true already inside _ready(), so it cannot tell.
func _store(value: Variant) -> void:
    if _showing:
        return
    if store == Store.PROJECT:
        UserSettings.set_setting(PROJECT_SETTINGS_SECTION, key, value)
    else:
        UserSettings.set_setting(section, key, value)
    _update_affected_rows()


func _update_affected_rows() -> void:
    for row: SettingRow in affected_rows:
        row.update_available()


## The whole name under the mouse only while it does not fit
func _on_title_resized() -> void:
    var font: Font = %Title.get_theme_font(&"font")
    # the name as it is shown - translated
    var width: float = font.get_string_size(
        %Title.atr(%Title.text), HORIZONTAL_ALIGNMENT_LEFT, -1, %Title.get_theme_font_size(&"font_size")
    ).x
    %Title.tooltip_text = %Title.text if width > %Title.size.x else ""


func _notification(what: int) -> void:
    if what == NOTIFICATION_MOUSE_ENTER or what == NOTIFICATION_MOUSE_EXIT:
        _hovered = what == NOTIFICATION_MOUSE_ENTER
        _update_look()


func _update_look() -> void:
    if advanced:
        if _editing:
            theme_type_variation = ADVANCED_ITEM_EDITING
        elif _selected:
            theme_type_variation = ADVANCED_ITEM_SELECTED
        else:
            theme_type_variation = ADVANCED_ITEM_HOVERED if _hovered else ADVANCED_ITEM_IDLE
        return
    if _editing:
        theme_type_variation = ITEM_EDITING
    elif _selected:
        theme_type_variation = ITEM_SELECTED
    else:
        theme_type_variation = ITEM_HOVERED if _hovered else ITEM_IDLE


## The value the setting has now
func _read() -> Variant:
    if store == Store.PROJECT:
        return ProjectSettings.get_setting(key, default_value)
    return UserSettings.get_setting(section, key, default_value)
