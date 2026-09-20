extends RefCounted
class_name LegacyCabinCabActivation

## Cab activation of a cabin whose MMD has no cabactivation_sw gauge (dynamic/pkp/e186_v2). The
## original cab layer activates the cab regardless of the gauge -
## TTrain::OnCommand_cabactivationtoggle/enable/disable (Train.cpp:3077-3140) only update
## ggCabActivationButton, which does nothing when the cab has none - so the cab stays switchable
## from the keyboard and CabinSystem.

const CONTROL:StringName = &"cabactivation_sw"
const ACTION:StringName = &"cab_activation_toggle"

var _train_id:String
var _cab:int


func register(train_id:String, cab:int) -> void:
    _train_id = train_id
    _cab = cab
    CabinSystem.register_control(train_id, cab, CONTROL, _cab_activation)


func unregister() -> void:
    CabinSystem.unregister_control(_train_id, _cab, CONTROL, _cab_activation)


func _cab_activation(state:CabinState, action:StringName, value:Variant) -> Variant:
    if not action == &"toggle" and not action == &"set":
        return null
    # Train.cpp:3083 - toggling activates the cab when none is active
    var enabled:bool = state.vehicle_state().get("cabin", 0) == 0 if value == null else bool(value)
    state.set_value(CONTROL, enabled)
    return state.send_vehicle_command("cab_activation", enabled)
