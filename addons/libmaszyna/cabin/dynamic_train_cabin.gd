extends Cabin3D
class_name DynamicTrainCabin

## MMD-driven cabin builder, analogous to E3DModelInstance/FIZTrainController: given
## data_path/mmd_filename/skin it parses the vehicle's MMD file, resolves cab0/cab1/cab2 from the
## controller's cabin_occupied state, and builds a real, interactive cabin (Etap A+B scope -
## see mmd_cabin_instancer.gd) instead of requiring a hand-authored cabin_scene.
##
## Deliberately overrides _ready() and does not call super(): the base Cabin3D._ready() emits
## cabin_ready immediately, before this class's own children (cab model, widgets) exist -
## readiness here must wait until the whole MMD-derived "Generated" subtree is actually built.
## Everything below runs synchronously within one _ready() call (MMD parsing and E3D loading
## are both synchronous), so RailVehicle3D.enter_cabin()'s one-shot wait on cabin_ready still
## resolves within the same add_child() call that creates this node.

@export var data_path:String = ""
@export var mmd_filename:String = ""
@export var skin:String = ""

## Cab light height above the driver's eyes, kept under the top of the cab model - the model top
## is its outer shell (roof pipes, heaters), not the ceiling the light has to stay under.
const CAB_LIGHT_ABOVE_DRIVER:float = 0.4
const CAB_LIGHT_CEILING_OFFSET:float = 0.15
## Quirk: MMD has no cab light position (cablight: holds only unused colors, Train.cpp:118-133), so the
## light goes to the cab model's ceiling lamp, found by the submodel names the game's cab models use
## for it (most specific first); CAB_LIGHT_ABOVE_DRIVER is the fallback without such a lamp.
const CAB_LAMP_SUBMODEL_NAMES:Array[String] = [
    "lampa_suf0", "lampy_sufit", "lampa_sufi", "lampa_sufit", "lampasufitowa", "lampasufit", "swiatlo_sufit",
    "cablight", "lampa",
]
## Distance of the cab light below the found ceiling lamp - inside the lamp's shadow casting mesh
## it would light nothing.
const CAB_LIGHT_BELOW_LAMP:float = 0.05

var _controller:TrainController
var _generated:Node3D
var _diagnostics:Array[Dictionary] = []
var _random_choices:Dictionary = {}
var _last_cab_number:int = 0


func _ready() -> void:
    # controller_path (inherited from Cabin3D) is already set by RailVehicle3D.enter_cabin()
    # before add_child() - resolve it here directly rather than waiting for Cabin3D's own
    # _process()-based dirty resolution, which only runs a frame later.
    if controller_path:
        set_train_controller(get_node_or_null(controller_path))
    _cabin_ready = true
    cabin_ready.emit()


func set_train_controller(controller:TrainController) -> void:
    if _controller == controller:
        return
    if _controller:
        _controller.cabin_occupied_changed.disconnect(_on_cabin_occupied_changed)
    _controller = controller
    super.set_train_controller(controller)
    if _controller:
        _controller.cabin_occupied_changed.connect(_on_cabin_occupied_changed)
    _rebuild_generated()


func _exit_tree() -> void:
    if _controller:
        _controller.cabin_occupied_changed.disconnect(_on_cabin_occupied_changed)
    _controller = null
    _shake_controller = null


func get_diagnostics() -> Array[Dictionary]:
    return _diagnostics


func reload() -> void:
    _rebuild_generated()


## Rebuilds when the crew moves to another cab (cab0 = machine room, cab1, cab2).
func _on_cabin_occupied_changed(_cabin_occupied:int) -> void:
    if not _select_cab_number() == _last_cab_number:
        _rebuild_generated()


func _select_cab_number() -> int:
    if not _controller:
        return 1
    # Train.cpp:8684 (InitializeCab) - CabOccupied -1 loads cab2definition:, 0 cab0, 1 cab1.
    var cabin_occupied:int = _controller.state.get("cabin_occupied", 0)
    return 2 if cabin_occupied < 0 else cabin_occupied


func _rebuild_generated() -> void:
    if _generated:
        remove_child(_generated)
        _generated.queue_free()
        _generated = null

    _diagnostics.clear()
    if not mmd_filename or not _controller:
        return

    _last_cab_number = _select_cab_number()
    cab_number = -1 if _last_cab_number == 2 else _last_cab_number

    var abs_mmd_path:String = (
            UserSettings.get_maszyna_game_dir().path_join(data_path).path_join(mmd_filename + ".mmd"))
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(abs_mmd_path, _last_cab_number, _random_choices)
    _diagnostics.append_array(definition.diagnostics)

    camera_bound_min = definition.bounds_min
    camera_bound_max = definition.bounds_max
    camera_bound_enabled = true
    has_cab_model = true if definition.model_relpath else false
    driver_position = definition.driver_pos
    shake_spring_stiffness = definition.shake_spring_stiffness
    shake_spring_damping = definition.shake_spring_damping
    shake_jolt_scale = definition.shake_jolt_scale
    shake_jolt_limit = definition.shake_jolt_limit
    shake_angle_scale = definition.shake_angle_scale
    engine_shake_scale = definition.engine_shake_scale
    engine_shake_fade_in_rpm = definition.engine_shake_fade_in_rpm
    engine_shake_fade_in_factor = definition.engine_shake_fade_in_factor
    engine_shake_fade_out_rpm = definition.engine_shake_fade_out_rpm
    engine_shake_fade_out_factor = definition.engine_shake_fade_out_factor

    _generated = Node3D.new()
    _generated.name = "Generated"
    add_child(_generated, false, INTERNAL_MODE_BACK)

    var build_diagnostics:Array[Dictionary] = []
    MmdCabinInstancer.build_into(_generated, definition, _controller, data_path, skin, build_diagnostics)
    _diagnostics.append_array(build_diagnostics)

    _build_driver_aid_commands()
    # a cab with an i-cablight lamp already has its own ceiling light (MmdSemanticCatalog)
    if not definition.instruments.any(
            func(instrument:MmdInstrumentDescriptor) -> bool: return instrument.label == "i-cablight"):
        _build_cab_light(definition)
    # cabin logic of the original engine (CabinSystem callbacks) - added last, after every control
    var logic := LegacyCabinLogicDelegate.new()
    logic.name = "LegacyCabinLogic"
    logic.controller = _controller
    logic.cab = cab_number
    _generated.add_child(logic)
    camera_configuration_changed.emit()

    print("DynamicTrainCabin: built cab %d from %s - %d instruments parsed, %d generated children" % [
        _last_cab_number, abs_mmd_path, definition.instruments.size(), _generated.get_child_count()])
    for d:Dictionary in _diagnostics:
        print("  [%s] %s (label=%s submodel=%s)" % [d["severity"], d["message"], d["mmd_label"], d["submodel_name"]])


## Keyboard-only driver aids that have no cabin lever/MMD instrument of their own (nothing to
## parse, nothing to animate) - demo/vehicles/sm42/sm_42_cabin.tscn wires the same thing by hand
## via a plain "Commands/" CabinCommand node. brake_level_set_position/_str (TrainBrake.cpp) is
## already generic across handle types - it resolves a NAMED position ("drive" -> Maszyna::bh_RP,
## the original engine's own "running position" handle-position constant, McZapkie/hamulce.h) per
## vehicle rather than a hardcoded value, so this "jump the brake handle to driving/release
## position" shortcut is safe to attach unconditionally on every dynamically-built cabin, not just
## SM42 - a vehicle whose handle type has no equivalent named position just gets no visible effect.
func _build_driver_aid_commands() -> void:
    var release_to_drive := CabinCommand.new()
    release_to_drive.name = "BrakeLevelSet_Drive"
    release_to_drive.action_name = "brake_level_drive"
    release_to_drive.control_id = &"brake_level_drive"
    release_to_drive.command = "brake_level_set_position"
    release_to_drive.command_param = "drive"
    _generated.add_child(release_to_drive)
    release_to_drive.controller_path = release_to_drive.get_path_to(_controller)


## Cab interior lighting: the original lights the cab model with a tungsten ambient term
## InteriorLight * InteriorLightLevel (DynObj.h:240, openglrenderer.cpp:3684-3692); here a shadow
## casting light at the cab ceiling lamp, driven by the same level (roof_light_level,
## Train.cpp:8041-8060).
func _build_cab_light(definition:MmdCabinDefinition) -> void:
    var light := CabinOmniLight3D.new()
    light.name = "CabLight"
    light.light_color = Color(0.9, 0.9 * 216.0 / 255.0, 0.9 * 176.0 / 255.0)
    light.shadow_enabled = true
    light.shadow_reverse_cull_face = ProjectSettings.get_setting("maszyna/rendering/lights_shadow_reverse_cull_face", true)
    # without a cab model: the top of the camera bounds
    light.position = (definition.bounds_min + definition.bounds_max) * 0.5
    light.position.y = definition.bounds_max.y
    light.omni_range = maxf((definition.bounds_max - definition.bounds_min).length(), 1.0)
    light.state_property = "roof_light_level"
    _generated.add_child(light)
    light.controller_path = light.get_path_to(_controller)

    var cab_model:E3DModelInstance = _generated.get_node_or_null("CabModel") as E3DModelInstance
    if not cab_model:
        return
    if cab_model.is_e3d_loaded():
        _place_cab_light(light, cab_model, definition.driver_pos.y)
    else:
        cab_model.e3d_loaded.connect(
                _place_cab_light.bind(light, cab_model, definition.driver_pos.y), CONNECT_ONE_SHOT)


## Just under the cab model's ceiling lamp; without one, horizontal center of the cab model above
## the driver's eyes.
func _place_cab_light(light:CabinOmniLight3D, cab_model:E3DModelInstance, driver_height:float) -> void:
    var bounds:AABB = cab_model.submodels_aabb
    light.omni_range = maxf(bounds.size.length(), 1.0)
    var lamp_bounds:AABB = _find_cab_lamp_bounds(cab_model)
    if lamp_bounds.has_volume() or lamp_bounds.has_surface():
        var lamp_center:Vector3 = lamp_bounds.get_center()
        light.position = cab_model.transform * Vector3(
                lamp_center.x, lamp_bounds.position.y - CAB_LIGHT_BELOW_LAMP, lamp_center.z)
        return
    var center:Vector3 = bounds.get_center()
    var height:float = minf(driver_height + CAB_LIGHT_ABOVE_DRIVER, bounds.end.y - CAB_LIGHT_CEILING_OFFSET)
    light.position = cab_model.transform * Vector3(center.x, height, center.z)


## Bounds (in cab model space) of the first ceiling lamp submodel found by CAB_LAMP_SUBMODEL_NAMES,
## matched case-insensitively like TSubModel::GetFromName, also as an _on/_off indicator pair.
func _find_cab_lamp_bounds(cab_model:E3DModelInstance) -> AABB:
    var meshes_by_name:Dictionary = {}
    for mesh:Node in cab_model.find_children("*", "MeshInstance3D", true, false):
        var mesh_name:String = String(mesh.name).to_lower()
        if not meshes_by_name.has(mesh_name):
            meshes_by_name[mesh_name] = mesh
    for lamp_name:String in CAB_LAMP_SUBMODEL_NAMES:
        for suffix:String in ["", "_on", "_off"]:
            var mesh:MeshInstance3D = meshes_by_name.get(lamp_name + suffix) as MeshInstance3D
            if mesh:
                return cab_model.global_transform.affine_inverse() * mesh.global_transform * mesh.get_aabb()
    return AABB()
