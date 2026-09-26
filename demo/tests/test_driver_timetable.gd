extends MaszynaGutTest

## MaszynaLegacyDriverTimetable: how far a driver got through its timetable (TTrainParameters'
## StationIndex, NextStationName, UpdateMTable(), IsTimeToGo(), mtable.cpp:36-190).

const FIRST_VELOCITY:float = 40.0
const SECOND_VELOCITY:float = 70.0
## 10:30, and a minute either side of it
const DEPARTURE:float = 10.5
const MINUTE:float = 1.0 / 60.0
## A time left out of the table (TimetableEntry::NO_TIME)
const NO_TIME:float = -1.0

var timetable:MaszynaLegacyDriverTimetable


func before_each():
    timetable = MaszynaLegacyDriverTimetable.new()
    var entries:Array[TimetableEntry] = [
        _entry("Start", FIRST_VELOCITY, DEPARTURE, ""),
        _entry("Through", SECOND_VELOCITY, NO_TIME, ""),
        _entry("Turn", SECOND_VELOCITY, DEPARTURE, "@"),
        _entry("End", SECOND_VELOCITY, NO_TIME, ""),
    ]
    # the one it only passes has no arrival; the last stops
    entries[3].arrival = DEPARTURE
    var data:Timetable = Timetable.new()
    data.entries = entries
    timetable.take(data)


func test_it_starts_for_the_first_station():
    assert_eq(timetable.next_stop, "Start")
    assert_eq(timetable.velocity, FIRST_VELOCITY)
    assert_true(timetable.is_stop())


func test_arriving_counts_once_and_leaving_moves_on():
    assert_true(timetable.arrive(DEPARTURE), "arrived")
    assert_false(timetable.arrive(DEPARTURE), "only once while it stands")
    assert_eq(timetable.next_stop, "Start", "it stops there until it leaves")
    assert_eq(timetable.velocity, SECOND_VELOCITY, "the speed on to the next station")
    timetable.advance()
    assert_eq(timetable.next_stop, "Through")
    assert_false(timetable.is_stop(), "it only passes the next one")


func test_it_leaves_at_the_departure_time():
    assert_false(timetable.is_time_to_go(DEPARTURE - MINUTE))
    assert_true(timetable.is_time_to_go(DEPARTURE))
    assert_true(timetable.is_time_to_go(DEPARTURE + MINUTE))


func test_midnight_is_not_twelve_hours_away():
    timetable.get_entries()[0].departure = MINUTE
    assert_false(timetable.is_time_to_go(24.0 - MINUTE), "a minute before midnight is early")


func test_it_turns_where_the_table_says_and_not_at_the_last():
    timetable.rewind("turn")
    assert_eq(timetable.next_stop, "Turn", "the name is matched regardless of case")
    assert_true(timetable.turns_here())
    timetable.arrive(DEPARTURE)
    timetable.advance()
    assert_true(timetable.is_last_station())
    assert_false(timetable.is_time_to_go(DEPARTURE), "never from the last")
    assert_false(timetable.turns_here())


func test_without_a_timetable_there_is_no_limit():
    timetable.finish()
    assert_eq(timetable.velocity, MaszynaLegacyDriverSpeed.NO_LIMIT)
    assert_eq(timetable.next_stop, "")
    assert_true(timetable.is_stop(), "past the end it always stops")


func _entry(station:String, velocity:float, departure:float, facilities:String) -> TimetableEntry:
    var entry:TimetableEntry = TimetableEntry.new()
    entry.station_name = station
    entry.velocity = velocity
    entry.arrival = departure
    entry.departure = departure
    entry.facilities = facilities
    return entry
