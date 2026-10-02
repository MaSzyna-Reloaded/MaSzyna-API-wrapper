extends PanelContainer
class_name TopBar

## The game's menus as a floating chip in the top left corner, like the vehicle chip in the bottom
## right one (FloatingChip). It waits above the screen and slides in when the mouse touches the top
## edge, and back out once the mouse is below it and none of its menus is open. It slides rather
## than hides: a hidden MenuBar would stop handling the keys of its entries.

## Pixels from the top edge of the screen that bring the bar in
const REVEAL_EDGE: float = 2.0
## Seconds of the slide in or out
const SLIDE_TIME: float = 0.15
## Pixels between the chip and the screen's top edge while it is shown
const SHOWN_TOP: float = 8.0

var _shown: bool = false
var _slide_tween: Tween = null


func _ready() -> void:
    position.y = -get_combined_minimum_size().y


func _input(event: InputEvent) -> void:
    var motion: InputEventMouseMotion = event as InputEventMouseMotion
    if not motion:
        return
    if motion.global_position.y <= REVEAL_EDGE:
        _slide(true)
    # an open menu is a popup window of its own, and the mouse on it is below the bar
    elif motion.global_position.y > SHOWN_TOP + size.y and not get_viewport().get_embedded_subwindows():
        _slide(false)


func _slide(shown: bool) -> void:
    if shown == _shown:
        return
    _shown = shown
    if _slide_tween:
        _slide_tween.kill()
    _slide_tween = create_tween()
    _slide_tween.tween_property(self, "position:y", SHOWN_TOP if shown else -size.y, SLIDE_TIME)
