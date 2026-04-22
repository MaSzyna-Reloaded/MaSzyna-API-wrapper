@tool
extends RailVehicle3D
class_name Train3D

var _train_registered: bool = false

@export var train_id: String

func _ready() -> void:
    super._ready()

    if Engine.is_editor_hint():
        return

    var active_controller := get_controller()
    if not train_id:
        push_warning(self, ": Train3D requires a non-empty train_id before registration")
        return

    if not active_controller:
        push_error(self, ": Train3D could not register because TrainController is not available")
        return

    TrainSystem.register_train(train_id, active_controller)
    _train_registered = true

    
func _exit_tree() -> void:
    if not Engine.is_editor_hint() and train_id:
        TrainSystem.unregister_train(train_id)
    super._exit_tree()


func get_controller() -> TrainController:
    return super.get_controller() as TrainController


func get_supported_commands() -> Dictionary:
    var active_controller := get_controller()
    return active_controller.get_supported_commands() if active_controller else {}


func send_command(command_name: StringName, p1: Variant = null, p2: Variant = null) -> void:
    if train_id:
        TrainSystem.send_command(train_id, command_name, p1, p2)
