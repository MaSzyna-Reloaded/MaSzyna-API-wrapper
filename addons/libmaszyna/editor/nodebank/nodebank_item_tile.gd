@tool
extends Button
class_name NodebankItemTile

signal tile_selected(item_data: NodebankGridItem)
signal tile_dragged(item_data: NodebankGridItem)

var _item_data: NodebankGridItem

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

# Control._get_drag_data
func _get_drag_data(_at_position: Vector2) -> Variant:
    var preview: Label = Label.new()
    preview.text = text
    set_drag_preview(preview)
    tile_dragged.emit(_item_data)
    return _item_data

func _on_pressed() -> void:
    tile_selected.emit(_item_data)
    
