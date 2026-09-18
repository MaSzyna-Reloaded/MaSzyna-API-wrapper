extends Node

## Cabin layer (#94) - the counterpart of the original engine's TTrain (Train.cpp), kept separate
## from vehicle commands (TrainSystem), which execute on the vehicle immediately.
##
## Holds a CabinState per (train_id, cab) and a registry of cabin control handlers. Cabin controls
## only report manipulations through act(); the handlers are registered by cabin logic delegates
## (e.g. LegacyCabinLogicDelegate) and translate them into vehicle commands. CabinSystem itself
## has no cabin logic and forwards nothing by default.

signal control_changed(train_id:String, cab:int, control_id:StringName, value:Variant)

## Manipulations a control can report (Train.cpp OnCommand_* press/release/repeat/set events).
const ACTIONS:Array[StringName] = [&"increase", &"decrease", &"hold", &"release", &"toggle", &"set"]

var _states:Dictionary = {}
var _controls:Dictionary = {}
var _processes:Dictionary = {}


func _ready() -> void:
    TrainSystem.train_unregistered.connect(_on_train_unregistered)


func _exit_tree() -> void:
    TrainSystem.train_unregistered.disconnect(_on_train_unregistered)


## A removed train takes its cabins along - a new vehicle with the same train_id starts clean.
func _on_train_unregistered(train_id:String) -> void:
    for cab:int in [1, 0, -1]:
        var key:String = _key(train_id, cab)
        _states.erase(key)
        _controls.erase(key)
        _processes.erase(key)


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
