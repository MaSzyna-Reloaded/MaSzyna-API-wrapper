class_name UISelect
extends OptionButton

## One of a few named values, as the game draws it - the button and its list: it takes its look
## from ui_theme.tres itself, and the list it opens takes it from the button.

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
    theme_type_variation = &"UISelectCompact" if _compact else &"UISelect"
