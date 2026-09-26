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

const ACTION_FIELDS:Array[String] = LegacyCabinControls.ACTION_FIELDS
## Only these take their keys here: a knob takes a value, which a key does not give - it is
## registered for whoever sets it (the AI driver), but its keys stay unwired
const KEY_WIRED_KINDS:Array[StringName] = [&"button", &"switch"]

var _cab_controls:LegacyCabinControls
var _vehicle_rid:RID
var _cab:int
var _handlers:Dictionary[StringName, Callable] = {}
## control_id -> fixed fields of its catalog entry
var _controls:Dictionary[StringName, Dictionary] = {}


func _init(cab_controls:LegacyCabinControls) -> void:
    _cab_controls = cab_controls


func register(vehicle_rid:RID, cab:int) -> void:
    _vehicle_rid = vehicle_rid
    _cab = cab
    var taken_actions:Dictionary[String, bool] = _cab_controls.get_actions()

    for label:String in MmdSemanticCatalog.get_labels():
        var control_id:StringName = StringName(label)
        if _cab_controls.has_control(control_id):
            continue
        var entry:Dictionary = MmdSemanticCatalog.get_entry(label)
        var fields:Dictionary = entry["fixed_fields"]
        var wiring:Dictionary = LegacyCabinForwardCommands.wiring(entry["widget_class"], fields)
        var actions:Array = ACTION_FIELDS.map(func(field:String) -> String: return fields.get(field, ""))
        actions = actions.filter(func(action:String) -> bool: return not action == "")
        if not actions or actions.any(func(action:String) -> bool: return taken_actions.has(action)):
            continue
        var registered:bool = CabinSystem.has_control(vehicle_rid, cab, control_id)
        if not registered and not wiring:
            continue
        for action:String in actions:
            taken_actions[action] = true
        if registered or wiring["kind"] in KEY_WIRED_KINDS:
            _controls[control_id] = fields
        if registered:
            continue
        wiring["control_id"] = control_id
        _handlers[control_id] = LegacyCabinForwardCommands._handle.bind(wiring)
        CabinSystem.register_control(vehicle_rid, cab, control_id, _handlers[control_id])


func unregister() -> void:
    for control_id:StringName in _handlers:
        CabinSystem.unregister_control(_vehicle_rid, _cab, control_id, _handlers[control_id])
    _handlers.clear()
    _controls.clear()


## Keys of the registered controls, as CabinButton and CabinSwitch take theirs.
func input(event:InputEvent) -> void:
    for control_id:StringName in _controls:
        var fields:Dictionary = _controls[control_id]
        var repeat:bool = fields.get("repeat_on_hold", false)
        if fields.get("action_increase", "") and event.is_action_pressed(fields["action_increase"], repeat, true):
            CabinSystem.act(_vehicle_rid, _cab, control_id, &"increase")
        elif fields.get("action_decrease", "") and event.is_action_pressed(fields["action_decrease"], repeat, true):
            CabinSystem.act(_vehicle_rid, _cab, control_id, &"decrease")
        if not fields.get("action", ""):
            continue
        if fields.get("monostable", false):
            if event.is_action_pressed(fields["action"], false, true):
                CabinSystem.act(_vehicle_rid, _cab, control_id, &"hold")
            elif event.is_action_released(fields["action"], true):
                CabinSystem.act(_vehicle_rid, _cab, control_id, &"release")
        elif event.is_action_pressed(fields["action"], false, true):
            # a two-state control follows the vehicle, there is no widget holding its position
            var state_property:String = fields.get("state_property", "")

            CabinSystem.act(
                    _vehicle_rid, _cab, control_id, &"toggle",
                    not bool(CabinSystem.vehicle_state_value(_vehicle_rid, state_property, false))
                    if state_property else null)

