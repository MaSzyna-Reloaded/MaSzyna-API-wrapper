class_name UIButton
extends Button

## A button of the game in the colour of what it does, wherever it stands: it takes its look from
## ui_theme.tres itself, so no window or theme around it has to provide it. It takes no Godot focus;
## the row it stands in (UIActionButtons) chooses it from the keyboard, and the chosen one is lit as
## under the mouse and framed.

## Default (plain, blue-outlined), success (green: "Save", "Send report", "I understand"), warning
## (orange) and danger (reddish: "Discard changes", "Remove", "Exit to menu")
enum Intent {
    DEFAULT,
    SUCCESS,
    WARNING,
    DANGER,
}

const UI_THEME: Theme = preload("ui_theme.tres")
## ui_theme.tres
const INTENT_VARIATIONS: Dictionary[Intent, StringName] = {
    Intent.DEFAULT: &"UIButtonDefault",
    Intent.SUCCESS: &"UIButtonSuccess",
    Intent.WARNING: &"UIButtonWarning",
    Intent.DANGER: &"UIButtonDanger",
}
## The same, chosen from the keyboard: lit as under the mouse, with a white frame plain to see on
## every intent's colour (ui_theme.tres)
const CHOSEN_VARIATIONS: Dictionary[Intent, StringName] = {
    Intent.DEFAULT: &"UIButtonDefaultChosen",
    Intent.SUCCESS: &"UIButtonSuccessChosen",
    Intent.WARNING: &"UIButtonWarningChosen",
    Intent.DANGER: &"UIButtonDangerChosen",
}

@export var intent: Intent = Intent.DEFAULT

var _chosen: bool = false


func _ready() -> void:
    focus_mode = Control.FOCUS_NONE
    theme = UI_THEME
    _apply_look()


## Chosen from the keyboard: the background lights up as the mouse lights it, and a white frame
## goes round it; the content margins are the style's own, so nothing moves
func set_chosen(p_chosen: bool) -> void:
    _chosen = p_chosen
    _apply_look()


func _apply_look() -> void:
    theme_type_variation = CHOSEN_VARIATIONS[intent] if _chosen else INTENT_VARIATIONS[intent]
