class_name UIDialog
extends Control

## A dialog over a screen in the look of the game's notices and buttons: "Back" on the left, then
## the alternative, then the answer on the right, each in the colour of what it does. Left and
## right, Tab and Shift+Tab walk the buttons shown - the chosen one is the row's, and its
## background lights up - and Enter presses it; the answer is chosen when the dialog is asked.
## "Back" and Escape take the dialog back unanswered. A button whose text is empty is not shown:
## with neither back_text nor alternative_text the dialog is a notice with one button. While it is
## shown it takes every key, so the screen under it sees none of them, and the mouse is stopped by
## the dimmed backdrop. It is a top-level control: it covers the whole screen and draws over the
## rest wherever it stands in the tree - under a screen, a list row, a button. The texts are put on
## it when it is asked, so a message made at that moment (a vehicle's name) is set just before.

signal confirmed
signal alternative_chosen
## "Back" or Escape: the dialog went without an answer
signal dismissed

## Texts as msgids - the labels and buttons translate them
@export var title: String = ""
@export_multiline var message: String = ""
@export var confirm_text: String = ""
@export var alternative_text: String = ""
@export var back_text: String = ""
@export var confirm_intent: UIButton.Intent = UIButton.Intent.SUCCESS
@export var alternative_intent: UIButton.Intent = UIButton.Intent.DEFAULT


## The buttons' intents are the dialog's settings, so they are given before the buttons are ready
## and take them on
func _enter_tree() -> void:
    %AlternativeButton.intent = alternative_intent
    %ConfirmButton.intent = confirm_intent


func ask() -> void:
    %Title.text = title
    %Message.text = message
    %BackButton.text = back_text
    %BackButton.visible = not back_text == ""
    %AlternativeButton.text = alternative_text
    %AlternativeButton.visible = not alternative_text == ""
    %ConfirmButton.text = confirm_text
    visible = true


## The answer takes the focus once the dialog is on the screen - asked from a screen that is not
## shown yet (the development notice), that is when the screen shows
func _notification(what: int) -> void:
    if not what == NOTIFICATION_VISIBILITY_CHANGED:
        return
    if is_visible_in_tree():
        %Buttons.grab_button_focus()
    else:
        %Buttons.release_button_focus()


func confirm() -> void:
    visible = false
    confirmed.emit()


func choose_alternative() -> void:
    visible = false
    alternative_chosen.emit()


func dismiss() -> void:
    visible = false
    dismissed.emit()


## The action row walks its buttons and presses the chosen one before this sees the key
func _input(event: InputEvent) -> void:
    if not visible or not event is InputEventKey:
        return
    get_viewport().set_input_as_handled()
    if event.is_action_pressed("menu_back", false, true):
        dismiss()
