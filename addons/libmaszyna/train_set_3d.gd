@tool
extends Node3D
class_name TrainSet3D

## Consist of a scenery "trainset:" block (simulationstateserializer.cpp:deserialize_trainset).
## Child DynamicRailVehicle3D nodes are its vehicles in scenery order, coupled by couple() the same
## way the original couples them at "endtrainset" (TDynamicObject::AttachNext, DynObj.cpp:2590).

## Coupling flags between child vehicle i and i + 1 (the scenery "couplingdata" of vehicle i).
@export var couplings:PackedInt32Array = []
## The trainset's timetable (a file in the scenery's directory, `none` for none) and the velocity
## it starts with - its driver's first orders (deserialize_endtrainset(),
## simulationstateserializer.cpp:839-848)
@export var timetable:String = ""
@export var velocity:float = 0.0


## endtrainset (simulationstateserializer.cpp:818-837): every vehicle coupled with the next one, once
## the vehicles are built and before the trainset's driver is given anything to do
func couple() -> void:
    var vehicles:Array[DynamicRailVehicle3D] = []
    var controllers:Array[VehicleController] = []
    for child:Node in get_children():
        var vehicle:DynamicRailVehicle3D = child as DynamicRailVehicle3D
        if not vehicle or not vehicle.get_controller():
            continue
        vehicles.append(vehicle)
        controllers.append(vehicle.get_controller())

    for index:int in range(1, controllers.size()):
        var coupling_type:int = couplings[index - 1] if index - 1 < couplings.size() else 0
        # AttachNext: this vehicle's end = iDirection, the next vehicle's end = its iDirection ^ 1,
        # where iDirection is 1 for a normal and 0 for a reversed vehicle (DynObj.cpp:1807)
        var end:int = 0 if vehicles[index - 1].start_direction == TrackManager.DIRECTION_REVERSED else 1
        var other_end:int = 1 if vehicles[index].start_direction == TrackManager.DIRECTION_REVERSED else 0
        controllers[index - 1].couple(controllers[index], end, other_end, coupling_type)
