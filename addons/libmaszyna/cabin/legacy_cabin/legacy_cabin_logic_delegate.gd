extends Node
class_name LegacyCabinLogicDelegate

## Cabin logic of the original engine (TTrain, Train.cpp) for an MMD-built cabin. Inserted by the
## MMD cabin factory (DynamicTrainCabin) after all controls are built; on _ready it composes the
## legacy behaviours and registers their callbacks in CabinSystem for this cabin's
## (vehicle_rid, cab), unregistering them on _exit_tree. The cabin state itself stays in CabinSystem.
##
## Behaviours with dedicated cabin logic claim their controls first; every remaining control is
## wired straight to its vehicle command by LegacyCabinForwardCommands.

## Set by the factory before the node enters the tree.
## Which vehicle this cabin logic drives; everything goes through CabinSystem.
var vehicle_rid:RID
var cab:int = 1

var _behaviours:Array = []
var _manual_brake:LegacyCabinManualBrake
var _wipers:LegacyCabinWipers
var _unmodelled_controls:LegacyCabinUnmodelledControls
var _brake_charging:LegacyCabinBrakeCharging


func _ready() -> void:
    var main_switch := LegacyCabinMainSwitch.new(
            _button_type(LegacyCabinMainSwitch.TOGGLE_SWITCH), _has_control(LegacyCabinMainSwitch.ON_BUTTON),
            _has_control(LegacyCabinMainSwitch.OFF_BUTTON), _has_control(LegacyCabinMainSwitch.TOGGLE_SWITCH))
    var claimed:Array[StringName] = main_switch.control_ids()
    _behaviours = [main_switch]
    # controls whose original handler branches on the kind of switch (TGaugeType, CabinButton.ButtonType) - each gets the
    # type of its own control, as TTrain reads ggX.type()
    var pantograph_selected := LegacyCabinPantographSelected.new(
            _button_type(LegacyCabinPantographSelected.RAISE), _button_type(LegacyCabinPantographSelected.LOWER),
            _has_control(LegacyCabinPantographSelected.LOWER))
    claimed.append_array(pantograph_selected.control_ids())
    _behaviours.append(pantograph_selected)
    var present:Dictionary[StringName, bool] = {}
    for control_id:StringName in LegacyCabinPantographs.SWITCHES:
        present[control_id] = _has_control(control_id)
    var pantographs := LegacyCabinPantographs.new(present, _has_control(&"pantselect_sw"))
    claimed.append_array(pantographs.control_ids())
    _behaviours.append(pantographs)
    var switch_behaviours:Array[RefCounted] = [
        LegacyCabinBattery.new(),
        LegacyCabinCabActivation.new(),
        LegacyCabinPump.new(&"fuelpump_sw", "fuel_pump", "fuel_pump_switch_off", "fuel_pump_enabled",
                _button_type(&"fuelpump_sw")),
        LegacyCabinPump.new(&"oilpump_sw", "oil_pump", "oil_pump_switch_off", "oil_pump_enabled",
                _button_type(&"oilpump_sw")),
        LegacyCabinTrainHeating.new(_has_control(LegacyCabinTrainHeating.CONTROL)),
        LegacyCabinPantographsDropAll.new(
                _button_type(LegacyCabinPantographsDropAll.CONTROL), _has_control(LegacyCabinPantographsDropAll.CONTROL)),
    ]
    for behaviour:RefCounted in switch_behaviours:
        claimed.append_array(behaviour.control_ids())
    _behaviours.append_array(switch_behaviours)
    var reverser := LegacyCabinReverser.new()
    claimed.append_array(reverser.control_ids())
    _behaviours.append(reverser)
    if _has_control(LegacyCabinJointController.CONTROL):
        var joint_controller := LegacyCabinJointController.new()
        claimed.append_array(joint_controller.control_ids())
        _behaviours.append(joint_controller)
    _behaviours.append(LegacyCabinForwardCommands.new(get_parent(), claimed))
    if not _has_control(LegacyCabinManualBrake.CONTROL):
        _manual_brake = LegacyCabinManualBrake.new()
        _behaviours.append(_manual_brake)
    if not _has_control(LegacyCabinWipers.CONTROL):
        _wipers = LegacyCabinWipers.new()
        _behaviours.append(_wipers)
    _brake_charging = LegacyCabinBrakeCharging.new()
    _behaviours.append(_brake_charging)
    for behaviour:RefCounted in _behaviours:
        behaviour.register(vehicle_rid, cab)
    # last: whatever is registered by now is taken care of
    _unmodelled_controls = LegacyCabinUnmodelledControls.new(get_parent())
    _unmodelled_controls.register(vehicle_rid, cab)
    _behaviours.append(_unmodelled_controls)


# Keys of controls outside of MmdSemanticCatalog; those of the catalog controls a cab does not
# model are taken by LegacyCabinUnmodelledControls.
func _unhandled_input(event:InputEvent) -> void:
    _unmodelled_controls.input(event)
    # Train.cpp:6285 OnCommand_occupiedcarcouplingdisconnect - uncouples at the occupied cab's end
    # (cab_to_end(), Train.h:216), with or without a couplingdisconnect_sw: gauge
    if event.is_action_pressed(&"coupler_disconnect_occupied", false, true) and not cab == 0:
        CabinSystem.get_cabin_state(vehicle_rid, cab).send_vehicle_command(
                "coupler_disconnect", 1 if cab < 0 else 0)
    # Train.cpp:6836 OnCommand_springbrakeshutofftoggle - no cab models the shut-off valve
    if event.is_action_pressed(&"spring_brake_shut_off_toggle", false, true):
        var state:CabinState = CabinSystem.get_cabin_state(vehicle_rid, cab)
        # the command takes "enabled", the opposite of the valve, so the valve's state is the new value
        state.send_vehicle_command(
                "set_spring_brake_enabled", bool(state.vehicle_state_value("spring_brake/shut_off", false)))
    if event.is_action_pressed(LegacyCabinBrakeCharging.ACTION, false, true):
        CabinSystem.act(vehicle_rid, cab, LegacyCabinBrakeCharging.CONTROL, &"hold")
    elif event.is_action_released(LegacyCabinBrakeCharging.ACTION, true):
        CabinSystem.act(vehicle_rid, cab, LegacyCabinBrakeCharging.CONTROL, &"release")


func _has_control(control_id:StringName) -> bool:
    return not _control(control_id) == null


## The kind of switch the cab models this control as; one the cab does not model is a toggle, as
## an undefined TGauge is (Gauge.h:89)
func _button_type(control_id:StringName) -> CabinButton.ButtonType:
    var control:CabinButton = _control(control_id) as CabinButton
    return control.button_type if control else CabinButton.ButtonType.TOGGLE


func _control(control_id:StringName) -> Node:
    for node:Node in get_parent().find_children("*", "", true, false):
        if "control_id" in node and StringName(node.get("control_id")) == control_id:
            return node
    return null


func _exit_tree() -> void:
    for behaviour:RefCounted in _behaviours:
        behaviour.unregister()
    _behaviours.clear()
    _manual_brake = null
    _wipers = null
    _unmodelled_controls = null
    _brake_charging = null
