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
    if _player.controlled_vehicle is Train3D:
        $MoverSwitches.train_controller = $MoverSwitches.get_path_to(_player.controlled_vehicle)
        $Gauges.train_controller = $Gauges.get_path_to(_player.controlled_vehicle)
    else:
        $MoverSwitches.train_controller = NodePath("")
        $Gauges.train_controller = NodePath("")

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta: float) -> void:
    pass
