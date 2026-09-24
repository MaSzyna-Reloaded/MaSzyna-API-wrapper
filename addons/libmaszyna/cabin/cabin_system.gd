extends Node

## Cabin layer (#94) - the counterpart of the original engine's TTrain (Train.cpp), kept separate
## from vehicle commands (TrainSystem), which execute on the vehicle immediately.
##
## Holds a CabinState per (train_id, cab) and a registry of cabin control handlers. Cabin controls
## only report manipulations through act(); the handlers are registered by cabin logic delegates
## (e.g. LegacyCabinLogicDelegate) and translate them into vehicle commands. CabinSystem itself
## has no cabin logic and forwards nothing by default.

signal control_changed(train_id:String, cab:int, control_id:StringName, value:Variant)
## A command reached the vehicle, from wherever - the console, a keybind, another cab. Relayed
## here so a cabin element can react without ever holding the vehicle itself.
signal vehicle_command_received(train_id:String, command:String, p1:Variant, p2:Variant)
## The occupied cab of a vehicle changed - relayed for the same reason as the commands above.
signal vehicle_cabin_occupied_changed(train_id:String, cabin_occupied:int)

## Manipulations a control can report (Train.cpp OnCommand_* press/release/repeat/set events).
const ACTIONS:Array[StringName] = [&"increase", &"decrease", &"hold", &"release", &"toggle", &"set"]

var _states:Dictionary = {}
## train_id -> the vehicle. The cabin is the one place that knows which vehicle it sits in, so it
## is the one place that talks to the vehicle servers; a cabin element never does.
var _vehicles:Dictionary = {}
## The vehicle dumps handed out this frame, by vehicle handle, with the command counter each was
## built at. A cab is dozens of elements asking the same vehicle within one frame, so the first of
## them builds the dump and the rest read what it built - but a control reports its manipulation
## and reads the result in that same frame, so a command run since makes the dump stale. That is
## what the counter is for, and it is the half the server's own cache is keyed on for the same
## reason (see `FINDINGS.md`, 2026-09-23). Held per frame rather than emptied by a tick of its
## own, so nothing here runs while no cab is looking.
var _vehicle_states:Dictionary[RID, Dictionary] = {}
var _vehicle_state_serials:Dictionary[RID, int] = {}
var _vehicle_states_frame:int = -1
var _controls:Dictionary = {}
var _processes:Dictionary = {}


func _ready() -> void:
    TrainSystem.train_registered.connect(register_vehicle)
    TrainSystem.train_unregistered.connect(_on_train_unregistered)


func _exit_tree() -> void:
    TrainSystem.train_registered.disconnect(register_vehicle)
    TrainSystem.train_unregistered.disconnect(_on_train_unregistered)


## A removed train takes its cabins along - a new vehicle with the same train_id starts clean.
func _on_train_unregistered(train_id:String) -> void:
    var vehicle:VehicleController = _vehicles.get(train_id)
    if vehicle:
        vehicle.command_received.disconnect(_on_vehicle_command_received.bind(train_id))
        vehicle.cabin_occupied_changed.disconnect(_on_vehicle_cabin_occupied_changed.bind(train_id))
        # the dump this frame described a vehicle that is gone; whoever asks after this gets none
        _vehicle_states.erase(vehicle.get_rid())
        _vehicle_state_serials.erase(vehicle.get_rid())
    _vehicles.erase(train_id)
    for cab:int in [1, 0, -1]:
        var key:String = _key(train_id, cab)
        _states.erase(key)
        _controls.erase(key)
        _processes.erase(key)


## Taken straight off TrainSystem's own announcement: a vehicle is known here from the moment it
## exists, so a cab only has to name the one it sits in. Everything else about it - the handle,
## the state, the components - this system takes from the servers. The matching disconnect is in
## _on_train_unregistered(), which is why the vehicle is kept here rather than looked up: by then
## TrainSystem has already let go of it.
func register_vehicle(train_id:String) -> void:
    var vehicle:VehicleController = TrainSystem.get_train(train_id)
    if not vehicle:
        return
    _vehicles[train_id] = vehicle
    vehicle.command_received.connect(_on_vehicle_command_received.bind(train_id))
    vehicle.cabin_occupied_changed.connect(_on_vehicle_cabin_occupied_changed.bind(train_id))


func _on_vehicle_command_received(command:String, p1:Variant, p2:Variant, train_id:String) -> void:
    vehicle_command_received.emit(train_id, command, p1, p2)


func _on_vehicle_cabin_occupied_changed(cabin_occupied:int, train_id:String) -> void:
    vehicle_cabin_occupied_changed.emit(train_id, cabin_occupied)


func vehicle_rid(train_id:String) -> RID:
    var vehicle:VehicleController = _vehicles.get(train_id)
    return vehicle.get_rid() if vehicle else RID()


## The whole vehicle's state, by name - built on the first ask of each frame and handed out
## unchanged to every element that asks after it. An element that reads one value often enough to
## care takes its component instead (vehicle_component() below); this is for the ones that read a
## handful of unrelated ones, which is most of a cab.
func vehicle_state(train_id:String) -> Dictionary:
    var rid:RID = vehicle_rid(train_id)
    if not rid.is_valid():
        return {}
    var frame:int = Engine.get_process_frames()
    if not _vehicle_states_frame == frame:
        _vehicle_states_frame = frame
        _vehicle_states.clear()
        _vehicle_state_serials.clear()
    var vehicle:VehicleController = _vehicles.get(train_id)
    var serial:int = vehicle.get_command_serial() if vehicle else 0
    if not _vehicle_states.has(rid) or not _vehicle_state_serials.get(rid, -1) == serial:
        _vehicle_states[rid] = RailVehicleServer.vehicle_dump_state(rid)
        _vehicle_state_serials[rid] = serial
    return _vehicle_states[rid]


func vehicle_config(train_id:String) -> Dictionary:
    var rid:RID = vehicle_rid(train_id)
    return RailVehicleServer.vehicle_dump_config(rid) if rid.is_valid() else {}


## One component of the vehicle, by kind - for an element that reads a value often enough to want
## the typed property rather than the dump.
func vehicle_component(train_id:String, type:int) -> VehicleComponent:
    var rid:RID = vehicle_rid(train_id)
    return RailVehicleServer.vehicle_component_get(rid, type) if rid.is_valid() else null


## Which cab of this vehicle is occupied - 1, 0 (machine room) or -1, as CabinState keys on.
func occupied_cab(train_id:String) -> int:
    return int(vehicle_state(train_id).get("cabin_occupied", 1))


static func _key(train_id:String, cab:int) -> String:
    return "%s:%d" % [train_id, cab]


func get_cabin_state(train_id:String, cab:int) -> CabinState:
    var key:String = _key(train_id, cab)
    if not _states.has(key):
        _states[key] = CabinState.new(train_id, cab)
    return _states[key]


## handler(state:CabinState, action:StringName, value:Variant) -> Variant
# FIXME(#184, #28): imperative, per-train_id registration mirroring TrainSystem.register_command;
# cabin behaviours should rather declare the control ids/actions they handle.
func register_control(train_id:String, cab:int, control_id:StringName, handler:Callable) -> void:
    var key:String = _key(train_id, cab)
    if not _controls.has(key):
        _controls[key] = {}
    _controls[key][control_id] = handler


func unregister_control(train_id:String, cab:int, control_id:StringName, handler:Callable) -> void:
    var controls:Dictionary = _controls.get(_key(train_id, cab), {})
    if controls.get(control_id) == handler:
        controls.erase(control_id)


func has_control(train_id:String, cab:int, control_id:StringName) -> bool:
    return _controls.get(_key(train_id, cab), {}).has(control_id)


## Control ids with a registered handler in the given cab, sorted.
func get_controls(train_id:String, cab:int) -> Array:
    var controls:Array = _controls.get(_key(train_id, cab), {}).keys()
    controls.sort()
    return controls


## callable(state:CabinState, delta:float) - called every frame while registered
func register_process(train_id:String, cab:int, callable:Callable) -> void:
    var key:String = _key(train_id, cab)
    get_cabin_state(train_id, cab)
    if not _processes.has(key):
        _processes[key] = []
    _processes[key].append(callable)


func unregister_process(train_id:String, cab:int, callable:Callable) -> void:
    var processes:Array = _processes.get(_key(train_id, cab), [])
    processes.erase(callable)


## Reports a manipulation of a cabin control; returns the handler's result (#43), or null when
## no handler is registered for the control.
func act(train_id:String, cab:int, control_id:StringName, action:StringName, value:Variant = null) -> Variant:
    if not action in ACTIONS:
        TrainSystem.log(train_id, GameLog.LogLevel.ERROR, "Unknown cabin action: %s" % action)
        return null
    var handler:Callable = _controls.get(_key(train_id, cab), {}).get(control_id, Callable())
    if not handler.is_valid():
        TrainSystem.log(train_id, GameLog.LogLevel.ERROR, "Unknown cabin control: %s (cab %d)" % [control_id, cab])
        return null
    return handler.call(get_cabin_state(train_id, cab), action, value)


func get_control(train_id:String, cab:int, control_id:StringName) -> Variant:
    return get_cabin_state(train_id, cab).get_value(control_id)


func get_state(train_id:String, cab:int) -> Dictionary:
    return get_cabin_state(train_id, cab).values.duplicate()


func _process(delta:float) -> void:
    for key:String in _processes:
        var state:CabinState = _states.get(key)
        for callable:Callable in _processes[key].duplicate():
            callable.call(state, delta)
