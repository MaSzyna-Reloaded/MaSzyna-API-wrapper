@tool
extends Node3D
class_name TrainSet3D

## Consist of a scenery "trainset:" block (simulationstateserializer.cpp:deserialize_trainset).
## Child DynamicRailVehicle3D nodes are its vehicles in scenery order. Once all their controllers
## are ready, every vehicle is coupled with the next one the same way the original couples them at
## "endtrainset" (simulationstateserializer.cpp:750, TDynamicObject::AttachNext, DynObj.cpp:2590).

## Coupling flags between child vehicle i and i + 1 (the scenery "couplingdata" of vehicle i).
@export var couplings:PackedInt32Array = []

var _coupling_dirty:bool = true


func _process(_delta:float) -> void:
    if Engine.is_editor_hint():
        return
    if _coupling_dirty:
        _process_coupling_dirty()


func _process_coupling_dirty() -> void:
    var vehicles:Array[DynamicRailVehicle3D] = []
    var controllers:Array[TrainController] = []
    for child:Node in get_children():
        var vehicle:DynamicRailVehicle3D = child as DynamicRailVehicle3D
        if not vehicle:
            continue
        var controller:TrainController = vehicle.get_controller()
        if not controller or not controller.is_node_ready():
            return
        vehicles.append(vehicle)
        controllers.append(controller)
    _coupling_dirty = false

    for index:int in range(1, controllers.size()):
        var coupling_type:int = couplings[index - 1] if index - 1 < couplings.size() else 0
        # AttachNext: this vehicle's end = iDirection, the next vehicle's end = its iDirection ^ 1,
        # where iDirection is 1 for a normal and 0 for a reversed vehicle (DynObj.cpp:1807)
        var end:int = 0 if vehicles[index - 1].start_direction == TrackManager.DIRECTION_REVERSED else 1
        var other_end:int = 1 if vehicles[index].start_direction == TrackManager.DIRECTION_REVERSED else 0
        controllers[index - 1].couple(controllers[index], end, other_end, coupling_type)
