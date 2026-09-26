extends RefCounted
class_name LegacyCabinPantographs

## The front and rear pantograph switches (pantfront_sw, pantrear_sw) and their separate lowering
## buttons (pantfrontoff_sw, pantrearoff_sw) - TTrain::OnCommand_pantographtogglefront/raisefront/
## lowerfront and the rear twins (Train.cpp:3150-3330).
##
## A press raises a pantograph whose valve is closed and lowers one that is up. How its own valve
## is operated follows the vehicle's switch type (Switches: Pantograph=, PantSwitchType): an
## impulse switch presses one side (ENABLE_ON/DISABLE_ON) and lets go on the release (NONE), a
## two-state one sets the valve (ENABLE/DISABLE). Outside the machine room a pantograph is raised
## only from a cab that has its switch, and lowered by an impulse type only from one that has the
## lowering button - which is why dynamic/pkp/e186_v2 declares pantfrontoff_sw with no submodel.
## A cab with a pantograph selector (pantselect_sw) leaves the individual valves alone.

## control -> the pantograph it works, and whether it is the lowering button
const SWITCHES:Dictionary[StringName, Array] = {
    &"pantfront_sw": [VehicleElectricEngine.PANTOGRAPH_FIRST, false],
    &"pantrear_sw": [VehicleElectricEngine.PANTOGRAPH_SECOND, false],
    &"pantfrontoff_sw": [VehicleElectricEngine.PANTOGRAPH_FIRST, true],
    &"pantrearoff_sw": [VehicleElectricEngine.PANTOGRAPH_SECOND, true],
}
## pantograph -> its switch, its lowering button and its state keys
const PANTOGRAPHS:Dictionary[VehicleElectricEngine.PantographSelector, Array] = {
    VehicleElectricEngine.PANTOGRAPH_FIRST: [&"pantfront_sw", &"pantfrontoff_sw",
            "current_collector/pantograph_first_valve_enabled", "current_collector/pantograph_first_active"],
    VehicleElectricEngine.PANTOGRAPH_SECOND: [&"pantrear_sw", &"pantrearoff_sw",
            "current_collector/pantograph_second_valve_enabled", "current_collector/pantograph_second_active"],
}
## the machine room, where levers are moved by hand whatever the cab models (Train.cpp:3228)
const MACHINE_ROOM_CAB:int = 0

## control -> whether the cab models it (m_controlmapper.contains)
var _present:Dictionary[StringName, bool] = {}
var _has_selector:bool
var _vehicle_rid:RID
var _cab:int
var _handlers:Dictionary[StringName, Callable] = {}


func _init(present:Dictionary[StringName, bool], has_selector:bool) -> void:
    _present = present
    _has_selector = has_selector


func control_ids() -> Array[StringName]:
    return SWITCHES.keys()


func register(vehicle_rid:RID, cab:int) -> void:
    _vehicle_rid = vehicle_rid
    _cab = cab
    for control_id:StringName in SWITCHES:
        _handlers[control_id] = _switch.bind(control_id)
        CabinSystem.register_control(vehicle_rid, cab, control_id, _handlers[control_id])


func unregister() -> void:
    for control_id:StringName in _handlers:
        CabinSystem.unregister_control(_vehicle_rid, _cab, control_id, _handlers[control_id])
    _handlers.clear()


func _switch(state:CabinState, action:StringName, value:Variant, control_id:StringName) -> Variant:
    # Train.cpp:3154 - a pantograph selector takes the individual valves over
    if _has_selector:
        return null
    var selector:VehicleElectricEngine.PantographSelector = SWITCHES[control_id][0]
    var lowering_button:bool = SWITCHES[control_id][1]
    var impulse:bool = CabinSystem.vehicle_config(state.vehicle_rid).get("pantograph_switch_impulse", false)
    var pressed:bool = state.is_pressed(control_id, action, value) or action == &"toggle"
    state.set_value(control_id, state.is_pressed(control_id, action, value))
    if not pressed:
        # Train.cpp:3170 - impulse switches return to neutral, and so does the valve
        if impulse:
            return state.send_vehicle_command("pantograph_valve_operate", selector,
                    VehicleElectricEngine.VALVE_OPERATION_NONE)
        return null
    var pantograph:Array = PANTOGRAPHS[selector]
    # Train.cpp:3161 - the switch lowers a pantograph whose valve is open or which is up
    var lower:bool = lowering_button or state.vehicle_state_value(pantograph[2], false) \
            or state.vehicle_state_value(pantograph[3], false)
    if lower:
        # Train.cpp:3285 - lowering needs the switch, or the lowering button for an impulse type
        if not state.cab == MACHINE_ROOM_CAB and not _present.get(pantograph[1] if impulse else pantograph[0], false):
            return null
        return state.send_vehicle_command("pantograph_valve_operate", selector,
                VehicleElectricEngine.VALVE_OPERATION_DISABLE_ON if impulse
                else VehicleElectricEngine.VALVE_OPERATION_DISABLE)
    # Train.cpp:3228 - raising needs the switch
    if not state.cab == MACHINE_ROOM_CAB and not _present.get(pantograph[0], false):
        return null
    return state.send_vehicle_command("pantograph_valve_operate", selector,
            VehicleElectricEngine.VALVE_OPERATION_ENABLE_ON if impulse
            else VehicleElectricEngine.VALVE_OPERATION_ENABLE)
