extends Node

## Cabin layer (#94) - the counterpart of the original engine's TTrain (Train.cpp), kept separate
## from vehicle commands (VehicleServer.vehicle_send_command), which execute on the vehicle
## immediately.
##
## Holds a CabinState per (vehicle, cab) and a registry of cabin control handlers. A vehicle is
## its VehicleServer handle - never its scenery name, which two vehicles may share and one may
## lack. Cabin controls only report manipulations through act(); the handlers are registered by
## the CabinLogic attached to the vehicle (e.g. LegacyCabinLogic) and translate them into vehicle
## commands. CabinSystem itself has no cabin logic and forwards nothing by default.

signal control_changed(vehicle_rid:RID, cab:int, control_id:StringName, value:Variant)
## A command reached the vehicle, from wherever - the console, a keybind, another cab. Relayed
## here so a cabin element can react without ever holding the vehicle itself.
signal vehicle_command_received(vehicle_rid:RID, command:String, p1:Variant, p2:Variant)
## The occupied cab of a vehicle changed - relayed for the same reason as the commands above.
signal vehicle_cabin_occupied_changed(vehicle_rid:RID, cabin_occupied:int)
## A radio message sent from `position`: heard on the radio of the player's cab tuned to
## `channel`, within `reach` [m] of it when that is positive (simulation::radio_message(),
## simulation.cpp:506); `transcript` is what it says, null when unknown
signal radio_message_sent(message:SfxEvent, transcript:Transcript, channel:int, position:Vector3, reach:float)

## Manipulations a control can report (Train.cpp OnCommand_* press/release/repeat/set events).
const ACTIONS:Array[StringName] = [&"increase", &"decrease", &"hold", &"release", &"toggle", &"set"]

var _states:Dictionary = {}
var _controls:Dictionary = {}
var _processes:Dictionary = {}
var _cab_logics:Dictionary[RID, CabinLogic] = {}
## The cab interior each vehicle's crew sits in, and the one shown, by instance id
var _cabin_scenes:Dictionary[RID, PackedScene] = {}
var _cabins:Dictionary[RID, int] = {}


func _ready() -> void:
    SimulationServer.simulation_advanced.connect(_on_simulation_advanced)
    VehicleServer.vehicle_command_received.connect(_on_vehicle_command_received)
    RailVehicleServer.vehicle_occupied_cab_changed.connect(_on_vehicle_occupied_cab_changed)
    VehicleServer.vehicle_freed.connect(_on_vehicle_freed)


func _exit_tree() -> void:
    SimulationServer.simulation_advanced.disconnect(_on_simulation_advanced)
    VehicleServer.vehicle_command_received.disconnect(_on_vehicle_command_received)
    RailVehicleServer.vehicle_occupied_cab_changed.disconnect(_on_vehicle_occupied_cab_changed)
    VehicleServer.vehicle_freed.disconnect(_on_vehicle_freed)


## A freed vehicle takes its cabins along - a handle is never reused for another vehicle.
func _on_vehicle_freed(vehicle_rid:RID) -> void:
    if _cab_logics.has(vehicle_rid):
        _cab_logics[vehicle_rid].unregister()
        _cab_logics.erase(vehicle_rid)
    _cabin_scenes.erase(vehicle_rid)
    _cabins.erase(vehicle_rid)
    for cab:int in [1, 0, -1]:
        var key:String = _key(vehicle_rid, cab)
        _states.erase(key)
        _controls.erase(key)
        _processes.erase(key)


func _on_vehicle_command_received(vehicle_rid:RID, command:String, p1:Variant, p2:Variant) -> void:
    vehicle_command_received.emit(vehicle_rid, command, p1, p2)


func _on_vehicle_occupied_cab_changed(vehicle_rid:RID, cabin_occupied:int) -> void:
    # the crew moved: the controls are those of the other cab now, before anybody hears of the move
    # (MaszynaDynamicTrainCabin rebuilds its widgets on the relayed signal)
    if _cab_logics.has(vehicle_rid):
        _cab_logics[vehicle_rid].unregister()
        _cab_logics[vehicle_rid].register(vehicle_rid, cabin_occupied)
    vehicle_cabin_occupied_changed.emit(vehicle_rid, cabin_occupied)


## The whole vehicle's state, by name. VehicleServer builds it once per step and keeps it until
## the step or a command moves it on, so the dozens of elements of a cab asking in one frame share
## one dump. An element that reads one value often enough to care takes its component instead
## (vehicle_component() below).
func vehicle_state(vehicle_rid:RID) -> Dictionary:
    return VehicleServer.vehicle_dump_state(vehicle_rid) if vehicle_rid.is_valid() else {}


## One named value of the vehicle's state. This is what a cabin element wants: it is driven by a
## property name out of the MMD and reads exactly one of them, so handing it the whole dump only
## gives it something to hold wrongly.
func vehicle_state_value(vehicle_rid:RID, key:String, default_value:Variant = null) -> Variant:
    return vehicle_state(vehicle_rid).get(key, default_value)


func vehicle_config(vehicle_rid:RID) -> Dictionary:
    return VehicleServer.vehicle_dump_config(vehicle_rid) if vehicle_rid.is_valid() else {}


## One component of the vehicle, by kind - for an element that reads a value often enough to want
## the typed property rather than the dump.
func vehicle_component(vehicle_rid:RID, type:VehicleComponentType.Type) -> VehicleComponent:
    return VehicleServer.vehicle_component_get(vehicle_rid, type) if vehicle_rid.is_valid() else null


## Which cab of this vehicle is occupied - 1, 0 (machine room) or -1, as CabinState keys on.
func occupied_cab(vehicle_rid:RID) -> int:
    return int(vehicle_state(vehicle_rid).get("cabin_occupied", 1))


## The cab logic of a driven vehicle, registered for its occupied cab; null detaches it. One per
## vehicle, whoever drives it - the player's cab and the AI act on the same controls.
func vehicle_attach_cab_logic(vehicle_rid:RID, logic:CabinLogic) -> void:
    if _cab_logics.has(vehicle_rid):
        _cab_logics[vehicle_rid].unregister()
        _cab_logics.erase(vehicle_rid)
    if not logic:
        return
    _cab_logics[vehicle_rid] = logic
    logic.register(vehicle_rid, occupied_cab(vehicle_rid))


func vehicle_get_cab_logic(vehicle_rid:RID) -> CabinLogic:
    return _cab_logics.get(vehicle_rid)


## The cab interior of a vehicle, a scene rooted in a Cabin3D - what vehicle_show_cabin() builds.
## Only a view: the cab logic is the vehicle's (vehicle_attach_cab_logic()).
func vehicle_set_cabin_scene(vehicle_rid:RID, scene:PackedScene) -> void:
    _cabin_scenes[vehicle_rid] = scene


func vehicle_get_cabin_scene(vehicle_rid:RID) -> PackedScene:
    return _cabin_scenes.get(vehicle_rid)


## The cab interior built into the node the vehicle is drawn at (RailVehicleRenderingServer); it is
## built within add_child() (Cabin3D's cabin_ready comes from its NOTIFICATION_READY), so it
## returns built. Null for a vehicle without a cab.
func vehicle_show_cabin(vehicle_rid:RID) -> Cabin3D:
    var shown:Cabin3D = vehicle_get_cabin(vehicle_rid)
    if shown:
        return shown
    var scene:PackedScene = _cabin_scenes.get(vehicle_rid)
    var node:Node3D = instance_from_id(RailVehicleRenderingServer.vehicle_get_node(vehicle_rid)) as Node3D
    if not scene or not node:
        push_warning("CabinSystem: the vehicle has no cab interior to show")
        return null
    var cabin:Cabin3D = scene.instantiate() as Cabin3D
    if not cabin:
        push_error("CabinSystem: the root of a cabin scene must be a Cabin3D")
        return null
    _cabins[vehicle_rid] = cabin.get_instance_id()
    cabin.camera_configuration_changed.connect(_on_cabin_camera_configuration_changed.bind(vehicle_rid))
    node.add_child(cabin)
    # a cabin holds the handle of the vehicle it sits in and takes everything else from here -
    # told once it is in the tree, because building its interior puts nodes there
    cabin.set_vehicle_rid(vehicle_rid)
    _on_cabin_camera_configuration_changed(vehicle_rid)
    return cabin


## The cab interior freed; a camera put into it is to be taken out first
func vehicle_hide_cabin(vehicle_rid:RID) -> void:
    var cabin:Cabin3D = vehicle_get_cabin(vehicle_rid)
    _cabins.erase(vehicle_rid)
    if not cabin:
        return
    cabin.camera_configuration_changed.disconnect(_on_cabin_camera_configuration_changed.bind(vehicle_rid))
    cabin.get_parent().remove_child(cabin)
    cabin.queue_free()
    RailVehicleRenderingServer.vehicle_set_cab(vehicle_rid, 0, false)


## The cab interior while it is shown, else null
func vehicle_get_cabin(vehicle_rid:RID) -> Cabin3D:
    return instance_from_id(_cabins.get(vehicle_rid, 0)) as Cabin3D


## The low-poly interior hides the cab the player sits in (RailVehicleRenderingServer.vehicle_set_cab())
func _on_cabin_camera_configuration_changed(vehicle_rid:RID) -> void:
    var cabin:Cabin3D = vehicle_get_cabin(vehicle_rid)
    RailVehicleRenderingServer.vehicle_set_cab(vehicle_rid, cabin.get_cab_number(), cabin.get_has_cab_model())


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
        GameLog.error("%s: Unknown cabin action: %s" % [VehicleServer.vehicle_get_name(vehicle_rid), action])
        return null
    var handler:Callable = _controls.get(_key(vehicle_rid, cab), {}).get(control_id, Callable())
    if not handler.is_valid():
        GameLog.error("%s: Unknown cabin control: %s (cab %d)" % [
            VehicleServer.vehicle_get_name(vehicle_rid), control_id, cab])
        return null
    return handler.call(get_cabin_state(vehicle_rid, cab), action, value)


func get_control(vehicle_rid:RID, cab:int, control_id:StringName) -> Variant:
    return get_cabin_state(vehicle_rid, cab).get_value(control_id)


## A scenery's radio message, to the cab radio of whoever listens (radio_message_sent)
func send_radio_message(
    message:SfxEvent, transcript:Transcript, channel:int, position:Vector3, reach:float
) -> void:
    radio_message_sent.emit(message, transcript, channel, position, reach)


func get_state(vehicle_rid:RID, cab:int) -> Dictionary:
    return get_cabin_state(vehicle_rid, cab).values.duplicate()


## The cabs' own timing runs on the simulation's clock, as the original's TTrain::Update(dt) does
## with the scaled time (Train.cpp:8436-8474): a relay held for its delay at x10 closes in a tenth
## of the real time, and nothing runs while paused
func _on_simulation_advanced(seconds:float) -> void:
    for key:String in _processes:
        var state:CabinState = _states.get(key)
        for callable:Callable in _processes[key].duplicate():
            callable.call(state, seconds)
