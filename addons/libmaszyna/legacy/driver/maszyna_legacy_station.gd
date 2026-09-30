extends RefCounted
class_name MaszynaLegacyStation

## The passengers of a station (basic_station::update_load(), station.cpp:25-88): at a stop of its
## timetable a train's passenger cars let a random group off and take one on at the platform; the
## train's dispatch (StationServer) waits for them. Only cars with doors and passengers, or empty
## ones that take passengers, exchange anything.

const PASSENGERS:String = "passengers"
## Up to this share of a car's capacity gets off, and as much gets on (station.cpp:62-66)
const GROUP_SHARE:float = 0.15
## A station exchanges twice the group of a small stop (station.cpp:35)
const STATION_GROUP:float = 2.0
const STOP_GROUP:float = 1.0
## Twice as many get on at the first station (station.cpp:71-74)
const FIRST_STATION_BOARDING:int = 2
## A small stop has "po" at the end of its name or among its facilities, a maintenance stop "pt"
## there (station.cpp:31-33)
const SMALL_STOP_MARK:String = "po"
const MAINTENANCE_MARK:String = "pt"
## TMoverParameters::DamageFlag dtrain_out, a derailed car (MOVER.h:151)
const DERAILED:int = 128


## The exchange at the station `timetable` stands at, on the `platform` side of the way `trainset`
## drives: its cars told what to let off and take on
static func update_load(
    trainset:MaszynaLegacyDriverTrainset, timetable:MaszynaLegacyDriverTimetable, platform:RailVehicleLoad.PlatformSide
) -> void:
    var entries:Array = timetable.get_entries()
    var index:int = timetable.station_index
    if index >= entries.size():
        return
    var entry:TimetableEntry = entries[index]
    var first_stop:bool = index == 0
    var last_stop:bool = index == entries.size() - 1
    var small_stop:bool = entry.station_name.ends_with(SMALL_STOP_MARK) or entry.facilities.contains(SMALL_STOP_MARK)
    var maintenance:bool = entry.facilities.contains(MAINTENANCE_MARK)
    var group:float = GROUP_SHARE * (STOP_GROUP if small_stop else STATION_GROUP)
    for vehicle_index:int in trainset.vehicles.size():
        var vehicle:RID = trainset.vehicles[vehicle_index]
        var load:RailVehicleLoad = VehicleServer.vehicle_component_get(vehicle, VehicleComponentType.COMPONENT_LOAD)
        if not load or not VehicleServer.vehicle_component_get(vehicle, VehicleComponentType.COMPONENT_DOORS):
            continue
        var load_name:String = load.get_load_name()
        if not (load_name == PASSENGERS or (not load_name and load.accepted_loads.has(PASSENGERS))):
            continue
        var amount:float = load.get_load_amount()
        var derailed:bool = ((VehicleServer.vehicle_get_controller(vehicle) as RailVehicleController)
                .get_train_damage() & DERAILED) > 0
        var getting_off:int = 0
        if derailed or last_stop:
            getting_off = int(amount)
        elif not (first_stop or maintenance):
            getting_off = int(minf(amount, randf_range(0.0, load.max_load * group)))
        var getting_on:int = 0
        if not (derailed or last_stop or maintenance):
            getting_on = int(randf_range(0.0, load.max_load * group))
            if first_stop:
                getting_on *= FIRST_STATION_BOARDING
        if getting_off == 0 and getting_on == 0:
            continue
        # the platform is on the train's side; a car the other way round has it on its other side
        # (TDynamicObject::LoadExchangeSpeed(), DynObj.cpp:2859-2860)
        var side:RailVehicleLoad.PlatformSide = platform if trainset.directions[vehicle_index] > 0 \
                else opposite_side(platform)
        if getting_off > 0:
            RailVehicleServer.load_remove(vehicle, getting_off, side)
        if getting_on > 0:
            RailVehicleServer.load_add(vehicle, getting_on, side, PASSENGERS)


## The platform seen from a car the other way round
static func opposite_side(side:RailVehicleLoad.PlatformSide) -> RailVehicleLoad.PlatformSide:
    match side:
        RailVehicleLoad.PLATFORM_SIDE_LEFT:
            return RailVehicleLoad.PLATFORM_SIDE_RIGHT
        RailVehicleLoad.PLATFORM_SIDE_RIGHT:
            return RailVehicleLoad.PLATFORM_SIDE_LEFT
    return side
