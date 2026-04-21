@tool
extends RailVehicle3D
class_name Train3D

signal power_changed(is_powered)
signal radio_toggled(is_enabled)
signal radio_channel_changed(channel)
signal command_received(command, p1, p2)
signal mover_initialized
signal mover_config_changed
signal config_changed

var train_id: String:
    get:
        var active_controller := get_controller() as TrainController
        return active_controller.get_train_id() if active_controller else ""

var type_name: String:
    get:
        var active_controller := get_controller() as TrainController
        return active_controller.get_type_name() if active_controller else ""


func get_controller() -> TrainController:
    return super.get_controller() as TrainController


func get_supported_commands() -> Dictionary:
    var active_controller := get_controller()
    return active_controller.get_supported_commands() if active_controller else {}


func send_command(command_name: StringName, p1: Variant = null, p2: Variant = null) -> void:
    var active_controller := get_controller()
    if active_controller:
        active_controller.send_command(command_name, p1, p2)


func _instantiate_runtime_controller() -> void:
    super._instantiate_runtime_controller()
    _bind_train_controller_signals()


func _bind_train_controller_signals() -> void:
    if _controller_signals_bound:
        return

    var active_controller := get_controller()
    if active_controller == null:
        return

    _controller_signals_bound = true
    active_controller.power_changed.connect(func(is_powered): power_changed.emit(is_powered))
    active_controller.radio_toggled.connect(func(is_enabled): radio_toggled.emit(is_enabled))
    active_controller.radio_channel_changed.connect(func(channel): radio_channel_changed.emit(channel))
    active_controller.command_received.connect(func(command_name, p1, p2): command_received.emit(command_name, p1, p2))
    active_controller.mover_initialized.connect(func(): mover_initialized.emit())
    active_controller.mover_config_changed.connect(func(): mover_config_changed.emit())
    active_controller.config_changed.connect(func(): config_changed.emit())
