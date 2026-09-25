extends "res://hud/mover_switches_section.gd"


## Two different things end up in the main readout, which is why it has two sources: a vehicle
## with a universal controller shows that controller's selector, and every other vehicle shows the
## position of its main controller. Dropping the second one emptied the panel on every locomotive
## that has no universal controller, which is most of them.
func _on_refresh_timer_timeout() -> void:
    if not controller:
        return
    var direction:int = controller.get_direction()
    %Forward.modulate = Color.GREEN if direction > 0 else Color.WHITE
    %Reverse.modulate = Color.GREEN if direction < 0 else Color.WHITE
    if universal_controller:
        %MainPosition.text = tr("Pos: %s") % universal_controller.get_selector_position()
    else:
        %MainPosition.text = tr("Pos: %s") % controller.get_controller_main_position()
    %SecondPosition.text = tr("Pos: %s") % controller.get_controller_second_position()
