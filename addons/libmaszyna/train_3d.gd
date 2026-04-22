extends RailVehicle3D
class_name Train3D

@export var train_id : StringName = ""

func _ready() -> void:
    super._ready()
    var controller := get_node_or_null(controller_path) as TrainController

    if controller:
        controller.set_train_id(train_id)
