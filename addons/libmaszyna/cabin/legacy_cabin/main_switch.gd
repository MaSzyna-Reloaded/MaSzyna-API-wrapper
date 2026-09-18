extends RefCounted
class_name LegacyCabinMainSwitch

## Line breaker (main switch) closed by holding an impulse button, ported from the original cab
## layer: TTrain::OnCommand_linebreakerclose/open (Train.cpp:2976-3070) and the line breaker block
## of TTrain::Update() (Train.cpp:6779-6827).
##
## The breaker becomes ready to close once main_on_bt was held for InitialCtrlDelay while
## MainSwitchCheck() allows it; an electric series motor vehicle then closes it on button release,
## other vehicles immediately. Releasing earlier does nothing. The vehicle-level "main_switch"
## command itself still acts immediately (AI, console, tests).

const ON_BUTTON:StringName = &"main_on_bt"
const OFF_BUTTON:StringName = &"main_off_bt"

## m_linebreakerstate: 0 = open, 1 = closed, 2 = ready to close
const OPEN:int = 0
const CLOSED:int = 1
const READY:int = 2

var _train_id:String
var _cab:int


func control_ids() -> Array[StringName]:
    return [ON_BUTTON, OFF_BUTTON]


func register(train_id:String, cab:int) -> void:
    _train_id = train_id
    _cab = cab
    CabinSystem.register_control(train_id, cab, ON_BUTTON, _on_button)
    CabinSystem.register_control(train_id, cab, OFF_BUTTON, _off_button)
    CabinSystem.register_process(train_id, cab, _process)


func unregister() -> void:
    CabinSystem.unregister_control(_train_id, _cab, ON_BUTTON, _on_button)
    CabinSystem.unregister_control(_train_id, _cab, OFF_BUTTON, _off_button)
    CabinSystem.unregister_process(_train_id, _cab, _process)


static func _pressed(action:StringName, value:Variant) -> bool:
    return action == &"hold" or (action == &"toggle" and bool(value))


# Train.cpp:3021-3070 - closing happens on release, and only once the breaker is ready.
func _on_button(state:CabinState, action:StringName, value:Variant) -> Variant:
    var pressed:bool = _pressed(action, value)
    state.set_value(ON_BUTTON, pressed)
    if pressed:
        return null
    var result:Variant = null
    if state.data.get("linebreaker_state", OPEN) == READY \
            and int(state.vehicle_state().get("engine_type", 0)) == TrainEngine.ELECTRIC_SERIES_MOTOR:
        result = state.send_vehicle_command("main_switch", true)
        state.data["linebreaker_state"] = CLOSED if result else OPEN
    state.data["relay_timer"] = 0.0
    return result


# Train.cpp:2976-2998 - opening happens on press.
func _off_button(state:CabinState, action:StringName, value:Variant) -> Variant:
    var pressed:bool = _pressed(action, value)
    state.set_value(OFF_BUTTON, pressed)
    if not pressed:
        return null
    state.data["relay_timer"] = 0.0
    if state.data.get("linebreaker_state", OPEN) == OPEN:
        return null
    var result:Variant = state.send_vehicle_command("main_switch", false)
    if result:
        state.data["linebreaker_state"] = OPEN
    return result


# Train.cpp:6779-6827
func _process(state:CabinState, delta:float) -> void:
    var vehicle:Dictionary = state.vehicle_state()
    var linebreaker_state:int = state.data.get("linebreaker_state", OPEN)
    var mains:bool = vehicle.get("main_switch_enabled", false)
    # sync with the vehicle - closed by someone else, or knocked out
    if linebreaker_state == OPEN and mains:
        linebreaker_state = CLOSED
    elif linebreaker_state == CLOSED and not mains:
        linebreaker_state = OPEN

    var relay_timer:float = state.data.get("relay_timer", 0.0)
    if state.get_value(ON_BUTTON, false):
        if vehicle.get("main_switch_closable", false):
            relay_timer += delta
    else:
        relay_timer = 0.0
    if state.get_value(OFF_BUTTON, false):
        relay_timer = 0.0

    if linebreaker_state == OPEN and relay_timer > float(vehicle.get("line_breaker_initial_delay", 0.0)):
        linebreaker_state = READY
    if linebreaker_state == READY \
            and not int(vehicle.get("engine_type", 0)) == TrainEngine.ELECTRIC_SERIES_MOTOR:
        linebreaker_state = CLOSED if state.send_vehicle_command("main_switch", true) else OPEN

    state.data["linebreaker_state"] = linebreaker_state
    state.data["relay_timer"] = relay_timer
