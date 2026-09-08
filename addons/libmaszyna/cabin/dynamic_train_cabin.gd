extends Cabin3D
class_name DynamicTrainCabin

## MMD-driven cabin builder, analogous to E3DModelInstance/FIZTrainController: given
## data_path/mmd_filename/skin it parses the vehicle's MMD file, resolves cab1/cab2 from the
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

var _generated:Node3D
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
    if _train_controller == controller:
        return
    if _train_controller:
        _train_controller.mover_config_changed.disconnect(_on_mover_config_changed)
    super.set_train_controller(controller)
    if _train_controller:
        _train_controller.mover_config_changed.connect(_on_mover_config_changed)
    _rebuild()


func _exit_tree() -> void:
    if _train_controller:
        _train_controller.mover_config_changed.disconnect(_on_mover_config_changed)
    _train_controller = null


func reload() -> void:
    _rebuild()


## Only the cab1<->cab2 sign flip triggers a rebuild - any other mover config change is not
## this class's concern.
func _on_mover_config_changed() -> void:
    if not _select_cab_number() == _last_cab_number:
        _rebuild()


func _select_cab_number() -> int:
    if not _train_controller:
        return 1
    var cabin_occupied:int = _train_controller.state.get("cabin_occupied", 0)
    return 2 if cabin_occupied < 0 else 1


func _rebuild() -> void:
    if _generated:
        remove_child(_generated)
        _generated.queue_free()
        _generated = null

    if not mmd_filename or not _train_controller:
        return

    _last_cab_number = _select_cab_number()

    var definition:MmdCabinDefinition = MmdManager.load_cabin(data_path, mmd_filename, _last_cab_number)
    if not definition:
        return
    camera_bound_min = definition.bounds_min
    camera_bound_max = definition.bounds_max
    camera_bound_enabled = true
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

    MmdCabinInstancer.build_into(_generated, definition, _train_controller, data_path, skin)

    _build_driver_aid_commands()
    camera_configuration_changed.emit()



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
    release_to_drive.command = "brake_level_set_position"
    release_to_drive.command_param = "drive"
    _generated.add_child(release_to_drive)
    release_to_drive.controller_path = release_to_drive.get_path_to(_train_controller)
