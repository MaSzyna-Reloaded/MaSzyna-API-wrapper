class_name TileGrid
extends FocusSection

## Tiles of vehicle side views: the vehicles of a trainset in one scrolling row, or the skins of a
## vehicle over as many rows as they need. One component for both - they differ in the layout, the
## tile size and the sounds, and in nothing else.
##
## The owner keeps its own data and hands over one Tile per side view; what an activated tile means
## is its decision too.

## The selection moved, by key or by pointer - the listener asks get_selected()
signal item_selected
## A tile was clicked, and is selected now. What a click means is the owner's: the vehicles of a
## trainset open on it, as on Enter (activated); a skin is only tried on
signal item_clicked
## Ctrl+Left / Ctrl+Right on the selected tile, or its buttons (tile_actions): the owner moves what
## the tile at `index` stands for that way
signal move_left_requested(index: int)
signal move_right_requested(index: int)
## Ctrl+R on the selected tile, or its button: the owner turns what it stands for round
signal flip_requested(index: int)
## A tile's buttons: the owner puts something else in its place, or takes it away
signal change_requested(index: int)
signal remove_requested(index: int)
## A tile was dragged onto another (reorderable): the owner moves what the first stands for to the
## place of the second
signal move_requested(from: int, to: int)

## A single scrolling row (the vehicles of a trainset), or as many rows as the tiles need (skins)
enum Layout { ROW, GRID }

## The bank this component plays from - a slot the screen using it fills. Events are named after
## what happened, not after the sample, and the bank has to name "keystroke" and "change_focus".
@export var sounds: SfxBank = null

## Tile under the pointer or the keyboard gets a lit background; the marked one keeps a green one
const GLOW_SHADER: Shader = preload("skin_glow.gdshader")
## A tile whose side view is still rendering shows the silhouette and a spinner instead
const PLACEHOLDER_SHADER: Shader = preload("tile_placeholder.gdshader")
## The silhouette is drawn shorter than a locomotive really is, so the placeholder stretches it by
## a third - closer to the side view that replaces it
const PLACEHOLDER_STRETCH: float = 4.0 / 3.0
const SELECTED_COLOR: Color = Color(1.0, 1.0, 1.0)
const MARKED_COLOR: Color = Color(0.35, 1.0, 0.45)

## Tiles PgUp and PgDown move over
const PAGE_STEP: int = 4
## A tile dragged to another place is drawn beside the pointer, this faint and this much smaller,
## so it does not hide where it goes
const DRAG_PREVIEW_ALPHA: float = 0.6
const DRAG_PREVIEW_SCALE: float = 0.5
const DRAG_PREVIEW_OFFSET: Vector2 = Vector2(16.0, 16.0)
## The buttons over the corner of a tile under the pointer (tile_actions), small enough to leave
## the side view seen, on a dark ground that shows them over a light one
const ACTION_BUTTON_SIZE: float = 18.0
const ACTIONS_BACKGROUND: Color = Color(0.02, 0.04, 0.08, 0.75)
const ACTIONS_PADDING: float = 2.0
const MOVE_LEFT_ICON: Texture2D = preload("arrow_left_icon.svg")
const MOVE_RIGHT_ICON: Texture2D = preload("arrow_right_icon.svg")
const FLIP_ICON: Texture2D = preload("flip_icon.svg")
const CHANGE_ICON: Texture2D = preload("folder_icon.svg")
const REMOVE_ICON: Texture2D = preload("trash_icon.svg")
## Where a dragged tile would land: an orange line in the gap between two tiles
const DROP_MARKER_COLOR: Color = Color(1.0, 0.6, 0.15)
const DROP_MARKER_WIDTH: float = 3.0


## One side view to render: the vehicle it belongs to, the skin to render it with, and what to say
## about it. An empty caption writes nothing under the tile.
class Tile:
    var data_path: String
    var file_name: String
    var skin: String
    ## The vehicle's own name, which its MMD may name as (p1)
    var vehicle_name: String
    var tooltip: String
    var caption: String
    ## The side view drawn mirrored - a vehicle that stands the other way round
    var flipped: bool = false
    ## Its button (tile_actions) can ask for it to be taken away
    var removable: bool = true

    func _init(
        p_data_path: String, p_file_name: String, p_skin: String, p_vehicle_name: String,
        p_tooltip: String = "", p_caption: String = ""
    ) -> void:
        data_path = p_data_path
        file_name = p_file_name
        skin = p_skin
        vehicle_name = p_vehicle_name
        tooltip = p_tooltip
        caption = p_caption


@export var layout: Layout = Layout.ROW
## Height of a side view. The tile is taller than that, so its background shows around it.
@export var tile_height: float = 60.0
@export var tile_padding: float = 1.35
## What the bank calls a click and a hover here - the vehicles and the skins are different gestures
@export var click_event: StringName = &"vehicle_click"
@export var hover_event: StringName = &"vehicle_hover"
## Drawn faint under the spinner while a side view is still being rendered. The data says nothing
## about how long a vehicle is, so the tile takes this picture's own shape, stretched by
## PLACEHOLDER_STRETCH.
@export var placeholder_silhouette: Texture2D = null
## Tiles can be dragged with the mouse onto the place of another (move_requested) - the vehicles of
## a trainset
@export var reorderable: bool = false
## Every tile under the pointer shows small buttons in its corner: left, right, turn round, change
## and remove - the vehicles of a trainset
@export var tile_actions: bool = false

var _tiles: Array[Tile] = []
var _controls: Array[Control] = []
var _selected: int = -1
## Tile the owner marked as the one it has open, green while it is
var _marked: int = -1
var _ui_sounds: SfxPlayer
## The row or the flow the tiles are laid out in, by the layout
var _container: Container = null
## The line in the gap a dragged tile would land in (reorderable), on the tile beside that gap
var _drop_marker: ColorRect = null


func _ready() -> void:
    super()
    _ui_sounds = SfxPlayer.new()
    _ui_sounds.bank = sounds
    add_child(_ui_sounds)
    focus_taken.connect(_ui_sounds.play.bind(&"change_focus"))
    _container = HBoxContainer.new() if layout == Layout.ROW else HFlowContainer.new()
    _container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    # the vehicles of a trainset meet buffer to buffer, as they stand on the track
    _container.add_theme_constant_override("separation", 0)
    _container.add_theme_constant_override("h_separation", 10)
    _container.add_theme_constant_override("v_separation", 10)
    # a tile's place is known once the container has laid it out - after new tiles, and after a
    # side view has given its tile its own width
    _container.sort_children.connect(_scroll_to_selected)
    %Scroll.add_child(_container)
    %Scroll.vertical_scroll_mode = (
        ScrollContainer.SCROLL_MODE_DISABLED if layout == Layout.ROW
        else ScrollContainer.SCROLL_MODE_AUTO
    )
    %Scroll.horizontal_scroll_mode = (
        ScrollContainer.SCROLL_MODE_AUTO if layout == Layout.ROW
        else ScrollContainer.SCROLL_MODE_DISABLED
    )
    if reorderable:
        _drop_marker = ColorRect.new()
        _drop_marker.color = DROP_MARKER_COLOR
        _drop_marker.mouse_filter = Control.MOUSE_FILTER_IGNORE
        _drop_marker.visible = false
        add_child(_drop_marker)


## The drag is over, dropped or not - the line goes, back to the grid, as the tile it stood on may
## be freed with the next tiles
func _notification(what: int) -> void:
    if what == NOTIFICATION_DRAG_END and _drop_marker:
        _drop_marker.visible = false
        _drop_marker.reparent(self, false)


## The side views to show, in the order they belong in. The first tile is selected right away, and
## nothing to show takes the whole grid off the screen.
func set_tiles(tiles: Array[Tile]) -> void:
    for control: Control in _controls:
        control.queue_free()
    _controls.clear()
    _tiles = tiles
    _selected = -1
    _marked = -1
    visible = _tiles.size() > 0
    for index: int in _tiles.size():
        var control: Control = _create_tile(index)
        _controls.append(control)
        _container.add_child(control)
    %Scroll.scroll_horizontal = 0
    %Scroll.scroll_vertical = 0
    if _controls:
        _select(0)


## Tile the selection is on, or -1 while the grid is empty
func get_selected() -> int:
    return _selected


## Tile the owner has open elsewhere - it keeps a green background; -1 says none has
func set_marked(index: int) -> void:
    _paint(_marked, Color.TRANSPARENT)
    _marked = index
    _paint(index, MARKED_COLOR)
    _paint(_selected, SELECTED_COLOR if not _selected == index else MARKED_COLOR)


## The tile at `from` goes to the place `to` as it is - its side view, the scroll and the selection
## stay; the tiles between close up
func move_tile(from: int, to: int) -> void:
    _tiles.insert(to, _tiles.pop_at(from))
    _controls.insert(to, _controls.pop_at(from))
    _container.move_child(_controls[to], to)
    _selected = _moved_index(_selected, from, to)
    _marked = _moved_index(_marked, from, to)


static func _moved_index(index: int, from: int, to: int) -> int:
    if index == from:
        return to
    if from < index and index <= to:
        return index - 1
    if to <= index and index < from:
        return index + 1
    return index


## The side view of a tile mirrored or not, after the owner turned what it stands for round
func set_tile_flipped(index: int, flipped: bool) -> void:
    _tiles[index].flipped = flipped
    (_controls[index].get_node("Preview") as TextureButton).flip_h = flipped


## The side view of a tile again, after the owner gave it another skin
func reload_tile(index: int, skin: String) -> void:
    _tiles[index].skin = skin
    _load_profile(_controls[index].get_node("Preview") as TextureButton, _tiles[index])


## The keys of the grid, taken before the viewport can turn them into focus navigation of its own,
## and only while this section has the focus. Left and right walk the tiles, up and down the rows
## of a grid, and a step that would leave the tiles leaves the section instead.
func _input(event: InputEvent) -> void:
    if not focused or not is_visible_in_tree():
        return
    if event.is_action_pressed("menu_right", true, true):
        _walk(_tile_in_row(1), navigate_right)
    elif event.is_action_pressed("menu_left", true, true):
        _walk(_tile_in_row(-1), navigate_left)
    elif event.is_action_pressed("menu_down", true, true):
        _walk(_tile_in_next_row(1), navigate_down)
    elif event.is_action_pressed("menu_up", true, true):
        _walk(_tile_in_next_row(-1), navigate_up)
    elif event.is_action_pressed("menu_page_down", true, true):
        _go_to(_selected + PAGE_STEP)
    elif event.is_action_pressed("menu_page_up", true, true):
        _go_to(_selected - PAGE_STEP)
    elif event.is_action_pressed("menu_end", false, true):
        _go_to(_controls.size() - 1)
    elif event.is_action_pressed("menu_home", false, true):
        _go_to(0)
    elif event.is_action_pressed("menu_move_left", true, true):
        move_left_requested.emit(_selected)
    elif event.is_action_pressed("menu_move_right", true, true):
        move_right_requested.emit(_selected)
    elif event.is_action_pressed("menu_reverse", false, true):
        flip_requested.emit(_selected)
    else:
        super(event)
        return
    get_viewport().set_input_as_handled()


## One step of the keyboard: the tile it lands on, or the edge it went over. A single row has no
## row above or below it, so _tiles_per_row() is the whole grid there and up or down always leave.
func _walk(index: int, over_the_edge: Signal) -> void:
    if index < 0 or index >= _controls.size():
        over_the_edge.emit()
        return
    _go_to(index)


## The tile beside the selected one, inside its own row. A step sideways never drops into another
## row - at the edge of the row it leaves the section instead, and -1 says so.
func _tile_in_row(step: int) -> int:
    if _selected < 0:
        return -1
    var index: int = _selected + step
    if index < 0 or index >= _controls.size():
        return -1
    if not _controls[index].position.y == _controls[_selected].position.y:
        return -1
    return index


## The tile a step up or down lands on, read from where the tiles actually ended up: the nearest row
## in that direction, and in it the tile whose centre is closest to the selected one's. A flow
## container packs a different number of tiles into every row, so a fixed step would drift out of
## the column; a single row has no row above or below it at all, and -1 says so.
func _tile_in_next_row(step: int) -> int:
    if _selected < 0:
        return -1
    var current: Control = _controls[_selected]
    var row_found: bool = false
    var row_y: float = 0.0
    for control: Control in _controls:
        # step * distance is positive only for the tiles on the side the arrow points at
        if step * (control.position.y - current.position.y) <= 0.0:
            continue
        if not row_found or absf(control.position.y - current.position.y) < absf(row_y - current.position.y):
            row_found = true
            row_y = control.position.y
    if not row_found:
        return -1
    var centre: float = current.position.x + current.size.x * 0.5
    var nearest: int = -1
    var nearest_distance: float = 0.0
    for index: int in _controls.size():
        if not _controls[index].position.y == row_y:
            continue
        var distance: float = absf(
            _controls[index].position.x + _controls[index].size.x * 0.5 - centre
        )
        if nearest < 0 or distance < nearest_distance:
            nearest = index
            nearest_distance = distance
    return nearest


## The tile a key asked for, clamped to the grid and silent when it is the one already selected
func _go_to(index: int) -> void:
    if not _controls:
        return
    var clamped: int = clampi(index, 0, _controls.size() - 1)
    if clamped == _selected:
        return
    _ui_sounds.play(&"keystroke")
    select(clamped)


## The selection moved by the owner, silently, and scrolled into view
func select(index: int) -> void:
    _select(index)
    _scroll_to_selected()


## The selected tile scrolled into view, from where the container last laid it out
func _scroll_to_selected() -> void:
    if _selected < 0:
        return
    if layout == Layout.ROW:
        scroll_to_item_in_row(%Scroll, _controls[_selected])
    else:
        scroll_to_item(%Scroll, _controls[_selected])


func _select(index: int) -> void:
    _paint(_selected, Color.TRANSPARENT if not _selected == _marked else MARKED_COLOR)
    _selected = index
    _paint(index, SELECTED_COLOR if not index == _marked else MARKED_COLOR)
    item_selected.emit()


## The tile: its background glow, the side view over it and, when the owner named it, a caption
func _create_tile(index: int) -> Control:
    var tile: Tile = _tiles[index]
    var control := Control.new()
    control.custom_minimum_size = Vector2(tile_height, tile_height * tile_padding)
    if placeholder_silhouette:
        control.custom_minimum_size.x = tile_height * PLACEHOLDER_STRETCH * (
            float(placeholder_silhouette.get_width()) / float(placeholder_silhouette.get_height())
        )

    var background := ColorRect.new()
    background.name = "Background"
    background.set_anchors_preset(Control.PRESET_FULL_RECT)
    background.mouse_filter = Control.MOUSE_FILTER_IGNORE
    background.material = ShaderMaterial.new()
    (background.material as ShaderMaterial).shader = GLOW_SHADER
    control.add_child(background)

    var preview := TextureButton.new()
    preview.name = "Preview"
    preview.ignore_texture_size = true
    preview.stretch_mode = TextureButton.STRETCH_KEEP_ASPECT_CENTERED
    preview.set_anchors_preset(Control.PRESET_FULL_RECT)
    # the profiles are small, linear filtering turns them into a blur when scaled up
    preview.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
    preview.tooltip_text = tile.tooltip
    preview.flip_h = tile.flipped
    preview.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
    # bound to the tile and not to its index - a moved tile keeps its signals (move_tile())
    preview.pressed.connect(_on_tile_pressed.bind(control))
    if reorderable:
        preview.set_drag_forwarding(
            _get_tile_drag_data.bind(control), _can_drop_tile.bind(control), _drop_tile.bind(control)
        )
    preview.mouse_entered.connect(_on_tile_hovered.bind(control))
    control.add_child(preview)
    if placeholder_silhouette:
        control.add_child(_create_placeholder(control.custom_minimum_size))
    _load_profile(preview, tile)
    if tile_actions:
        # the tile itself hears nothing of the pointer - its side view and its buttons over it do,
        # so they show the buttons, and any of them left checks whether the pointer left the tile
        var actions: PanelContainer = _create_actions(control, tile)
        control.add_child(actions)
        preview.mouse_entered.connect(actions.show)
        preview.mouse_exited.connect(_hide_actions_off_tile.bind(control, actions))
        actions.mouse_exited.connect(_hide_actions_off_tile.bind(control, actions))
        for button: Node in actions.get_child(0).get_children():
            (button as Control).mouse_exited.connect(_hide_actions_off_tile.bind(control, actions))
    if not tile.caption:
        return control

    # the tile is taller than the side view, so the caption goes in that room and needs no wrapper
    var label := Label.new()
    label.text = tile.caption
    label.set_anchors_preset(Control.PRESET_BOTTOM_WIDE)
    label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    label.vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM
    label.mouse_filter = Control.MOUSE_FILTER_IGNORE
    label.clip_text = true
    label.add_theme_font_size_override("font_size", 13)
    control.add_child(label)
    return control


## The tile's buttons in its bottom right corner, hidden until the pointer is over the tile. Each
## asks with the tile's index as it stands when pressed - a moved tile keeps its buttons.
func _create_actions(control: Control, tile: Tile) -> PanelContainer:
    var actions := PanelContainer.new()
    var background := StyleBoxFlat.new()
    background.bg_color = ACTIONS_BACKGROUND
    background.set_corner_radius_all(int(ACTIONS_PADDING * 2.0))
    background.set_content_margin_all(ACTIONS_PADDING)
    actions.add_theme_stylebox_override("panel", background)
    actions.set_anchors_and_offsets_preset(Control.PRESET_BOTTOM_RIGHT, Control.PRESET_MODE_MINSIZE)
    actions.grow_horizontal = Control.GROW_DIRECTION_BEGIN
    actions.grow_vertical = Control.GROW_DIRECTION_BEGIN
    actions.visible = false
    var row := HBoxContainer.new()
    row.add_theme_constant_override("separation", 0)
    actions.add_child(row)
    for action: Array in [
        [MOVE_LEFT_ICON, "Move left", move_left_requested],
        [MOVE_RIGHT_ICON, "Move right", move_right_requested],
        [FLIP_ICON, "Turn round", flip_requested],
        [CHANGE_ICON, "Change vehicle", change_requested],
        [REMOVE_ICON, "Remove vehicle", remove_requested],
    ]:
        var button := UIIconButton.new()
        button.icon = action[0]
        button.tooltip_text = action[1]
        button.expand_icon = true
        button.custom_minimum_size = Vector2(ACTION_BUTTON_SIZE, ACTION_BUTTON_SIZE)
        button.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
        button.disabled = action[2] == remove_requested and not tile.removable
        button.pressed.connect(_emit_tile_action.bind(action[2], control, actions))
        row.add_child(button)
    return actions


## The pointer left the side view or a button - the buttons go once it is off the tile
func _hide_actions_off_tile(control: Control, actions: Control) -> void:
    if not control.get_global_rect().has_point(control.get_global_mouse_position()):
        actions.hide()


## A pressed button takes the buttons away - what it asks for may cover the grid (a window) or move
## the tile from under the pointer, and the pointer leaving is then never heard
func _emit_tile_action(request: Signal, control: Control, actions: Control) -> void:
    _ui_sounds.play(click_event)
    actions.hide()
    request.emit(_controls.find(control))


## The silhouette and the spinner of a tile that has nothing to show yet
func _create_placeholder(size: Vector2) -> ColorRect:
    var placeholder := ColorRect.new()
    placeholder.name = "Placeholder"
    placeholder.set_anchors_preset(Control.PRESET_FULL_RECT)
    placeholder.mouse_filter = Control.MOUSE_FILTER_IGNORE
    placeholder.material = ShaderMaterial.new()
    var material: ShaderMaterial = placeholder.material as ShaderMaterial
    material.shader = PLACEHOLDER_SHADER
    material.set_shader_parameter("silhouette", placeholder_silhouette)
    material.set_shader_parameter("rect_size", size)
    return placeholder


## Side view from MaszynaVehicleProfileManager (rendered on the spot when the data has no image of it)
func _load_profile(preview: TextureButton, tile: Tile) -> void:
    # nothing of the button is drawn while it has no texture - the placeholder is what shows
    var profile: Texture2D = await MaszynaVehicleProfileManager.get_profile(
        tile.data_path, tile.file_name, tile.skin, tile.vehicle_name
    )
    if not is_instance_valid(preview):
        return
    if not profile:
        return
    var placeholder: Node = preview.get_parent().get_node_or_null("Placeholder")
    if placeholder:
        placeholder.queue_free()
    preview.texture_normal = profile
    var control: Control = preview.get_parent() as Control
    # as wide as the side view is long
    var tile_size: Vector2 = Vector2(
        tile_height * float(profile.get_width()) / float(profile.get_height()),
        tile_height * tile_padding
    )
    # a vehicle of a trainset takes its length over the buffers, and its side view overhangs that
    # onto its neighbours when it is longer - the shared bogies of an articulated unit
    var coupling_width: float = (
        MaszynaVehicleProfileManager.get_profile_coupling_width(tile.data_path, tile.file_name)
        if layout == Layout.ROW else 0.0
    )
    if coupling_width > 0.0:
        var overhang: float = (tile_size.x - coupling_width * tile_height / float(profile.get_height())) * 0.5
        tile_size.x -= 2.0 * overhang
        preview.offset_left = -overhang
        preview.offset_right = overhang
    control.custom_minimum_size = tile_size
    var background: ColorRect = control.get_node("Background") as ColorRect
    (background.material as ShaderMaterial).set_shader_parameter("rect_size", tile_size)


func _on_tile_pressed(control: Control) -> void:
    _ui_sounds.play(click_event)
    focus_requested.emit()
    _select(_controls.find(control))
    item_clicked.emit()


## The dragged tile: its index, and which grid it came from - a tile of another grid is no place of
## this one. The picture beside the pointer is a small faint copy of its side view.
func _get_tile_drag_data(_at_position: Vector2, control: Control) -> Dictionary:
    var ghost: Control = Control.new()
    var picture: TextureRect = TextureRect.new()
    picture.texture = (control.get_node("Preview") as TextureButton).texture_normal
    picture.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
    picture.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
    picture.position = DRAG_PREVIEW_OFFSET
    picture.size = control.size * DRAG_PREVIEW_SCALE
    picture.modulate.a = DRAG_PREVIEW_ALPHA
    ghost.add_child(picture)
    set_drag_preview(ghost)
    return {"grid": get_instance_id(), "index": _controls.find(control)}


## A tile of this grid over a tile: the line goes into the gap on the side of it the pointer is on
func _can_drop_tile(at_position: Vector2, data: Variant, control: Control) -> bool:
    if not (data is Dictionary and data.get("grid") == get_instance_id()):
        return false
    var after: bool = _is_after(at_position, control)
    _drop_marker.reparent(control, false)
    _drop_marker.position = Vector2(
        (control.size.x if after else 0.0) - DROP_MARKER_WIDTH * 0.5, 0.0
    )
    _drop_marker.size = Vector2(DROP_MARKER_WIDTH, control.size.y)
    _drop_marker.visible = true
    return true


## Dropped into a gap: the dragged tile's new place counts without the tile itself
func _drop_tile(at_position: Vector2, data: Variant, control: Control) -> void:
    var from: int = data["index"]
    var gap: int = _controls.find(control) + (1 if _is_after(at_position, control) else 0)
    var to: int = gap if gap <= from else gap - 1
    if to == from:
        return
    _ui_sounds.play(click_event)
    focus_requested.emit()
    move_requested.emit(from, to)


## The pointer is over the half of a tile after its middle - the gap after it
func _is_after(at_position: Vector2, control: Control) -> bool:
    return at_position.x >= (control.get_node("Preview") as Control).size.x * 0.5


func _on_tile_hovered(control: Control) -> void:
    if _controls.find(control) == _selected:
        return
    _ui_sounds.play(hover_event)


func _paint(index: int, color: Color) -> void:
    if index < 0 or index >= _controls.size():
        return
    var control: Control = _controls[index]
    var background: ColorRect = control.get_node("Background") as ColorRect
    var material: ShaderMaterial = background.material as ShaderMaterial
    material.set_shader_parameter("glow_color", color)
    material.set_shader_parameter("glow_strength", 0.0 if color == Color.TRANSPARENT else 1.0)
