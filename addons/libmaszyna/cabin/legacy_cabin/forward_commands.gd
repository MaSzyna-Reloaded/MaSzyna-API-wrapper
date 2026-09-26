extends RefCounted
class_name LegacyCabinForwardCommands

## Direct control -> vehicle command wiring of an MMD-built cabin: every control that has no
## dedicated cabin behaviour is registered in CabinSystem with a handler translating its
## manipulations into the vehicle command it is wired to. The wiring (command, command_param,
## controller mode, ...) is the control's MmdSemanticCatalog entry, as the cab's MMD lists it
## (LegacyCabinControls) - not read off the widgets, which a cab only the AI drives does not have.

var _controls:LegacyCabinControls
var _skip:Array[StringName] = []
var _vehicle_rid:RID
var _cab:int
var _handlers:Dictionary = {}


func _init(controls:LegacyCabinControls, skip:Array[StringName]) -> void:
    _controls = controls
    _skip = skip


func register(vehicle_rid:RID, cab:int) -> void:
    _vehicle_rid = vehicle_rid
    _cab = cab
    for control_id:StringName in _controls.get_control_ids():
        if control_id in _skip:
            continue
        var wiring:Dictionary = _controls.wiring(control_id)
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


## The wiring of a control of the catalog class, by the fields of its catalog entry - the fields a
## widget of that class would have been given (MmdCabinInstancer._build_widget); a gauge has none
static func wiring(widget_class:Variant, fields:Dictionary) -> Dictionary:
    if widget_class == CabinButton:
        return {"kind": &"button", "command": fields.get("command", ""),
                "command_param": fields.get("command_param"),
                "controller_mode": fields.get("controller_mode", CabinButton.ControllerMode.OnOff)}
    if widget_class == CabinSwitch:
        return {"kind": &"switch", "command_increase": fields.get("command_increase", ""),
                "command_decrease": fields.get("command_decrease", ""),
                "command_set": fields.get("command_set", "")}
    if widget_class == CabinKnob:
        return {"kind": &"knob", "command": fields.get("command", "")}
    if widget_class == CabinCommand:
        return {"kind": &"command", "command": fields.get("command", ""),
                "command_param": fields.get("command_param")}
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
