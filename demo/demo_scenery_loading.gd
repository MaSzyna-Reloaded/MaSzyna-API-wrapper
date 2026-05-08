extends Node3D


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    var args = OS.get_cmdline_user_args()
    if args:
        $MaszynaSceneryNode.filename = args[0]  # load scenery from arg
    $MaszynaSceneryNode.load()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    pass
