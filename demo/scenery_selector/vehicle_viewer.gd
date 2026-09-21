extends Control

## Vehicle of a consist: its exterior model rotating in a SubViewport, its name and the skins
## found next to it (the .mat files of the vehicle). Shown in place of the scenery list.

## Emitted whichever way the viewer goes away. by_back_button tells the back button apart from
## accepting a skin and from the screen closing the viewer on its own - they are the same
## transition but not the same gesture, and only one of them is a "back".
signal closed(by_back_button: bool)
## The skin picked in the viewer was accepted
signal skin_applied(skin: String)
## A skin was clicked - the screen's keyboard focus follows the pointer into the viewer
signal skin_clicked

const ROTATION_SPEED: float = 0.35

## Same bank the scenery selector uses - the viewer is its own scene and plays its own gestures.
const UI_SOUNDS: SfxBank = preload("res://startup/ui_sounds.tres")
const FADE_TIME: float = 0.5
## Distance of the camera from the model, in model lengths
const CAMERA_DISTANCE: float = 1.15
## Tile of a skin: the vehicle is as tall as this, as wide as it is long
const SKIN_PREVIEW_SIZE: Vector2 = Vector2(300.0, 60.0)
## The tile is taller than the vehicle, so the halo under it has somewhere to show
const SKIN_TILE_PADDING: float = 1.8

## Glow around a skin silhouette: white under the pointer, yellow on the skin in use
const SKIN_GLOW_SHADER: Shader = preload("res://scenery_selector/skin_glow.gdshader")
const SKIN_HOVER_COLOR: Color = Color(1.0, 1.0, 1.0)
const SKIN_SELECTED_COLOR: Color = Color(0.35, 1.0, 0.45)

var _vehicle: MaszynaSceneryInfo.Vehicle = null
var _data_path: String = ""
var _model: E3DModelInstance = null
## Skin of the model, the one the consist gives the vehicle until another is picked
var _skin: String = ""
## Skins of the vehicle and their tiles, in the order they are shown
var _skins: Array[String] = []
var _skin_buttons: Array[TextureButton] = []
var _skin_index: int = -1
## Fading in or out; killed when the other fade starts, so they never fight over modulate
var _fade_tween: Tween = null
var _ui_sounds: SfxPlayer


func _ready() -> void:
    _ui_sounds = SfxPlayer.new()
    _ui_sounds.bank = UI_SOUNDS
    add_child(_ui_sounds)


func _process(delta: float) -> void:
    if _model:
        %ModelRoot.rotate_y(delta * ROTATION_SPEED)


func show_vehicle(vehicle: MaszynaSceneryInfo.Vehicle) -> void:
    _vehicle = vehicle
    # every vehicle path is used with a leading slash (see maszyna_rail_vehicle_3d_instancer.gd)
    _data_path = "/" + vehicle.data_path
    %Name.text = vehicle.file_name if vehicle.file_name else vehicle.data_path.get_file()
    %Subtitle.text = "%s - %s" % [vehicle.train_id, vehicle.data_path]
    %ModelRoot.rotation = Vector3.ZERO
    _build_model(vehicle.skin)
    _build_skin_grid()
    modulate.a = 0.0
    visible = true
    if _fade_tween:
        _fade_tween.kill()
    _fade_tween = create_tween()
    _fade_tween.tween_property(self, "modulate:a", 1.0, FADE_TIME)


## Keeps the turntable where it is - picking a skin only swaps the model
func _build_model(skin: String) -> void:
    _skin = skin
    if _model:
        _model.queue_free()

    var abs_mmd_path: String = (
        UserSettings.get_maszyna_game_dir().path_join(_data_path).path_join(_vehicle.file_name.to_lower() + ".mmd")
    )
    var body_model: String = MmdCabinInstancer.parse_body_model(abs_mmd_path)
    if not body_model:
        body_model = _vehicle.file_name
    _model = E3DModelInstance.new()
    _model.instancer = E3DModelInstance.Instancer.OPTIMIZED
    _model.data_path = _data_path
    _model.model_filename = MmdCabinInstancer.resolve_model_case(_data_path, body_model)
    _model.skins = MmdCabinInstancer.resolve_skins(_data_path, skin)
    %ModelRoot.add_child(_model)
    _frame_model()
    _update_skin_selection()


## Puts the model in the middle of the turntable and pulls the camera back to fit it
func _frame_model() -> void:
    var bounds: AABB = _model.submodels_aabb
    _model.position = -bounds.get_center()
    var radius: float = maxf(bounds.size.length() * 0.5, 1.0)
    %Camera.position = Vector3(0.0, radius * 0.35, radius * CAMERA_DISTANCE * 2.0)
    %Camera.look_at(Vector3.ZERO)


## Skins of the vehicle (VehicleSkins), each with a side view
func _build_skin_grid() -> void:
    for child: Node in %Skins.get_children():
        child.queue_free()
    _skins.clear()
    _skin_buttons.clear()
    _skin_index = -1

    # the consist can give the vehicle a skin that is not listed - it is still a skin, and it goes
    # first, so it is the one selected
    if _skin:
        _skins.append(_skin)
    var vehicle_dir: String = UserSettings.get_maszyna_game_dir().path_join(_data_path)
    for skin: String in VehicleSkins.list_skins(vehicle_dir, _vehicle.file_name):
        if not skin == _skin.to_lower():
            _skins.append(skin)

    for index: int in _skins.size():
        var entry: Control = _create_skin_entry(index)
        %Skins.add_child(entry)
    _skin_index = 0 if _skins else -1
    # no skin is left deselected either: with none coming from the consist the first one is put on
    if _skin_index == 0 and not _skin:
        _select_skin(0)
    else:
        _update_skin_selection()


## Side view of the skin over its name, both clickable
func _create_skin_entry(index: int) -> Control:
    var skin: String = _skins[index]
    var entry := VBoxContainer.new()
    entry.tooltip_text = skin

    var tile := Control.new()
    tile.name = "Tile"
    tile.custom_minimum_size = Vector2(SKIN_PREVIEW_SIZE.x, SKIN_PREVIEW_SIZE.y * SKIN_TILE_PADDING)
    entry.add_child(tile)

    # the background of the tile, lit up under the vehicle
    var background := ColorRect.new()
    background.name = "Background"
    background.set_anchors_preset(Control.PRESET_FULL_RECT)
    background.mouse_filter = Control.MOUSE_FILTER_IGNORE
    background.material = ShaderMaterial.new()
    (background.material as ShaderMaterial).shader = SKIN_GLOW_SHADER
    tile.add_child(background)

    var button := TextureButton.new()
    button.ignore_texture_size = true
    button.stretch_mode = TextureButton.STRETCH_KEEP_ASPECT_CENTERED
    button.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
    button.set_anchors_preset(Control.PRESET_FULL_RECT)
    button.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
    button.pressed.connect(_on_skin_pressed.bind(index))
    button.mouse_entered.connect(_on_skin_hovered.bind(index, true))
    button.mouse_exited.connect(_on_skin_hovered.bind(index, false))
    tile.add_child(button)
    _skin_buttons.append(button)
    # after the tile is built: the profile may arrive right away, from the cache
    _load_skin_profile(button, skin)

    var label := Label.new()
    label.text = skin
    label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    label.add_theme_font_size_override("font_size", 13)
    label.clip_text = true
    entry.add_child(label)
    return entry


func _select_skin(index: int) -> void:
    _skin_index = index
    _build_model(_skins[index])
    _update_skin_selection()


## The skin in use keeps a green background, the rest none until the pointer is over them
func _update_skin_selection() -> void:
    for index: int in _skin_buttons.size():
        _set_skin_glow(index, SKIN_SELECTED_COLOR if index == _skin_index else Color.TRANSPARENT)


## The skin list as one section of the selector's keyboard navigation: the viewer owns the skins,
## the screen only says which section Tab is on and which way the arrows went.
##
## Up/Down on the skins: the change a click would have made, under the keyboard's own sound. The
## end of the row changes nothing - reselecting a skin would rebuild the model for the same skin.
func move_skin_selection(step: int) -> void:
    if not _skin_buttons:
        return
    var index: int = clampi(_skin_index + step, 0, _skin_buttons.size() - 1)
    if index == _skin_index:
        return
    _ui_sounds.play(&"keystroke")
    _select_skin(index)
    # the flow container lays out the entry, not the button inside its tile
    FocusSection.scroll_to_item(%SkinsScroll, _skin_buttons[index].get_parent().get_parent())


## Enter on the skins: the gesture the mouse would have made, sound included
func activate_skin_selection() -> void:
    if _skin_index >= 0:
        _on_skin_pressed(_skin_index)


func set_section_focused(focused: bool) -> void:
    %SkinsFocus.focused = focused


func _on_skin_pressed(index: int) -> void:
    _ui_sounds.play(&"skin_click")
    skin_clicked.emit()
    _select_skin(index)


func _on_skin_hovered(index: int, hovered: bool) -> void:
    if index == _skin_index:
        return
    if hovered:
        _ui_sounds.play(&"skin_hover")
    _set_skin_glow(index, SKIN_HOVER_COLOR if hovered else Color.TRANSPARENT)


func _set_skin_glow(index: int, color: Color) -> void:
    if index < 0 or index >= _skin_buttons.size():
        return
    var background: ColorRect = _skin_buttons[index].get_parent().get_node("Background") as ColorRect
    var material: ShaderMaterial = background.material as ShaderMaterial
    material.set_shader_parameter("glow_color", color)
    material.set_shader_parameter("glow_strength", 0.0 if color == Color.TRANSPARENT else 1.0)


## Side view of the skin from VehicleProfileManager
func _load_skin_profile(button: TextureButton, skin: String) -> void:
    var profile: Texture2D = await VehicleProfileManager.get_profile(_data_path, _vehicle.file_name, skin)
    if not is_instance_valid(button) or not profile:
        return
    button.texture_normal = profile
    var tile: Control = button.get_parent() as Control
    var tile_size: Vector2 = Vector2(
        SKIN_PREVIEW_SIZE.y * float(profile.get_width()) / float(profile.get_height()),
        SKIN_PREVIEW_SIZE.y * SKIN_TILE_PADDING
    )
    tile.custom_minimum_size = tile_size
    # the shader rounds the corners in pixels, so it needs to know how big the tile is
    var background: ColorRect = tile.get_node("Background") as ColorRect
    (background.material as ShaderMaterial).set_shader_parameter("rect_size", tile_size)


func _on_back_button_pressed() -> void:
    close(true)


func _on_apply_button_pressed() -> void:
    if _skin_index >= 0:
        skin_applied.emit(_skins[_skin_index])
    close()


## Fades the viewer out; the scenery list fades in under it at the same time
func close(by_back_button: bool = false) -> void:
    closed.emit(by_back_button)
    if _fade_tween:
        _fade_tween.kill()
    _fade_tween = create_tween()
    _fade_tween.tween_property(self, "modulate:a", 0.0, FADE_TIME)
    # a fade in started meanwhile kills this tween, and the viewer stays open
    await _fade_tween.finished
    visible = false
    if _model:
        _model.queue_free()
        _model = null
