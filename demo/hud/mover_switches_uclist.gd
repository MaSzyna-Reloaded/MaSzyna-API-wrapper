extends "res://hud/mover_switches_section.gd"

@onready var POS = %MainCtrlPos

## A vehicle without a universal controller has no selector to show, which is what the panel's
## own emptiness says - there is no older shape of this to fall back to any more.
func _process(_delta: float) -> void:
    if not universal_controller:
        return
    POS.text = "Pos: " + str(universal_controller.get_selector_position())
