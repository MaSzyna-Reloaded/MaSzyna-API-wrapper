class_name FocusSection
extends PanelContainer

## Container of one keyboard-navigable section of a screen. The keyboard focus of the selector is
## virtual - no Control ever takes the Godot focus away from the search field - so the section
## itself has to show where that focus is. What it looks like is the theme's decision
## (selector_theme.tres: FocusSection / FocusSectionActive), the same way the list rows work.

## The section asks for the focus - a click landed on it. The screen keeps the sections exclusive,
## so it is the screen that decides and calls grab_section_focus() back.
signal focus_requested
## The section was given the focus. The focus sound hangs off this.
signal focus_taken
## The keyboard left the section on that side. A section with nothing to walk - a row of buttons -
## leaves on the first press; one that walks items emits it when a step would go past its edge. One
## signal per side and no side in a parameter: each one is wired to wherever the screen wants the
## keyboard to land, and only the ones that lead somewhere are wired at all.
signal navigate_left
signal navigate_right
signal navigate_up
signal navigate_down
## Enter while this section had the focus. A section of plain controls needs nothing more than this
## to be usable from the keyboard - the "Wczytaj" row is one.
signal activated
## Escape while this section had the focus: one step back, and what back means is the user's own
## decision - it wires this to the screen's own "focus that section" method, to a close(), to a
## quit.
signal cancelled

const VARIATION_IDLE: StringName = &"FocusSection"
const VARIATION_ACTIVE: StringName = &"FocusSectionActive"

var focused: bool = false:
    set(value):
        focused = value
        _dirty = true
        set_process(true)

var _dirty: bool = false


## Called by the screen that manages the sections, and by nobody else: the sections are exclusive,
## so one of them taking the focus for itself would leave two of them lit. A section that wants the
## focus asks for it with focus_requested.
func grab_section_focus() -> void:
    if focused:
        return
    focused = true
    focus_taken.emit()


func release_section_focus() -> void:
    focused = false


## The keys every section has: Enter, Escape, and the four arrows as the way out of it. A subclass
## that walks items takes the arrows first and ends its own handler with super(event), so only what
## it did not use reaches this.
## ui_text_submit and not ui_accept: that one is Space as well, and Space belongs to a search field.
func _input(event: InputEvent) -> void:
    if not focused or not is_visible_in_tree():
        return
    if event.is_action_pressed("ui_text_submit"):
        activated.emit()
    elif event.is_action_pressed("ui_cancel"):
        cancelled.emit()
    elif event.is_action_pressed("ui_left", true):
        navigate_left.emit()
    elif event.is_action_pressed("ui_right", true):
        navigate_right.emit()
    elif event.is_action_pressed("ui_up", true):
        navigate_up.emit()
    elif event.is_action_pressed("ui_down", true):
        navigate_down.emit()
    else:
        return
    get_viewport().set_input_as_handled()


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


## The same for a section whose items stand in a row - the vehicles of a trainset
static func scroll_to_item_in_row(scroll: ScrollContainer, item: Control) -> void:
    var offset: int = scroll.scroll_horizontal
    if item.position.x < float(offset):
        scroll.scroll_horizontal = int(item.position.x)
    elif item.position.x + item.size.x > float(offset) + scroll.size.x:
        scroll.scroll_horizontal = int(item.position.x + item.size.x - scroll.size.x)
