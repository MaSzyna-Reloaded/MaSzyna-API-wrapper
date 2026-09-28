extends PanelContainer

## The timetable of the player's train: the train, its relation, the clock and the delay, and every
## station with the times it is due - the original's timetable panel (timetable_panel::update(),
## driveruipanels.cpp:300-472; F2, driveruilayer.cpp:155) laid out anew. It shows the timetable of
## the trainset the player's vehicle belongs to: the scenario gives it to one driver of the
## trainset only, not necessarily the one of the vehicle the player sits in.

## The close button asks the owner of the View menu to hide the panel and untick its entry
signal close_requested

const ROW:PackedScene = preload("timetable_row.tscn")
const FRONT_END:int = 0
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
var _station_index:int = 0
var _at_passenger_stop:bool = false
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
    var state:Dictionary = {}
    var trainset:Array[RID] = []
    if _vehicle.is_valid():
        trainset = RailVehicleServer.vehicle_get_coupled(
                _vehicle, FRONT_END, RailVehicleController.COUPLING_ELEMENT_COUPLER)
    for vehicle:RID in trainset:
        var driver:RID = DriverSystem.vehicle_get_driver(vehicle)
        var driver_state:Dictionary = DriverSystem.driver_get_timetable_state(driver) if driver.is_valid() else {}
        if driver_state.get("timetable"):
            _driver = driver
            state = driver_state
            break
    var timetable:Timetable = state.get("timetable")
    _station_index = state.get("station_index", 0)
    _at_passenger_stop = state.get("at_passenger_stop", false)
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
        # a new timetable fits the card to what it shows; its size is the player's until the next
        reset_size()
    # late or early on leaving the last station [min]
    var late_minutes:int = roundi(state.get("latency", 0.0))
    %DelayText.text = tr("On time") if late_minutes == 0 else "%+d min" % late_minutes
    %DelayText.add_theme_color_override(&"font_color",
            ON_TIME_COLOR if late_minutes == 0 else (LATE_COLOR if late_minutes > 0 else EARLY_COLOR))
    for index:int in _rows.size():
        var progress:TimetableRow.Progress = TimetableRow.Progress.AHEAD
        if index < _station_index:
            progress = TimetableRow.Progress.PASSED
        elif index == _station_index:
            progress = TimetableRow.Progress.NEXT
        _rows[index].show_progress(progress)
    _scroll_to_next_station()
    _tick()


## The list scrolled to the station passed last with the next one below it - on every new progress,
## and once the rows are laid out (the list's sort_children, a [connection] in the scene)
func _scroll_to_next_station() -> void:
    var shown:int = clampi(_station_index - 1, 0, _rows.size() - 1)
    %StationsScroll.scroll_vertical = int(_rows[shown].position.y) if _rows else 0


## The clock and the status of the next station, on the panel's timer (a [connection] in the scene)
func _tick() -> void:
    var hours:float = SimulationServer.time_of_day
    var seconds:int = floori(hours * MINUTES_PER_HOUR * SECONDS_PER_MINUTE)
    var seconds_per_hour:int = int(MINUTES_PER_HOUR * SECONDS_PER_MINUTE)
    %Clock.text = "%02d:%02d:%02d" % [
        (seconds / seconds_per_hour) % HOURS_PER_DAY, (seconds / int(SECONDS_PER_MINUTE)) % int(MINUTES_PER_HOUR),
        seconds % int(SECONDS_PER_MINUTE)
    ]
    if _station_index >= _rows.size():
        return
    var entry:TimetableEntry = _timetable.entries[_station_index]
    var row:TimetableRow = _rows[_station_index]
    if not _at_passenger_stop or not entry.is_stop():
        row.show_status(tr("Next station"), TimetableRow.Tone.NEUTRAL)
    elif _station_index == _rows.size() - 1:
        row.show_status(tr("Terminus"), TimetableRow.Tone.NEUTRAL)
    else:
        # CompareTime() (utilities.cpp:50): the shorter way round the clock
        var minutes:float = (entry.departure - hours) * MINUTES_PER_HOUR
        if minutes < -HALF_DAY_MINUTES:
            minutes += DAY_MINUTES
        if minutes > HALF_DAY_MINUTES:
            minutes -= DAY_MINUTES
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
