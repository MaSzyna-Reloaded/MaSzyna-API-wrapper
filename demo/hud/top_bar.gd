@tool
extends HUDWindow
class_name TopBar

## The bar waits above the screen and slides in when the mouse touches the top edge, and back out
## once the mouse is below it and none of its menus is open. It slides rather than hides: a hidden
## MenuBar would stop handling the keys of its entries.

## Pixels from the top edge of the screen that bring the bar in
const REVEAL_EDGE: float = 2.0
## Seconds of the slide in or out
const SLIDE_TIME: float = 0.15

var _shown: bool = false
var _slide_tween: Tween = null


func _enter_tree() -> void:
    allow_close = false
    drag_enabled = false
    resize_enabled = false
    window_decorations = false
    custom_minimum_size = Vector2.ZERO
    super()
    top_level = false
    set_anchors_and_offsets_preset(Control.PRESET_TOP_WIDE, Control.PRESET_MODE_MINSIZE, 0)
    grow_horizontal = Control.GROW_DIRECTION_BOTH


func _ready() -> void:
    super()
    custom_minimum_size = Vector2.ZERO
    set_anchors_and_offsets_preset(Control.PRESET_TOP_WIDE, Control.PRESET_MODE_MINSIZE, 0)
    if not Engine.is_editor_hint():
        position.y = -size.y


func _input(event: InputEvent) -> void:
    super(event)
    var motion: InputEventMouseMotion = event as InputEventMouseMotion
    if not motion or Engine.is_editor_hint():
        return
    if motion.global_position.y <= REVEAL_EDGE:
        _slide(true)
    # an open menu is a popup window of its own, and the mouse on it is below the bar
    elif motion.global_position.y > size.y and not get_viewport().get_embedded_subwindows():
        _slide(false)


func _slide(shown: bool) -> void:
    if shown == _shown:
        return
    _shown = shown
    if _slide_tween:
        _slide_tween.kill()
    _slide_tween = create_tween()
    _slide_tween.tween_property(self, "position:y", 0.0 if shown else -size.y, SLIDE_TIME)


func _get_content_rect() -> Rect2:
    return Rect2(Vector2.ZERO, size)


func _get_minimum_size() -> Vector2:
    var minimum_size: Vector2 = Vector2.ZERO
    for child: Node in get_children():
        if _is_managed_child(child):
            continue
        if child is Control:
            var child_control: Control = child as Control
            minimum_size.y = maxf(minimum_size.y, child_control.get_combined_minimum_size().y)
    return minimum_size


func _notification(what: int) -> void:
    super(what)
    if Engine.is_editor_hint() and (what == NOTIFICATION_THEME_CHANGED or what == NOTIFICATION_SORT_CHILDREN):
        set_anchors_and_offsets_preset(Control.PRESET_TOP_WIDE, Control.PRESET_MODE_MINSIZE, 0)
