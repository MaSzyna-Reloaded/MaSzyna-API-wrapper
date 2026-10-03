extends Control

## The notice that this is a development version, over the scenery selector until the player
## acknowledges it. While it is shown it takes every key, so neither the selector's sections nor
## its search field see them; the mouse is stopped by the dimmed backdrop.

signal acknowledged


func acknowledge() -> void:
    visible = false
    acknowledged.emit()


func _input(event: InputEvent) -> void:
    if not visible or not event is InputEventKey:
        return
    get_viewport().set_input_as_handled()
    if event.is_action_pressed("menu_activate", false, true):
        acknowledge()
