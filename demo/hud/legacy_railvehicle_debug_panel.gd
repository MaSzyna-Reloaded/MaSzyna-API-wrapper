extends DebugPanel

@export var controller_path = NodePath("")

var _controller

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    var vehicle := get_node_or_null(controller_path) as RailVehicle3D
    _controller = vehicle.get_controller() if vehicle else null


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    super(delta)
    _draw_state()

func _draw_state():
    if not _controller:
        text = "[ .. no controller .. ]"
        return
    var dict = _controller.get_state()
    
    var lines = []
    for k in dict.keys():
        lines.append("%s=%s" % [k, dict[k]])
    text = "\n".join(lines)
