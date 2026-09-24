extends RefCounted
class_name LegacyCabinBattery

## Battery switch of a cabin whose MMD has no battery_sw gauge. The original cab layer switches
## the battery regardless of the gauge - TTrain::OnCommand_batterytoggle/enable/disable
## (Train.cpp:2359-2420) only update ggBatteryButton, which does nothing when the cab has none -
## so the battery stays switchable from the keyboard and CabinSystem.

const CONTROL:StringName = &"battery_sw"
const ACTION:StringName = &"battery_toggle"

var _train_id:String
var _cab:int


func register(train_id:String, cab:int) -> void:
    _train_id = train_id
    _cab = cab
    CabinSystem.register_control(train_id, cab, CONTROL, _battery)


func unregister() -> void:
    CabinSystem.unregister_control(_train_id, _cab, CONTROL, _battery)


func _battery(state:CabinState, action:StringName, value:Variant) -> Variant:
    if not action == &"toggle" and not action == &"set":
        return null
    # Train.cpp:2363 - toggling turns the battery on when the 24V circuit is not powered
    var enabled:bool = not state.vehicle_state_value("power24_available", false) if value == null else bool(value)
    state.set_value(CONTROL, enabled)
    return state.send_vehicle_command("battery", enabled)
