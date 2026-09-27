extends RefCounted
class_name CabinLogic

## The logic of a vehicle's cabs, attached to the vehicle in CabinSystem
## (vehicle_attach_cab_logic()). It registers the control handlers of one cab in CabinSystem;
## CabinSystem registers it for the occupied cab and moves it along when the crew changes cabs.
## It needs no 3D cab: whoever drives - the player through the widgets and keys, the AI - reports
## manipulations with CabinSystem.act(). The original's is LegacyCabinLogic.


## Registers the handlers of `cab` (1, 0 or -1, as CabinState)
func register(_vehicle_rid:RID, _cab:int) -> void:
    pass


func unregister() -> void:
    pass


## The player's keys of the controls no widget takes
func input(_event:InputEvent) -> void:
    pass
