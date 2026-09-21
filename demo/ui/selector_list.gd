class_name SelectorList
extends FocusSection

## One list section of the selector screen: the rows, the search that filters them, the selection
## with its blinking marker, the keyboard walking over them and the sounds of all of it. Both lists
## of the screen are this scene, so they look and behave the same and only their content differs.
##
## The screen supplies the rows, reads the selection and decides what an activated row means. Left
## and right are the screen's bindings, not the list's - they leave as navigate_left/right.

## The selection moved, by key, by search or by click. -1 when the search matched nothing, which is
## the only state with no row selected at all.
signal item_selected(index: int)

## The bank this component plays from - a slot the screen using it fills. Events are named after
## what happened, not after the sample, and the bank has to name "keystroke" and "change_focus".
@export var sounds: SfxBank = null

## Look of a row, by how lit it is - selector_theme.tres
const ITEM_IDLE: StringName = &"ListItem"
const ITEM_HOVERED: StringName = &"ListItemHovered"
const ITEM_SELECTED: StringName = &"ListItemSelected"

## Blinking triangle at the left edge of the selected row - the blink lives in the shader
const SELECTION_MARKER_SHADER: Shader = preload("selection_marker.gdshader")
const SELECTION_MARKER_SIZE: Vector2 = Vector2(10.0, 12.0)
## Room kept for the note on the right of a row
const NOTE_WIDTH: float = 180.0

## Rows PgUp and PgDown move over - about what a list shows at once
const PAGE_STEP: int = 10

## What counts as a word in a search: letters and digits, so whitespace, dashes, brackets and
## punctuation all separate tokens and none of them ever has to be typed
const TOKEN_PATTERN: String = "[\\p{L}\\p{N}]+"
## A typed token shorter than this filters nothing - one letter would only throw rows away
const MIN_FRAGMENT: int = 2

## Lists without a search field keep the whole row for their content. Read once, in _ready().
@export var searchable: bool = false
## What the bank calls a click and a hover on a row
@export var click_event: StringName = &"list_item_click"
@export var hover_event: StringName = &"list_item_hover"

var _titles: PackedStringArray = []
var _notes: PackedStringArray = []
## Everything a row can be found by, tokenized once when the rows are set, in the order of _rows
var _row_tokens: Array[PackedStringArray] = []
var _tokenizer: RegEx = RegEx.create_from_string(TOKEN_PATTERN)
var _rows: Array[PanelContainer] = []
## Marker of each row, shown on the selected one, in the order of _rows
var _markers: Array[ColorRect] = []
var _selected: int = -1
var _ui_sounds: SfxPlayer


func _ready() -> void:
    super()
    _ui_sounds = SfxPlayer.new()
    _ui_sounds.bank = sounds
    add_child(_ui_sounds)
    focus_taken.connect(_ui_sounds.play.bind(&"change_focus"))
    %SearchPanel.visible = searchable


## The ring and the marker are the same change seen twice: the marker belongs to the selected row of
## the focused list, so it comes and goes with the ring.
func _process_dirty() -> void:
    super()
    if _selected >= 0:
        _markers[_selected].visible = focused


## The keyboard of the focused list belongs to its own search field, so typing can never land in
## another list's field.
func grab_section_focus() -> void:
    super()
    if focused and searchable:
        %Search.grab_focus()
        %Search.grab_click_focus()


## Only a field that holds the focus gives it up: release_focus() clears the whole viewport's focus,
## so an unconditional one would take away what the section activated a line earlier just grabbed.
func release_section_focus() -> void:
    super()
    if %Search.has_focus():
        %Search.release_focus()


## Rows of the list, given as what they show: a title and the smaller grey note beside it. The first
## row is selected right away - nothing here is ever left deselected while there is a row to select.
func set_rows(titles: PackedStringArray, notes: PackedStringArray) -> void:
    for row: PanelContainer in _rows:
        row.queue_free()
    _rows.clear()
    _markers.clear()
    _row_tokens.clear()
    _titles = titles
    _notes = notes
    _selected = -1
    for index: int in _titles.size():
        var row: PanelContainer = _create_row(index)
        _rows.append(row)
        _row_tokens.append(_tokenize("%s %s" % [_titles[index], _notes[index]]))
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
        _walk(_next_visible_row(_selected, 1), navigate_down)
    elif event.is_action_pressed("ui_up", true):
        _walk(_next_visible_row(_selected, -1), navigate_up)
    elif event.is_action_pressed("ui_page_down", true):
        _go_to(_next_visible_row(_selected, PAGE_STEP))
    elif event.is_action_pressed("ui_page_up", true):
        _go_to(_next_visible_row(_selected, -PAGE_STEP))
    elif event.is_action_pressed("ui_end"):
        _go_to(_next_visible_row(_rows.size(), -1))
    elif event.is_action_pressed("ui_home"):
        _go_to(_next_visible_row(-1, 1))
    elif event.is_action_pressed("ui_right"):
        navigate_right.emit()
    elif event.is_action_pressed("ui_left"):
        navigate_left.emit()
    else:
        super(event)
        return
    get_viewport().set_input_as_handled()


## One step of the keyboard: the row it lands on, or the edge it went over - the search having left
## no visible row that way counts as the edge too.
func _walk(index: int, over_the_edge: Signal) -> void:
    if index < 0:
        over_the_edge.emit()
        return
    _go_to(index)


## The row a key asked for, under the keyboard's own sound. -1 is the search having left no row that
## way, and standing still costs nothing - a reselected row would have the screen rebuild everything
## that hangs off it.
func _go_to(index: int) -> void:
    if index < 0 or index == _selected:
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
        _rows[_selected].theme_type_variation = ITEM_IDLE
        _markers[_selected].visible = false
    _selected = index
    if index >= 0:
        _rows[index].theme_type_variation = ITEM_SELECTED
        _markers[index].visible = focused
    item_selected.emit(index)


## One row: its name and, on the right, a smaller grey note
func _create_row(index: int) -> PanelContainer:
    var row := PanelContainer.new()
    row.theme_type_variation = ITEM_IDLE
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
        _ui_sounds.play(click_event)
        focus_requested.emit()
        _select(index)
        activated.emit()


func _on_row_hovered(index: int, hovered: bool) -> void:
    if index == _selected:
        return
    if hovered:
        _ui_sounds.play(hover_event)
    _rows[index].theme_type_variation = ITEM_HOVERED if hovered else ITEM_IDLE


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
    %Search.grab_click_focus()


## Rows that carry every token of the search, and carry each one as a fragment: "krak tarn" finds
## "Krakow - Tarnow" whichever order the two are typed in, and neither the dash nor the spacing has
## to be guessed.
func _filter(text: String) -> void:
    var needles: PackedStringArray = _tokenize(text)
    for index: int in _rows.size():
        _rows[index].visible = _row_carries(index, needles)


## Every needle long enough to mean something has to sit inside one of the row's own tokens - AND
## over the needles, a fragment match over each
func _row_carries(index: int, needles: PackedStringArray) -> bool:
    for needle: String in needles:
        if needle.length() < MIN_FRAGMENT:
            continue
        var carried: bool = false
        for token: String in _row_tokens[index]:
            if token.contains(needle):
                carried = true
                break
        if not carried:
            return false
    return true


## The searchable words of a text: lower case, letters and digits only
func _tokenize(text: String) -> PackedStringArray:
    var tokens: PackedStringArray = []
    for found: RegExMatch in _tokenizer.search_all(text.to_lower()):
        tokens.append(found.get_string())
    return tokens
