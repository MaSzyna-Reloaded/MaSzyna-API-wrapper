class_name FocusSection
extends PanelContainer

## Container of one keyboard-navigable section of a screen. The keyboard focus of the selector is
## virtual - no Control ever takes the Godot focus away from the search field - so the section
## itself has to show where that focus is. What it looks like is the theme's decision
## (selector_theme.tres: FocusSection / FocusSectionActive), the same way the list rows work.

const VARIATION_IDLE: StringName = &"FocusSection"
const VARIATION_ACTIVE: StringName = &"FocusSectionActive"

var focused: bool = false:
    set(value):
        focused = value
        _dirty = true
        set_process(true)

var _dirty: bool = false


func _ready() -> void:
    set_process(false)


func _process(_delta: float) -> void:
    if _dirty:
        _process_dirty()
    set_process(false)


func _process_dirty() -> void:
    _dirty = false
    theme_type_variation = VARIATION_ACTIVE if focused else VARIATION_IDLE


## Scroll of a section's list, set from the item's own place in it: to the item's near edge when it
## is above the view, to its far edge when it is below, and left alone when it is already inside.
static func scroll_to_item(scroll: ScrollContainer, item: Control) -> void:
    var offset: int = scroll.scroll_vertical
    if item.position.y < float(offset):
        scroll.scroll_vertical = int(item.position.y)
    elif item.position.y + item.size.y > float(offset) + scroll.size.y:
        scroll.scroll_vertical = int(item.position.y + item.size.y - scroll.size.y)


## The same for a section whose items stand in a row - the consist preview
static func scroll_to_item_in_row(scroll: ScrollContainer, item: Control) -> void:
    var offset: int = scroll.scroll_horizontal
    if item.position.x < float(offset):
        scroll.scroll_horizontal = int(item.position.x)
    elif item.position.x + item.size.x > float(offset) + scroll.size.x:
        scroll.scroll_horizontal = int(item.position.x + item.size.x - scroll.size.x)
