extends Node
class_name LegacyCabinLogicDelegate

## Cabin logic of the original engine (TTrain, Train.cpp) for an MMD-built cabin. Inserted by the
## MMD cabin factory (DynamicTrainCabin) after all controls are built; on _ready it composes the
## legacy behaviours and registers their callbacks in CabinSystem for this cabin's
## (train_id, cab), unregistering them on _exit_tree. The cabin state itself stays in CabinSystem.
##
## Behaviours with dedicated cabin logic claim their controls first; every remaining control is
## wired straight to its vehicle command by LegacyCabinForwardCommands.

## Set by the factory before the node enters the tree.
## Which vehicle this cabin logic drives; everything goes through CabinSystem.
var train_id:String = ""
var cab:int = 1

var _behaviours:Array = []
var _battery:LegacyCabinBattery
var _cab_activation:LegacyCabinCabActivation
var _manual_brake:LegacyCabinManualBrake
var _wipers:LegacyCabinWipers
var _unmodelled_controls:LegacyCabinUnmodelledControls
var _brake_charging:LegacyCabinBrakeCharging


# FIXME(#184): train_id comes from the VehicleController, see BaseCabinTool3D._act().
func _ready() -> void:
    var main_switch := LegacyCabinMainSwitch.new()
    var claimed:Array[StringName] = main_switch.control_ids()
    _behaviours = [main_switch]
    var reverser := LegacyCabinReverser.new()
    claimed.append_array(reverser.control_ids())
    _behaviours.append(reverser)
    if _has_control(LegacyCabinJointController.CONTROL):
        var joint_controller := LegacyCabinJointController.new()
        claimed.append_array(joint_controller.control_ids())
        _behaviours.append(joint_controller)
    _behaviours.append(LegacyCabinForwardCommands.new(get_parent(), claimed))
    if not _has_control(LegacyCabinBattery.CONTROL):
        _battery = LegacyCabinBattery.new()
        _behaviours.append(_battery)
    if not _has_control(LegacyCabinCabActivation.CONTROL):
        _cab_activation = LegacyCabinCabActivation.new()
        _behaviours.append(_cab_activation)
    if not _has_control(LegacyCabinManualBrake.CONTROL):
        _manual_brake = LegacyCabinManualBrake.new()
        _behaviours.append(_manual_brake)
    if not _has_control(LegacyCabinWipers.CONTROL):
        _wipers = LegacyCabinWipers.new()
        _behaviours.append(_wipers)
    _brake_charging = LegacyCabinBrakeCharging.new()
    _behaviours.append(_brake_charging)
    for behaviour:RefCounted in _behaviours:
        behaviour.register(train_id, cab)
    # last: whatever is registered by now is taken care of
    _unmodelled_controls = LegacyCabinUnmodelledControls.new(get_parent())
    _unmodelled_controls.register(train_id, cab)
    _behaviours.append(_unmodelled_controls)


# Keys of controls outside of MmdSemanticCatalog; those of the catalog controls a cab does not
# model are taken by LegacyCabinUnmodelledControls.
func _unhandled_input(event:InputEvent) -> void:
    _unmodelled_controls.input(event)
    # Train.cpp:6285 OnCommand_occupiedcarcouplingdisconnect - uncouples at the occupied cab's end
    # (cab_to_end(), Train.h:216), with or without a couplingdisconnect_sw: gauge
    if event.is_action_pressed(&"coupler_disconnect_occupied", false, true) and not cab == 0:
        CabinSystem.get_cabin_state(train_id, cab).send_vehicle_command(
                "coupler_disconnect", 1 if cab < 0 else 0)
    # Train.cpp:6836 OnCommand_springbrakeshutofftoggle - no cab models the shut-off valve
    if event.is_action_pressed(&"spring_brake_shut_off_toggle", false, true):
        var state:CabinState = CabinSystem.get_cabin_state(train_id, cab)
        # the command takes "enabled", the opposite of the valve, so the valve's state is the new value
        state.send_vehicle_command(
                "set_spring_brake_enabled", bool(state.vehicle_state().get("spring_brake/shut_off", false)))
    if event.is_action_pressed(LegacyCabinBrakeCharging.ACTION, false, true):
        CabinSystem.act(train_id, cab, LegacyCabinBrakeCharging.CONTROL, &"hold")
    elif event.is_action_released(LegacyCabinBrakeCharging.ACTION, true):
        CabinSystem.act(train_id, cab, LegacyCabinBrakeCharging.CONTROL, &"release")


func _has_control(control_id:StringName) -> bool:
    for node:Node in get_parent().find_children("*", "", true, false):
        if "control_id" in node and StringName(node.get("control_id")) == control_id:
            return true
    return false


func _exit_tree() -> void:
    for behaviour:RefCounted in _behaviours:
        behaviour.unregister()
    _behaviours.clear()
    _battery = null
    _cab_activation = null
    _manual_brake = null
    _wipers = null
    _unmodelled_controls = null
    _brake_charging = null
