class_name UIActionButtons
extends HBoxContainer

## The action row of a window or a dialog: its UIButtons in a row, whatever else stands in it (a
## status line) left alone. The row's keyboard focus is its own, like a FocusSection's - no button
## takes Godot's focus, so a search field or a text box keeps it: while the row has it, the chosen
## button lights up as under the mouse, left and right, Tab and Shift+Tab choose among the buttons
## that can be pressed, and Enter presses the chosen one; other keys go on to whoever has them.

## Tab and Shift+Tab choose among the buttons too - off in a window whose Tab walks its sections
## (the settings), so the row is left by it instead
@export var tab_walks_buttons: bool = true

var _focused: bool = false
var _chosen: UIButton = null


## The row takes the keyboard, its answer - the rightmost button that can be pressed - chosen
func grab_button_focus() -> void:
    _focused = true
    _choose(_pressable_buttons()[-1])


func release_button_focus() -> void:
    _focused = false
    _choose(null)


func _input(event: InputEvent) -> void:
    if not _focused or not is_visible_in_tree():
        return
    var buttons: Array[UIButton] = _pressable_buttons()
    var at: int = buttons.find(_chosen)
    if event.is_action_pressed("menu_activate", false, true):
        # the key presses the chosen button, as a click does
        _chosen.pressed.emit()
    elif (
        event.is_action_pressed("menu_left", true, true)
        or tab_walks_buttons and event.is_action_pressed("menu_previous_section", true, true)
    ):
        _choose(buttons[wrapi(at - 1, 0, buttons.size())])
    elif (
        event.is_action_pressed("menu_right", true, true)
        or tab_walks_buttons and event.is_action_pressed("menu_next_section", true, true)
    ):
        _choose(buttons[wrapi(at + 1, 0, buttons.size())])
    else:
        return
    get_viewport().set_input_as_handled()


func _choose(button: UIButton) -> void:
    if _chosen:
        _chosen.set_chosen(false)
    _chosen = button
    if _chosen:
        _chosen.set_chosen(true)


func _pressable_buttons() -> Array[UIButton]:
    var buttons: Array[UIButton] = []
    for child: Node in get_children():
        var button: UIButton = child as UIButton
        if button and button.visible and not button.disabled:
            buttons.append(button)
    return buttons
