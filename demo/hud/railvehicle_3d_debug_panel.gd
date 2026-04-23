@tool
extends DebugPanel

@export_node_path("RailVehicle3D") var vehicle_path = NodePath("")

var _vehicle : RailVehicle3D
var _controller : TrainController

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    super()
    _vehicle = get_node_or_null(vehicle_path) as RailVehicle3D
    if _vehicle:
        _controller =_vehicle.get_controller()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    super(delta)
    _draw_state()

func _draw_state():
    if not _controller:
        text = "[ .. no controller .. ]"
        return
    var dict = _controller.state
    
    var lines = []
    for k in dict.keys():
        lines.append("%s=%s" % [k, dict[k]])
    text = "\n".join(lines)
