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
        _controller = get_node_or_null(controller_path)
    _rebuild_generated()
    _cabin_ready = true
    cabin_ready.emit()


func get_diagnostics() -> Array[Dictionary]:
    return _diagnostics


func reload() -> void:
    _rebuild_generated()


## Only the cab1<->cab2 sign flip triggers a rebuild - any other mover config change is not
## this class's concern.
func _on_mover_config_changed() -> void:
    if _select_cab_number() != _last_cab_number:
        _rebuild_generated()


func _select_cab_number() -> int:
    if not _controller:
        return 1
    var cabin_occupied:int = _controller.state.get("cabin_occupied", 0)
    return 2 if cabin_occupied < 0 else 1


func _rebuild_generated() -> void:
    if _generated:
        remove_child(_generated)
        _generated.queue_free()
        _generated = null

    _diagnostics.clear()
    if not mmd_filename or not _controller:
        return

    if not _controller.mover_config_changed.is_connected(_on_mover_config_changed):
        _controller.mover_config_changed.connect(_on_mover_config_changed)

    var cabin_occupied:int = _controller.state.get("cabin_occupied", 0)
    _last_cab_number = _select_cab_number()
    cab_number = 1 if _last_cab_number == 1 else -1
    if cabin_occupied == 0:
        _diagnostics.append({
            "severity": "info", "code": "MMD_CABIN_OCCUPIED_UNKNOWN", "source_file": "", "line": 0,
            "cabin_number": _last_cab_number, "mmd_label": "", "submodel_name": "",
            "message": "cabin_occupied is 0 at build time - defaulting to cab1",
        })

    var abs_mmd_path:String = (
            UserSettings.get_maszyna_game_dir().path_join(data_path).path_join(mmd_filename + ".mmd"))
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(abs_mmd_path, _last_cab_number, _random_choices)
    _diagnostics.append_array(definition.diagnostics)

    camera_bound_min = definition.bounds_min
    camera_bound_max = definition.bounds_max
    camera_bound_enabled = true
    driver_position = definition.driver_pos

    _generated = Node3D.new()
    _generated.name = "Generated"
    add_child(_generated, false, INTERNAL_MODE_BACK)

    var build_diagnostics:Array[Dictionary] = []
    MmdCabinInstancer.build_into(_generated, definition, _controller, data_path, skin, build_diagnostics)
    _diagnostics.append_array(build_diagnostics)

    _build_driver_aid_commands()

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
    release_to_drive.command = "brake_level_set_position"
    release_to_drive.command_param = "drive"
    _generated.add_child(release_to_drive)
    release_to_drive.controller_path = release_to_drive.get_path_to(_controller)
