extends RefCounted
class_name LegacyCabinUnmodelledControls

## Controls of MmdSemanticCatalog the cab does not model. Every OnCommand_* of the original works
## without its gauge - the keys reach TTrain, which only updates a gauge that may be missing - so
## a cab with no main_on_bt (dynamic/pkp/e186_v2 has one main_sw instead), no dirkey, no
## shp_reset_bt, ... still takes their keys. Each of these controls is registered in CabinSystem
## with the wiring of its catalog entry (LegacyCabinForwardCommands) unless a cabin behaviour has
## registered it already, and input() translates its keys into manipulations.
##
## A control is left out when a control of the cab already takes one of its keys: jointctrl and
## mainctrl share theirs, both would step the controller.

const ACTION_FIELDS:Array[String] = ["action", "action_increase", "action_decrease"]

var _controls_root:Node
var _train_id:String
var _cab:int
var _handlers:Dictionary[StringName, Callable] = {}
## control_id -> fixed fields of its catalog entry
var _controls:Dictionary[StringName, Dictionary] = {}


func _init(controls_root:Node) -> void:
    _controls_root = controls_root


func register(train_id:String, cab:int) -> void:
    _train_id = train_id
    _cab = cab
    var taken_actions:Dictionary[String, bool] = {}
    var modelled:Dictionary[StringName, bool] = {}
    for node:Node in _controls_root.find_children("*", "", true, false):
        if "control_id" in node:
            modelled[StringName(node.get("control_id"))] = true
        for field:String in ACTION_FIELDS:
            if field in node and node.get(field):
                taken_actions[node.get(field)] = true

    for label:String in MmdSemanticCatalog.get_labels():
        var control_id:StringName = StringName(label)
        if modelled.has(control_id):
            continue
        var entry:Dictionary = MmdSemanticCatalog.get_entry(label)
        var fields:Dictionary = entry["fixed_fields"]
        var wiring:Dictionary = _wiring(entry["widget_class"], fields)
        var actions:Array = ACTION_FIELDS.map(func(field:String) -> String: return fields.get(field, ""))
        actions = actions.filter(func(action:String) -> bool: return not action == "")
        if not actions or actions.any(func(action:String) -> bool: return taken_actions.has(action)):
            continue
        var registered:bool = CabinSystem.has_control(train_id, cab, control_id)
        if not registered and not wiring:
            continue
        for action:String in actions:
            taken_actions[action] = true
        _controls[control_id] = fields
        if registered:
            continue
        wiring["control_id"] = control_id
        _handlers[control_id] = LegacyCabinForwardCommands._handle.bind(wiring)
        CabinSystem.register_control(train_id, cab, control_id, _handlers[control_id])


func unregister() -> void:
    for control_id:StringName in _handlers:
        CabinSystem.unregister_control(_train_id, _cab, control_id, _handlers[control_id])
    _handlers.clear()
    _controls.clear()


## Keys of the registered controls, as CabinButton and CabinSwitch take theirs.
func input(event:InputEvent) -> void:
    for control_id:StringName in _controls:
        var fields:Dictionary = _controls[control_id]
        var repeat:bool = fields.get("repeat_on_hold", false)
        if fields.get("action_increase", "") and event.is_action_pressed(fields["action_increase"], repeat, true):
            CabinSystem.act(_train_id, _cab, control_id, &"increase")
        elif fields.get("action_decrease", "") and event.is_action_pressed(fields["action_decrease"], repeat, true):
            CabinSystem.act(_train_id, _cab, control_id, &"decrease")
        if not fields.get("action", ""):
            continue
        if fields.get("monostable", false):
            if event.is_action_pressed(fields["action"], false, true):
                CabinSystem.act(_train_id, _cab, control_id, &"hold")
            elif event.is_action_released(fields["action"], true):
                CabinSystem.act(_train_id, _cab, control_id, &"release")
        elif event.is_action_pressed(fields["action"], false, true):
            # a two-state control follows the vehicle, there is no widget holding its position
            var state_property:String = fields.get("state_property", "")
            var vehicle:Dictionary = CabinSystem.get_cabin_state(_train_id, _cab).vehicle_state()
            CabinSystem.act(
                    _train_id, _cab, control_id, &"toggle",
                    not bool(vehicle.get(state_property, false)) if state_property else null)


static func _wiring(widget_class:Variant, fields:Dictionary) -> Dictionary:
    if widget_class == CabinButton:
        return {"kind": &"button", "command": fields.get("command", ""),
                "command_param": fields.get("command_param"),
                "controller_mode": fields.get("controller_mode", CabinButton.ControllerMode.OnOff)}
    if widget_class == CabinSwitch:
        return {"kind": &"switch", "command_increase": fields.get("command_increase", ""),
                "command_decrease": fields.get("command_decrease", ""),
                "command_set": fields.get("command_set", "")}
    return {}
