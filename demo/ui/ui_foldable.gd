class_name UIFoldable
extends FoldableContainer

## A group that folds away under its title - the settings' "Advanced", a vehicle card's sections -
## its title in the look of a list row (ui_theme.tres), which it takes itself. A list that walks
## its items from the keyboard chooses the title as one of them (set_selected()), and it is lit
## then as a chosen row is.

const UI_THEME: Theme = preload("ui_theme.tres")

var _selected: bool = false


func _ready() -> void:
    theme = UI_THEME
    _apply_look()


func set_selected(p_selected: bool) -> void:
    _selected = p_selected
    _apply_look()


func _apply_look() -> void:
    theme_type_variation = &"UIFoldableSelected" if _selected else &"UIFoldable"
