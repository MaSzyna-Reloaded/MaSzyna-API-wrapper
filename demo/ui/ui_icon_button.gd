class_name UIIconButton
extends Button

## A small button that is only its icon (a settings row's reset), in the game's colours: lit under
## the mouse, faint while it cannot be pressed. It takes its look from ui_theme.tres itself and no
## Godot focus - the keyboard has the row's own ways.

const UI_THEME: Theme = preload("ui_theme.tres")


func _ready() -> void:
    focus_mode = Control.FOCUS_NONE
    theme = UI_THEME
    theme_type_variation = &"UIIconButton"
