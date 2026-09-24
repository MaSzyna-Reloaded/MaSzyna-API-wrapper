extends "res://hud/mover_switches_section.gd"

@onready var POS = %MainCtrlPos

## Two different things end up in this one readout, which is why it has two sources: a vehicle
## with a universal controller shows that controller's selector, and every other vehicle shows the
## position of its main controller. Dropping the second one emptied the panel on every locomotive
## that has no universal controller, which is most of them.
func _process(_delta: float) -> void:
    if universal_controller:
        POS.text = "Pos: " + str(universal_controller.get_selector_position())
    elif controller:
        POS.text = "Pos: " + str(controller.get_controller_main_position())
