@tool
extends RefCounted
class_name MaszynaLegacyDriverPantographs

## The original driver's pantographs (the pantograph part of TController::PrepareEngine(),
## control_pantographs(), Driver.cpp:2782-2811, 6219-6345): the air for them from the small
## compressor until the main reservoir takes over, both raised to start, and on the move the one at
## the rear the way it drives - or the vehicle's suggested setup - the front one lowered once the
## rear one carries the current. The valves go through the cab (MaszynaLegacyDriverHints); the
## compressor and its three-way valve are the pantograph unit's, set by the crew.
##
## Not ported: running without current or with the pantographs down where the track says so
## (fOverhead2, iOverheadDown), and the front one raised too when standing long at a stop
## (AIHintPantUpIfIdle, IdleTime) - see TODO.md, "Drivers".

## The pantographs rise from this tank pressure [bar], an EMU's from EMU_RAISING_PRESSURE, with
## RAISING_MARGIN to spare (Driver.cpp:2786-2790)
const RAISING_PRESSURE:float = 3.5
const EMU_RAISING_PRESSURE:float = 2.5
const RAISING_MARGIN:float = 0.1
## The main reservoir feeds the pantographs once it holds this [bar] (Driver.cpp:6226)
const MAIN_FEEDING_PRESSURE:float = 4.3
## Moving faster than this [km/h] the front pantograph comes down and the suggested setup is used
## (Driver.cpp:6280, 6311)
const SETUP_SPEED:float = 5.0


## PrepareEngine()'s pantographs: the small compressor while the tank is short of air, then off;
## both raised
static func prepare(vehicle:RID, cab:int, trainset:MaszynaLegacyDriverTrainset, emu:bool) -> void:
    var unit:RID = trainset.pantograph_unit
    if not unit.is_valid():
        return
    var engine:RailVehicleElectricEngine = VehicleServer.vehicle_component_get(
            unit, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleElectricEngine
    var brake:RailVehicleBrake = RailVehicleServer.vehicle_component_get(
            unit, RailVehicleComponentType.COMPONENT_BRAKES) as RailVehicleBrake
    var tank:float = engine.get_collector_pantograph_tank_pressure()
    var feeding_from_compressor:bool = engine.get_collector_pantograph_compressor_valve()
    if tank < (EMU_RAISING_PRESSURE if emu else RAISING_PRESSURE) + RAISING_MARGIN:
        # the main reservoir cannot fill it: the three-way valve to the small compressor
        if not engine.cntrl_pantograph_auto_valve and not feeding_from_compressor:
            MaszynaLegacyDriverHints.send(unit, "pantograph_compressor_valve", true)
        if not engine.get_collector_pantograph_compressor_enabled():
            MaszynaLegacyDriverHints.send(unit, "pantograph_compressor", true)
    elif not feeding_from_compressor or tank <= (brake.get_compressor_pressure() if brake else 0.0):
        if engine.get_collector_pantograph_compressor_enabled():
            MaszynaLegacyDriverHints.send(unit, "pantograph_compressor", false)
    # pantographsvalveon: the pantographs' master valve (OperatePantographsValve(), no cab control)
    if not engine.get_collector_valve_active():
        MaszynaLegacyDriverHints.send(unit, "pantographs_valve", true)
    MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.FRONT_PANTOGRAPH_VALVE_ON, unit)
    MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.REAR_PANTOGRAPH_VALVE_ON, unit)


## control_pantographs() (Driver.cpp:6219-6345), on every update the driver acts on: the main
## reservoir to the pantographs once it holds enough; on the move the rear one up, the front one
## down when both carry the current, or the vehicle's suggested setup
static func control(
    vehicle:RID, cab:int, trainset:MaszynaLegacyDriverTrainset, direction:int, emu:bool, waiting:bool
) -> void:
    var unit:RID = trainset.pantograph_unit
    if not unit.is_valid():
        return
    var engine:RailVehicleElectricEngine = VehicleServer.vehicle_component_get(
            unit, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleElectricEngine
    var brake:RailVehicleBrake = RailVehicleServer.vehicle_component_get(
            vehicle, RailVehicleComponentType.COMPONENT_BRAKES) as RailVehicleBrake
    if not engine.cntrl_pantograph_auto_valve \
            and (brake.get_compressor_pressure() if brake else 0.0) > MAIN_FEEDING_PRESSURE \
            and engine.get_collector_pantograph_compressor_valve():
        MaszynaLegacyDriverHints.send(unit, "pantograph_compressor_valve", false)
    var speed:float = VehicleServer.vehicle_get_speed(vehicle)
    if speed <= MaszynaLegacyDriverTrainset.NO_MOVEMENT_SPEED or waiting:
        return
    var hints:RailVehicleAIHints = RailVehicleServer.vehicle_component_get(
            vehicle, RailVehicleComponentType.COMPONENT_AI_HINTS) as RailVehicleAIHints
    var setup:RailVehicleAIHints.PantographState = hints.pantograph_state if hints else RailVehicleAIHints.PANTOGRAPH_STATE_AUTOMATIC
    if not setup == RailVehicleAIHints.PANTOGRAPH_STATE_AUTOMATIC:
        if speed > SETUP_SPEED:
            MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.FRONT_PANTOGRAPH_VALVE_ON
                    if setup & RailVehicleAIHints.PANTOGRAPH_STATE_FRONT else MaszynaLegacyDriverHints.Hint.FRONT_PANTOGRAPH_VALVE_OFF, unit)
            MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.REAR_PANTOGRAPH_VALVE_ON
                    if setup & RailVehicleAIHints.PANTOGRAPH_STATE_REAR else MaszynaLegacyDriverHints.Hint.REAR_PANTOGRAPH_VALVE_OFF, unit)
        return
    # the regular layout: a lone vehicle, an EMU, an ET41 (Driver.cpp:6243-6246)
    var train_type:RailVehicleController.TrainType = (
            VehicleServer.vehicle_get_controller(unit) as RailVehicleController).train_type
    var regular:bool = RailVehicleServer.vehicle_get_coupled(
            vehicle, RailVehicleController.COUPLER_END_FRONT, RailVehicleController.COUPLING_FLAG_CONTROL).size() == 1 \
            or emu or train_type == RailVehicleController.TRAIN_TYPE_ET41
    var collectors:int = engine.power_current_collector_number_of_collectors
    var voltage:float = engine.get_collector_voltage()
    var front_voltage:float = engine.get_collector_pantograph_first_voltage()
    var rear_voltage:float = engine.get_collector_pantograph_second_voltage()
    var on_rear:bool = direction >= 0 and regular
    # the one at the rear up, unless another one works and it is the only one
    var raised_voltage:float = rear_voltage if on_rear else front_voltage
    if raised_voltage == 0.0 and (voltage == 0.0 or collectors > 1):
        MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.REAR_PANTOGRAPH_VALVE_ON
                if on_rear else MaszynaLegacyDriverHints.Hint.FRONT_PANTOGRAPH_VALVE_ON, unit)
    # having gathered speed, the other one down once the first carries the current
    if speed > SETUP_SPEED and collectors > 1 and not front_voltage == 0.0 and not rear_voltage == 0.0:
        MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.FRONT_PANTOGRAPH_VALVE_OFF
                if on_rear else MaszynaLegacyDriverHints.Hint.REAR_PANTOGRAPH_VALVE_OFF, unit)
