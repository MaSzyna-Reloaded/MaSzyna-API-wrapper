@tool
extends RefCounted
class_name MaszynaRailVehicle3DInstancer

## Builds a complete, driveable RailVehicle3D from nothing but a data_path/file_name/skin
## triple: exterior E3D model, FIZ physics controller, interactive MMD-driven cabin, sound bank,
## head display, and bogie/wheel animation bindings. Used by DynamicRailVehicle3D, which owns
## only the dirty-flag lifecycle and track placement around whatever build() returns.

## Fixed original-engine submodel naming convention for bogies/wheel axles (DynObj.cpp:2341-2346
## for bogies, DynObj.cpp:5132-5176 for wheel axles) - confirmed against real game data (e.g.
## dynamic/pkp/su45_v2/301d.e3d, dynamic/pkp/sm42_v1/6d1.e3d both contain bogie1/bogie2
## submodels; every .mmd in the game data declaring animwheelprefix: uses "wheel0").
##
## NOTE: every discovered wheel submodel is treated as powered. The original engine also splits
## axles into front-rolling/powered/rear-rolling groups based on AxleArangement letters/digits
## (DynObj.cpp:5150-5176), but only when a vehicle's rolling-wheel diameter differs from its
## powered-wheel diameter - no vehicle in the current game data needs that, and it would require
## threading the parsed TrainWheels config through to spawn time. Left unimplemented until an
## actual vehicle needs it.
const FRONT_BOGIE_SUBMODEL_NAMES:Array[String] = ["bogie1", "boogie01"]
const REAR_BOGIE_SUBMODEL_NAMES:Array[String] = ["bogie2", "boogie02"]
const WHEEL_SUBMODEL_PREFIX:String = "wheel0"
const MAX_WHEEL_AXLES:int = 20

## Fixed original-engine submodel naming convention for pantograph arms
## (DynObj.cpp's animpantrd1prefix:/rd2/rg1/rg2/sl tokens - configurable in
## principle, but confirmed identical across every real vehicle checked,
## e.g. dynamic/pkp/303e_v1/303e-ep-tv.mmd, dynamic/pkp/ep09_v1/104e_1.mmd,
## dynamic/pkp/sr61_v2/sr61v1.mmd - same simplification already made above
## for WHEEL_SUBMODEL_PREFIX). Order matches RailVehicle3D's own
## pantograph_*_arm_paths index meaning (lower arm pair, upper arm pair,
## slider); the trailing pantograph number (1=front, 2=rear) is appended by
## _find_pantograph_arm_paths() below.
const PANTOGRAPH_ARM_SUBMODEL_PREFIXES:Array[String] = [
    "ramiedolne1_pant0", "ramiedolne2_pant0", "ramiegorne1_pant0", "ramiegorne2_pant0", "slizg_pant0",
]


## Builds a fully wired RailVehicle3D (not yet track-placed, not yet parented under a
## DynamicRailVehicle3D). Returns null if data_path/file_name are missing.
static func build(
        data_path:String, file_name:String, skin:String, train_id:String,
        initial_velocity:float, head_display_material:Material) -> RailVehicle3D:
    var vehicle:RailVehicle3D = _build_structure(data_path, file_name, skin, train_id, initial_velocity)
    if vehicle:
        _initialize_instance(vehicle, file_name, head_display_material)
    return vehicle


## Only declarative nodes belong in a PackedScene template; runtime sound pools and bindings do not.
static func _build_structure(
        data_path:String, file_name:String, skin:String, train_id:String,
        initial_velocity:float) -> RailVehicle3D:
    if not data_path or not file_name:
        return null

    # E3DInstancer._get_material_override() builds its material-search path by dropping
    # data_path's FIRST "/"-separated segment - meant to strip the artifact empty segment from a
    # LEADING slash, not the "dynamic" directory name itself. Every hand-authored vehicle scene
    # except su45 (whose skin is consequently broken the same way) uses a leading slash
    # (e.g. "/dynamic/pkp/ep09_v1/") for exactly this reason. Normalize here so operators don't
    # need to know about this quirk.
    var normalized_data_path:String = data_path if data_path.begins_with("/") else "/" + data_path

    var abs_mmd_path:String = (
            UserSettings.get_maszyna_game_dir().path_join(normalized_data_path).path_join(file_name + ".mmd"))
    # The exterior body model filename is NOT the same as file_name in general (confirmed
    # against real data: dynamic/pkp/st44_v2's body model isn't named after its .fiz/.mmd base) -
    # it comes from the MMD's own top-level "models:" line. Fall back to file_name only if that
    # can't be read, rather than silently building an ExteriorModel with no model at all.
    var body_model_filename:String = MmdCabinInstancer.parse_body_model(abs_mmd_path)
    if not body_model_filename:
        body_model_filename = file_name
    body_model_filename = MmdCabinInstancer.resolve_model_case(normalized_data_path, body_model_filename)

    var model := E3DModelInstance.new()
    model.name = "ExteriorModel"
    model.data_path = normalized_data_path
    model.model_filename = body_model_filename
    model.skins = MmdCabinInstancer.resolve_skins(normalized_data_path, skin)
    # Every MaSzyna-authored piece of a vehicle (exterior, low-poly interior, passengers, cab)
    # lives in one vehicle-local frame where +Z is the direction of travel: the original draws
    # all of them under the same TDynamicObject::mMatrix, built by BasisChange(vLeft, vUp,
    # vFront) (DynObj.cpp:2506-2508; opengl33renderer.cpp:1174, 2856, 2976). RailVehicle3D uses
    # Godot's -Z forward, so each of them gets the same 180 degree yaw - the cab via
    # cabin_rotate_180deg below.
    model.rotation.y = PI

    # Optional: the lower-detail interior seen from outside (through windows) before the player
    # enters the cabin. Most MMD files don't declare one - only build it if present.
    var low_poly_model:E3DModelInstance = null
    var lowpoly_filename:String = MmdCabinInstancer.parse_lowpoly_interior_model(abs_mmd_path)
    if lowpoly_filename:
        lowpoly_filename = MmdCabinInstancer.resolve_model_case(normalized_data_path, lowpoly_filename)
        low_poly_model = E3DModelInstance.new()
        low_poly_model.name = "LowPolyInterior"
        low_poly_model.data_path = normalized_data_path
        low_poly_model.model_filename = lowpoly_filename
        low_poly_model.skins = MmdCabinInstancer.resolve_skins(normalized_data_path, skin)
        low_poly_model.rotation.y = PI

    var passengers_model:E3DModelInstance = null
    var passengers_filename:String = MmdCabinInstancer.parse_passengers_model(abs_mmd_path)
    if passengers_filename:
        passengers_filename = MmdCabinInstancer.resolve_model_case(normalized_data_path, passengers_filename)
        passengers_model = E3DModelInstance.new()
        passengers_model.name = "Passengers"
        passengers_model.data_path = normalized_data_path
        passengers_model.model_filename = passengers_filename
        passengers_model.rotation.y = PI

    var fiz_controller := FIZTrainController.new()
    fiz_controller.name = "FIZTrainController"
    fiz_controller.data_path = normalized_data_path
    fiz_controller.fiz_filename = file_name
    fiz_controller.train_id = train_id
    fiz_controller.initial_velocity = initial_velocity

    var vehicle := RailVehicle3D.new()
    vehicle.name = "RailVehicle3D"
    vehicle.add_child(model, false, Node.INTERNAL_MODE_BACK)
    vehicle.add_child(fiz_controller, false, Node.INTERNAL_MODE_BACK)
    if low_poly_model:
        vehicle.add_child(low_poly_model, false, Node.INTERNAL_MODE_BACK)
        vehicle.low_poly_cabin_path = vehicle.get_path_to(low_poly_model)
    if passengers_model:
        vehicle.add_child(passengers_model, false, Node.INTERNAL_MODE_BACK)
    vehicle.model_instance_path = vehicle.get_path_to(model)
    # FizTrainControllerInstancer.build() hardcodes the generated controller's name to
    # "TrainController", so this relative path is deterministic even though the controller
    # itself doesn't exist yet (FIZTrainController defers its own build by one frame) - do not
    # resolve it with get_path_to() here, RailVehicle3D's own _process_dirty() will do that once
    # the deferred build has actually run.
    vehicle.controller_path = NodePath("%s/TrainController" % fiz_controller.name)
    vehicle.cabin_scene = _build_cabin_scene(normalized_data_path, file_name, skin)
    vehicle.cabin_rotate_180deg = true
    return vehicle


static func _initialize_instance(vehicle:RailVehicle3D, file_name:String, head_display_material:Material) -> void:
    var model:E3DModelInstance = vehicle.get_node(vehicle.model_instance_path) as E3DModelInstance
    _bind_animation_paths(vehicle, model)
    var abs_mmd_path:String = UserSettings.get_maszyna_game_dir().path_join(model.data_path).path_join(file_name + ".mmd")
    var sound_diagnostics:Array[Dictionary] = []
    MmdSoundBankInstancer.build_into(vehicle, abs_mmd_path, "FIZTrainController", {}, sound_diagnostics)
    for diagnostic:Dictionary in sound_diagnostics:
        if diagnostic["severity"] != "info":
            push_warning("MaszynaRailVehicle3DInstancer: [%s] %s" % [diagnostic["code"], diagnostic["message"]])

    configure_head_display(vehicle, model, head_display_material)


## Public: also called by DynamicRailVehicle3DManager to (re-)apply the per-instance
## head_display_material onto a cached vehicle template's fresh instantiate()'d copy, since
## unlike data_path/file_name/skin it isn't part of that template's cache key (see
## DynamicRailVehicle3DManager's own doc comment for why).
static func configure_head_display(
        vehicle:RailVehicle3D, model:E3DModelInstance, head_display_material:Material) -> void:
    if not head_display_material:
        return
    var head_display:MeshInstance3D = model.find_child("tablice_relacyjne", true, false) as MeshInstance3D
    if not head_display:
        return
    vehicle.head_display_e3d_path = vehicle.get_path_to(model)
    vehicle.head_display_material = head_display_material
    vehicle.head_display_node_path = vehicle.get_path_to(head_display)


## PackedScene.pack()-in-memory trick, already used in production by
## FizTrainControllerInstancer.build_scene() - lets RailVehicle3D.enter_cabin()'s existing
## cabin_scene.instantiate() produce a correctly pre-configured DynamicTrainCabin every time,
## with no changes to rail_vehicle_3d.gd.
static func _build_cabin_scene(normalized_data_path:String, file_name:String, skin:String) -> PackedScene:
    var cabin := DynamicTrainCabin.new()
    cabin.data_path = normalized_data_path
    cabin.mmd_filename = file_name
    cabin.skin = skin
    var packed := PackedScene.new()
    var err:Error = packed.pack(cabin)
    cabin.free()
    if err != OK:
        push_error("MaszynaRailVehicle3DInstancer: could not pack cabin scene for %s/%s" % [normalized_data_path, file_name])
        return null
    return packed


## Resolves vehicle's bogie/wheel animation paths against model's submodel tree. model may not
## be e3d_loaded yet (E3DModelInstance builds its submodel tree in _process(), not synchronously
## in add_child()) - in that case resolution is deferred to model's own e3d_loaded signal.
static func _bind_animation_paths(vehicle:RailVehicle3D, model:E3DModelInstance) -> void:
    if model.is_e3d_loaded():
        _resolve_animation_paths(vehicle, model)
    else:
        model.e3d_loaded.connect(_resolve_animation_paths.bind(vehicle, model), CONNECT_ONE_SHOT)


static func _resolve_animation_paths(vehicle:RailVehicle3D, model:E3DModelInstance) -> void:
    var submodel_index:Dictionary = {}
    _index_submodels(model, submodel_index)

    var front_bogie:Node3D = _find_submodel(submodel_index, FRONT_BOGIE_SUBMODEL_NAMES)
    if front_bogie:
        vehicle.front_bogie_path = vehicle.get_path_to(front_bogie)

    var rear_bogie:Node3D = _find_submodel(submodel_index, REAR_BOGIE_SUBMODEL_NAMES)
    if rear_bogie:
        vehicle.rear_bogie_path = vehicle.get_path_to(rear_bogie)

    var powered_wheel_paths:Array[NodePath] = []
    for axle_index:int in range(1, MAX_WHEEL_AXLES + 1):
        var wheel:Node3D = _find_submodel(submodel_index, ["%s%d" % [WHEEL_SUBMODEL_PREFIX, axle_index]])
        if wheel:
            powered_wheel_paths.append(vehicle.get_path_to(wheel))
    if powered_wheel_paths:
        vehicle.powered_wheel_paths = powered_wheel_paths

    var front_arm_paths:Array[NodePath] = _find_pantograph_arm_paths(vehicle, submodel_index, 1)
    if front_arm_paths:
        vehicle.pantograph_front_arm_paths = front_arm_paths
    var rear_arm_paths:Array[NodePath] = _find_pantograph_arm_paths(vehicle, submodel_index, 2)
    if rear_arm_paths:
        vehicle.pantograph_rear_arm_paths = rear_arm_paths


## Returns all 5 arm/slider paths for the given pantograph number (1=front,
## 2=rear), or [] if any single one is missing - RailVehicle3D only enables
## the raise simulation for an end with all 5 configured, so a partial match
## is treated the same as none (matches _resolve_animation_paths()'s
## "front and rear bogie must be set together" precedent above, per-end here).
static func _find_pantograph_arm_paths(
        vehicle:RailVehicle3D, submodel_index:Dictionary, pantograph_number:int) -> Array[NodePath]:
    var paths:Array[NodePath] = []
    for prefix:String in PANTOGRAPH_ARM_SUBMODEL_PREFIXES:
        var node:Node3D = _find_submodel(submodel_index, ["%s%d" % [prefix, pantograph_number]])
        if not node:
            return []
        paths.append(vehicle.get_path_to(node))
    return paths


static func _find_submodel(submodel_index:Dictionary, names:Array[String]) -> Node3D:
    for submodel_name:String in names:
        var node:Node = submodel_index.get(submodel_name)
        if node is Node3D:
            return node
    return null


## Indexed by LOWERCASED name, matching the original engine's own TSubModel::GetFromName
## (case-insensitive by default) - see mmd_cabin_instancer.gd's _index_submodels for the
## real-data case mismatch (su45_v2) this guards against.
##
## get_children(true) is required: E3DNodesInstancer adds every submodel node as an INTERNAL
## child (INTERNAL_MODE_BACK), which plain get_children() silently skips.
static func _index_submodels(node:Node, index:Dictionary) -> void:
    for child:Node in node.get_children(true):
        var child_name:String = child.name.to_lower()
        if not index.has(child_name):
            index[child_name] = child
        _index_submodels(child, index)
