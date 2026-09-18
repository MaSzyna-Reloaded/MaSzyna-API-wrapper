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


# FIXME(#184): train_id comes from the TrainController, see BaseCabinTool3D._act().
func _ready() -> void:
    var main_switch := LegacyCabinMainSwitch.new()
    var claimed:Array[StringName] = main_switch.control_ids()
    _behaviours = [main_switch, LegacyCabinForwardCommands.new(get_parent(), claimed)]
    for behaviour:RefCounted in _behaviours:
        behaviour.register(controller.train_id, cab)


func _exit_tree() -> void:
    for behaviour:RefCounted in _behaviours:
        behaviour.unregister()
    _behaviours.clear()
