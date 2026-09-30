@tool
extends RefCounted
class_name MaszynaLegacyDriverLights

## The original driver's lights (TController::CheckVehicles(), control_lights(), Driver.cpp:2349-2525,
## 6389-6428; the headcode hints, driverhints.cpp:1238-1308): every vehicle of the trainset put out,
## then the headcode of the order lit on the front vehicle's leading end and the last vehicle's
## trailing end. A lamp is lit by the vehicle's `light` command on its own end, as RaLightsSet()
## writes iLights (DynObj.cpp:7261-7355).
##
## Not ported: the model's lamp inventory (iInventory) is not known, so a rear end shows its red
## markers where the original would pick plates for a model without them - see TODO.md, "Drivers".
## A vehicle without RailVehicleLighting stands for an empty inventory.

## The original's lamp bits (light::, MOVER.h:299-323) and the lamps of the `light` command they
## name; the other bits have no lamp here
const HEADLIGHT_LEFT:int = 1
const REDMARKER_LEFT:int = 2
const HEADLIGHT_UPPER:int = 4
const HEADLIGHT_RIGHT:int = 16
const REDMARKER_RIGHT:int = 32
const REAR_END_SIGNALS:int = 64
const LAMPS:Dictionary[int, String] = {
    HEADLIGHT_LEFT: "headlight_left",
    REDMARKER_LEFT: "redmarker_left",
    HEADLIGHT_UPPER: "headlight_upper",
    HEADLIGHT_RIGHT: "headlight_right",
    REDMARKER_RIGHT: "redmarker_right",
}
## Pc1, the train's head (headcodepc1, driverhints.cpp:1239)
const PC1:int = HEADLIGHT_LEFT | HEADLIGHT_RIGHT | HEADLIGHT_UPPER
## Pc5, the end of the train: red markers or plates, whichever the vehicle shows (headcodepc5,
## driverhints.cpp:1284; RaLightsSet(), DynObj.cpp:7266-7288)
const PC5:int = REDMARKER_LEFT | REDMARKER_RIGHT | REAR_END_SIGNALS
const RED_MARKERS:int = REDMARKER_LEFT | REDMARKER_RIGHT
## A vehicle with more power than this [kW] is an engine (Power > 1.0, DynObj.cpp:7272)
const POWERED:float = 1.0
## No pattern asked for (m_lighthints, Driver.h)
const NO_HINT:int = -1
## The vehicle's couplers (end::front, end::rear) and the prefix of their lamps' names
const FRONT_END:int = 0
const REAR_END:int = 1
const END_PREFIXES:PackedStringArray = ["front_", "rear_"]
## The orders that light a headcode (TOrders, Driver.h:29-45)
const SHUNTING_ORDERS:int = (MaszynaLegacyAIDriver.Order.SHUNT | MaszynaLegacyAIDriver.Order.LOOSE_SHUNT
        | MaszynaLegacyAIDriver.Order.CONNECT)


## CheckVehicles()'s lights for a driver the computer is (AIControllFlag): every vehicle put out
## (RaLightsSet(0, 0), Driver.cpp:2451), then those of the order (control())
static func check_vehicles(vehicle:RID, direction:int, order:int, hints:Vector2i) -> void:
    var vehicles:Array[RID] = _trainset(vehicle, direction)
    for other:RID in vehicles:
        set_end(other, FRONT_END, 0)
        set_end(other, REAR_END, 0)
    _control(vehicles, vehicle, direction, order, hints)


## control_lights(): Pc1 and Pc5 on a train, or the patterns the scenery asked for (SetLights); Tb1
## shunting, one white lamp at each end, diagonally (headcodetb1, driverhints.cpp:1265). Any other
## order lights nothing.
static func control(vehicle:RID, direction:int, order:int, hints:Vector2i) -> void:
    _control(_trainset(vehicle, direction), vehicle, direction, order, hints)


## The headcode of the order on the trainset's `vehicles`, from the front the way the driver drives
static func _control(vehicles:Array[RID], vehicle:RID, direction:int, order:int, hints:Vector2i) -> void:
    if order & MaszynaLegacyAIDriver.Order.OBEY_TRAIN:
        _light(vehicles, direction, PC1 if hints.x == NO_HINT else hints.x,
                _end_of_train(vehicles[-1], PC5 if hints.y == NO_HINT else hints.y))
    elif order & SHUNTING_ORDERS:
        if int(VehicleServer.vehicle_dump_state(vehicle).get("direction", 0)) >= 0:
            _light(vehicles, direction, HEADLIGHT_RIGHT, HEADLIGHT_LEFT)
        else:
            _light(vehicles, direction, HEADLIGHT_LEFT, HEADLIGHT_RIGHT)


## The lightsoff hint (Lights(0, 0), driverhints.cpp:1297): the trainset's head and tail put out
static func off(vehicle:RID, direction:int) -> void:
    _light(_trainset(vehicle, direction), direction, 0, 0)


## One end of a vehicle showing `pattern` (the original's bits): only the lamps that differ are
## switched. A vehicle without lamps lights none (head & iInventory[end], DynObj.cpp:7293).
static func set_end(vehicle:RID, end:int, pattern:int) -> void:
    var lighting:RailVehicleLighting = VehicleServer.vehicle_component_get(
            vehicle, VehicleComponentType.COMPONENT_LIGHTING) as RailVehicleLighting
    if not lighting:
        return
    for bit:int in LAMPS:
        var lamp:String = END_PREFIXES[end] + LAMPS[bit]
        var lit:bool = bool(pattern & bit)
        if not lighting.light_is_enabled(lamp) == lit:
            MaszynaLegacyDriverHints.send(vehicle, "light", lamp, lit)


## Lights(head, rear) (Driver.cpp:2561-2565): `head` on the front vehicle's leading end, `rear` on
## the last vehicle's trailing end - each the end nothing is coupled to, a lone vehicle's the way the
## driver drives
static func _light(vehicles:Array[RID], direction:int, head:int, rear:int) -> void:
    var leading:int = FRONT_END if direction >= 0 else REAR_END
    var trailing:int = 1 - leading
    if vehicles.size() > 1:
        leading = FRONT_END if _is_free(vehicles[0], FRONT_END) else REAR_END
        trailing = FRONT_END if _is_free(vehicles[-1], FRONT_END) else REAR_END
    set_end(vehicles[0], leading, head)
    set_end(vehicles[-1], trailing, rear)


## Pc5 resolved (RaLightsSet(), DynObj.cpp:7266-7288): an engine with no direction set shows
## plates - no lamp here - any other vehicle its red markers
static func _end_of_train(vehicle:RID, pattern:int) -> int:
    if not pattern == PC5:
        return pattern
    var powered:bool = float(VehicleServer.vehicle_dump_config(vehicle).get("power", 0.0)) > POWERED
    if powered and int(VehicleServer.vehicle_dump_state(vehicle).get("direction", 0)) == 0:
        return 0
    return RED_MARKERS


## The trainset from the front the way the driver drives, read afresh - after coupling or uncoupling
## it is already the new one (pVehicles[], CheckVehicles(), Driver.cpp:2358-2389)
static func _trainset(vehicle:RID, direction:int) -> Array[RID]:
    return RailVehicleServer.vehicle_get_coupled(
            vehicle, FRONT_END if direction >= 0 else REAR_END, RailVehicleController.COUPLING_ELEMENT_COUPLER)


## Nothing is coupled at the vehicle's end - the walk out through it starts at the vehicle itself
static func _is_free(vehicle:RID, end:int) -> bool:
    return RailVehicleServer.vehicle_get_coupled(vehicle, end, RailVehicleController.COUPLING_ELEMENT_COUPLER)[0] == vehicle
