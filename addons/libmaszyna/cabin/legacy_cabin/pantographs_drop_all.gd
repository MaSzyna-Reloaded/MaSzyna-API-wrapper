extends RefCounted
class_name LegacyCabinPantographsDropAll

## Lower-all-pantographs switch (pantalloff_sw) - TTrain::OnCommand_pantographlowerall
## (Train.cpp:3334). A two-state switch flips PantAllDown on a press; any other kind holds the
## pantographs down while it is held. The original does nothing in a cab without the gauge
## (Train.cpp:3342).

const CONTROL:StringName = &"pantalloff_sw"

var _button_type:CabinButton.ButtonType
var _has_gauge:bool
var _vehicle_rid:RID
var _cab:int


func _init(button_type:CabinButton.ButtonType, has_gauge:bool) -> void:
    _button_type = button_type
    _has_gauge = has_gauge


func control_ids() -> Array[StringName]:
    return [CONTROL]


func register(vehicle_rid:RID, cab:int) -> void:
    _vehicle_rid = vehicle_rid
    _cab = cab
    CabinSystem.register_control(vehicle_rid, cab, CONTROL, _drop_all)


func unregister() -> void:
    CabinSystem.unregister_control(_vehicle_rid, _cab, CONTROL, _drop_all)


func _drop_all(state:CabinState, action:StringName, value:Variant) -> Variant:
    if not _has_gauge:
        return null
    if _button_type == CabinButton.ButtonType.TOGGLE:
        if action == &"release":
            return null
        var dropped:bool = (not state.vehicle_state_value("current_collector/pantographs_dropped", false)
                if value == null or action == &"hold" else bool(value))
        state.set_value(CONTROL, dropped)
        return state.send_vehicle_command("pantographs_drop_all", dropped)
    var held:bool = state.is_pressed(CONTROL, action, value)
    state.set_value(CONTROL, held)
    return state.send_vehicle_command("pantographs_drop_all", held)
