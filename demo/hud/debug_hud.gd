extends HBoxContainer

@export_node_path("MaszynaPlayer") var player_path:NodePath = NodePath("")

var _player:MaszynaPlayer

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    if player_path:
        _player = get_node(player_path)
        if _player:
            _player.controlled_vehicle_changed.connect(_on_controlled_vehicle_changed)

## The panels are handed the vehicle itself - what they show is its state, and a path to a node
## inside it says nothing they need.
func _on_controlled_vehicle_changed():
    var vehicle:RailVehicle3D = _player.controlled_vehicle
    var controller:VehicleController = vehicle.get_controller() if vehicle else null
    $MoverSwitches.vehicle = controller
