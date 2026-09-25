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
    if visible and event is InputEventMouseMotion:
        _place(event.position)


func _on_control_hovered(caption:String, hints:String, state:String) -> void:
    _caption.text = caption
    _hints.text = hints
    _hints.visible = not hints == ""
    _state.text = state
    _state.visible = not state == ""
    reset_size()
    _place(get_viewport().get_mouse_position())
    show()


func _on_control_state_changed(state:String) -> void:
    _state.text = state
    _state.visible = not state == ""
    reset_size()


## Next to the cursor, kept inside the screen
func _place(cursor_position:Vector2) -> void:
    position = (cursor_position + CURSOR_OFFSET).min(get_viewport_rect().size - size)
