class_name UISwitch
extends CheckButton

## An on/off control of the game - a switch, wherever it stands (a settings row, a form): it takes its look from ui_theme.tres itself, so no theme around it has to provide it.

const UI_THEME: Theme = preload("ui_theme.tres")


var _compact: bool = false


func _ready() -> void:
    theme = UI_THEME
    _apply_look()


## Smaller, for what stands below the main things (the settings' "Advanced")
func set_compact(p_compact: bool) -> void:
    _compact = p_compact
    _apply_look()


func _apply_look() -> void:
    theme_type_variation = &"UISwitchCompact" if _compact else &"UISwitch"
