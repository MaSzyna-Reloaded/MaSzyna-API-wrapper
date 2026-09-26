extends RefCounted
class_name LegacyCabinForwardCommands

## Direct control -> vehicle command wiring of an MMD-built cabin: every control that has no
## dedicated cabin behaviour is registered in CabinSystem with a handler translating its
## manipulations into the vehicle command it is wired to. The wiring (command, command_param,
## controller mode, ...) is taken from the controls of this cabin, set there by the MMD cabin
## factory from MmdSemanticCatalog; the controls themselves only report manipulations.

var _controls_root:Node
var _skip:Array[StringName] = []
var _vehicle_rid:RID
var _cab:int
var _handlers:Dictionary = {}


func _init(controls_root:Node, skip:Array[StringName]) -> void:
    _controls_root = controls_root
    _skip = skip


func register(vehicle_rid:RID, cab:int) -> void:
    _vehicle_rid = vehicle_rid
    _cab = cab
    for node:Node in _controls_root.find_children("*", "", true, false):
        var control_id:StringName = StringName(node.get("control_id")) if "control_id" in node else &""
        if not control_id or control_id in _skip or _handlers.has(control_id):
            continue
        var wiring:Dictionary = _wiring(node)
        if not wiring:
            continue
        wiring["control_id"] = control_id
        var handler:Callable = _handle.bind(wiring)
        _handlers[control_id] = handler
        CabinSystem.register_control(vehicle_rid, cab, control_id, handler)


func unregister() -> void:
    for control_id:StringName in _handlers:
        CabinSystem.unregister_control(_vehicle_rid, _cab, control_id, _handlers[control_id])
    _handlers.clear()


static func _wiring(node:Node) -> Dictionary:
    if node is CabinButton:
        return {"kind": &"button", "command": node.command, "command_param": node.command_param,
                "controller_mode": node.controller_mode}
    if node is CabinSwitch:
        return {"kind": &"switch", "command_increase": node.command_increase,
                "command_decrease": node.command_decrease, "command_set": node.command_set}
    if node is CabinKnob:
        return {"kind": &"knob", "command": node.command}
    if node is CabinCommand:
        return {"kind": &"command", "command": node.command, "command_param": node.command_param}
    return {}


static func _handle(state:CabinState, action:StringName, value:Variant, wiring:Dictionary) -> Variant:
    match wiring["kind"]:
        &"button":
            return _handle_button(state, action, value, wiring)
        &"switch":
            return _handle_switch(state, action, value, wiring)
        &"knob":
            state.set_value(wiring["control_id"], value)
            if wiring["command"]:
                return state.send_vehicle_command(wiring["command"], value)
        &"command":
            if wiring["command"]:
                return state.send_vehicle_command(wiring["command"], wiring["command_param"])
    return null


static func _handle_button(state:CabinState, action:StringName, value:Variant, wiring:Dictionary) -> Variant:
    # toggle without a value (e.g. from the console) flips the current position
    if action == &"toggle" and value == null:
        value = not state.get_value(wiring["control_id"], false)
    var pressed:bool = action == &"hold" or (action == &"toggle" and bool(value))
    state.set_value(wiring["control_id"], pressed)
    var command:String = wiring["command"]
    if not command:
        return null
    match int(wiring["controller_mode"]):
        CabinButton.ControllerMode.OnOff:
            if not wiring["command_param"] == null:
                return state.send_vehicle_command(command, wiring["command_param"], pressed)
            return state.send_vehicle_command(command, pressed)
        CabinButton.ControllerMode.On:
            if pressed:
                return state.send_vehicle_command(command, true)
        CabinButton.ControllerMode.Off:
            if pressed:
                return state.send_vehicle_command(command, false)
    return null


static func _handle_switch(state:CabinState, action:StringName, value:Variant, wiring:Dictionary) -> Variant:
    var result:Variant = null
    if not value == null:
        state.set_value(wiring["control_id"], int(value))
    var step_command:String = (
            wiring["command_increase"] if action == &"increase"
            else wiring["command_decrease"] if action == &"decrease" else "")
    if step_command:
        result = state.send_vehicle_command(step_command)
    if wiring["command_set"] and not value == null:
        result = state.send_vehicle_command(wiring["command_set"], int(value))
    return result
