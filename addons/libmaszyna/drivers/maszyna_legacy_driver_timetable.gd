@tool
extends RefCounted
class_name MaszynaLegacyDriverTimetable

## How far the driver has got through its timetable (TTrainParameters' StationIndex,
## NextStationName, TTVmax, LastStationLatency, mtable.cpp:36-190, 632-640) and how it treats the
## passenger stops on the way (moveStopPoint, moveStopCloser, Driver.h:58-59). The Timetable itself
## is data only. `station_index` counts the timetable's entries from 0, the original's from 1.
##
## Dropped: the original's TTVmax of 100 without a timetable (mtable.cpp:224, marked for removal
## there) - no timetable is no limit here; IsMaintenance(), whose flag the original reads from the
## wrong token and never sets (mtable.cpp:486).

const MINUTES_PER_HOUR:float = 60.0
## CompareTime() (utilities.cpp:50): a difference over half a day is the other way round the clock
const HALF_DAY_MINUTES:float = 720.0
const DAY_MINUTES:float = 1440.0

var timetable:Timetable = null
## The entry it drives to next
var station_index:int = 0
## The station whose stop it looks for (asNextStop), empty past the last - it changes on leaving
## a station, not on arriving
var next_stop:String = ""
## The timetable's speed on the way to the next station [km/h], -1 for none (TTVmax)
var velocity:float = MaszynaLegacyDriverSpeed.NO_LIMIT
## Late on leaving the last station [min], early when negative (LastStationLatency)
var latency:float = 0.0
## It stops at passenger stops (moveStopPoint) - not while it couples up or turns there
var stop_point:bool = true
## It draws up close to the next passenger stop (moveStopCloser) - not at the start, nor on the far
## side after turning
var stop_closer:bool = false
## The station after the one arrived at (NextStationName)
var _next_station:String = ""


## A new timetable, the first station next (`Timetable:`, Driver.cpp:4526-4535)
func take(new_timetable:Timetable) -> void:
    timetable = new_timetable
    station_index = 0
    latency = 0.0
    next_stop = ""
    _next_station = ""
    velocity = MaszynaLegacyDriverSpeed.NO_LIMIT
    if not timetable:
        return
    # a number instead of a timetable is the speed (mtable.cpp:279)
    if timetable.velocity > 0.0:
        velocity = timetable.velocity
    if timetable.entries:
        var first:TimetableEntry = timetable.entries[0]
        next_stop = first.station_name
        _next_station = next_stop
        velocity = first.velocity


## The stations
func get_entries() -> Array:
    return timetable.entries if timetable else []


## Whether it stops at the next station (IsStop()) - past the last one it always does
func is_stop() -> bool:
    var entries:Array = get_entries()
    return entries[station_index].is_stop() if station_index < entries.size() else true


## Whether the next station is the last (IsLastStop(), "StationIndex < StationCount")
func is_last_station() -> bool:
    return station_index >= get_entries().size() - 1


## Arrived at the next station at `hours` (UpdateMTable()): the delay counted and the station after
## it named; true once, on the first arrival
func arrive(hours:float) -> bool:
    var entries:Array = get_entries()
    if station_index >= entries.size() or not next_stop == _next_station:
        return false
    var entry:TimetableEntry = entries[station_index]
    latency = _compare_time(hours, entry.departure)
    if station_index < entries.size() - 1:
        var following:TimetableEntry = entries[station_index + 1]
        _next_station = following.station_name
        velocity = following.velocity
    else:
        _next_station = ""
    return true


## On to the next station (StationIndexInc(), NextStop())
func advance() -> void:
    station_index += 1
    next_stop = _next_station


## Ended: no timetable any more (NewName("none"))
func finish() -> void:
    take(null)


## It drives past the passenger stops - coupling up, or a locomotive turning at one
func pass_stops() -> void:
    stop_point = false


## It stops at the passenger stops again - as a train
func mind_stops() -> void:
    stop_point = true


## It draws up close to the next passenger stop - once it has left one
func draw_up_close() -> void:
    stop_closer = true


## It stops where it sees the next passenger stop - the far side after turning, the end
func stop_short() -> void:
    stop_closer = false


## Whether it may leave the station it stands at, at `hours` (IsTimeToGo()): at once where it only
## passes, at the departure time where it stops, never from the last
func is_time_to_go(hours:float) -> bool:
    if is_last_station():
        return false
    var entry:TimetableEntry = get_entries()[station_index]
    if not entry.is_stop():
        return true
    return _compare_time(hours, entry.departure) <= 0.0


## Whether the train turns at the station it stands at (DirectionChange(), `@`) - not at the last
func turns_here() -> bool:
    return not is_last_station() and get_entries()[station_index].facilities.contains("@")


## A passenger stop of another station than the next one: the timetable goes on from it
## (RewindTimeTable()); false when the timetable has no such station
func rewind(station:String) -> bool:
    var entries:Array = get_entries()
    for index:int in entries.size():
        var entry:TimetableEntry = entries[index]
        if entry.station_name.to_lower() == station.to_lower():
            station_index = index
            next_stop = entry.station_name
            _next_station = next_stop
            velocity = entry.velocity
            return true
    return false


## CompareTime() (utilities.cpp:50): `to` less `from` [min], the shorter way round the clock; 0 when
## `to` is not given
static func _compare_time(from:float, to:float) -> float:
    if to < 0.0:
        return 0.0
    var minutes:float = (to - from) * MINUTES_PER_HOUR
    if minutes < -HALF_DAY_MINUTES:
        minutes += DAY_MINUTES
    if minutes > HALF_DAY_MINUTES:
        minutes -= DAY_MINUTES
    return minutes
