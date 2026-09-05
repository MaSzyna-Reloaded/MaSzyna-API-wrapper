@tool
extends Node3D
class_name DynamicRailVehicle3D

## Spawns a complete, driveable MaSzyna vehicle from nothing but a data_path/file_name/skin
## triple: exterior E3D model, FIZ physics controller, and an interactive MMD-driven cabin,
## placed on a named track at a given offset. Deliberately does NOT extend RailVehicle3D -
## it builds one internally and delegates all vehicle behavior (motion, enter_cabin/leave_cabin,
## player-detection Area3D, light sync) to it unmodified, exactly like FIZTrainController wraps
## a generated TrainController instead of extending it.
##
## MaszynaPlayer needs no changes to detect the generated RailVehicle3D: its detection Area3D
## is `_update_detection_area()`'s own direct child of that RailVehicle3D, so a raycast hit on
## it resolves straight to the real RailVehicle3D instance regardless of DynamicRailVehicle3D
## wrapping it.

@export var data_path:String = "":
    set(x):
        if not x == data_path:
            data_path = x
            _dirty = true

## Base filename, without extension, shared by this vehicle's .e3d (exterior model), .fiz
## (physics) and .mmd (cabin) files under data_path.
@export var file_name:String = "":
    set(x):
        if not x == file_name:
            file_name = x
            _dirty = true

## Single skin name applied to the exterior model's first dynamic-material slot.
@export var skin:String = "":
    set(x):
        if not x == skin:
            skin = x
            _dirty = true

## Forwarded to the generated FIZTrainController.train_id (TrainSystem registration/console
## lookups). Not derived from any data file - every DynamicRailVehicle3D otherwise builds its
## FIZTrainController with train_id left at "", so two or more dynamic vehicles all collide on
## the same empty TrainSystem registry key. Must be set explicitly and kept unique per vehicle,
## same as on a hand-authored TrainController.
@export var train_id:String = "":
    set(x):
        if not x == train_id:
            train_id = x
            _dirty = true

## Godot node name of a MaszynaTrack3D found anywhere in the scene tree.
@export var start_track_name:String = "":
    set(x):
        if not x == start_track_name:
            start_track_name = x
            _dirty = true

## Distance in meters along the track's baked curve.
@export var start_track_offset:float = 0.0:
    set(x):
        if not x == start_track_offset:
            start_track_offset = x
            _dirty = true

var _dirty:bool = true
var _vehicle:RailVehicle3D


func _ready() -> void:
    _dirty = true


func _process(_delta:float) -> void:
    if _dirty:
        _dirty = false
        _rebuild()


func _rebuild() -> void:
    if _vehicle:
        remove_child(_vehicle)
        _vehicle.queue_free()
        _vehicle = null

    if not data_path or not file_name:
        return

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
    if body_model_filename.is_empty():
        body_model_filename = file_name
    body_model_filename = MmdCabinInstancer.resolve_model_case(normalized_data_path, body_model_filename)

    var model := E3DModelInstance.new()
    model.name = "ExteriorModel"
    model.data_path = normalized_data_path
    model.model_filename = body_model_filename
    model.skins = MmdCabinInstancer.resolve_skins(normalized_data_path, skin)

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

    var fiz_controller := FIZTrainController.new()
    fiz_controller.name = "FIZTrainController"
    fiz_controller.data_path = normalized_data_path
    fiz_controller.fiz_filename = file_name
    fiz_controller.train_id = train_id

    var vehicle := RailVehicle3D.new()
    vehicle.name = "RailVehicle3D"
    vehicle.add_child(model, false, INTERNAL_MODE_BACK)
    vehicle.add_child(fiz_controller, false, INTERNAL_MODE_BACK)
    if low_poly_model:
        vehicle.add_child(low_poly_model, false, INTERNAL_MODE_BACK)
        vehicle.low_poly_cabin_path = vehicle.get_path_to(low_poly_model)
    vehicle.model_instance_path = vehicle.get_path_to(model)
    # FizTrainControllerInstancer.build() hardcodes the generated controller's name to
    # "TrainController", so this relative path is deterministic even though the controller
    # itself doesn't exist yet (FIZTrainController defers its own build by one frame) - do not
    # resolve it with get_path_to() here, RailVehicle3D's own _process_dirty() will do that once
    # the deferred build has actually run.
    vehicle.controller_path = NodePath("%s/TrainController" % fiz_controller.name)
    vehicle.cabin_scene = _build_cabin_scene(normalized_data_path)

    var sound_diagnostics:Array[Dictionary] = []
    MmdSoundBankInstancer.build_into(vehicle, abs_mmd_path, fiz_controller.name, {}, sound_diagnostics)
    for diagnostic:Dictionary in sound_diagnostics:
        if diagnostic["severity"] != "info":
            push_warning("DynamicRailVehicle3D: [%s] %s" % [diagnostic["code"], diagnostic["message"]])

    _resolve_start_track()

    _vehicle = vehicle
    add_child(_vehicle, false, INTERNAL_MODE_BACK)


## PackedScene.pack()-in-memory trick, already used in production by
## FizTrainControllerInstancer.build_scene() - lets RailVehicle3D.enter_cabin()'s existing
## cabin_scene.instantiate() produce a correctly pre-configured DynamicTrainCabin every time,
## with no changes to rail_vehicle_3d.gd.
func _build_cabin_scene(normalized_data_path:String) -> PackedScene:
    var cabin := DynamicTrainCabin.new()
    cabin.data_path = normalized_data_path
    cabin.mmd_filename = file_name
    cabin.skin = skin
    var packed := PackedScene.new()
    var err:Error = packed.pack(cabin)
    cabin.free()
    if err != OK:
        push_error("DynamicRailVehicle3D: could not pack cabin scene for %s/%s" % [data_path, file_name])
        return null
    return packed


func _resolve_start_track() -> void:
    if not start_track_name:
        return
    var track:MaszynaTrack3D = get_tree().root.find_child(start_track_name, true, false) as MaszynaTrack3D
    if not track:
        push_error("DynamicRailVehicle3D: start_track_name '%s' not found" % start_track_name)
        return
    global_transform = track.global_transform * track.curve.sample_baked_with_rotation(start_track_offset)
