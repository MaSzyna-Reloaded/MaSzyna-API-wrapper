@tool
extends HUDWindow
class_name TopBar

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
