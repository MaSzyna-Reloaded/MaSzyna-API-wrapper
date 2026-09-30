@tool
extends Node3D
class_name MaszynaRailVehicle3D

## Spawns a complete, driveable MaSzyna vehicle from nothing but a data_path/file_name/skin
## triple: exterior E3D model, FIZ physics controller, and an interactive MMD-driven cabin,
## placed on a named track at a given offset. Deliberately does NOT extend RailVehicle3D -
## it builds one internally and delegates all vehicle behavior (motion, show_cabin/hide_cabin,
## player-detection Area3D, light sync) to it unmodified, exactly like MaszynaRailVehiclePhysicsNode wraps
## a generated VehicleController instead of extending it.
##
## MaszynaPlayer needs no changes to detect the generated RailVehicle3D: its detection Area3D
## is `_update_detection_area()`'s own direct child of that RailVehicle3D, so a raycast hit on
## it resolves straight to the real RailVehicle3D instance regardless of MaszynaRailVehicle3D
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

## The scenery's name for this vehicle, forwarded to the generated MaszynaRailVehiclePhysicsNode.vehicle_id
## and registered with VehicleServer.vehicle_set_name(), which is how an event, a scenario or
## the console find a vehicle by name. It may be empty or repeated - everything that holds the
## vehicle uses its RID, so only a lookup by that name is affected.
@export var vehicle_id:String = "":
    set(x):
        if not x == vehicle_id:
            vehicle_id = x
            _dirty = true

## Forwarded to the generated MaszynaRailVehiclePhysicsNode.initial_velocity. 0.0 (default) means the
## vehicle starts not-ready-to-depart (battery off, matching the original engine's scenery
## velocity token); a non-zero value marks it ready (battery on per battery_start_mode).
@export var initial_velocity:float = 0.0:
    set(x):
        if not x == initial_velocity:
            initial_velocity = x
            _dirty = true

## Who is aboard, in the words the `.scn` uses for it - `headdriver`, `reardriver` or `nobody`
## (DynObj.cpp:1812-1825). Not the number of a cab: it says which cab is occupied, and a vehicle
## nobody occupies is not simulated at all (Driver.cpp:2126).
@export var driver_type:VehicleController.DriverType = VehicleController.DRIVER_NOBODY:
    set(x):
        if not x == driver_type:
            driver_type = x
            _dirty = true

## What the scenery loaded the vehicle with: the cargo's own name and how much of it
## (`loadcount` and `loadtype` of a `dynamic`).
@export var load_name:String = "":
    set(x):
        if not x == load_name:
            load_name = x
            _dirty = true

@export var load_amount:float = 0.0:
    set(x):
        if not x == load_amount:
            load_amount = x
            _dirty = true

## TrackServer name of the track used to place the generated vehicle.
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

@export_enum("NORMAL", "REVERSED") var start_direction:int = TrackServer.DIRECTION_NORMAL:
    set(x):
        if not x == start_direction:
            start_direction = x
            _track_dirty = true

## Toggle via the "Edit FIZ" 3D-viewport toolbar button (see
## addons/libmaszyna/editor/fiz_toolbar/) when the wrapped vehicle needs to be visible/selectable
## in the Scene dock for inspection - by default _vehicle is added as an INTERNAL child (see class
## doc above), and the Scene dock skips internal nodes and their whole subtree outright regardless
## of node ownership, so nothing under it can otherwise be reached.
var editable_in_editor:bool = false:
    set(x):
        if not editable_in_editor == x:
            editable_in_editor = x
            _apply_editable_in_editor()

var _dirty:bool = true
var _track_dirty:bool = false
var _vehicle:RailVehicle3D
## The vehicle's internal parts "Edit FIZ" shows, by instance id - hidden again as they were
var _shown_parts:Array[int] = []


func _ready() -> void:
    _dirty = true


func _enter_tree() -> void:
    # the editor drives no vehicle, and has no CabinSystem
    if not Engine.is_editor_hint():
        DriverSystem.vehicle_driven_changed.connect(_on_vehicle_driven_changed)


func _exit_tree() -> void:
    if not Engine.is_editor_hint():
        DriverSystem.vehicle_driven_changed.disconnect(_on_vehicle_driven_changed)


## false while a rebuild (building the vehicle in _process) is pending - true once it ran, even when
## the vehicle failed to load
func is_built() -> bool:
    return not _dirty


## whether the vehicle stands on its start track - false until its simulation placed it there
func is_placed() -> bool:
    return RailVehicleServer.vehicle_get_track_position(get_rid()).get("track_rid", RID()).is_valid()


## The vehicle's handle; nothing can be read off it before its simulation is
## (VehicleServer.vehicle_is_simulation_ready())
func get_rid() -> RID:
    return _vehicle.get_rid() if _vehicle else RID()


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

    var vehicle:RailVehicle3D = MaszynaRailVehicle3DManager.load(
            data_path, file_name, skin, vehicle_id, initial_velocity, head_display_material,
            driver_type, load_name, load_amount)
    if not vehicle:
        return

    _vehicle = vehicle
    add_child(_vehicle, false, INTERNAL_MODE_DISABLED if editable_in_editor else INTERNAL_MODE_BACK)
    _set_owner_recursive(_vehicle, owner if editable_in_editor else self)
    _process_track_dirty()


## The cab logic is the vehicle's while somebody drives it - its driver or the player - whether a
## 3D cab is shown or not: the AI and the player act on the same controls (CabinSystem). The original
## keeps a TTrain only for a driven train; a cab of every vehicle at work costs every frame.
func _on_vehicle_driven_changed(vehicle:RID, driven:bool) -> void:
    if not get_rid() == vehicle:
        return
    CabinSystem.vehicle_attach_cab_logic(vehicle, LegacyCabinLogic.from_mmd(data_path, file_name) if driven else null)


## Internal mode can only be chosen at add_child() time, so making _vehicle visible/hidden in
## the Scene dock means removing and re-adding it with the other mode (see editable_in_editor
## above).
func _apply_editable_in_editor() -> void:
    if not _vehicle:
        return
    var mode:InternalMode = INTERNAL_MODE_DISABLED if editable_in_editor else INTERNAL_MODE_BACK
    var idx:int = _vehicle.get_index()
    remove_child(_vehicle)
    # the vehicle's parts are internal children too (MaszynaRailVehicle3DInstancer) - switched while
    # it is out of the tree, so they do not leave and enter it a second time; the ones shown are
    # the ones hidden again
    if editable_in_editor:
        _shown_parts.clear()
        var visible_parts:Array[Node] = _vehicle.get_children(false)
        for part:Node in _vehicle.get_children(true):
            if not part in visible_parts:
                _shown_parts.append(part.get_instance_id())
    for part_id:int in _shown_parts:
        var part:Node = instance_from_id(part_id) as Node
        if part:
            _vehicle.remove_child(part)
            _vehicle.add_child(part, false, mode)
    add_child(_vehicle, false, mode)
    move_child(_vehicle, idx)
    _set_owner_recursive(_vehicle, owner if editable_in_editor else self)


func _set_owner_recursive(node:Node, target_owner:Node) -> void:
    node.owner = target_owner
    for child:Node in node.get_children(true):
        _set_owner_recursive(child, target_owner)
