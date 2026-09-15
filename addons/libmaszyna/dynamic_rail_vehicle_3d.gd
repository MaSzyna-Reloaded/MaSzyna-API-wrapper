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

## Base skin name expanded to numbered dynamic-material slots, or an explicit pipe-separated
## slot list when a vehicle uses mixed material names.
@export var skin:String = "":
    set(x):
        if not x == skin:
            skin = x
            _dirty = true

@export var head_display_material:Material:
    set(x):
        if not x == head_display_material:
            head_display_material = x
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

## Forwarded to the generated FIZTrainController.initial_velocity. 0.0 (default) means the
## vehicle starts not-ready-to-depart (battery off, matching the original engine's scenery
## velocity token); a non-zero value marks it ready (battery on per battery_start_mode).
@export var initial_velocity:float = 0.0:
    set(x):
        if not x == initial_velocity:
            initial_velocity = x
            _dirty = true

## Forwarded to the generated FIZTrainController.cabin_number: 1 = cab 1 (headdriver), -1 = cab 2
## (reardriver), 0 = nobody (original engine's scenery driver type, DynObj.cpp:1812-1825).
@export var cabin_number:int = 0:
    set(x):
        if not x == cabin_number:
            cabin_number = x
            _dirty = true

## TrackManager name of the track used to place the generated vehicle.
@export var start_track_name:String = "":
    set(x):
        if not x == start_track_name:
            start_track_name = x
            _track_dirty = true

## Distance in meters along the track's baked curve.
@export var start_track_offset:float = 0.0:
    set(x):
        if not x == start_track_offset:
            start_track_offset = x
            _track_dirty = true

@export_enum("NORMAL", "REVERSED") var start_direction:int = TrackManager.Direction.DIRECTION_NORMAL:
    set(x):
        if not x == start_direction:
            start_direction = x
            _track_dirty = true

## Toggle via the "Edit FIZ" 3D-viewport toolbar button (see
## addons/libmaszyna/editor/fiz_toolbar/) when the wrapped vehicle's FIZTrainController needs to
## be visible/selectable in the Scene dock for inspection - by default _vehicle is added as an
## INTERNAL child (see class doc above), and the Scene dock skips internal nodes and their whole
## subtree outright regardless of node ownership, so nothing under it can otherwise be reached.
## Mirrors FIZTrainController.editable_in_editor exactly.
var editable_in_editor:bool = false:
    set(x):
        if not editable_in_editor == x:
            editable_in_editor = x
            _apply_editable_in_editor()

var _dirty:bool = true
var _track_dirty:bool = false
var _vehicle:RailVehicle3D


func _ready() -> void:
    _dirty = true


func _process(_delta:float) -> void:
    if _dirty:
        _dirty = false
        _track_dirty = false
        _rebuild()
    if _track_dirty:
        _track_dirty = false
        _process_track_dirty()


func _process_track_dirty() -> void:
    if not _vehicle:
        return
    _vehicle.start_track_name = start_track_name
    _vehicle.start_track_offset = start_track_offset
    _vehicle.start_direction = start_direction


func _rebuild() -> void:
    if _vehicle:
        _vehicle.free()
        _vehicle = null

    var vehicle:RailVehicle3D = DynamicRailVehicle3DManager.load(
            data_path, file_name, skin, train_id, initial_velocity, head_display_material, cabin_number)
    if not vehicle:
        return

    _vehicle = vehicle
    add_child(_vehicle, false, INTERNAL_MODE_DISABLED if editable_in_editor else INTERNAL_MODE_BACK)
    _set_owner_recursive(_vehicle, owner if editable_in_editor else self)
    _process_track_dirty()


## Internal mode can only be chosen at add_child() time, so making _vehicle visible/hidden in
## the Scene dock means removing and re-adding it with the other mode (see editable_in_editor
## above).
func _apply_editable_in_editor() -> void:
    if not _vehicle:
        return
    var idx:int = _vehicle.get_index()
    remove_child(_vehicle)
    add_child(_vehicle, false, INTERNAL_MODE_DISABLED if editable_in_editor else INTERNAL_MODE_BACK)
    move_child(_vehicle, idx)
    _set_owner_recursive(_vehicle, owner if editable_in_editor else self)


func _set_owner_recursive(node:Node, target_owner:Node) -> void:
    node.owner = target_owner
    for child:Node in node.get_children(true):
        _set_owner_recursive(child, target_owner)
