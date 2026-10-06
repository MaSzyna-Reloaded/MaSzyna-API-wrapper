class_name TimetablePanel
extends PanelContainer

## The timetable of the player's train: the train, its relation, the clock and the delay, and every
## station with the times it is due - the original's timetable panel (timetable_panel::update(),
## driveruipanels.cpp:300-472; F2, driveruilayer.cpp:155) laid out anew. It shows the timetable of
## the trainset the player's vehicle belongs to: the scenario gives it to one driver of the
## trainset only, not necessarily the one of the vehicle the player sits in.

## The close button asks the owner of the View menu to hide the panel and untick its entry
signal close_requested

const ROW:PackedScene = preload("timetable_row.tscn")
const MINUTES_PER_HOUR:float = 60.0
const SECONDS_PER_MINUTE:float = 60.0
const HOURS_PER_DAY:int = 24
## CompareTime() (utilities.cpp:50): a difference over half a day is the other way round the clock
const HALF_DAY_MINUTES:float = 720.0
const DAY_MINUTES:float = 1440.0
const ON_TIME_COLOR:Color = Color(0.4, 0.85, 0.55)
const LATE_COLOR:Color = Color(1.0, 0.45, 0.4)
const EARLY_COLOR:Color = Color(0.45, 0.72, 1.0)

## The vehicle the player drives, and the driver of its trainset that follows a timetable
var _vehicle:RID = RID()
var _driver:RID = RID()
var _timetable:Timetable = null
## How far the driver got through the timetable (DriverSystem.driver_get_timetable_state())
var _state:Dictionary = {}
## The row shown as the train's station: the one it has left until it has driven clear of it
## (StationStart), else the one it drives to
var _current:int = 0
var _rows:Array[TimetableRow] = []


func _ready() -> void:
    DriverSystem.driver_timetable_changed.connect(_on_driver_timetable_changed)
    _follow_trainset()


func _exit_tree() -> void:
    DriverSystem.driver_timetable_changed.disconnect(_on_driver_timetable_changed)


## The player drives another vehicle, or none
func follow_vehicle(vehicle:RID) -> void:
    _vehicle = vehicle
    _follow_trainset()


## The driver followed moved on through its timetable; without one, any driver may be the one of
## the player's trainset that has just been given a timetable
func _on_driver_timetable_changed(driver:RID) -> void:
    if driver == _driver or not _driver.is_valid():
        _follow_trainset()


## The first driver of the player's trainset with a timetable, and how far it got through it:
## the train and its stations rebuilt for a new timetable, the progress and the delay shown
func _follow_trainset() -> void:
    _driver = RID()
    _state = {}
    var trainset:Array[RID] = []
    if _vehicle.is_valid():
        trainset = RailVehicleServer.vehicle_get_coupled(
                _vehicle, RailVehicleController.COUPLER_END_FRONT, RailVehicleController.COUPLING_FLAG_COUPLER)
    for vehicle:RID in trainset:
        var driver:RID = DriverSystem.vehicle_get_driver(vehicle)
        var driver_state:Dictionary = DriverSystem.driver_get_timetable_state(driver) if driver.is_valid() else {}
        if driver_state.get("timetable"):
            _driver = driver
            _state = driver_state
            break
    var timetable:Timetable = _state.get("timetable")
    var station_index:int = _state.get("station_index", 0)
    var station_start:int = _state.get("station_start", 0)
    # a stop left stays the train's station until it has driven clear of it (driveruipanels.cpp:392)
    var leaving:bool = station_start < station_index and (timetable.entries[station_start] as TimetableEntry).is_stop()
    _current = station_start if leaving else station_index
    if not timetable == _timetable:
        _timetable = timetable
        # the train and a row for each of its stations
        for row:TimetableRow in _rows:
            row.queue_free()
        _rows.clear()
        %Train.text = "%s %s" % [timetable.train_category, timetable.train_name] if timetable else tr("No timetable")
        %TrainLabel.text = timetable.train_label if timetable else ""
        %TrainLabel.visible = not %TrainLabel.text == ""
        %Relation.text = "%s  →  %s" % [
            timetable.relation_from.replace("_", " "), timetable.relation_to.replace("_", " ")
        ] if timetable and (timetable.relation_from or timetable.relation_to) else ""
        %Relation.visible = not %Relation.text == ""
        var details:PackedStringArray = []
        if timetable and timetable.locomotive_series:
            details.append(timetable.locomotive_series)
        if timetable and timetable.locomotive_load > 0.0:
            details.append("%d t" % timetable.locomotive_load)
        if timetable and timetable.brake_ratio > 0.0:
            details.append(tr("braked %d%%") % timetable.brake_ratio)
        %Details.text = "  ·  ".join(details)
        %Details.visible = details.size() > 0
        var entries:Array = timetable.entries if timetable else []
        # a number in place of a timetable is only the speed (mtable.cpp:279)
        %NoStations.text = tr("Line speed %d km/h, no stations") % timetable.velocity \
                if timetable and timetable.velocity > 0.0 else tr("The train follows no timetable")
        %NoStations.visible = not entries
        %Delay.visible = entries.size() > 0
        %StationsScroll.visible = entries.size() > 0
        var velocity:float = TimetableRow.NO_SPEED
        for index:int in entries.size():
            var entry:TimetableEntry = entries[index]
            var row:TimetableRow = ROW.instantiate()
            %Stations.add_child(row)
            var place:TimetableRow.Place = TimetableRow.Place.BETWEEN
            if index == 0:
                place = TimetableRow.Place.FIRST
            elif index == entries.size() - 1:
                place = TimetableRow.Place.LAST
            row.show_entry(entry, place, entry.velocity if not entry.velocity == velocity else TimetableRow.NO_SPEED)
            velocity = entry.velocity
            _rows.append(row)
        # a new timetable fits the card to what it shows; its size is the player's until the next.
        # The card stands in the bottom right corner, above the vehicle chips, and grows up and
        # left: its bottom right corner stays where it is
        var corner:Vector2 = position + size
        reset_size()
        position = corner - size
    for index:int in _rows.size():
        var progress:TimetableRow.Progress = TimetableRow.Progress.AHEAD
        if index == _current:
            progress = TimetableRow.Progress.NEXT
        elif index < station_index:
            progress = TimetableRow.Progress.PASSED
        _rows[index].show_progress(progress)
    _scroll_to_next_station()
    _tick()


## The list scrolled to the station passed last with the next one below it - on every new progress,
## and once the rows are laid out (the list's sort_children, a [connection] in the scene)
func _scroll_to_next_station() -> void:
    var shown:int = clampi(_current - 1, 0, _rows.size() - 1)
    %StationsScroll.scroll_vertical = int(_rows[shown].position.y) if _rows else 0


## Whether the train of a timetable state stands at its station (the row `_current` shows): it has
## arrived there, or left it and not driven clear of it yet
static func is_standing(state:Dictionary) -> bool:
    return int(state.get("station_start", 0)) < int(state.get("station_index", 0)) or state.get("arrived", false)


## The delay [min] of a train by its timetable state at `hours`, late when positive: at a stop,
## what it arrived with and, past the departure, the time since - an early arrival waits for the
## departure and is none; on the way, what it left the last station with
static func delay_minutes(state:Dictionary, hours:float) -> int:
    var timetable:Timetable = state.get("timetable")
    var station_start:int = state.get("station_start", 0)
    var delay:int = roundi(state.get("delay", 0.0))
    if is_standing(state) and timetable and station_start < timetable.entries.size():
        var entry:TimetableEntry = timetable.entries[station_start]
        if entry.is_stop() and entry.departure >= 0.0:
            return maxi(0, maxi(delay, floori(-minutes_to(entry.departure, hours))))
    return delay


## CompareTime() (utilities.cpp:50): the minutes from `hours` to `time`, the shorter way round the
## clock
static func minutes_to(time:float, hours:float) -> float:
    var minutes:float = (time - hours) * MINUTES_PER_HOUR
    if minutes < -HALF_DAY_MINUTES:
        minutes += DAY_MINUTES
    if minutes > HALF_DAY_MINUTES:
        minutes -= DAY_MINUTES
    return minutes


## The clock, the delay and the status of the train's station, on the panel's timer (a [connection]
## in the scene)
func _tick() -> void:
    var hours:float = SimulationServer.time_of_day
    var seconds:int = floori(hours * MINUTES_PER_HOUR * SECONDS_PER_MINUTE)
    var seconds_per_hour:int = int(MINUTES_PER_HOUR * SECONDS_PER_MINUTE)
    %Clock.text = "%02d:%02d:%02d" % [
        (seconds / seconds_per_hour) % HOURS_PER_DAY, (seconds / int(SECONDS_PER_MINUTE)) % int(MINUTES_PER_HOUR),
        seconds % int(SECONDS_PER_MINUTE)
    ]
    var late_minutes:int = delay_minutes(_state, hours)
    %DelayText.text = tr("On time") if late_minutes == 0 else "%+d min" % late_minutes
    %DelayText.add_theme_color_override(&"font_color",
            ON_TIME_COLOR if late_minutes == 0 else (LATE_COLOR if late_minutes > 0 else EARLY_COLOR))
    if _current >= _rows.size():
        return
    var entry:TimetableEntry = _timetable.entries[_current]
    var row:TimetableRow = _rows[_current]
    # the train's dispatch at the stop, kept under the vehicle its driver drives
    var vehicle:RID = VehicleServer.person_get_vehicle(_driver)
    var dispatch_step:StationServer.DispatchStep = StationServer.dispatch_get_step(vehicle)
    if not is_standing(_state) or not entry.is_stop():
        row.show_status(tr("Next station"), TimetableRow.Tone.NEUTRAL)
    elif _current == _rows.size() - 1:
        row.show_status(tr("Terminus"), TimetableRow.Tone.NEUTRAL)
    elif dispatch_step == StationServer.DISPATCH_STEP_EXCHANGE:
        row.show_status(tr("Passenger exchange · %d s") % ceili(StationServer.dispatch_get_exchange_time(vehicle)),
                TimetableRow.Tone.EXCHANGE)
    elif dispatch_step == StationServer.DISPATCH_STEP_CLOSE_DOORS:
        row.show_status(tr("Closing doors"), TimetableRow.Tone.WAIT)
    else:
        var minutes:float = minutes_to(entry.departure, hours)
        if minutes > 0.0:
            var wait:int = ceili(minutes * SECONDS_PER_MINUTE)
            row.show_status(tr("Stop · departure in %d:%02d") % [
                wait / int(SECONDS_PER_MINUTE), wait % int(SECONDS_PER_MINUTE)
            ], TimetableRow.Tone.WAIT)
        else:
            row.show_status(tr("Departure"), TimetableRow.Tone.GO)


## The clock runs only while the panel is shown
func _on_visibility_changed() -> void:
    if not visible:
        %Timer.stop()
        return
    _tick()
    %Timer.start()


func _on_close_button_pressed() -> void:
    close_requested.emit()
