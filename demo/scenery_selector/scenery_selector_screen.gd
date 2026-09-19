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

var _files: PackedStringArray = []
var _info: MaszynaSceneryInfo = null
## Vehicles of the shown consist and their tiles, in the order they run
var _vehicles: Array[MaszynaSceneryInfo.Vehicle] = []
var _vehicle_tiles: Array[Control] = []
## Vehicle whose viewer is open
var _shown_index: int = -1



func _ready() -> void:
    var files: PackedStringArray = DirAccess.get_files_at(UserSettings.get_maszyna_game_dir().path_join("scenery"))
    files.sort()
    for file: String in files:
        # "$" files are not scenarios to start (e.g. $stary_jawor_eszelon.scn)
        if file.get_extension().to_lower() == "scn" and not file.begins_with("$"):
            _files.append(file)
            %List.add_item(file.get_basename())
    _show_details(-1)
    %BuildLabel.text = "Pre-Alpha Demo Release %s" % ProjectSettings.get_setting("application/config/version")


func open() -> void:
    (%Background.material as ShaderMaterial).set_shader_parameter("dissolve", 0.0)
    %Content.visible = true
    visible = true
    %List.grab_focus()


func _unhandled_input(event: InputEvent) -> void:
    if not visible or not %Content.visible or not event.is_action_pressed("ui_cancel"):
        return
    get_viewport().set_input_as_handled()
    if %VehicleViewer.visible:
        %VehicleViewer.close()
        return
    quit_requested.emit()


func _on_list_item_selected(index: int) -> void:
    _show_details(index)


func _on_load_button_pressed() -> void:
    var selected: PackedInt32Array = %List.get_selected_items()
    %Content.visible = false
    scenery_selected.emit(_files[selected[0]], _get_selected_train_id(), _get_skin_overrides())
    var tween: Tween = create_tween()
    tween.tween_property(%Background.material, "shader_parameter/dissolve", 1.0, DISSOLVE_TIME)
    tween.tween_callback(hide)


func _on_consists_item_selected(index: int) -> void:
    _show_consist(index)


## The player starts in the headdriver vehicle of the selected consist
func _get_selected_train_id() -> String:
    var selected: PackedInt32Array = %Consists.get_selected_items()
    if not _info or not selected:
        return ""
    return _info.trainsets[selected[0]].get_driver_train_id()


func _show_details(index: int) -> void:
    %LoadButton.disabled = index < 0
    %Image.texture = null
    %Image.visible = false
    %Consists.clear()
    _info = null
    if index < 0:
        %Title.text = ""
        %Description.text = ""
        %ConsistsHeader.visible = false
        %Consists.visible = false
        _show_consist(-1)
        return
    _info = MaszynaSceneryInfo.read(_files[index])
    %Title.text = _info.title if _info.title else _files[index].get_basename()
    %Description.text = _info.description
    if _info.image_path:
        var image: Image = Image.load_from_file(_info.image_path)
        if image:
            %Image.texture = ImageTexture.create_from_image(image)
            %Image.visible = true
    for trainset: MaszynaSceneryInfo.Trainset in _info.trainsets:
        %Consists.add_item(_format_consist(trainset))
    var has_consists: bool = _info.trainsets.size() > 0
    %ConsistsHeader.visible = has_consists
    %Consists.visible = has_consists
    if has_consists:
        %Consists.select(0)
    _show_consist(0 if has_consists else -1)


## The vehicles of the consist as their skin textures, and its mission description
func _show_consist(index: int) -> void:
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
    var tween: Tween = create_tween()
    tween.tween_property(%ListPanel, "modulate:a", 0.0, VehicleViewer.FADE_TIME)
    tween.tween_callback(%ListPanel.hide)
    %VehicleViewer.show_vehicle(_vehicles[index])


## Emitted when the viewer starts fading out
func _on_vehicle_viewer_closed() -> void:
    _set_vehicle_glow(_shown_index, Color.TRANSPARENT)
    _shown_index = -1
    %ListPanel.modulate.a = 0.0
    %ListPanel.visible = true
    var tween: Tween = create_tween()
    tween.tween_property(%ListPanel, "modulate:a", 1.0, VehicleViewer.FADE_TIME)


static func _format_consist(trainset: MaszynaSceneryInfo.Trainset) -> String:
    var driver_train_id: String = trainset.get_driver_train_id()
    var consist_name: String = (
        trainset.name if trainset.name and not trainset.name.to_lower() == "none" else driver_train_id
    )
    return "%s - %d pojazdów, start: %s" % [consist_name, trainset.vehicles.size(), driver_train_id]
