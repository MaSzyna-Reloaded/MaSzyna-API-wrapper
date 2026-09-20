extends Control

## Full screen scenario selector: <game_dir>/scenery/*.scn on the left, details of the selected
## one (MaszynaSceneryInfo) on the right, with the consists it declares. After "Wczytaj" the background dissolves into the
## loading screen below. Escape asks to quit (quit_requested).

signal scenery_selected(filename: String, train_id: String, skin_overrides: Dictionary)
## Escape - the game fades out and quits
signal quit_requested

const DISSOLVE_TIME: float = 1.0

## Height of a vehicle side view in the consist preview - twice the 30 px the images have
const VEHICLE_PREVIEW_HEIGHT: float = 60.0
const VehicleViewer = preload("res://scenery_selector/vehicle_viewer.gd")
## Vehicle under the mouse pointer gets a lit background, the same as the skins of the viewer
const VEHICLE_GLOW_SHADER: Shader = preload("res://scenery_selector/skin_glow.gdshader")
const VEHICLE_HOVER_COLOR: Color = Color(1.0, 1.0, 1.0)
## The tile is taller than the vehicle, so its background shows around it
const VEHICLE_TILE_PADDING: float = 1.35

const VEHICLE_SELECTED_COLOR: Color = Color(0.35, 1.0, 0.45)
## Look of a scenery on the list, by how lit it is - selector_theme.tres
const ITEM_VARIATIONS: Array[StringName] = [&"ListItem", &"ListItemHovered", &"ListItemSelected"]
const ITEM_STATE_IDLE: int = 0
const ITEM_STATE_HOVERED: int = 1
const ITEM_STATE_SELECTED: int = 2
## Room kept for the note on the right of a list row
const NOTE_WIDTH: float = 180.0

var _files: PackedStringArray = []
## Title of each scenery and its item on the list, in the order of _files
var _titles: PackedStringArray = []
var _items: Array[PanelContainer] = []
var _selected_index: int = -1
var _info: MaszynaSceneryInfo = null
## Items of the consist list, in the order of _info.trainsets
var _consist_items: Array[PanelContainer] = []
var _selected_consist_index: int = -1
## Vehicles of the shown consist and their tiles, in the order they run
var _vehicles: Array[MaszynaSceneryInfo.Vehicle] = []
var _vehicle_tiles: Array[Control] = []
## Vehicle whose viewer is open
var _shown_index: int = -1
## Fade of the scenery list under the viewer, killed when the other fade starts
var _list_fade_tween: Tween = null



func _ready() -> void:
    var files: PackedStringArray = DirAccess.get_files_at(UserSettings.get_maszyna_game_dir().path_join("scenery"))
    files.sort()
    for file: String in files:
        # "$" files are not scenarios to start (e.g. $stary_jawor_eszelon.scn)
        if not file.get_extension().to_lower() == "scn" or file.begins_with("$"):
            continue
        _files.append(file)
        _titles.append(MaszynaSceneryInfo.read_display_name(file))
    for index: int in _files.size():
        var item: PanelContainer = _create_list_item(
            index,
            _titles[index],
            _files[index].get_basename().to_upper(),
            _select_scenery.bind(index),
            _on_list_item_hovered,
        )
        _items.append(item)
        %List.add_child(item)
    _show_details(-1)
    %BuildLabel.text = "Pre-Alpha Demo Release %s (build %s)" % [
        ProjectSettings.get_setting("application/config/version"), _build_number()
    ]


## Stamped by the build itself (cmake/write_build_number.cmake), so it names the library that is
## actually loaded. A checkout that was never built has no stamp.
func _build_number() -> String:
    var stamp:String = FileAccess.get_file_as_string("res://build_number.txt").strip_edges()
    return stamp if stamp else "unbuilt"


## One row of a list: its name and, on the right, a smaller grey note
func _create_list_item(
    index: int, text: String, note: String, on_clicked: Callable, on_hovered: Callable
) -> PanelContainer:
    var item := PanelContainer.new()
    item.theme_type_variation = ITEM_VARIATIONS[ITEM_STATE_IDLE]
    item.mouse_filter = Control.MOUSE_FILTER_STOP
    item.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
    item.gui_input.connect(_on_list_item_gui_input.bind(on_clicked))
    item.mouse_entered.connect(on_hovered.bind(index, true))
    item.mouse_exited.connect(on_hovered.bind(index, false))

    var row := HBoxContainer.new()
    row.mouse_filter = Control.MOUSE_FILTER_IGNORE
    row.add_theme_constant_override("separation", 16)
    item.add_child(row)

    var label := Label.new()
    label.text = text
    label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    label.mouse_filter = Control.MOUSE_FILTER_IGNORE
    # a long name must not push the note out of the panel
    label.clip_text = true
    label.custom_minimum_size = Vector2(120.0, 0.0)
    label.add_theme_font_size_override("font_size", 16)
    row.add_child(label)

    var note_label := Label.new()
    note_label.text = note
    note_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
    note_label.vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM
    note_label.custom_minimum_size = Vector2(NOTE_WIDTH, 0.0)
    note_label.clip_text = true
    note_label.mouse_filter = Control.MOUSE_FILTER_IGNORE
    note_label.add_theme_font_size_override("font_size", 12)
    note_label.add_theme_color_override("font_color", Color(0.72, 0.76, 0.82, 0.65))
    row.add_child(note_label)
    return item


func _on_list_item_gui_input(event: InputEvent, on_clicked: Callable) -> void:
    if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
        on_clicked.call()


func _on_list_item_hovered(index: int, hovered: bool) -> void:
    if index == _selected_index:
        return
    _items[index].theme_type_variation = ITEM_VARIATIONS[ITEM_STATE_HOVERED if hovered else ITEM_STATE_IDLE]


func _on_consist_item_hovered(index: int, hovered: bool) -> void:
    if index == _selected_consist_index:
        return
    _consist_items[index].theme_type_variation = ITEM_VARIATIONS[
        ITEM_STATE_HOVERED if hovered else ITEM_STATE_IDLE
    ]


func _select_consist(index: int) -> void:
    if _selected_consist_index >= 0:
        _consist_items[_selected_consist_index].theme_type_variation = ITEM_VARIATIONS[ITEM_STATE_IDLE]
    _selected_consist_index = index
    _consist_items[index].theme_type_variation = ITEM_VARIATIONS[ITEM_STATE_SELECTED]
    _show_consist(index)


func _select_scenery(index: int) -> void:
    if _selected_index >= 0:
        _items[_selected_index].theme_type_variation = ITEM_VARIATIONS[ITEM_STATE_IDLE]
    _selected_index = index
    _items[index].theme_type_variation = ITEM_VARIATIONS[ITEM_STATE_SELECTED]
    _show_details(index)


## Sceneries whose title or file name contain the searched text
func _filter_list(text: String) -> void:
    var needle: String = text.strip_edges().to_lower()
    for index: int in _items.size():
        _items[index].visible = (
            not needle
            or _titles[index].to_lower().contains(needle)
            or _files[index].to_lower().contains(needle)
        )


func _on_search_text_changed(text: String) -> void:
    %ClearSearch.visible = not text.is_empty()
    %SearchDebounce.start()


func _on_search_debounce_timeout() -> void:
    _filter_list(%Search.text)


func _on_clear_search_pressed() -> void:
    %Search.text = ""
    %ClearSearch.visible = false
    _filter_list("")
    %Search.grab_focus()


func open() -> void:
    (%Background.material as ShaderMaterial).set_shader_parameter("dissolve", 0.0)
    %Content.visible = true
    visible = true
    %Search.grab_focus()


func _unhandled_input(event: InputEvent) -> void:
    if not visible or not %Content.visible or not event.is_action_pressed("ui_cancel"):
        return
    get_viewport().set_input_as_handled()
    if %VehicleViewer.visible:
        %VehicleViewer.close()
        return
    quit_requested.emit()


func _on_load_button_pressed() -> void:
    %Content.visible = false
    scenery_selected.emit(_files[_selected_index], _get_selected_train_id(), _get_skin_overrides())
    var tween: Tween = create_tween()
    tween.tween_property(%Background.material, "shader_parameter/dissolve", 1.0, DISSOLVE_TIME)
    tween.tween_callback(hide)


## The player starts in the headdriver vehicle of the selected consist
func _get_selected_train_id() -> String:
    if not _info or _selected_consist_index < 0:
        return ""
    return _info.trainsets[_selected_consist_index].get_driver_train_id()


func _show_details(index: int) -> void:
    %LoadButton.disabled = index < 0
    %Image.texture = null
    %Image.visible = false
    for item: PanelContainer in _consist_items:
        item.queue_free()
    _consist_items.clear()
    _selected_consist_index = -1
    _info = null
    if index < 0:
        %Title.text = ""
        %FileName.text = ""
        %Description.text = ""
        %ConsistsHeader.visible = false
        %ConsistsScroll.visible = false
        _show_consist(-1)
        return
    _info = MaszynaSceneryInfo.read(_files[index])
    %Title.text = _titles[index]
    %FileName.text = _files[index].get_basename().to_upper()
    %Description.text = _info.description
    if _info.image_path:
        var image: Image = Image.load_from_file(_info.image_path)
        if image:
            %Image.texture = ImageTexture.create_from_image(image)
            %Image.visible = true
    for consist_index: int in _info.trainsets.size():
        var trainset: MaszynaSceneryInfo.Trainset = _info.trainsets[consist_index]
        var item: PanelContainer = _create_list_item(
            consist_index,
            _get_consist_name(trainset),
            _format_consist_note(trainset),
            _select_consist.bind(consist_index),
            _on_consist_item_hovered,
        )
        _consist_items.append(item)
        %Consists.add_child(item)
    var has_consists: bool = _info.trainsets.size() > 0
    %ConsistsHeader.visible = has_consists
    %ConsistsScroll.visible = has_consists
    if has_consists:
        _select_consist(0)
    else:
        _show_consist(-1)


## The vehicles of the consist as their skin textures, and its mission description
func _show_consist(index: int) -> void:
    # the viewer shows a vehicle of the consist that is going away
    if %VehicleViewer.visible:
        %VehicleViewer.close()
    for child: Node in %Vehicles.get_children():
        child.queue_free()
    _vehicles.clear()
    _vehicle_tiles.clear()
    _shown_index = -1
    %ConsistPreview.visible = index >= 0
    if not _info or index < 0:
        return
    var trainset: MaszynaSceneryInfo.Trainset = _info.trainsets[index]
    %Description.text = (
        "%s\n\n%s" % [trainset.description, _info.description]
        if trainset.description
        else _info.description
    )
    _vehicles.assign(trainset.vehicles)
    for vehicle_index: int in _vehicles.size():
        var tile: Control = _create_vehicle_preview(vehicle_index)
        _vehicle_tiles.append(tile)
        %Vehicles.add_child(tile)


## Side view of the vehicle from VehicleProfileManager (rendered on the spot when the data has
## no image of it)
func _create_vehicle_preview(index: int) -> Control:
    var vehicle: MaszynaSceneryInfo.Vehicle = _vehicles[index]
    var tile := Control.new()
    tile.custom_minimum_size = Vector2(
        VEHICLE_PREVIEW_HEIGHT, VEHICLE_PREVIEW_HEIGHT * VEHICLE_TILE_PADDING
    )


    var background := ColorRect.new()
    background.name = "Background"
    background.set_anchors_preset(Control.PRESET_FULL_RECT)
    background.mouse_filter = Control.MOUSE_FILTER_IGNORE
    background.material = ShaderMaterial.new()
    (background.material as ShaderMaterial).shader = VEHICLE_GLOW_SHADER
    tile.add_child(background)

    var preview := TextureButton.new()
    preview.ignore_texture_size = true
    preview.stretch_mode = TextureButton.STRETCH_KEEP_ASPECT_CENTERED
    preview.set_anchors_preset(Control.PRESET_FULL_RECT)
    # the profiles are small, linear filtering turns them into a blur when scaled up
    preview.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
    preview.tooltip_text = "%s (%s)" % [vehicle.train_id, vehicle.data_path.get_file()]
    preview.name = "Preview"
    preview.pressed.connect(_show_vehicle.bind(index))
    preview.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
    preview.mouse_entered.connect(_on_vehicle_preview_hovered.bind(index, true))
    preview.mouse_exited.connect(_on_vehicle_preview_hovered.bind(index, false))
    tile.add_child(preview)
    _load_vehicle_profile(preview, vehicle)
    return tile


## Side view of the vehicle at index, reloaded after its skin changes
func _refresh_vehicle_profile(index: int) -> void:
    var tile: Control = _vehicle_tiles[index]
    _load_vehicle_profile(tile.get_node("Preview") as TextureButton, _vehicles[index])


func _load_vehicle_profile(preview: TextureButton, vehicle: MaszynaSceneryInfo.Vehicle) -> void:
    # dimmed until the side view of the new skin is there
    preview.modulate.a = 0.3
    var profile: Texture2D = await VehicleProfileManager.get_profile(
        vehicle.data_path, vehicle.file_name, vehicle.skin
    )
    if not is_instance_valid(preview):
        return
    preview.modulate.a = 1.0
    if not profile:
        return
    preview.texture_normal = profile
    var tile: Control = preview.get_parent() as Control
    # as wide as the vehicle is long, so the vehicles of a consist line up without gaps
    var tile_size: Vector2 = Vector2(
        VEHICLE_PREVIEW_HEIGHT * float(profile.get_width()) / float(profile.get_height()),
        VEHICLE_PREVIEW_HEIGHT * VEHICLE_TILE_PADDING
    )
    tile.custom_minimum_size = tile_size
    var background: ColorRect = tile.get_node("Background") as ColorRect
    (background.material as ShaderMaterial).set_shader_parameter("rect_size", tile_size)


func _on_vehicle_preview_hovered(index: int, hovered: bool) -> void:
    if index == _shown_index:
        return
    _set_vehicle_glow(index, VEHICLE_HOVER_COLOR if hovered else Color.TRANSPARENT)


## The vehicle whose viewer is open keeps a green background
func _set_vehicle_glow(index: int, color: Color) -> void:
    if index < 0 or index >= _vehicle_tiles.size():
        return
    var tile: Control = _vehicle_tiles[index]
    var material: ShaderMaterial = (tile.get_node("Background") as ColorRect).material as ShaderMaterial
    material.set_shader_parameter("glow_color", color)
    material.set_shader_parameter("glow_strength", 0.0 if color == Color.TRANSPARENT else 1.0)


## A skin accepted in the viewer: the consist shows it and the scenery is loaded with it
func _on_vehicle_viewer_skin_applied(skin: String) -> void:
    if _shown_index < 0:
        return
    _vehicles[_shown_index].skin = skin
    _refresh_vehicle_profile(_shown_index)


## The scenery is loaded with the skins of the consist as they are shown here
func _get_skin_overrides() -> Dictionary[String, String]:
    var overrides: Dictionary[String, String] = {}
    for vehicle: MaszynaSceneryInfo.Vehicle in _vehicles:
        overrides[vehicle.train_id] = vehicle.skin
    return overrides


## The vehicle viewer takes the place of the scenery list, the two cross-fade
func _show_vehicle(index: int) -> void:
    _set_vehicle_glow(_shown_index, Color.TRANSPARENT)
    _shown_index = index
    _set_vehicle_glow(index, VEHICLE_SELECTED_COLOR)
    if _list_fade_tween:
        _list_fade_tween.kill()
    _list_fade_tween = create_tween()
    _list_fade_tween.tween_property(%ListPanel, "modulate:a", 0.0, VehicleViewer.FADE_TIME)
    _list_fade_tween.tween_callback(%ListPanel.hide)
    %VehicleViewer.show_vehicle(_vehicles[index])


## Emitted when the viewer starts fading out
func _on_vehicle_viewer_closed() -> void:
    _set_vehicle_glow(_shown_index, Color.TRANSPARENT)
    _shown_index = -1
    %ListPanel.modulate.a = 0.0
    %ListPanel.visible = true
    if _list_fade_tween:
        _list_fade_tween.kill()
    _list_fade_tween = create_tween()
    _list_fade_tween.tween_property(%ListPanel, "modulate:a", 1.0, VehicleViewer.FADE_TIME)


## A consist is named by the vehicle the player starts in - the "trainset" line carries the
## starting track, not a name; the vehicles are named in "node <x> <y> <name> dynamic"
static func _get_consist_name(trainset: MaszynaSceneryInfo.Trainset) -> String:
    return trainset.get_driver_train_id()


static func _format_consist_note(trainset: MaszynaSceneryInfo.Trainset) -> String:
    return "%d POJAZDÓW" % trainset.vehicles.size()
