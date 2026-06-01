extends "res://hud/mover_switches_section.gd"

@onready var POS = %MainCtrlPos

func _process(delta: float) -> void:
    if controller:
        var state = controller.state
        if "selector_position" in state:
            POS.text = "Pos: " + str(state.get("selector_position", 0))
        elif "controller_main_position" in state: # for fallback/older version compatibility if needed
            POS.text = "Pos: " + str(state.get("controller_main_position", 0))
