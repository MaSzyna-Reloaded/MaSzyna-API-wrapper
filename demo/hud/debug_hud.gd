extends HBoxContainer

@export_node_path("MaszynaPlayer") var player_path:NodePath = NodePath("")

var _player:MaszynaPlayer

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    if player_path:
        _player = get_node(player_path)
        if _player:
            _player.controlled_vehicle_changed.connect(_on_controlled_vehicle_changed)

func _on_controlled_vehicle_changed():
    var controller:TrainNode
    if _player.controlled_vehicle and _player.controlled_vehicle.controller_path:
        controller = _player.controlled_vehicle.get_controller()
        $MoverSwitches.train_controller = $MoverSwitches.get_path_to(controller)
        $Gauges.train_controller = $Gauges.get_path_to(controller)
    else:
        $MoverSwitches.train_controller = NodePath("")
        $Gauges.train_controller = NodePath("")

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    pass
