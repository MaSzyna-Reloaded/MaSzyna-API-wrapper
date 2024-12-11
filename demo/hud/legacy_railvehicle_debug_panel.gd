extends DebugPanel

@export var controller_path = NodePath("")

var _controller : LegacyRailVehicle

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    _controller = get_node_or_null(controller_path) as LegacyRailVehicle


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
