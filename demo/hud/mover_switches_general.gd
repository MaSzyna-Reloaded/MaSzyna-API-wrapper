extends "res://hud/mover_switches_section.gd"


@onready var FORWARD = $HBoxContainer2/Forward
@onready var REVERSE = $HBoxContainer2/Reverse


func _ready() -> void:
    pass # Replace with function body.


func _process(delta: float) -> void:
    if controller:
        var state = controller.state

        FORWARD.modulate = Color.GREEN if state.get("direction", 0) > 0 else Color.WHITE
        REVERSE.modulate = Color.GREEN if state.get("direction", 0) < 0 else Color.WHITE
