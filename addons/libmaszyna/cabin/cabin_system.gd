extends Node

## Cabin layer (#94) - the counterpart of the original engine's TTrain (Train.cpp), kept separate
## from vehicle commands (RailVehicleServer.vehicle_send_command), which execute on the vehicle
## immediately.
##
## Holds a CabinState per (vehicle, cab) and a registry of cabin control handlers. A vehicle is
## its RailVehicleServer handle - never its scenery name, which two vehicles may share and one may
## lack. Cabin controls only report manipulations through act(); the handlers are registered by
## cabin logic delegates (e.g. LegacyCabinLogicDelegate) and translate them into vehicle commands.
## CabinSystem itself has no cabin logic and forwards nothing by default.

signal control_changed(vehicle_rid:RID, cab:int, control_id:StringName, value:Variant)
## A command reached the vehicle, from wherever - the console, a keybind, another cab. Relayed
## here so a cabin element can react without ever holding the vehicle itself.
signal vehicle_command_received(vehicle_rid:RID, command:String, p1:Variant, p2:Variant)
## The occupied cab of a vehicle changed - relayed for the same reason as the commands above.
signal vehicle_cabin_occupied_changed(vehicle_rid:RID, cabin_occupied:int)

## Manipulations a control can report (Train.cpp OnCommand_* press/release/repeat/set events).
const ACTIONS:Array[StringName] = [&"increase", &"decrease", &"hold", &"release", &"toggle", &"set"]

var _states:Dictionary = {}
var _controls:Dictionary = {}
var _processes:Dictionary = {}


func _ready() -> void:
    RailVehicleServer.vehicle_command_received.connect(_on_vehicle_command_received)
    RailVehicleServer.vehicle_occupied_cab_changed.connect(_on_vehicle_occupied_cab_changed)
    RailVehicleServer.vehicle_freed.connect(_on_vehicle_freed)


func _exit_tree() -> void:
    RailVehicleServer.vehicle_command_received.disconnect(_on_vehicle_command_received)
    RailVehicleServer.vehicle_occupied_cab_changed.disconnect(_on_vehicle_occupied_cab_changed)
    RailVehicleServer.vehicle_freed.disconnect(_on_vehicle_freed)


## A freed vehicle takes its cabins along - a handle is never reused for another vehicle.
func _on_vehicle_freed(vehicle_rid:RID) -> void:
    for cab:int in [1, 0, -1]:
        var key:String = _key(vehicle_rid, cab)
        _states.erase(key)
        _controls.erase(key)
        _processes.erase(key)


func _on_vehicle_command_received(vehicle_rid:RID, command:String, p1:Variant, p2:Variant) -> void:
    vehicle_command_received.emit(vehicle_rid, command, p1, p2)


func _on_vehicle_occupied_cab_changed(vehicle_rid:RID, cabin_occupied:int) -> void:
    vehicle_cabin_occupied_changed.emit(vehicle_rid, cabin_occupied)


## The whole vehicle's state, by name. RailVehicleServer builds it once per step and keeps it until
## the step or a command moves it on, so the dozens of elements of a cab asking in one frame share
## one dump. An element that reads one value often enough to care takes its component instead
## (vehicle_component() below).
func vehicle_state(vehicle_rid:RID) -> Dictionary:
    return RailVehicleServer.vehicle_dump_state(vehicle_rid) if vehicle_rid.is_valid() else {}


## One named value of the vehicle's state. This is what a cabin element wants: it is driven by a
## property name out of the MMD and reads exactly one of them, so handing it the whole dump only
## gives it something to hold wrongly.
func vehicle_state_value(vehicle_rid:RID, key:String, default_value:Variant = null) -> Variant:
    return vehicle_state(vehicle_rid).get(key, default_value)


func vehicle_config(vehicle_rid:RID) -> Dictionary:
    return RailVehicleServer.vehicle_dump_config(vehicle_rid) if vehicle_rid.is_valid() else {}


## One component of the vehicle, by kind - for an element that reads a value often enough to want
## the typed property rather than the dump.
func vehicle_component(vehicle_rid:RID, type:int) -> VehicleComponent:
    return RailVehicleServer.vehicle_component_get(vehicle_rid, type) if vehicle_rid.is_valid() else null


## Which cab of this vehicle is occupied - 1, 0 (machine room) or -1, as CabinState keys on.
func occupied_cab(vehicle_rid:RID) -> int:
    return int(vehicle_state(vehicle_rid).get("cabin_occupied", 1))


static func _key(vehicle_rid:RID, cab:int) -> String:
    return "%d:%d" % [vehicle_rid.get_id(), cab]


func get_cabin_state(vehicle_rid:RID, cab:int) -> CabinState:
    var key:String = _key(vehicle_rid, cab)
    if not _states.has(key):
        _states[key] = CabinState.new(vehicle_rid, cab)
    return _states[key]


## handler(state:CabinState, action:StringName, value:Variant) -> Variant
# FIXME(#184, #28): imperative, per-vehicle registration mirroring VehicleController.register_command;
# cabin behaviours should rather declare the control ids/actions they handle.
func register_control(vehicle_rid:RID, cab:int, control_id:StringName, handler:Callable) -> void:
    var key:String = _key(vehicle_rid, cab)
    if not _controls.has(key):
        _controls[key] = {}
    _controls[key][control_id] = handler


func unregister_control(vehicle_rid:RID, cab:int, control_id:StringName, handler:Callable) -> void:
    var controls:Dictionary = _controls.get(_key(vehicle_rid, cab), {})
    if controls.get(control_id) == handler:
        controls.erase(control_id)


func has_control(vehicle_rid:RID, cab:int, control_id:StringName) -> bool:
    return _controls.get(_key(vehicle_rid, cab), {}).has(control_id)


## Control ids with a registered handler in the given cab, sorted.
func get_controls(vehicle_rid:RID, cab:int) -> Array:
    var controls:Array = _controls.get(_key(vehicle_rid, cab), {}).keys()
    controls.sort()
    return controls


## callable(state:CabinState, delta:float) - called every frame while registered
func register_process(vehicle_rid:RID, cab:int, callable:Callable) -> void:
    var key:String = _key(vehicle_rid, cab)
    get_cabin_state(vehicle_rid, cab)
    if not _processes.has(key):
        _processes[key] = []
    _processes[key].append(callable)


func unregister_process(vehicle_rid:RID, cab:int, callable:Callable) -> void:
    var processes:Array = _processes.get(_key(vehicle_rid, cab), [])
    processes.erase(callable)


## Reports a manipulation of a cabin control; returns the handler's result (#43), or null when
## no handler is registered for the control.
func act(vehicle_rid:RID, cab:int, control_id:StringName, action:StringName, value:Variant = null) -> Variant:
    if not action in ACTIONS:
        GameLog.error("%s: Unknown cabin action: %s" % [RailVehicleServer.vehicle_get_name(vehicle_rid), action])
        return null
    var handler:Callable = _controls.get(_key(vehicle_rid, cab), {}).get(control_id, Callable())
    if not handler.is_valid():
        GameLog.error("%s: Unknown cabin control: %s (cab %d)" % [
            RailVehicleServer.vehicle_get_name(vehicle_rid), control_id, cab])
        return null
    return handler.call(get_cabin_state(vehicle_rid, cab), action, value)


func get_control(vehicle_rid:RID, cab:int, control_id:StringName) -> Variant:
    return get_cabin_state(vehicle_rid, cab).get_value(control_id)


func get_state(vehicle_rid:RID, cab:int) -> Dictionary:
    return get_cabin_state(vehicle_rid, cab).values.duplicate()


func _process(delta:float) -> void:
    for key:String in _processes:
        var state:CabinState = _states.get(key)
        for callable:Callable in _processes[key].duplicate():
            callable.call(state, delta)
