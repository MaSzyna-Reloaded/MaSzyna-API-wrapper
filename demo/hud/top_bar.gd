extends PanelContainer
class_name TopBar

## The game's menus as a brow in the middle of the top edge (MenuBrow), clear of the HUD's tiles
## in the corners. It waits above the screen and slides in when the mouse touches the top edge,
## anywhere along it, and back out once the mouse is below it and none of its menus is open - also
## right after an entry was chosen. It slides rather than hides: a hidden MenuBar would stop
## handling the keys of its entries.

## Pixels from the top edge of the screen that bring the bar in
const REVEAL_EDGE: float = 2.0
## Seconds of the slide in or out
const SLIDE_TIME: float = 0.15

var _shown: bool = false
## Menus of the bar open now - a menu opened from another one may come before that one closes
var _open_menus: int = 0
var _slide_tween: Tween = null


func _ready() -> void:
    position.y = -get_combined_minimum_size().y


func _input(event: InputEvent) -> void:
    var motion: InputEventMouseMotion = event as InputEventMouseMotion
    if not motion:
        return
    if motion.global_position.y <= REVEAL_EDGE:
        _slide(true)
    # an open menu is a popup window of its own, and the mouse on it is below the bar
    elif motion.global_position.y > size.y and not _open_menus:
        _slide(false)


func _on_menu_about_to_popup() -> void:
    _open_menus += 1


## A menu closed - an entry chosen, or the menu left: the bar goes unless the mouse is on it
func _on_menu_popup_hide() -> void:
    _open_menus -= 1
    if not _open_menus and get_global_mouse_position().y > size.y:
        _slide(false)


func _slide(shown: bool) -> void:
    if shown == _shown:
        return
    _shown = shown
    if _slide_tween:
        _slide_tween.kill()
    _slide_tween = create_tween()
    _slide_tween.tween_property(self, "position:y", 0.0 if shown else -size.y, SLIDE_TIME)
