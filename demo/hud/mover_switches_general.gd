extends "res://hud/mover_switches_section.gd"


@onready var FORWARD = $HBoxContainer2/Forward
@onready var REVERSE = $HBoxContainer2/Reverse


func _ready() -> void:
    pass # Replace with function body.


func _process(_delta: float) -> void:
    if not controller:
        return
    var direction:int = controller.get_direction()
    FORWARD.modulate = Color.GREEN if direction > 0 else Color.WHITE
    REVERSE.modulate = Color.GREEN if direction < 0 else Color.WHITE
