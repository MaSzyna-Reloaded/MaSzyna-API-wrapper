@tool
extends Button
class_name NodebankItemTile

signal tile_selected(item_data: NodebankGridItem)
signal tile_dragged(item_data: NodebankGridItem)
signal tile_dropped(item_data: NodebankGridItem)

var _item_data: NodebankGridItem
var _dragging: bool = false

func _ready() -> void:
    focus_mode = Control.FOCUS_NONE
    custom_minimum_size = Vector2(160, 160)
    text_overrun_behavior = TextServer.OVERRUN_TRIM_ELLIPSIS
    clip_text = true
    icon_alignment = HORIZONTAL_ALIGNMENT_CENTER
    vertical_icon_alignment = VERTICAL_ALIGNMENT_TOP
    expand_icon = true
    pressed.connect(_on_pressed)

func configure(item_data: NodebankGridItem, preview_renderer: NodebankPreviewRenderer) -> void:
    _item_data = item_data
    text = item_data.model.name
    tooltip_text = text
    icon = await preview_renderer.get_preview(item_data)
    _item_data.preview_texture = icon

func _gui_input(event: InputEvent) -> void:
    if event is InputEventMouseButton:

        var mouse_button: InputEventMouseButton = event as InputEventMouseButton
        if mouse_button.button_index == MOUSE_BUTTON_LEFT:
            if mouse_button.pressed:
                if not _dragging:
                    _dragging = true
                    tile_dragged.emit(_item_data)
            else:
                _dragging = false
                tile_dropped.emit(_item_data)

func _on_pressed() -> void:
    tile_selected.emit(_item_data)
