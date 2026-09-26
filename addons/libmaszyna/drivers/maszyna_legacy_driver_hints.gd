@tool
extends RefCounted
class_name MaszynaLegacyDriverHints

## The original's driver hints (driver_hint, TController::cue_action(), driverhints.cpp:79): the
## steps a driver takes to prepare a vehicle or to put it away, each a control of the cab and what
## the vehicle shows once the step is done. The original's AI sets the Mover directly
## (`if( AIControllFlag ) mvOccupied->BatterySwitch( true )`); here it operates the control, as a
## player does (CabinSystem.act()), and only while the vehicle does not show the step done yet - so
## a step can be cued on every update, as the original cues them.

enum Hint {
    BATTERY_ON,
    BATTERY_OFF,
    CAB_ACTIVATION,
    RADIO_ON,
    RADIO_OFF,
    OIL_PUMP_ON,
    OIL_PUMP_OFF,
    FUEL_PUMP_ON,
    FUEL_PUMP_OFF,
    CONVERTER_ON,
    CONVERTER_OFF,
    COMPRESSOR_ON,
    COMPRESSOR_OFF,
    FRONT_PANTOGRAPH_VALVE_ON,
    FRONT_PANTOGRAPH_VALVE_OFF,
    REAR_PANTOGRAPH_VALVE_ON,
    REAR_PANTOGRAPH_VALVE_OFF,
}

## A switch of the cab: its control, the state key that shows it done and the value that does
const SWITCHES:Dictionary = {
    Hint.BATTERY_ON: [&"battery_sw", "battery_enabled", true],
    Hint.BATTERY_OFF: [&"battery_sw", "battery_enabled", false],
    Hint.CAB_ACTIVATION: [&"cabactivation_sw", "cabin_controleable", true],
    Hint.RADIO_ON: [&"radio_sw", "radio_enabled", true],
    Hint.RADIO_OFF: [&"radio_sw", "radio_enabled", false],
    Hint.OIL_PUMP_ON: [&"oilpump_sw", "oil_pump_enabled", true],
    Hint.OIL_PUMP_OFF: [&"oilpump_sw", "oil_pump_enabled", false],
    Hint.FUEL_PUMP_ON: [&"fuelpump_sw", "fuel_pump_enabled", true],
    Hint.FUEL_PUMP_OFF: [&"fuelpump_sw", "fuel_pump_enabled", false],
    Hint.CONVERTER_ON: [&"converter_sw", "converter_enabled", true],
    Hint.CONVERTER_OFF: [&"converter_sw", "converter_enabled", false],
    Hint.COMPRESSOR_ON: [&"compressor_sw", "compressor_enabled", true],
    Hint.COMPRESSOR_OFF: [&"compressor_sw", "compressor_enabled", false],
    Hint.FRONT_PANTOGRAPH_VALVE_ON: [&"pantfront_sw", "current_collector/pantograph_first_active", true],
    Hint.FRONT_PANTOGRAPH_VALVE_OFF: [&"pantfront_sw", "current_collector/pantograph_first_active", false],
    Hint.REAR_PANTOGRAPH_VALVE_ON: [&"pantrear_sw", "current_collector/pantograph_second_active", true],
    Hint.REAR_PANTOGRAPH_VALVE_OFF: [&"pantrear_sw", "current_collector/pantograph_second_active", false],
}
const LINE_BREAKER_CLOSE:StringName = LegacyCabinMainSwitch.ON_BUTTON
const LINE_BREAKER_OPEN:StringName = LegacyCabinMainSwitch.OFF_BUTTON
const MASTER_CONTROLLER:StringName = &"mainctrl"
const SECOND_CONTROLLER:StringName = &"scndctrl"
const REVERSER:StringName = &"dirkey"
const TRAIN_BRAKE_RELEASE:StringName = LegacyCabinControls.BRAKE_LEVEL_DRIVE
const SECURITY_RESET:StringName = &"security_reset_bt"
const CABSIGNAL_RESET:StringName = &"shp_reset_bt"


## Operates the switch unless the vehicle shows the step done; true when it is. A vehicle without
## the device does not report its state, and has nothing to do.
static func cue(vehicle:RID, cab:int, hint:Hint) -> bool:
    var control:StringName = SWITCHES[hint][0]
    var wanted:bool = SWITCHES[hint][2]
    var shown:Variant = CabinSystem.vehicle_state_value(vehicle, SWITCHES[hint][1])
    if shown == null or bool(shown) == wanted:
        return true
    CabinSystem.act(vehicle, cab, control, &"toggle", wanted)
    return false


## linebreakerclose: main_on_bt held down on one update and let go on the next - the cab closes the
## breaker after InitialCtrlDelay of holding, or on the release (LegacyCabinMainSwitch), and a
## driver's update comes after its reaction time, longer than the delay (PrepareTime,
## Driver.cpp:158)
static func close_line_breaker(vehicle:RID, cab:int) -> void:
    if CabinSystem.get_control(vehicle, cab, LINE_BREAKER_CLOSE):
        CabinSystem.act(vehicle, cab, LINE_BREAKER_CLOSE, &"release")
        return
    if not CabinSystem.vehicle_state_value(vehicle, "main_switch_enabled", false):
        CabinSystem.act(vehicle, cab, LINE_BREAKER_CLOSE, &"hold")


static func open_line_breaker(vehicle:RID, cab:int) -> void:
    if not CabinSystem.vehicle_state_value(vehicle, "main_switch_enabled", false):
        return
    CabinSystem.act(vehicle, cab, LINE_BREAKER_OPEN, &"hold")
    CabinSystem.act(vehicle, cab, LINE_BREAKER_OPEN, &"release")


## mastercontrollersetzerospeed (ZeroSpeed(), Driver.cpp:3683): both controllers back to no power,
## a step at a time as the handle goes; the positions bound the steps. A cab with a joint
## controller has it in place of the master controller.
static func set_zero_speed(vehicle:RID, cab:int) -> void:
    for _step:int in int(CabinSystem.vehicle_state_value(vehicle, "controller_second_position", 0)):
        CabinSystem.act(vehicle, cab, SECOND_CONTROLLER, &"decrease")
    var controller:StringName = (
            MASTER_CONTROLLER if CabinSystem.has_control(vehicle, cab, MASTER_CONTROLLER)
            else LegacyCabinJointController.CONTROL)
    for _step:int in int(CabinSystem.vehicle_state_value(vehicle, "controller_main_position", 0)):
        CabinSystem.act(vehicle, cab, controller, &"decrease")


static func is_zero_speed(vehicle:RID) -> bool:
    return int(CabinSystem.vehicle_state_value(vehicle, "controller_main_position", 0)) == 0 \
            and int(CabinSystem.vehicle_state_value(vehicle, "controller_second_position", 0)) == 0


## directionforward/directionbackward/directionnone (DirectionForward(), ZeroDirection(),
## Driver.cpp:5756-5791): the reverser, relative to the cab, stepped until it stands at `direction`
## (+1, -1 or 0); a step is refused when the vehicle does not allow it, which ends the stepping
static func set_direction(vehicle:RID, cab:int, direction:int) -> void:
    var current:int = int(CabinSystem.vehicle_state_value(vehicle, "direction", 0))
    while not current == direction:
        CabinSystem.act(vehicle, cab, REVERSER, &"increase" if direction > current else &"decrease")
        var stepped:int = int(CabinSystem.vehicle_state_value(vehicle, "direction", 0))
        if stepped == current:
            return
        current = stepped


## securitysystemreset / shpsystemreset (driverhints.cpp): a press of the vigilance button, or of the
## cab signal's own when the vehicle has one, while it flashes
static func reset_security_system(vehicle:RID, cab:int, control:StringName) -> void:
    CabinSystem.act(vehicle, cab, control, &"hold")
    CabinSystem.act(vehicle, cab, control, &"release")


## trainbrakerelease: the handle to its driving position; the state does not tell that position,
## so it is cued, not checked (TODO.md)
static func release_train_brake(vehicle:RID, cab:int) -> void:
    CabinSystem.act(vehicle, cab, TRAIN_BRAKE_RELEASE, &"hold")
