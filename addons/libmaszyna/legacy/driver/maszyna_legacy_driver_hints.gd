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
## The switch of a hint and the position it wants; what the vehicle shows of it is read in cue()
const SWITCHES:Dictionary = {
    Hint.BATTERY_ON: [&"battery_sw", true],
    Hint.BATTERY_OFF: [&"battery_sw", false],
    Hint.CAB_ACTIVATION: [&"cabactivation_sw", true],
    Hint.RADIO_ON: [&"radio_sw", true],
    Hint.RADIO_OFF: [&"radio_sw", false],
    Hint.OIL_PUMP_ON: [&"oilpump_sw", true],
    Hint.OIL_PUMP_OFF: [&"oilpump_sw", false],
    Hint.FUEL_PUMP_ON: [&"fuelpump_sw", true],
    Hint.FUEL_PUMP_OFF: [&"fuelpump_sw", false],
    Hint.CONVERTER_ON: [&"converter_sw", true],
    Hint.CONVERTER_OFF: [&"converter_sw", false],
    Hint.COMPRESSOR_ON: [&"compressor_sw", true],
    Hint.COMPRESSOR_OFF: [&"compressor_sw", false],
}
## The pantographs' valves are operated directly, as the original's driver does
## (`mvOccupied->OperatePantographValve(end::front, operation_t::enable)`, driverhints.cpp:269-313):
## through the cab they could not be - a cab with a pantograph selector (pantselect_sw: E186,
## ES64F4) has no pantfront_sw/pantrear_sw, and its selector takes the valves over (Train.cpp:3154),
## so its driver never raised them (docs/findings-archive.md, 2026-10-03 scenarios that did not
## start). The valve and the operation of a hint; what the vehicle shows of it is read in cue()
const PANTOGRAPH_VALVES:Dictionary = {
    Hint.FRONT_PANTOGRAPH_VALVE_ON: [RailVehicleEnginePowerSource.PANTOGRAPH_FIRST, RailVehicleEnginePowerSource.VALVE_OPERATION_ENABLE, true],
    Hint.FRONT_PANTOGRAPH_VALVE_OFF: [RailVehicleEnginePowerSource.PANTOGRAPH_FIRST, RailVehicleEnginePowerSource.VALVE_OPERATION_DISABLE, false],
    Hint.REAR_PANTOGRAPH_VALVE_ON: [RailVehicleEnginePowerSource.PANTOGRAPH_SECOND, RailVehicleEnginePowerSource.VALVE_OPERATION_ENABLE, true],
    Hint.REAR_PANTOGRAPH_VALVE_OFF: [RailVehicleEnginePowerSource.PANTOGRAPH_SECOND, RailVehicleEnginePowerSource.VALVE_OPERATION_DISABLE, false],
}
const LINE_BREAKER_CLOSE:StringName = LegacyCabinMainSwitch.ON_BUTTON
const LINE_BREAKER_OPEN:StringName = LegacyCabinMainSwitch.OFF_BUTTON
const MASTER_CONTROLLER:StringName = &"mainctrl"
const SECOND_CONTROLLER:StringName = &"scndctrl"
const REVERSER:StringName = &"dirkey"
const TRAIN_BRAKE_RELEASE:StringName = LegacyCabinControls.BRAKE_LEVEL_DRIVE
const SECURITY_RESET:StringName = &"security_reset_bt"
const CABSIGNAL_RESET:StringName = &"shp_reset_bt"


## A vehicle command the driver gives, sent only when the vehicle has it; null when it has not.
## The original's driver calls the Mover, where a device the vehicle lacks does nothing
## (Sandbox() switches nothing on without sand, Mover.cpp:3070); here the component that registers
## the command is missing (an EN96 has no RailVehicleSwitches, so no "sand").
static func send(vehicle:RID, command:StringName, p1:Variant = null, p2:Variant = null) -> Variant:
    if not VehicleServer.vehicle_has_command(vehicle, command):
        return null
    return VehicleServer.vehicle_send_command(vehicle, command, p1, p2)


## Operates the switch unless the vehicle shows the step done; true when it is. A vehicle without
## the device does not report its state, and has nothing to do. `shown_by`: the vehicle whose
## device the switch works, when it is another one of the unit - an EMU's pantographs are its
## motor car's (mvPantographUnit)
static func cue(vehicle:RID, cab:int, hint:Hint, shown_by:RID = RID()) -> bool:
    var valve:Array = PANTOGRAPH_VALVES.get(hint, [])
    var wanted:bool = valve[2] if valve else SWITCHES[hint][1]
    var device:RID = shown_by if shown_by.is_valid() else vehicle
    var power_supply:RailVehiclePowerSupply = RailVehicleServer.vehicle_component_get(
            device, RailVehicleComponentType.COMPONENT_POWER_SUPPLY) as RailVehiclePowerSupply
    var master:RailVehicleMasterController = RailVehicleServer.vehicle_component_get(
            device, RailVehicleComponentType.COMPONENT_MASTER_CONTROLLER) as RailVehicleMasterController
    var engine:RailVehicleEngine = VehicleServer.vehicle_component_get(
            device, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleEngine
    var diesel:RailVehicleDieselEngine = engine as RailVehicleDieselEngine
    var power_source:RailVehicleEnginePowerSource = RailVehicleServer.vehicle_component_get(
            device, RailVehicleComponentType.COMPONENT_ENGINE_POWER_SOURCE) as RailVehicleEnginePowerSource
    var radio:RailVehicleRadio = VehicleServer.vehicle_component_get(
            device, VehicleComponentType.COMPONENT_RADIO) as RailVehicleRadio
    var brake:RailVehicleBrake = RailVehicleServer.vehicle_component_get(
            device, RailVehicleComponentType.COMPONENT_BRAKES) as RailVehicleBrake
    var shown:Variant = null
    match hint:
        # on once the low voltage is there - a car without a battery of its own takes it from the
        # unit's (batteryon's check, driverhints.cpp:88-91)
        Hint.BATTERY_ON: shown = power_supply.get_power24_available() if power_supply else null
        Hint.BATTERY_OFF: shown = power_supply.get_battery_enabled() if power_supply else null
        Hint.CAB_ACTIVATION: shown = master.get_cabin_controleable() if master else null
        Hint.CONVERTER_ON, Hint.CONVERTER_OFF: shown = power_supply.get_converter_enabled() if power_supply else null
        Hint.RADIO_ON, Hint.RADIO_OFF: shown = radio.get_enabled() if radio else null
        Hint.OIL_PUMP_ON, Hint.OIL_PUMP_OFF: shown = diesel.get_oil_pump_enabled() if diesel else null
        Hint.FUEL_PUMP_ON, Hint.FUEL_PUMP_OFF: shown = diesel.get_fuel_pump_enabled() if diesel else null
        Hint.COMPRESSOR_ON, Hint.COMPRESSOR_OFF: shown = brake.get_compressor_enabled() if brake else null
        Hint.FRONT_PANTOGRAPH_VALVE_ON, Hint.FRONT_PANTOGRAPH_VALVE_OFF:
            shown = power_source.get_collector_pantograph_first_active() if power_source else null
        Hint.REAR_PANTOGRAPH_VALVE_ON, Hint.REAR_PANTOGRAPH_VALVE_OFF:
            shown = power_source.get_collector_pantograph_second_active() if power_source else null
    if shown == null or bool(shown) == wanted:
        return true
    if valve:
        send(device, &"pantograph_valve_operate", valve[0], valve[1])
        return false
    CabinSystem.act(vehicle, cab, SWITCHES[hint][0], &"toggle", wanted)
    return false


## linebreakerclose: main_on_bt held down on one update and let go on the next - the cab closes the
## breaker after InitialCtrlDelay of holding, or on the release (LegacyCabinMainSwitch), and a
## driver's update comes after its reaction time, longer than the delay (PrepareTime,
## Driver.cpp:158)
static func close_line_breaker(vehicle:RID, cab:int) -> void:
    if CabinSystem.get_control(vehicle, cab, LINE_BREAKER_CLOSE):
        CabinSystem.act(vehicle, cab, LINE_BREAKER_CLOSE, &"release")
        return
    if not _main_switch_enabled(vehicle):
        CabinSystem.act(vehicle, cab, LINE_BREAKER_CLOSE, &"hold")


static func open_line_breaker(vehicle:RID, cab:int) -> void:
    if not _main_switch_enabled(vehicle):
        return
    CabinSystem.act(vehicle, cab, LINE_BREAKER_OPEN, &"hold")
    CabinSystem.act(vehicle, cab, LINE_BREAKER_OPEN, &"release")


## mastercontrollersetzerospeed (ZeroSpeed(), Driver.cpp:3683): both controllers back to no power,
## a step at a time as the handle goes; the positions bound the steps. The master controller stops
## at its no-power position (DecMainCtrl(MainCtrlPowerPos()), Driver.cpp:3712): below it a universal
## controller brakes (SM42 6Dg, UCList with IntegratedLocBrake).
static func set_zero_speed(vehicle:RID, cab:int) -> void:
    # the controllers are the driven engine's (mvControlling)
    var controlled:RailVehicleMasterController = RailVehicleServer.vehicle_component_get(
            RailVehicleServer.vehicle_find_powered(vehicle), RailVehicleComponentType.COMPONENT_MASTER_CONTROLLER
    ) as RailVehicleMasterController
    if controlled == null:
        return
    for _step:int in controlled.get_second_position():
        CabinSystem.act(vehicle, cab, SECOND_CONTROLLER, &"decrease")
    var controller:StringName = master_controller(vehicle, cab)
    for _step:int in controlled.get_main_position() - controlled.get_main_no_power_position():
        CabinSystem.act(vehicle, cab, controller, &"decrease")


## mastercontrollersetidle (driverhints.cpp:489-501): a diesel's master controller up to its first
## position with the clutch in (RList[].Mn), so that it does not stall - SN61's idle
static func set_idle(vehicle:RID, cab:int) -> void:
    var controlled:RID = RailVehicleServer.vehicle_find_powered(vehicle)
    var engine:RailVehicleDieselEngine = VehicleServer.vehicle_component_get(
            controlled, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleDieselEngine
    var controlled_controller:RailVehicleMasterController = RailVehicleServer.vehicle_component_get(
            controlled, RailVehicleComponentType.COMPONENT_MASTER_CONTROLLER) as RailVehicleMasterController
    if engine == null or controlled_controller == null:
        return
    var positions:Array = engine.throttle_table_positions
    var controller:StringName = master_controller(vehicle, cab)
    var position:int = controlled_controller.get_main_position()
    while position < positions.size() and (positions[position] as RailVehicleThrottlePositionItem).clutch_behavior == 0:
        CabinSystem.act(vehicle, cab, controller, &"increase")
        var stepped:int = controlled_controller.get_main_position()
        if stepped == position:
            return
        position = stepped


## The cab's master controller - a joint controller where the cab has one in its place (SM42's)
static func master_controller(vehicle:RID, cab:int) -> StringName:
    return (MASTER_CONTROLLER if CabinSystem.has_control(vehicle, cab, MASTER_CONTROLLER)
            else LegacyCabinJointController.CONTROL)


static func is_zero_speed(vehicle:RID) -> bool:
    var controlled:RailVehicleMasterController = RailVehicleServer.vehicle_component_get(
            RailVehicleServer.vehicle_find_powered(vehicle), RailVehicleComponentType.COMPONENT_MASTER_CONTROLLER
    ) as RailVehicleMasterController
    return controlled == null or (controlled.get_main_position() == 0 and controlled.get_second_position() == 0)


## directionforward/directionbackward/directionnone (DirectionForward(), ZeroDirection(),
## Driver.cpp:5756-5791): the reverser, relative to the cab, stepped until it stands at `direction`
## (+1, -1 or 0); a step is refused when the vehicle does not allow it, which ends the stepping
static func set_direction(vehicle:RID, cab:int, direction:int) -> void:
    var controller:VehicleController = VehicleServer.vehicle_get_controller(vehicle)
    var current:int = controller.get_direction()
    while not current == direction:
        CabinSystem.act(vehicle, cab, REVERSER, &"increase" if direction > current else &"decrease")
        var stepped:int = controller.get_direction()
        if stepped == current:
            return
        current = stepped


## The line breaker of the vehicle's engine closed; a vehicle without an engine has none
static func _main_switch_enabled(vehicle:RID) -> bool:
    var engine:RailVehicleEngine = VehicleServer.vehicle_component_get(
            vehicle, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleEngine
    return engine != null and engine.get_main_switch_enabled()


## securitysystemreset / shpsystemreset (driverhints.cpp): a press of the vigilance button, or of the
## cab signal's own when the vehicle has one, while it flashes
static func reset_security_system(vehicle:RID, cab:int, control:StringName) -> void:
    CabinSystem.act(vehicle, cab, control, &"hold")
    CabinSystem.act(vehicle, cab, control, &"release")


## trainbrakerelease: the handle to its driving position; the state does not tell that position,
## so it is cued, not checked (TODO.md)
static func release_train_brake(vehicle:RID, cab:int) -> void:
    CabinSystem.act(vehicle, cab, TRAIN_BRAKE_RELEASE, &"hold")
