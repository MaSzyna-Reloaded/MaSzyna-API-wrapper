class_name UITextField
extends LineEdit

## A text the player types, as the game draws it - framed as its search field is, the frame lit
## while it is typed in: it takes its look from ui_theme.tres itself, so no theme around it has to
## provide it.

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
    theme_type_variation = &"UITextFieldCompact" if _compact else &"UITextField"
