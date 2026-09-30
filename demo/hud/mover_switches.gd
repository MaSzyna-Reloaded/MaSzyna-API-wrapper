extends HBoxContainer

## The vehicle this panel shows, handed to it by the HUD - never looked up by a path into
## somebody else's scene.
var vehicle:RID = RID():
    set(x):
        if not vehicle == x:
            vehicle = x
            _do_update()


func _do_update():
    modulate = Color.WHITE
    modulate.a = 1.0 if vehicle.is_valid() else 0.1

func _ready():
    _do_update()
