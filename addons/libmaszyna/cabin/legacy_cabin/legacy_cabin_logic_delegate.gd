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
var controller:TrainController
var cab:int = 1

var _behaviours:Array = []
var _battery:LegacyCabinBattery
var _manual_brake:LegacyCabinManualBrake


# FIXME(#184): train_id comes from the TrainController, see BaseCabinTool3D._act().
func _ready() -> void:
    var main_switch := LegacyCabinMainSwitch.new()
    var claimed:Array[StringName] = main_switch.control_ids()
    _behaviours = [main_switch]
    if _has_control(LegacyCabinJointController.CONTROL):
        var joint_controller := LegacyCabinJointController.new()
        claimed.append_array(joint_controller.control_ids())
        _behaviours.append(joint_controller)
    _behaviours.append(LegacyCabinForwardCommands.new(get_parent(), claimed))
    if not _has_control(LegacyCabinBattery.CONTROL):
        _battery = LegacyCabinBattery.new()
        _behaviours.append(_battery)
    if not _has_control(LegacyCabinManualBrake.CONTROL):
        _manual_brake = LegacyCabinManualBrake.new()
        _behaviours.append(_manual_brake)
    for behaviour:RefCounted in _behaviours:
        behaviour.register(controller.train_id, cab)


# keyboard bindings of controls the MMD doesn't have (see LegacyCabinBattery, LegacyCabinManualBrake)
func _unhandled_input(event:InputEvent) -> void:
    # Train.cpp:6285 OnCommand_occupiedcarcouplingdisconnect - uncouples at the occupied cab's end
    # (cab_to_end(), Train.h:216), with or without a couplingdisconnect_sw: gauge
    if event.is_action_pressed(&"coupler_disconnect_occupied", false, true) and not cab == 0:
        CabinSystem.get_cabin_state(controller.train_id, cab).send_vehicle_command(
                "coupler_disconnect", 1 if cab < 0 else 0)
    if _battery and event.is_action_pressed(LegacyCabinBattery.ACTION, false, true):
        CabinSystem.act(controller.train_id, cab, LegacyCabinBattery.CONTROL, &"toggle")
    if _manual_brake:
        # acts on key repeat too (Train.cpp:1811)
        if event.is_action_pressed(LegacyCabinManualBrake.ACTION_INCREASE, true, true):
            CabinSystem.act(controller.train_id, cab, LegacyCabinManualBrake.CONTROL, &"increase")
        elif event.is_action_pressed(LegacyCabinManualBrake.ACTION_DECREASE, true, true):
            CabinSystem.act(controller.train_id, cab, LegacyCabinManualBrake.CONTROL, &"decrease")


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
    _manual_brake = null
