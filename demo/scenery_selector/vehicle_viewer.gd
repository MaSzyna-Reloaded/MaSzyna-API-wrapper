extends Control

## Vehicle of a trainset: its exterior model rotating in a SubViewport, its name and the skins
## found next to it (the .mat files of the vehicle). Shown in place of the scenery list.

## Emitted whichever way the viewer goes away. No "was it the back button" in it: the back button
## is a gesture of this scene and plays its own sound here.
signal closed
## The skin picked in the viewer was accepted
signal skin_applied(skin: String)

const ROTATION_SPEED: float = 0.35

## Same bank the scenery selector uses - the viewer is its own scene and plays its own gestures.
const UI_SOUNDS: SfxBank = preload("res://startup/ui_sounds.tres")
const FADE_TIME: float = 0.5
## Distance of the camera from the model, in model lengths
const CAMERA_DISTANCE: float = 1.15

var _vehicle: MaszynaSceneryInfo.Vehicle = null
var _data_path: String = ""
var _model: E3DModelInstance = null
## Skin of the model, the one the trainset gives the vehicle until another is picked
var _skin: String = ""
## Skins of the vehicle, in the order the grid shows them
var _skins: Array[String] = []
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
    # a viewer that is still fading out carries on from the alpha it reached, and does not blink
    if not visible:
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


## Puts the model in the middle of the turntable and pulls the camera back to fit it
func _frame_model() -> void:
    var bounds: AABB = _model.submodels_aabb
    _model.position = -bounds.get_center()
    var radius: float = maxf(bounds.size.length() * 0.5, 1.0)
    %Camera.position = Vector3(0.0, radius * 0.35, radius * CAMERA_DISTANCE * 2.0)
    %Camera.look_at(Vector3.ZERO)


## Skins of the vehicle (VehicleSkins), each as a tile of the grid
func _build_skin_grid() -> void:
    _skins.clear()
    # the trainset can give the vehicle a skin that is not listed - it is still a skin, and it goes
    # first, so it is the one the grid selects
    if _skin:
        _skins.append(_skin)
    var vehicle_dir: String = UserSettings.get_maszyna_game_dir().path_join(_data_path)
    for skin: String in VehicleSkins.list_skins(vehicle_dir, _vehicle.file_name):
        if not skin == _skin.to_lower():
            _skins.append(skin)
    var tiles: Array[TileGrid.Tile] = []
    for skin: String in _skins:
        tiles.append(TileGrid.Tile.new(_data_path, _vehicle.file_name, skin, skin, skin))
    %SkinsGrid.set_tiles(tiles)


## The grid moved onto another skin, so the model wears it from now on. Nothing to do when it is
## the skin the model was built with - that is the grid selecting its first tile after a rebuild.
func _on_skins_grid_item_selected() -> void:
    var index: int = %SkinsGrid.get_selected()
    if index < 0 or _skins[index] == _skin:
        return
    _build_model(_skins[index])


## The skins are a section of the screen's focus cycle, and the screen drives it through this node
func get_skins_section() -> FocusSection:
    return %SkinsGrid


func _on_back_button_pressed() -> void:
    _ui_sounds.play(&"back_button")
    close()


## Accepts the skin the grid has selected and leaves - the button and Enter on a tile are both
## wired straight to this, in the scene.
func apply_selected_skin() -> void:
    var index: int = %SkinsGrid.get_selected()
    if index >= 0:
        skin_applied.emit(_skins[index])
    close()


## Fades the viewer out; the scenery list fades in under it at the same time. A viewer that is
## already gone closes no second time - a repeated "closed" restarted the list's fade from zero.
func close() -> void:
    if not visible:
        return
    closed.emit()
    if _fade_tween:
        _fade_tween.kill()
    _fade_tween = create_tween()
    _fade_tween.tween_property(self, "modulate:a", 0.0, FADE_TIME)
    # a callback and not "await finished": killing a tween never finishes it, so a fade in started
    # meanwhile would leave the await hanging and the model alive
    _fade_tween.tween_callback(_close_finished)


func _close_finished() -> void:
    visible = false
    if _model:
        _model.queue_free()
        _model = null
