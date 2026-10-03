extends Button

## The report's button at the right edge of the screen: it waits past the edge and slides in when
## the mouse touches the edge, anywhere along it, and back out once the mouse is left of it - as
## the top bar does at the top edge (hud/top_bar.gd). Anchored to the right edge, it slides by its
## offsets, so a resized window keeps it where it is.

## Pixels from the right edge of the screen that bring the button in
const REVEAL_EDGE: float = 2.0
## Seconds of the slide in or out
const SLIDE_TIME: float = 0.15

var _shown: bool = false
var _slide_tween: Tween = null
## The width the scene gives it - not size.x, which shrinks to the minimum while the offsets pass
## through each other (a retract from the shown position)
var _width: float = 0.0


func _ready() -> void:
    _width = offset_right - offset_left


func _input(event: InputEvent) -> void:
    var motion: InputEventMouseMotion = event as InputEventMouseMotion
    if not motion:
        return
    var right: float = get_viewport_rect().size.x
    if motion.global_position.x >= right - REVEAL_EDGE:
        _slide(true)
    elif motion.global_position.x < right - _width:
        _slide(false)


## Out of sight at once - before a screenshot is taken
func retract() -> void:
    if _slide_tween:
        _slide_tween.kill()
    _shown = false
    offset_left = 0.0
    offset_right = _width


func _slide(shown: bool) -> void:
    if shown == _shown:
        return
    _shown = shown
    if _slide_tween:
        _slide_tween.kill()
    _slide_tween = create_tween().set_parallel()
    _slide_tween.tween_property(self, "offset_left", -_width if shown else 0.0, SLIDE_TIME)
    _slide_tween.tween_property(self, "offset_right", 0.0 if shown else _width, SLIDE_TIME)
