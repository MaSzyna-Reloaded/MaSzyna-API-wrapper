@tool
extends Container
class_name DebugPanel

var _dirty = false
var _filter:String = ""
var _dragging := false
var _drag_grab_offset := Vector2.ZERO
var _resizing := false
var _resize_start_mouse := Vector2.ZERO
var _resize_start_size := Vector2.ZERO
var _minimum_resize_size := Vector2.ZERO

@export var drag_enabled := false
@export var resize_enabled := false

@export var text:String:
    set(x):
        _dirty = true
        text = x

@export var title:String:
    set(x):
        _dirty = true
        title = x


func _process(_delta):
    _update_resize_handle_position()

    if _dirty:
        _dirty = false
        $%Title.text = title
        var final_text = text
        if _filter:
            var _tokens = _filter.split(" ")
            var _f = []
            for line in text.split("\n"):
                var _matched_count = 0
                for _t in _tokens:
                    if not _t or line.match("*"+_t+"*"):
                        _matched_count += 1
                if _tokens.size() == _matched_count:
                    _f.append(line)

            final_text = "\n".join(_f)
        $DebugPanel/RichTextLabel.text = final_text


func _ready() -> void:
    _minimum_resize_size = Vector2(
        custom_minimum_size.x if custom_minimum_size.x > 0.0 else size.x,
        custom_minimum_size.y if custom_minimum_size.y > 0.0 else size.y
    )
    $%ResizeHandle.top_level = true
    $%ResizeHandle.visible = resize_enabled
    _update_resize_handle_position()


func _on_filter_text_changed(new_text):
    _filter = new_text


func _on_clear_button_up():
    _filter = ""
    $%Filter.text = ""


func _bring_to_front() -> void:
    move_to_front()
    _update_resize_handle_position()


func _update_resize_handle_position() -> void:
    var handle := $%ResizeHandle as Control
    if not handle.visible:
        return
    handle.size = handle.custom_minimum_size
    handle.global_position = global_position + size - handle.size


func _try_release_filter_focus(event: InputEvent) -> void:
    if not event is InputEventMouseButton:
        return
    if not event.pressed or event.button_index != MOUSE_BUTTON_LEFT:
        return

    var filter := $%Filter as Control
    if not filter.has_focus():
        return

    if not filter.get_global_rect().has_point(event.global_position):
        filter.release_focus()


func _input(event: InputEvent) -> void:
    _try_release_filter_focus(event)

    if _resizing:
        if event is InputEventMouseMotion:
            var requested_size: Vector2 = _resize_start_size + (event.global_position - _resize_start_mouse)
            size = Vector2(
                maxf(requested_size.x, _minimum_resize_size.x),
                maxf(requested_size.y, _minimum_resize_size.y)
            )
            get_viewport().set_input_as_handled()
            return
        elif event is InputEventMouseButton and not event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
            _resizing = false
            get_viewport().set_input_as_handled()
            return

    if drag_enabled and _dragging:
        if event is InputEventMouseMotion:
            global_position = event.global_position - _drag_grab_offset
            get_viewport().set_input_as_handled()
        elif event is InputEventMouseButton and not event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
            _dragging = false
            get_viewport().set_input_as_handled()



func _on_header_gui_input(event: InputEvent) -> void:
    if not drag_enabled:
        return

    if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT:
        if event.pressed:
            _bring_to_front()
        _dragging = event.pressed
        if event.pressed:
            _drag_grab_offset = event.global_position - global_position
        get_viewport().set_input_as_handled()


func _on_panel_body_gui_input(event: InputEvent) -> void:
    if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
        _bring_to_front()


func _on_resize_handle_gui_input(event: InputEvent) -> void:
    if not resize_enabled:
        return

    if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT:
        if event.pressed:
            _bring_to_front()
            _resizing = true
            _dragging = false
            _resize_start_mouse = event.global_position
            _resize_start_size = size
        else:
            _resizing = false
        get_viewport().set_input_as_handled()
