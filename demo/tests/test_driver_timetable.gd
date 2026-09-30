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
## Float rounding of the hours-to-minutes conversion
const EPSILON:float = 1e-6

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


func test_latency_is_early_positive_late_negative():
    # LastStationLatency is the departure less the arrival (mtable.cpp:122)
    timetable.arrive(DEPARTURE - MINUTE)
    assert_almost_eq(timetable.latency, 1.0, EPSILON, "a minute early")
    timetable.take(timetable.timetable)
    timetable.arrive(DEPARTURE + MINUTE)
    assert_almost_eq(timetable.latency, -1.0, EPSILON, "a minute late")


func test_the_station_left_stays_shown_until_the_train_is_clear_of_it():
    # StationStart (driveruipanels.cpp:392) follows StationIndex only in UpdateNextStop()
    timetable.arrive(DEPARTURE)
    timetable.advance()
    assert_eq(timetable.station_start, 0, "still at the station it has left")
    timetable.show_next_station(DEPARTURE)
    assert_eq(timetable.station_start, 1, "clear of it")
    timetable.rewind("End")
    assert_eq(timetable.station_start, 3, "shown from the station it rewound to (Driver.cpp:1092)")


func test_the_delay_is_the_arrival_then_the_departure():
    timetable.get_entries()[0].arrival = DEPARTURE - 2 * MINUTE
    timetable.arrive(DEPARTURE + MINUTE)
    assert_almost_eq(timetable.delay, 3.0, EPSILON, "three minutes late at the arrival")
    assert_true(timetable.arrived)
    timetable.advance()
    assert_false(timetable.arrived)
    timetable.show_next_station(DEPARTURE + 2 * MINUTE)
    assert_almost_eq(timetable.delay, 2.0, EPSILON, "two minutes late at the departure")


func test_the_delay_counts_on_while_the_train_stands_past_its_departure():
    timetable.get_entries()[0].arrival = DEPARTURE - 2 * MINUTE
    timetable.arrive(DEPARTURE - 3 * MINUTE)
    assert_eq(TimetablePanel.delay_minutes(_state(), DEPARTURE - MINUTE), 0, "early, it waits for the departure")
    assert_eq(TimetablePanel.delay_minutes(_state(), DEPARTURE + 2.5 * MINUTE), 2, "two whole minutes past it")
    timetable.advance()
    assert_eq(TimetablePanel.delay_minutes(_state(), DEPARTURE + 2.5 * MINUTE), 2, "still at the station it has left")
    timetable.show_next_station(DEPARTURE + 3 * MINUTE)
    assert_eq(TimetablePanel.delay_minutes(_state(), DEPARTURE + 10 * MINUTE), 3, "on the way: as it left")


func test_the_delay_of_a_late_arrival_stays_until_the_departure():
    timetable.get_entries()[0].arrival = DEPARTURE - 5 * MINUTE
    timetable.arrive(DEPARTURE - MINUTE)
    assert_eq(TimetablePanel.delay_minutes(_state(), DEPARTURE - MINUTE), 4, "four minutes late at the arrival")


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


func test_every_step_through_it_is_announced():
    watch_signals(timetable)
    timetable.arrive(DEPARTURE)
    timetable.arrive(DEPARTURE)
    timetable.advance()
    timetable.rewind("End")
    timetable.rewind("Nowhere")
    timetable.finish()
    assert_signal_emit_count(timetable, "changed", 4, "arrival, leaving, a rewind and the end - not a repeated arrival nor an unknown station")


## The timetable's state as the driver reports it (MaszynaLegacyAIDriver._get_timetable_state())
func _state() -> Dictionary:
    return {
        "timetable": timetable.timetable,
        "station_index": timetable.station_index,
        "station_start": timetable.station_start,
        "latency": timetable.latency,
        "delay": timetable.delay,
        "arrived": timetable.arrived,
    }


func _entry(station:String, velocity:float, departure:float, facilities:String) -> TimetableEntry:
    var entry:TimetableEntry = TimetableEntry.new()
    entry.station_name = station
    entry.velocity = velocity
    entry.arrival = departure
    entry.departure = departure
    entry.facilities = facilities
    return entry
