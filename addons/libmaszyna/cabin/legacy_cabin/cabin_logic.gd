extends CabinLogic
class_name LegacyCabinLogic

## Cabin logic of the original engine (TTrain, Train.cpp) for one vehicle's cab. It composes the
## legacy behaviours for the cab and registers their callbacks in CabinSystem for its
## (vehicle_rid, cab); the cabin state itself stays in CabinSystem.
##
## It is no part of the 3D cab. CabinSystem holds it for a vehicle somebody drives - the player
## (DynamicTrainCabin), the AI (SceneryInstancer._build_drivers()) - and registers it for the
## occupied cab, so the AI's CabinSystem.act() does exactly what the player's controls do, without
## a single widget. What the cab has comes from its MMD (LegacyCabinControls), never from
## the widgets built of it.
##
## Behaviours with dedicated cabin logic claim their controls first; every remaining control is
## wired straight to its vehicle command by LegacyCabinForwardCommands; the catalog controls the cab
## does not have come last (LegacyCabinUnmodelledControls).

## (cab:int) -> LegacyCabinControls - the controls of the cab it is registered for
var _controls_for_cab:Callable
var _behaviours:Array = []
var _unmodelled_controls:LegacyCabinUnmodelledControls
var _vehicle_rid:RID
var _cab:int


func _init(controls_for_cab:Callable) -> void:
    _controls_for_cab = controls_for_cab


## The logic of the cabs a vehicle's MMD defines
static func from_mmd(data_path:String, mmd_filename:String) -> LegacyCabinLogic:
    var abs_mmd_path:String = UserSettings.get_maszyna_game_dir().path_join(data_path).path_join(mmd_filename + ".mmd")
    return LegacyCabinLogic.new(LegacyCabinControls.from_mmd.bind(abs_mmd_path))


func register(vehicle_rid:RID, cab:int) -> void:
    _vehicle_rid = vehicle_rid
    _cab = cab
    var controls:LegacyCabinControls = _controls_for_cab.call(cab)
    var main_switch:LegacyCabinMainSwitch = LegacyCabinMainSwitch.new(
            controls.button_type(LegacyCabinMainSwitch.TOGGLE_SWITCH),
            controls.has_control(LegacyCabinMainSwitch.ON_BUTTON),
            controls.has_control(LegacyCabinMainSwitch.OFF_BUTTON),
            controls.has_control(LegacyCabinMainSwitch.TOGGLE_SWITCH))
    var claimed:Array[StringName] = main_switch.control_ids()
    _behaviours = [main_switch]
    # controls whose original handler branches on the kind of switch (TGaugeType, CabinButton.ButtonType) - each gets the
    # type of its own control, as TTrain reads ggX.type()
    var pantograph_selected:LegacyCabinPantographSelected = LegacyCabinPantographSelected.new(
            controls.button_type(LegacyCabinPantographSelected.RAISE),
            controls.button_type(LegacyCabinPantographSelected.LOWER),
            controls.has_control(LegacyCabinPantographSelected.LOWER))
    claimed.append_array(pantograph_selected.control_ids())
    _behaviours.append(pantograph_selected)
    var present:Dictionary[StringName, bool] = {}
    for control_id:StringName in LegacyCabinPantographs.SWITCHES:
        present[control_id] = controls.has_control(control_id)
    var pantographs:LegacyCabinPantographs = LegacyCabinPantographs.new(present, controls.has_control(&"pantselect_sw"))
    claimed.append_array(pantographs.control_ids())
    _behaviours.append(pantographs)
    var switch_behaviours:Array[RefCounted] = [
        LegacyCabinBattery.new(),
        LegacyCabinCabActivation.new(),
        LegacyCabinPump.new(&"fuelpump_sw", "fuel_pump", "fuel_pump_switch_off", "fuel_pump_enabled",
                controls.button_type(&"fuelpump_sw")),
        LegacyCabinPump.new(&"oilpump_sw", "oil_pump", "oil_pump_switch_off", "oil_pump_enabled",
                controls.button_type(&"oilpump_sw")),
        LegacyCabinTrainHeating.new(controls.has_control(LegacyCabinTrainHeating.CONTROL)),
        LegacyCabinPantographsDropAll.new(
                controls.button_type(LegacyCabinPantographsDropAll.CONTROL),
                controls.has_control(LegacyCabinPantographsDropAll.CONTROL)),
    ]
    for behaviour:RefCounted in switch_behaviours:
        claimed.append_array(behaviour.control_ids())
    _behaviours.append_array(switch_behaviours)
    var reverser:LegacyCabinReverser = LegacyCabinReverser.new()
    claimed.append_array(reverser.control_ids())
    _behaviours.append(reverser)
    if controls.has_control(LegacyCabinJointController.CONTROL):
        var joint_controller:LegacyCabinJointController = LegacyCabinJointController.new()
        claimed.append_array(joint_controller.control_ids())
        _behaviours.append(joint_controller)
    _behaviours.append(LegacyCabinForwardCommands.new(controls, claimed))
    if not controls.has_control(LegacyCabinManualBrake.CONTROL):
        _behaviours.append(LegacyCabinManualBrake.new())
    if not controls.has_control(LegacyCabinWipers.CONTROL):
        _behaviours.append(LegacyCabinWipers.new())
    _behaviours.append(LegacyCabinBrakeCharging.new())
    _behaviours.append(LegacyCabinOccupiedCouplerDisconnect.new())
    _behaviours.append(LegacyCabinSpringBrakeShutOff.new())
    for behaviour:RefCounted in _behaviours:
        behaviour.register(vehicle_rid, cab)
    # last: whatever is registered by now is taken care of
    _unmodelled_controls = LegacyCabinUnmodelledControls.new(controls)
    _unmodelled_controls.register(vehicle_rid, cab)
    _behaviours.append(_unmodelled_controls)


func unregister() -> void:
    for behaviour:RefCounted in _behaviours:
        behaviour.unregister()
    _behaviours.clear()
    _unmodelled_controls = null


## The player's keys of the controls without a widget - the catalog controls the cab does not have
## and the keyboard-only ones. Only the player's cab passes them (DynamicTrainCabin); the AI acts
## on the controls directly.
func input(event:InputEvent) -> void:
    if not _unmodelled_controls:
        return
    _unmodelled_controls.input(event)
    if event.is_action_pressed(LegacyCabinOccupiedCouplerDisconnect.ACTION, false, true):
        CabinSystem.act(_vehicle_rid, _cab, LegacyCabinOccupiedCouplerDisconnect.CONTROL, &"hold")
    if event.is_action_pressed(LegacyCabinSpringBrakeShutOff.ACTION, false, true):
        CabinSystem.act(_vehicle_rid, _cab, LegacyCabinSpringBrakeShutOff.CONTROL, &"hold")
    if event.is_action_pressed(LegacyCabinBrakeCharging.ACTION, false, true):
        CabinSystem.act(_vehicle_rid, _cab, LegacyCabinBrakeCharging.CONTROL, &"hold")
    elif event.is_action_released(LegacyCabinBrakeCharging.ACTION, true):
        CabinSystem.act(_vehicle_rid, _cab, LegacyCabinBrakeCharging.CONTROL, &"release")
