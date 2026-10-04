class_name UISearchInput
extends PanelContainer

## A search field of the game: its frame, the magnifier, the field and the button that clears it,
## shown while there is something to clear. Whoever filters by it hears of the typing and of the
## clearing as two events, and reads the text when it filters.

## The player typed - the text as it is now
signal typed(text: String)
## The clear button emptied the field
signal cleared

## As a msgid - the field translates it
@export var placeholder_text: String = ""


func _ready() -> void:
    %Field.placeholder_text = placeholder_text


func get_text() -> String:
    return %Field.text


## Empty again, as if nothing had been typed - no event, the owner starts over itself
func reset() -> void:
    %Field.text = ""
    %ClearButton.visible = false


## The keyboard types into the field
func grab_typing_focus() -> void:
    %Field.grab_focus()
    %Field.grab_click_focus()


## Only a field that holds the focus gives it up: release_focus() clears the whole viewport's focus,
## so an unconditional one would take away what was granted elsewhere a moment before.
func release_typing_focus() -> void:
    if %Field.has_focus():
        %Field.release_focus()


func _on_field_text_changed(text: String) -> void:
    %ClearButton.visible = not text == ""
    typed.emit(text)


func _on_clear_button_pressed() -> void:
    reset()
    grab_typing_focus()
    cleared.emit()
