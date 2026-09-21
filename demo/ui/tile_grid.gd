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


## One side view to render: the vehicle it belongs to, the skin to render it with, and what to say
## about it. An empty caption writes nothing under the tile.
class Tile:
    var data_path: String
    var file_name: String
    var skin: String
    var tooltip: String
    var caption: String

    func _init(
        p_data_path: String, p_file_name: String, p_skin: String,
        p_tooltip: String = "", p_caption: String = ""
    ) -> void:
        data_path = p_data_path
        file_name = p_file_name
        skin = p_skin
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

var _tiles: Array[Tile] = []
var _controls: Array[Control] = []
var _selected: int = -1
## Tile the owner marked as the one it has open, green while it is
var _marked: int = -1
var _ui_sounds: SfxPlayer
## The row or the flow the tiles are laid out in, by the layout
var _container: Container = null


func _ready() -> void:
    super()
    _ui_sounds = SfxPlayer.new()
    _ui_sounds.bank = sounds
    add_child(_ui_sounds)
    focus_taken.connect(_ui_sounds.play.bind(&"change_focus"))
    _container = HBoxContainer.new() if layout == Layout.ROW else HFlowContainer.new()
    _container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    _container.add_theme_constant_override("separation", 8)
    _container.add_theme_constant_override("h_separation", 10)
    _container.add_theme_constant_override("v_separation", 10)
    %Scroll.add_child(_container)
    %Scroll.vertical_scroll_mode = (
        ScrollContainer.SCROLL_MODE_DISABLED if layout == Layout.ROW
        else ScrollContainer.SCROLL_MODE_AUTO
    )
    %Scroll.horizontal_scroll_mode = (
        ScrollContainer.SCROLL_MODE_AUTO if layout == Layout.ROW
        else ScrollContainer.SCROLL_MODE_DISABLED
    )


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
    if event.is_action_pressed("ui_right", true):
        _walk(_tile_in_row(1), navigate_right)
    elif event.is_action_pressed("ui_left", true):
        _walk(_tile_in_row(-1), navigate_left)
    elif event.is_action_pressed("ui_down", true):
        _walk(_tile_in_next_row(1), navigate_down)
    elif event.is_action_pressed("ui_up", true):
        _walk(_tile_in_next_row(-1), navigate_up)
    elif event.is_action_pressed("ui_page_down", true):
        _go_to(_selected + PAGE_STEP)
    elif event.is_action_pressed("ui_page_up", true):
        _go_to(_selected - PAGE_STEP)
    elif event.is_action_pressed("ui_end"):
        _go_to(_controls.size() - 1)
    elif event.is_action_pressed("ui_home"):
        _go_to(0)
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
    _select(clamped)
    if layout == Layout.ROW:
        scroll_to_item_in_row(%Scroll, _controls[clamped])
    else:
        scroll_to_item(%Scroll, _controls[clamped])


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
    preview.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
    preview.pressed.connect(_on_tile_pressed.bind(index))
    preview.mouse_entered.connect(_on_tile_hovered.bind(index))
    control.add_child(preview)
    if placeholder_silhouette:
        control.add_child(_create_placeholder(control.custom_minimum_size))
    _load_profile(preview, tile)
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


## Side view from VehicleProfileManager (rendered on the spot when the data has no image of it)
func _load_profile(preview: TextureButton, tile: Tile) -> void:
    # nothing of the button is drawn while it has no texture - the placeholder is what shows
    var profile: Texture2D = await VehicleProfileManager.get_profile(
        tile.data_path, tile.file_name, tile.skin
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
    # as wide as the vehicle is long, so the vehicles of a trainset line up without gaps
    var tile_size: Vector2 = Vector2(
        tile_height * float(profile.get_width()) / float(profile.get_height()),
        tile_height * tile_padding
    )
    control.custom_minimum_size = tile_size
    var background: ColorRect = control.get_node("Background") as ColorRect
    (background.material as ShaderMaterial).set_shader_parameter("rect_size", tile_size)


func _on_tile_pressed(index: int) -> void:
    _ui_sounds.play(click_event)
    focus_requested.emit()
    _select(index)
    activated.emit()


func _on_tile_hovered(index: int) -> void:
    if index == _selected:
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
