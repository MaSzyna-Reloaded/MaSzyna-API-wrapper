extends HBoxContainer

## The vehicle this panel shows, handed to it by the HUD - never looked up by a path into
## somebody else's scene.
var vehicle:VehicleController:
    set(x):
        if not vehicle == x:
            vehicle = x
            _do_update()


var controller:VehicleController

func _do_update():
    controller = vehicle
    modulate = Color.WHITE
    modulate.a = 1.0 if controller else 0.1

func _ready():
    _do_update()
