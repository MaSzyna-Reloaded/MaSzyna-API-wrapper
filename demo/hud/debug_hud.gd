extends HBoxContainer

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    PlayerServer.player_vehicle_changed.connect(_on_player_vehicle_changed)

## The panels are handed the vehicle itself - what they show is its state, and a path to a node
## inside it says nothing they need.
func _on_player_vehicle_changed(vehicle:RID, _previous:RID) -> void:
    var node:RailVehicle3D = instance_from_id(RailVehicleServer.vehicle_get_rail_vehicle(vehicle)) as RailVehicle3D
    $MoverSwitches.vehicle = node.get_controller() if node else null
