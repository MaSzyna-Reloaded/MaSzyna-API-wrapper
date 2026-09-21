class_name SelectorList
extends FocusSection

## One list section of the selector screen: the rows, the search that filters them, the selection
## with its blinking marker, the keyboard walking over them and the sounds of all of it. Both lists
## of the screen are this scene, so they look and behave the same and only their content differs.
##
## The screen supplies the rows, reads the selection and decides what an activated row means. Left
## and right are the screen's bindings, not the list's - they come back out as navigate_out().

## The selection moved, by key, by search or by click. -1 when the search matched nothing, which is
## the only state with no row selected at all.
signal item_selected(index: int)
## Enter on the selected row. The list makes no sound for it - the gesture belongs to the screen.
signal item_activated(index: int)
## Left (-1) or right (1) while this list has the focus
signal navigate_out(step: int)
## A row was clicked - the keyboard belongs to this list now, and the screen owns that decision
signal focus_requested

## UI feedback of the startup screens. Events are named after what happened, not after the sample.
const UI_SOUNDS: SfxBank = preload("res://startup/ui_sounds.tres")

## Look of a row, by how lit it is - selector_theme.tres
const ITEM_VARIATIONS: Array[StringName] = [&"ListItem", &"ListItemHovered", &"ListItemSelected"]
const ITEM_STATE_IDLE: int = 0
const ITEM_STATE_HOVERED: int = 1
const ITEM_STATE_SELECTED: int = 2

## Blinking triangle at the left edge of the selected row - the blink lives in the shader
const SELECTION_MARKER_SHADER: Shader = preload("res://scenery_selector/selection_marker.gdshader")
const SELECTION_MARKER_SIZE: Vector2 = Vector2(10.0, 12.0)
## Room kept for the note on the right of a row
const NOTE_WIDTH: float = 180.0

## Rows PgUp and PgDown move over - about what a list shows at once
const PAGE_STEP: int = 10
## Home and End: a step no list here is long enough to survive
const LIST_END_STEP: int = 1000000

## Lists without a search field keep the whole row for their content
@export var searchable: bool = false:
    set(value):
        searchable = value
        _dirty = true
        set_process(true)

var _titles: PackedStringArray = []
var _notes: PackedStringArray = []
var _rows: Array[PanelContainer] = []
## Marker of each row, shown on the selected one, in the order of _rows
var _markers: Array[ColorRect] = []
var _selected: int = -1
var _ui_sounds: SfxPlayer


func _ready() -> void:
    super()
    _ui_sounds = SfxPlayer.new()
    _ui_sounds.bank = UI_SOUNDS
    add_child(_ui_sounds)
    %SearchPanel.visible = searchable


## The ring and the marker are the same change seen twice: the marker belongs to the selected row of
## the focused list, so it comes and goes with the ring.
func _process_dirty() -> void:
    super()
    %SearchPanel.visible = searchable
    _update_marker()


## Activating the list hands the keyboard to its own search field; a list without one drops the
## focus instead, so typing can never land in another list's field.
##
## Two traps: release_focus() clears the whole viewport's focus, so only a field that actually holds
## it may give it up - otherwise the list being deactivated takes away what the activated one just
## grabbed. And a click on a row is still being processed by the viewport when this runs, which then
## drops the key focus because a row cannot take it, so the grab has to happen after that.
func set_section_focused(is_focused: bool) -> void:
    focused = is_focused
    if is_focused:
        _grab_search_focus()
        # again after the frame: when a click on a row is what activated the list, the viewport is
        # still processing that click and drops the key focus after this returns
        _grab_search_focus.call_deferred()
    elif %Search.has_focus():
        %Search.release_focus()


func _grab_search_focus() -> void:
    # the section may have moved on since this was queued
    if focused and searchable:
        %Search.grab_focus()


## Rows of the list, given as what they show: a title and the smaller grey note beside it. The first
## row is selected right away - nothing here is ever left deselected while there is a row to select.
func set_rows(titles: PackedStringArray, notes: PackedStringArray) -> void:
    for row: PanelContainer in _rows:
        row.queue_free()
    _rows.clear()
    _markers.clear()
    _titles = titles
    _notes = notes
    _selected = -1
    for index: int in _titles.size():
        var row: PanelContainer = _create_row(index)
        _rows.append(row)
        %List.add_child(row)
    if searchable:
        %Search.text = ""
        %ClearSearch.visible = false
    %Scroll.scroll_vertical = 0
    _select(0 if _rows else -1)


## Row the selection is on, or -1 when the search matched nothing
func get_selected() -> int:
    return _selected


## The keys of a list, taken before the viewport can turn them into focus navigation of its own.
## Only the focused list acts on them, and typing is left alone - it belongs to the search field.
func _input(event: InputEvent) -> void:
    if not focused or not is_visible_in_tree():
        return
    if event.is_action_pressed("ui_down", true):
        _move(1)
    elif event.is_action_pressed("ui_up", true):
        _move(-1)
    elif event.is_action_pressed("ui_page_down", true):
        _move(PAGE_STEP)
    elif event.is_action_pressed("ui_page_up", true):
        _move(-PAGE_STEP)
    elif event.is_action_pressed("ui_end"):
        _move(LIST_END_STEP)
    elif event.is_action_pressed("ui_home"):
        _move(-LIST_END_STEP)
    # ui_text_submit and not ui_accept: that one is Space as well, and Space belongs to the search
    elif event.is_action_pressed("ui_text_submit"):
        if _selected >= 0:
            item_activated.emit(_selected)
    elif event.is_action_pressed("ui_right"):
        navigate_out.emit(1)
    elif event.is_action_pressed("ui_left"):
        navigate_out.emit(-1)
    else:
        return
    get_viewport().set_input_as_handled()


## Up/Down/PgUp/PgDn/Home/End: the row change a click would have made, under the keyboard's own
## sound. An end of the list changes nothing and moves nothing - a reselected row would have the
## screen rebuild everything it hangs off.
func _move(step: int) -> void:
    var index: int = _next_visible_row(_selected, step)
    if index < 0:
        return
    _ui_sounds.play(&"keystroke")
    _select(index)
    scroll_to_item(%Scroll, _rows[index])


## The row abs(step) visible rows away from "from", or the last visible one in that direction when
## the list ends first - so a page or an End lands on the edge of the results, never outside them.
## -1 when the search left no visible row that way at all.
func _next_visible_row(from: int, step: int) -> int:
    var direction: int = signi(step)
    var left: int = absi(step)
    var index: int = from
    var found: int = -1
    while left > 0:
        index += direction
        if index < 0 or index >= _rows.size():
            break
        if _rows[index].visible:
            found = index
            left -= 1
    return found


func _select(index: int) -> void:
    if _selected >= 0:
        _rows[_selected].theme_type_variation = ITEM_VARIATIONS[ITEM_STATE_IDLE]
        _markers[_selected].visible = false
    _selected = index
    if index >= 0:
        _rows[index].theme_type_variation = ITEM_VARIATIONS[ITEM_STATE_SELECTED]
    _update_marker()
    item_selected.emit(index)


func _update_marker() -> void:
    if _selected >= 0:
        _markers[_selected].visible = focused


## One row: its name and, on the right, a smaller grey note
func _create_row(index: int) -> PanelContainer:
    var row := PanelContainer.new()
    row.theme_type_variation = ITEM_VARIATIONS[ITEM_STATE_IDLE]
    row.mouse_filter = Control.MOUSE_FILTER_STOP
    row.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
    row.gui_input.connect(_on_row_gui_input.bind(index))
    row.mouse_entered.connect(_on_row_hovered.bind(index, true))
    row.mouse_exited.connect(_on_row_hovered.bind(index, false))

    var line := HBoxContainer.new()
    line.mouse_filter = Control.MOUSE_FILTER_IGNORE
    line.add_theme_constant_override("separation", 16)
    row.add_child(line)
    _markers.append(_add_marker(line))

    var label := Label.new()
    label.text = _titles[index]
    label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    label.mouse_filter = Control.MOUSE_FILTER_IGNORE
    # a long name must not push the note out of the panel
    label.clip_text = true
    label.custom_minimum_size = Vector2(120.0, 0.0)
    label.add_theme_font_size_override("font_size", 16)
    line.add_child(label)

    var note := Label.new()
    note.text = _notes[index]
    note.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
    note.vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM
    note.custom_minimum_size = Vector2(NOTE_WIDTH, 0.0)
    note.clip_text = true
    note.mouse_filter = Control.MOUSE_FILTER_IGNORE
    note.add_theme_font_size_override("font_size", 12)
    note.add_theme_color_override("font_color", Color(0.72, 0.76, 0.82, 0.65))
    line.add_child(note)
    return row


## Room for the marker at the left edge of every row, marker or not, so a row does not shift when it
## gets one. The triangle inside is drawn and blinked by the shader; the script only shows it.
func _add_marker(line: HBoxContainer) -> ColorRect:
    var slot := Control.new()
    slot.custom_minimum_size = SELECTION_MARKER_SIZE
    slot.size_flags_vertical = Control.SIZE_SHRINK_CENTER
    slot.mouse_filter = Control.MOUSE_FILTER_IGNORE
    line.add_child(slot)

    var marker := ColorRect.new()
    marker.set_anchors_preset(Control.PRESET_FULL_RECT)
    marker.mouse_filter = Control.MOUSE_FILTER_IGNORE
    marker.material = ShaderMaterial.new()
    (marker.material as ShaderMaterial).shader = SELECTION_MARKER_SHADER
    marker.visible = false
    slot.add_child(marker)
    return marker


func _on_row_gui_input(event: InputEvent, index: int) -> void:
    if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
        _ui_sounds.play(&"list_item_click")
        focus_requested.emit()
        _select(index)


func _on_row_hovered(index: int, hovered: bool) -> void:
    if index == _selected:
        return
    if hovered:
        _ui_sounds.play(&"list_item_hover")
    _rows[index].theme_type_variation = ITEM_VARIATIONS[
        ITEM_STATE_HOVERED if hovered else ITEM_STATE_IDLE
    ]


func _on_search_text_changed(text: String) -> void:
    _ui_sounds.play(&"keystroke")
    %ClearSearch.visible = not text.is_empty()
    %SearchDebounce.start()


func _on_search_debounce_timeout() -> void:
    _filter(%Search.text)
    # the first result takes over the selection; nothing found is the one case with none at all
    var index: int = _next_visible_row(-1, 1)
    if not index == _selected:
        _select(index)
    # the filtered-out rows take no room, so the first result sits at the top of the list
    %Scroll.scroll_vertical = 0


func _on_clear_search_pressed() -> void:
    %Search.text = ""
    %ClearSearch.visible = false
    %SearchDebounce.start()
    %Search.grab_focus()


## Rows whose title or note contain the searched text
func _filter(text: String) -> void:
    var needle: String = text.strip_edges().to_lower()
    for index: int in _rows.size():
        _rows[index].visible = (
            not needle
            or _titles[index].to_lower().contains(needle)
            or _notes[index].to_lower().contains(needle)
        )
