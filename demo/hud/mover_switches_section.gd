extends HFlowContainer


## The vehicle this panel shows, handed to it by the HUD - never looked up by a path into
## somebody else's scene.
var vehicle:VehicleController:
    set(x):
        if not vehicle == x:
            vehicle = x
            _do_update()

var controller:VehicleController
## Taken once per vehicle rather than looked up per frame - a component is a live view on the
## vehicle, valid for as long as the vehicle is.
var universal_controller:VehicleUniversalController


func _ready() -> void:
    _do_update()

func _do_update():
    controller = vehicle
    universal_controller = (
            controller.get_component(VehicleComponentType.COMPONENT_UNIVERSAL_CONTROLLER)
            as VehicleUniversalController) if controller else null


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    pass
