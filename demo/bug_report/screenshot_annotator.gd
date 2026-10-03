class_name ScreenshotAnnotator
extends Control

## A screenshot on which the player marks what a report is about: a left drag draws a red
## rectangle, a right click takes the last one away. The image is drawn whole, its aspect kept;
## the rectangles are kept in the image's pixels, so they land where they were drawn whatever
## the size of the control.

const MARK_COLOR: Color = Color(1.0, 0.0, 0.0)
## Width of a rectangle's edge on the control
const MARK_LINE_WIDTH: float = 3.0
## ...and burnt into the image, in its pixels
const MARK_BORDER_PX: int = 4

var _image: Image = null
var _texture: ImageTexture = null
## The marks, in the image's pixels
var _marks: Array[Rect2] = []
## The corner a drag started at, in the image's pixels, while one goes on
var _drag_from: Vector2 = Vector2.ZERO
var _dragging: bool = false


## Shows the screenshot without marks
func set_image(image: Image) -> void:
    _image = image
    _texture = ImageTexture.create_from_image(image)
    _marks.clear()
    _dragging = false
    queue_redraw()


## The screenshot with its marks burnt in
func render_annotated_image() -> Image:
    var result: Image = _image.duplicate() as Image
    result.convert(Image.FORMAT_RGBA8)
    for mark: Rect2 in _marks:
        var rect: Rect2i = Rect2i(mark)
        result.fill_rect(Rect2i(rect.position, Vector2i(rect.size.x, MARK_BORDER_PX)), MARK_COLOR)
        result.fill_rect(Rect2i(rect.position.x, rect.end.y - MARK_BORDER_PX, rect.size.x, MARK_BORDER_PX), MARK_COLOR)
        result.fill_rect(Rect2i(rect.position, Vector2i(MARK_BORDER_PX, rect.size.y)), MARK_COLOR)
        result.fill_rect(Rect2i(rect.end.x - MARK_BORDER_PX, rect.position.y, MARK_BORDER_PX, rect.size.y), MARK_COLOR)
    return result


## Adds a mark, in the image's pixels, clipped to the image
func add_mark(rect: Rect2) -> void:
    var clipped: Rect2 = rect.abs().intersection(Rect2(Vector2.ZERO, Vector2(_image.get_size())))
    if clipped.has_area():
        _marks.append(clipped)
    queue_redraw()


func remove_last_mark() -> void:
    if _marks:
        _marks.pop_back()
    queue_redraw()


## Where the image is drawn on the control: the largest rectangle of its aspect, centred
func _image_rect() -> Rect2:
    var image_size: Vector2 = Vector2(_image.get_size())
    var scale_factor: float = minf(size.x / image_size.x, size.y / image_size.y)
    var drawn: Vector2 = image_size * scale_factor
    return Rect2((size - drawn) / 2.0, drawn)


func _to_image(point: Vector2) -> Vector2:
    var rect: Rect2 = _image_rect()
    return (point - rect.position) * Vector2(_image.get_size()) / rect.size


func _to_control(rect: Rect2) -> Rect2:
    var drawn: Rect2 = _image_rect()
    var scale_factor: Vector2 = drawn.size / Vector2(_image.get_size())
    return Rect2(drawn.position + rect.position * scale_factor, rect.size * scale_factor)


func _gui_input(event: InputEvent) -> void:
    if not _image:
        return
    var button: InputEventMouseButton = event as InputEventMouseButton
    if button and button.button_index == MOUSE_BUTTON_LEFT:
        if button.pressed:
            _drag_from = _to_image(button.position)
            _dragging = true
        elif _dragging:
            _dragging = false
            add_mark(Rect2(_drag_from, _to_image(button.position) - _drag_from))
        accept_event()
    elif button and button.button_index == MOUSE_BUTTON_RIGHT and button.pressed:
        remove_last_mark()
        accept_event()
    elif event is InputEventMouseMotion and _dragging:
        queue_redraw()


func _draw() -> void:
    if not _texture:
        return
    draw_texture_rect(_texture, _image_rect(), false)
    for mark: Rect2 in _marks:
        draw_rect(_to_control(mark), MARK_COLOR, false, MARK_LINE_WIDTH)
    if _dragging:
        var current: Vector2 = _to_image(get_local_mouse_position())
        draw_rect(_to_control(Rect2(_drag_from, current - _drag_from).abs()), MARK_COLOR, false, MARK_LINE_WIDTH)
