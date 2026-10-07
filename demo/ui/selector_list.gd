class_name SelectorList
extends FocusSection

## One list section of the selector screen: the rows, the search that filters them, the selection
## with its blinking marker, the keyboard walking over them and the sounds of all of it. Both lists
## of the screen are this scene, so they look and behave the same and only their content differs.
##
## The screen supplies the rows, reads the selection and decides what an activated row means. Left
## and right are the screen's bindings, not the list's - they leave as navigate_left/right.

## The selection moved, by key, by search or by click. -1 when it is on a group header, or when the
## search matched nothing - the only states with no row selected.
signal item_selected(index: int)
## The refresh button of the search field - the screen reads its rows again and gives them anew
signal refresh_requested

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
## Room between the marker, the folder, the title and the note of a row
const LINE_SEPARATION: int = 16
## Between a group's name and what tells its row apart ("Bałtyk - SKM1")
const GROUP_TITLE_SEPARATOR: String = " - "
## A group header's folder, folded and unfolded
const GROUP_FOLDED: Texture2D = preload("folder_icon.svg")
const GROUP_UNFOLDED: Texture2D = preload("folder_open_icon.svg")
const GROUP_ICON_SIZE: Vector2 = Vector2(18.0, 18.0)
## The folder's tint and its group's name's - the search field's icon's (selector_list.tscn)
const GROUP_ICON_COLOR: Color = Color(0.7, 0.82, 1, 0.75)
## Room between a header's folder and its name - closer than the parts of a row, they read as one
const GROUP_ICON_SEPARATION: int = 6

## What counts as a word in a search: letters and digits, so whitespace, dashes, brackets and
## punctuation all separate tokens and none of them ever has to be typed
const TOKEN_PATTERN: String = "[\\p{L}\\p{N}]+"
## A typed token shorter than this filters nothing - one letter would only throw rows away
const MIN_FRAGMENT: int = 2

## set_rows() without a row of the screen's choice: the first visible slot is selected
const FIRST_VISIBLE_ROW: int = -1

## Lists without a search field keep the whole row for their content. Read once, in _ready().
@export var searchable: bool = false
## The search field carries a refresh button, heard as refresh_requested. Read once, in _ready().
@export var refreshable: bool = false
## What joins the parts of a title ("Bałtyk · SKM1"), so a row of a group can show only the parts
## that tell it apart from the other rows of its group; empty when titles have no parts
@export var title_separator: String = ""
## What the bank calls a click and a hover on a row
@export var click_event: StringName = &"list_item_click"
@export var hover_event: StringName = &"list_item_hover"

var _titles: PackedStringArray = []
var _notes: PackedStringArray = []
## Everything a row can be found by, tokenized once when the rows are set, in the order of the rows
var _row_tokens: Array[PackedStringArray] = []
var _tokenizer: RegEx = RegEx.create_from_string(TOKEN_PATTERN)
## Every group header and row, in the order they stand in the list - what the keyboard walks
var _slots: Array[PanelContainer] = []
## Marker of each slot, shown on the selected one
var _markers: Array[ColorRect] = []
## Row of each slot, -1 for a group header
var _slot_rows: PackedInt32Array = []
## Group of each slot - the one it heads or belongs to, -1 for a row outside any group
var _slot_groups: PackedInt32Array = []
## Rows of each group, its header's slot, folder icon and count, and whether it is unfolded
var _group_rows: Array[PackedInt32Array] = []
var _group_slots: PackedInt32Array = []
var _group_icons: Array[TextureRect] = []
var _group_counts: Array[Label] = []
var _group_unfolded: Array[bool] = []
## Slot the selection is on, -1 for none
var _selected: int = -1
var _ui_sounds: SfxPlayer


func _ready() -> void:
    super()
    _ui_sounds = SfxPlayer.new()
    _ui_sounds.bank = sounds
    add_child(_ui_sounds)
    focus_taken.connect(_ui_sounds.play.bind(&"change_focus"))
    %SearchInput.visible = searchable
    if refreshable:
        %SearchInput.show_refresh_button()


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
        %SearchInput.grab_typing_focus()


func release_section_focus() -> void:
    super()
    %SearchInput.release_typing_focus()


## Rows of the list, given as what they show: a title and the smaller grey note beside it, and
## optionally the group of each row. Rows of one group stand together under a folded header, in the
## order the groups first appear; a group of a single row is no group. The typed search stays and
## filters the new rows. selected_row is selected right away - its group unfolded, the list scrolled
## to it - as the screen's own choice (the game directory in use, the scenery chosen before a
## refresh); without one, or when it is not on the list, the first visible slot is - nothing here is
## ever left deselected while there is something to select.
func set_rows(
    titles: PackedStringArray, notes: PackedStringArray, groups: PackedStringArray = PackedStringArray(),
    selected_row: int = FIRST_VISIBLE_ROW
) -> void:
    for slot: PanelContainer in _slots:
        slot.queue_free()
    _slots.clear()
    _markers.clear()
    _slot_rows.clear()
    _slot_groups.clear()
    _group_rows.clear()
    _group_slots.clear()
    _group_icons.clear()
    _group_counts.clear()
    _group_unfolded.clear()
    _row_tokens.clear()
    _titles = titles
    _notes = notes
    _selected = -1
    for index: int in _titles.size():
        var group_name: String = groups[index] if groups else ""
        _row_tokens.append(_tokenize("%s %s %s" % [group_name, _titles[index], _notes[index]]))

    # the rows of every group name, in the order the names first appear
    var names: PackedStringArray = []
    var members: Array[PackedInt32Array] = []
    for index: int in _titles.size():
        var group_name: String = groups[index] if groups else ""
        var at: int = names.find(group_name) if group_name else -1
        if at < 0:
            names.append(group_name)
            members.append(PackedInt32Array())
            at = members.size() - 1
        members[at].append(index)
    # with any group on the list, every title starts after a folder's room, so all of them line up
    # and only the headers' folders stand out on the left
    var grouped: bool = false
    for rows: PackedInt32Array in members:
        grouped = grouped or rows.size() > 1
    for at: int in names.size():
        if members[at].size() < 2:
            for index: int in members[at]:
                _add_slot(_create_line(_titles[index], _notes[index], _create_folder() if grouped else null), index, -1)
            continue
        var group: int = _group_rows.size()
        _group_rows.append(members[at])
        _group_unfolded.append(false)
        _group_slots.append(_slots.size())
        _add_slot(_create_header(group, names[at]), -1, group)
        var row_titles: PackedStringArray = _get_group_row_titles(names[at], members[at])
        for member: int in members[at].size():
            var index: int = members[at][member]
            _add_slot(_create_line(row_titles[member], _notes[index], _create_folder()), index, group)

    _filter(%SearchInput.get_text() if searchable else "")
    %Scroll.scroll_vertical = 0
    # a group header's slot carries -1 as its row, so FIRST_VISIBLE_ROW is found on none
    var slot: int = _slot_rows.find(selected_row) if selected_row >= 0 else -1
    if slot < 0:
        _select(_first_result_slot())
        return
    var group: int = _slot_groups[slot]
    if group >= 0 and not _group_unfolded[group]:
        _toggle_group(group)
    _select(slot)
    scroll_to_item(%Scroll, _slots[slot])


## The note of a row anew - what its owner says about it changed; the rows and the selection stay
func set_row_note(row: int, note: String) -> void:
    _notes[row] = note
    # the note is the last label of the row's line (_create_line())
    var line: HBoxContainer = _slots[_slot_rows.find(row)].get_child(0) as HBoxContainer
    (line.get_child(line.get_child_count() - 1) as Label).text = note


## Row the selection is on, or -1 when it is on a group header or the search matched nothing
func get_selected() -> int:
    return _slot_rows[_selected] if _selected >= 0 else -1


## The keys of a list, taken before the viewport can turn them into focus navigation of its own.
## Only the focused list acts on them, and typing is left alone - it belongs to the search field.
func _input(event: InputEvent) -> void:
    if not focused or not is_visible_in_tree():
        return
    if event.is_action_pressed("menu_down", true, true):
        _walk(_next_visible_slot(_selected, 1), navigate_down)
    elif event.is_action_pressed("menu_up", true, true):
        _walk(_next_visible_slot(_selected, -1), navigate_up)
    elif event.is_action_pressed("menu_page_down", true, true):
        _go_to(_next_visible_slot(_selected, PAGE_STEP))
    elif event.is_action_pressed("menu_page_up", true, true):
        _go_to(_next_visible_slot(_selected, -PAGE_STEP))
    elif event.is_action_pressed("menu_end", false, true):
        _go_to(_next_visible_slot(_slots.size(), -1))
    elif event.is_action_pressed("menu_home", false, true):
        _go_to(_next_visible_slot(-1, 1))
    # Enter on a group header folds it - on a row it is the screen's, as activated
    elif event.is_action_pressed("menu_activate", false, true) and _selected >= 0 and _slot_rows[_selected] < 0:
        _ui_sounds.play(click_event)
        _toggle_group(_slot_groups[_selected])
    elif event.is_action_pressed("menu_right", false, true):
        navigate_right.emit()
    elif event.is_action_pressed("menu_left", false, true):
        navigate_left.emit()
    else:
        super(event)
        return
    get_viewport().set_input_as_handled()


## One step of the keyboard: the slot it lands on, or the edge it went over - the search having left
## nothing visible that way counts as the edge too.
func _walk(index: int, over_the_edge: Signal) -> void:
    if index < 0:
        over_the_edge.emit()
        return
    _go_to(index)


## The slot a key asked for, under the keyboard's own sound. -1 is the search having left nothing
## that way, and standing still costs nothing - a reselected row would have the screen rebuild
## everything that hangs off it.
func _go_to(slot: int) -> void:
    if slot < 0 or slot == _selected:
        return
    _ui_sounds.play(&"keystroke")
    _select(slot)
    scroll_to_item(%Scroll, _slots[slot])


## The slot abs(step) visible slots away from "from", or the last visible one in that direction when
## the list ends first - so a page or an End lands on the edge of the results, never outside them.
## -1 when the search left nothing visible that way at all.
func _next_visible_slot(from: int, step: int) -> int:
    var direction: int = signi(step)
    var left: int = absi(step)
    var slot: int = from
    var found: int = -1
    while left > 0:
        slot += direction
        if slot < 0 or slot >= _slots.size():
            break
        if _slots[slot].visible:
            found = slot
            left -= 1
    return found


func _select(slot: int) -> void:
    if _selected >= 0:
        _slots[_selected].theme_type_variation = ITEM_IDLE
        _markers[_selected].visible = false
    _selected = slot
    if slot >= 0:
        _slots[slot].theme_type_variation = ITEM_SELECTED
        _markers[slot].visible = focused
    item_selected.emit(get_selected())


## Folds an unfolded group and unfolds a folded one. While a search is typed, every group with a
## result stands unfolded whatever it was - the fold shows again once the search is cleared.
func _toggle_group(group: int) -> void:
    _group_unfolded[group] = not _group_unfolded[group]
    _filter(%SearchInput.get_text() if searchable else "")


func _add_slot(slot: PanelContainer, row: int, group: int) -> void:
    slot.gui_input.connect(_on_slot_gui_input.bind(_slots.size()))
    slot.mouse_entered.connect(_on_slot_hovered.bind(_slots.size(), true))
    slot.mouse_exited.connect(_on_slot_hovered.bind(_slots.size(), false))
    _slots.append(slot)
    _slot_rows.append(row)
    _slot_groups.append(group)
    %List.add_child(slot)


## A group's header: a folder and the group's name, and on the right how many rows it holds -
## _filter() opens or shuts the folder and writes the count
func _create_header(group: int, group_name: String) -> PanelContainer:
    var folder: TextureRect = _create_folder()
    var header: PanelContainer = _create_line(group_name, "", folder)
    # the line holds the marker's room, the folder with the title and the note
    var line: HBoxContainer = header.get_child(0) as HBoxContainer
    _group_counts.append(line.get_child(line.get_child_count() - 1) as Label)
    _group_icons.append(folder)
    # the group's name in its folder's colour, standing after it
    (folder.get_parent().get_child(1) as Label).add_theme_color_override("font_color", GROUP_ICON_COLOR)
    return header


## Room for a header's folder before a title - a row's stays empty
func _create_folder() -> TextureRect:
    var folder := TextureRect.new()
    folder.custom_minimum_size = GROUP_ICON_SIZE
    folder.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
    folder.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
    folder.size_flags_vertical = Control.SIZE_SHRINK_CENTER
    folder.modulate = GROUP_ICON_COLOR
    folder.mouse_filter = Control.MOUSE_FILTER_IGNORE
    return folder


## What the rows of a group show: the group's name and the parts of the title that tell the row
## apart - not the group's name again, and not the leading parts all the rows of its variant share
## ("Całkowo · Macierzewo - Wiliś - Jarkawki · Lotos" -> "Całkowo - Lotos"). A variant is what the
## first part adds to the group's name: "Całkowo V2 Towarowe · Noc" in the group "Całkowo" is a row
## of "V2 Towarowe" -> "Całkowo - V2 Towarowe · Noc".
func _get_group_row_titles(group_name: String, rows: PackedInt32Array) -> PackedStringArray:
    var separator: String = title_separator if title_separator else " "
    var variants: PackedStringArray = []
    var parts: Array[PackedStringArray] = []
    for index: int in rows:
        var title_parts: PackedStringArray = (
            _titles[index].split(title_separator, false) if title_separator else PackedStringArray([_titles[index]])
        )
        var variant: String = ""
        if title_parts[0] == group_name:
            title_parts.remove_at(0)
        elif title_parts[0].begins_with(group_name + " "):
            variant = title_parts[0].substr(group_name.length() + 1)
            title_parts.remove_at(0)
        variants.append(variant)
        parts.append(title_parts)

    var dropping: bool = true
    while dropping:
        dropping = false
        # the first part of each row, when every other row of its variant begins with it too
        for at: int in parts.size():
            var title_parts: PackedStringArray = parts[at]
            if title_parts.size() < 2:
                continue
            var shared: bool = true
            var others: int = 0
            for other: int in parts.size():
                if other == at or not variants[other] == variants[at]:
                    continue
                others += 1
                shared = shared and parts[other].size() > 1 and parts[other][0] == title_parts[0]
            if not shared or others == 0:
                continue
            # the whole variant drops it at once - a PackedStringArray comes out of an Array as a copy
            for other: int in parts.size():
                if variants[other] == variants[at]:
                    var other_parts: PackedStringArray = parts[other]
                    other_parts.remove_at(0)
                    parts[other] = other_parts
            dropping = true

    var titles: PackedStringArray = []
    for at: int in parts.size():
        var rest: PackedStringArray = parts[at]
        if variants[at]:
            rest.insert(0, variants[at])
        titles.append(group_name + GROUP_TITLE_SEPARATOR + separator.join(rest) if rest else group_name)
    return titles


## One row: its title, after the folder's room when there is one, and on the right a smaller grey
## note
func _create_line(title: String, note_text: String, folder: TextureRect) -> PanelContainer:
    var row := PanelContainer.new()
    row.theme_type_variation = ITEM_IDLE
    row.mouse_filter = Control.MOUSE_FILTER_STOP
    row.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND

    var line := HBoxContainer.new()
    line.mouse_filter = Control.MOUSE_FILTER_IGNORE
    line.add_theme_constant_override("separation", LINE_SEPARATION)
    row.add_child(line)
    _markers.append(_add_marker(line))

    var label := Label.new()
    label.text = title
    label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    label.mouse_filter = Control.MOUSE_FILTER_IGNORE
    # a long name must not push the note out of the panel
    label.clip_text = true
    label.custom_minimum_size = Vector2(120.0, 0.0)
    label.add_theme_font_size_override("font_size", 16)
    if folder:
        var folder_and_title := HBoxContainer.new()
        folder_and_title.size_flags_horizontal = Control.SIZE_EXPAND_FILL
        folder_and_title.mouse_filter = Control.MOUSE_FILTER_IGNORE
        folder_and_title.add_theme_constant_override("separation", GROUP_ICON_SEPARATION)
        folder_and_title.add_child(folder)
        folder_and_title.add_child(label)
        line.add_child(folder_and_title)
    else:
        line.add_child(label)

    var note := Label.new()
    note.text = note_text
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


## A click on a row selects and activates it, on a group header folds the group
func _on_slot_gui_input(event: InputEvent, slot: int) -> void:
    if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
        _ui_sounds.play(click_event)
        focus_requested.emit()
        _select(slot)
        if _slot_rows[slot] < 0:
            _toggle_group(_slot_groups[slot])
            return
        activated.emit()


func _on_slot_hovered(slot: int, hovered: bool) -> void:
    if slot == _selected:
        return
    if hovered:
        _ui_sounds.play(hover_event)
    _slots[slot].theme_type_variation = ITEM_HOVERED if hovered else ITEM_IDLE


func _on_search_input_typed(_text: String) -> void:
    _ui_sounds.play(&"keystroke")
    %SearchDebounce.start()


func _on_search_input_cleared() -> void:
    %SearchDebounce.start()


func _on_search_input_refresh_requested() -> void:
    _ui_sounds.play(click_event)
    refresh_requested.emit()


func _on_search_debounce_timeout() -> void:
    _filter(%SearchInput.get_text())
    # the first result takes over the selection; nothing found is the one case with none at all
    var slot: int = _first_result_slot()
    if not slot == _selected:
        _select(slot)
    # the filtered-out rows take no room, so the first result sits at the top of the list
    %Scroll.scroll_vertical = 0


## The first visible slot - but when a typed search unfolded a group, that group leads with its
## header and the result is the row under it. -1 when nothing is visible.
func _first_result_slot() -> int:
    var slot: int = _next_visible_slot(-1, 1)
    var first_row: int = _next_visible_slot(slot, 1) if slot >= 0 and _slot_rows[slot] < 0 else -1
    if first_row >= 0 and _slot_rows[first_row] >= 0 and _slot_groups[first_row] == _slot_groups[slot]:
        return first_row
    return slot


## Rows that carry every token of the search, and carry each one as a fragment: "krak tarn" finds
## "Krakow - Tarnow" whichever order the two are typed in, and neither the dash nor the spacing has
## to be guessed. A group shows while one of its rows is found; while a search is typed the groups
## with results stand unfolded and count what was found of them, without one each shows as it was
## folded.
func _filter(text: String) -> void:
    var needles: PackedStringArray = _tokenize(text)
    var searching: bool = false
    for needle: String in needles:
        searching = searching or needle.length() >= MIN_FRAGMENT
    var group_found: PackedInt32Array = []
    group_found.resize(_group_rows.size())
    for slot: int in _slots.size():
        var row: int = _slot_rows[slot]
        if row < 0:
            continue
        var group: int = _slot_groups[slot]
        var found: bool = _row_carries(row, needles)
        if group >= 0:
            group_found[group] += 1 if found else 0
            found = found and (searching or _group_unfolded[group])
        _slots[slot].visible = found
    for group: int in _group_rows.size():
        var unfolded: bool = searching or _group_unfolded[group]
        var size: int = _group_rows[group].size()
        _slots[_group_slots[group]].visible = group_found[group] > 0
        _group_icons[group].texture = GROUP_UNFOLDED if unfolded else GROUP_FOLDED
        _group_counts[group].text = "%d / %d" % [group_found[group], size] if searching else str(size)


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
