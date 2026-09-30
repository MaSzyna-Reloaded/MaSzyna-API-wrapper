extends VBoxContainer


## The vehicle this panel shows, handed to it by the HUD - never looked up by a path into
## somebody else's scene.
var vehicle:RID = RID():
    set(x):
        if not vehicle == x:
            vehicle = x
            _do_update()

## Taken once per vehicle rather than looked up per frame - a component is a live view on the
## vehicle, valid for as long as the vehicle is.
var universal_controller:RailVehicleUniversalController


func _ready() -> void:
    _do_update()

func _do_update():
    universal_controller = _rail_component(RailVehicleComponentType.COMPONENT_UNIVERSAL_CONTROLLER) as RailVehicleUniversalController


func _component(type:VehicleComponentType.Type) -> VehicleComponent:
    return VehicleServer.vehicle_component_get(vehicle, type)


func _rail_component(type:RailVehicleComponentType.Type) -> VehicleComponent:
    return RailVehicleServer.vehicle_component_get(vehicle, type)
