extends PanelContainer
class_name CabinControlTooltip

## The caption of the cab control under the cursor (CabinHUDMouseSystem), drawn next to the cursor
## as the original's tooltip is (uilayer.cpp:526): what the control is, what it shows now and the
## keys that work it.

## Where the panel sits relative to the cursor, so the pointer does not cover the text
const CURSOR_OFFSET:Vector2 = Vector2(16.0, 20.0)

@onready var _caption:Label = %Caption
@onready var _hints:Label = %Hints
@onready var _state:Label = %State
## Where the cursor was last seen; a drag captures the mouse, and a captured mouse reports the
## centre of the window instead
var _cursor_position:Vector2 = Vector2.ZERO


func _ready() -> void:
    hide()
    CabinHUDMouseSystem.control_hovered.connect(_on_control_hovered)
    CabinHUDMouseSystem.control_unhovered.connect(hide)
    CabinHUDMouseSystem.control_state_changed.connect(_on_control_state_changed)


func _exit_tree() -> void:
    CabinHUDMouseSystem.control_hovered.disconnect(_on_control_hovered)
    CabinHUDMouseSystem.control_unhovered.disconnect(hide)
    CabinHUDMouseSystem.control_state_changed.disconnect(_on_control_state_changed)


func _input(event:InputEvent) -> void:
    # a drag (CabinHUDMouseSystem) captures the mouse, and its motion then carries the centre of the
    # window - the tooltip stays where the cursor was until the drag puts the cursor back
    if visible and event is InputEventMouseMotion and Input.mouse_mode == Input.MOUSE_MODE_VISIBLE:
        _cursor_position = event.position
        _place()


func _on_control_hovered(caption:String, hints:String, state:String) -> void:
    _caption.text = caption
    _hints.text = hints
    _hints.visible = not hints == ""
    _state.text = state
    _state.visible = not state == ""
    reset_size()
    if Input.mouse_mode == Input.MOUSE_MODE_VISIBLE:
        _cursor_position = get_viewport().get_mouse_position()
    _place()
    show()


func _on_control_state_changed(state:String) -> void:
    _state.text = state
    _state.visible = not state == ""
    reset_size()
    # the new width has to be kept inside the screen too
    _place()


## Next to the cursor, kept inside the screen
func _place() -> void:
    position = (_cursor_position + CURSOR_OFFSET).min(get_viewport_rect().size - size)
