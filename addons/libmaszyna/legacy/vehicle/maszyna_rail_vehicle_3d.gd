@tool
extends RailVehicle3D
class_name MaszynaRailVehicle3D

## A complete, driveable MaSzyna vehicle from nothing but a data_path/file_name/skin triple, as a
## scenery's `dynamic` places it. It builds itself (MaszynaRailVehicle3DManager.build_into()): its
## physics, sounds and the like as internal children, its appearance, cargo and cab handed to the
## servers by its handle - RailVehicleRenderingServer draws and moves it, CabinSystem keeps its cab.
## Everything it stores is its own data below and what RailVehicle3D has of a vehicle's place
## (start_track_*, head_display_material); what it builds is never saved with the scene.

## The vehicle has been (re)built: it has its handle and parts now, or none when its data cannot
## be read
signal vehicle_built

@export var data_path:String = "":
    set(x):
        if not x == data_path:
            data_path = x
            _dirty = true
            set_process(true)

## Base filename, without extension, shared by this vehicle's .e3d (exterior model), .fiz
## (physics) and .mmd (cabin) files under data_path.
@export var file_name:String = "":
    set(x):
        if not x == file_name:
            file_name = x
            _dirty = true
            set_process(true)

## Base skin name expanded to numbered dynamic-material slots, or an explicit pipe-separated
## slot list when a vehicle uses mixed material names.
@export var skin:String = "":
    set(x):
        if not x == skin:
            skin = x
            _dirty = true
            set_process(true)

## The scenery's name for this vehicle, registered with VehicleServer.vehicle_set_name(), which is
## how an event, a scenario or the console find a vehicle by name. It may be empty or repeated -
## everything that holds the vehicle uses its RID, so only a lookup by that name is affected.
@export var vehicle_id:String = "":
    set(x):
        if not x == vehicle_id:
            vehicle_id = x
            _dirty = true
            set_process(true)

## 0.0 (default) means the vehicle starts not-ready-to-depart (battery off, matching the original
## engine's scenery velocity token); a non-zero value marks it ready (battery on per
## battery_start_mode).
@export var initial_velocity:float = 0.0:
    set(x):
        if not x == initial_velocity:
            initial_velocity = x
            _dirty = true
            set_process(true)

## Who is aboard, in the words the `.scn` uses for it - `headdriver`, `reardriver` or `nobody`
## (DynObj.cpp:1812-1825). Not the number of a cab: it says which cab is occupied, and a vehicle
## nobody occupies is not simulated at all (Driver.cpp:2126).
@export var driver_type:VehicleController.DriverType = VehicleController.DRIVER_NOBODY:
    set(x):
        if not x == driver_type:
            driver_type = x
            _dirty = true
            set_process(true)

## What the scenery loaded the vehicle with: the cargo's own name and how much of it
## (`loadcount` and `loadtype` of a `dynamic`).
@export var load_name:String = "":
    set(x):
        if not x == load_name:
            load_name = x
            _dirty = true
            set_process(true)

@export var load_amount:float = 0.0:
    set(x):
        if not x == load_amount:
            load_amount = x
            _dirty = true
            set_process(true)

## Toggled by the "Edit FIZ" 3D-viewport toolbar button (addons/libmaszyna/editor/fiz_toolbar/) when
## the vehicle's parts need to be visible and selectable in the Scene dock for inspection - they are
## internal children, which the Scene dock skips.
var editable_in_editor:bool = false:
    set(x):
        if not editable_in_editor == x:
            editable_in_editor = x
            _apply_editable_in_editor()

## A change of the data rebuilds the vehicle once, whatever else changes in the same frame; the node
## processes only while a rebuild is pending
var _dirty:bool = true
## What the last build put into this node
var _parts:Array[Node] = []


func _enter_tree() -> void:
    RailVehicleRenderingServer.vehicle_model_built.connect(_on_vehicle_model_built)
    GameDataServer.data_unload_requested.connect(_on_data_unload_requested)
    # the editor drives no vehicle, and has no CabinSystem
    if not Engine.is_editor_hint():
        DriverSystem.vehicle_driven_changed.connect(_on_vehicle_driven_changed)


func _exit_tree() -> void:
    MaszynaRailVehicle3DManager.build_cancel(self)
    RailVehicleRenderingServer.vehicle_model_built.disconnect(_on_vehicle_model_built)
    GameDataServer.data_unload_requested.disconnect(_on_data_unload_requested)
    if not Engine.is_editor_hint():
        DriverSystem.vehicle_driven_changed.disconnect(_on_vehicle_driven_changed)


## false while a rebuild is pending - true once it ran, even when the vehicle failed to load
func is_built() -> bool:
    return not _dirty


## A change asks for a build; MaszynaRailVehicle3DManager runs it, spreading the vehicles of a
## scenery over frames
func _process(_delta:float) -> void:
    set_process(false)
    if _dirty:
        MaszynaRailVehicle3DManager.build_request(self)


## Builds the vehicle anew from what it is set to. Called by MaszynaRailVehicle3DManager in its turn.
func build() -> void:
    if not _dirty:
        return
    _dirty = false
    _free_parts()
    _parts = MaszynaRailVehicle3DManager.build_into(
            self, data_path, file_name, skin, vehicle_id, initial_velocity, driver_type, load_name, load_amount)
    # built as internal children; shown in the Scene dock only while "Edit FIZ" is on
    if editable_in_editor:
        _apply_editable_in_editor()
    vehicle_built.emit()


## The vehicle goes at once with the data it was built of - nothing of it is built again from the new
## data before it is itself - and is built anew in the next frame
func _on_data_unload_requested() -> void:
    _free_parts()
    _dirty = true
    set_process(true)


## The old vehicle goes with its parts - its physics frees its handle
func _free_parts() -> void:
    for part:Node in _parts:
        remove_child(part)
        part.free()
    _parts.clear()


## The cab logic is the vehicle's while somebody drives it - its driver or the player - whether a
## 3D cab is shown or not: the AI and the player act on the same controls (CabinSystem). The original
## keeps a TTrain only for a driven train; a cab of every vehicle at work costs every frame.
func _on_vehicle_driven_changed(vehicle:RID, driven:bool) -> void:
    if not get_rid() == vehicle:
        return
    CabinSystem.vehicle_attach_cab_logic(vehicle, LegacyCabinLogic.from_mmd(data_path, file_name) if driven else null)


func _on_vehicle_model_built(vehicle:RID) -> void:
    if get_rid() == vehicle:
        MaszynaRailVehicle3DInstancer.add_mirrors(self)


## Internal mode can only be chosen at add_child() time, so showing the parts in the Scene dock means
## removing and re-adding them with the other mode.
func _apply_editable_in_editor() -> void:
    var mode:InternalMode = INTERNAL_MODE_DISABLED if editable_in_editor else INTERNAL_MODE_BACK
    for part:Node in _parts:
        remove_child(part)
        add_child(part, false, mode)
        _set_owner_recursive(part, owner if editable_in_editor else null)


func _set_owner_recursive(node:Node, target_owner:Node) -> void:
    node.owner = target_owner
    for child:Node in node.get_children(true):
        _set_owner_recursive(child, target_owner)
