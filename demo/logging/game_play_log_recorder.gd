class_name GamePlayLogRecorder
extends Node

## The gameplay's log, from start() to stop() (a scenery, demo_scenery_loading.gd), to GameLog's
## loggers: "gameplay" - the commands the player's vehicle received (the player's and the
## scenario's - a command carries no sender), the persons made and freed, where they sit and in
## what role, the vehicles made and freed and the trainsets they form; "ai" - the commands the
## other vehicles received; "scenario" - the scenario's events that ran. They go to the loggers'
## handlers as they come, not into memory (the log files, demo_scenery_loading.gd), so a session of
## any length goes with a problem report. A command repeated without a pause - a lever dragged with
## the mouse, a key held - is one line: the first one, its count and the last one's time and
## values, written once the command has not come again for REPEAT_GAP_MSEC.

## A command coming this soon after the same one [ms of real time] is counted on its line
const REPEAT_GAP_MSEC: int = 1000
## What a role's and a cabin kind's constant name starts with, left out of the log
const ROLE_PREFIX: String = "VEHICLE_PERSON_ROLE_"
const CABIN_PREFIX: String = "RAIL_VEHICLE_CABIN_"

## Lines: "<time> <simulation time> <kind> <subject> <details>", the subject a name and
## its RID ("SN61-02#2"), the details key=value:
##   player|ai|person <name#rid> present|created|freed|attached|renamed previous=<name>
##       |entered vehicle=<name#rid> cab=<front|rear|machine> role=<driver|observer>
##       |left vehicle= cab=|moved vehicle= cab= from_vehicle= from_cab=|role=<role> vehicle= cab=
##   vehicle <name#rid> created|freed;  trainset <first#rid> vehicles=<name#rid>,...
##   command <name#rid> <command> p1=<p1> p2=<p2> [repeats=<n> until=<simulation time> last=<p1>,<p2>]
##   event <event> [activator=<name>];  scenery left;  game closed
var _gameplay_log: GameLogger = GameLog.get_logger("gameplay")
var _scenario_log: GameLogger = GameLog.get_logger("scenario")
var _ai_log: GameLogger = GameLog.get_logger("ai")
## Between start() and stop()
var _recording: bool = false
## The names of the vehicles logged as created: a freed vehicle's name is gone when it is announced
var _vehicle_names: Dictionary[RID, String] = {}
## The trainset logged last - every vehicle of a trainset announces the same change
var _last_trainset: String = ""
## VehiclePersonRole.Role and RailVehicleCabinKind.Kind by value, as words: the constant's own name
## without its prefix ("driver", "rear")
var _role_names: Dictionary[int, String] = {}
var _cabin_kind_names: Dictionary[int, String] = {}
## The last command, not written yet while it may repeat: its line, the logger it goes to, what
## tells a repeat (vehicle and command), how many times it came, the last one's time and values and
## when it came
var _repeated_line: String = ""
var _repeated_log: GameLogger = null
var _repeated_key: String = ""
var _repeats: int = 0
var _repeated_last: String = ""
var _repeated_msec: int = 0
## Writes the command being counted once it has not come again for REPEAT_GAP_MSEC - the file
## reads the same as when the next command wrote it, but the line is out at once
var _repeat_timer: Timer = null


func _ready() -> void:
    _repeat_timer = Timer.new()
    _repeat_timer.one_shot = true
    add_child(_repeat_timer)
    _repeat_timer.timeout.connect(_write_repeated)
    for constant: String in ClassDB.class_get_enum_constants(&"VehiclePersonRole", &"Role"):
        _role_names[ClassDB.class_get_integer_constant(&"VehiclePersonRole", constant)] = \
                constant.trim_prefix(ROLE_PREFIX).to_lower()
    for constant: String in ClassDB.class_get_enum_constants(&"RailVehicleCabinKind", &"Kind"):
        _cabin_kind_names[ClassDB.class_get_integer_constant(&"RailVehicleCabinKind", constant)] = \
                constant.trim_prefix(CABIN_PREFIX).to_lower()


## For a scenery being started
func start() -> void:
    _recording = true
    VehicleServer.vehicle_command_received.connect(_on_vehicle_command_received)
    ScenarioEventServer.event_launched.connect(_on_event_launched)
    PersonServer.person_created.connect(_on_person_created)
    PersonServer.person_name_changed.connect(_on_person_name_changed)
    PersonServer.person_freed.connect(_on_person_freed)
    DriverSystem.driver_attached.connect(_on_driver_attached)
    VehicleServer.cabin_person_entered.connect(_on_cabin_person_entered)
    VehicleServer.cabin_person_left.connect(_on_cabin_person_left)
    VehicleServer.cabin_person_moved.connect(_on_cabin_person_moved)
    VehicleServer.cabin_person_role_changed.connect(_on_cabin_person_role_changed)
    VehicleServer.vehicle_configured.connect(_on_vehicle_configured)
    VehicleServer.vehicle_freed.connect(_on_vehicle_freed)
    RailVehicleServer.vehicle_trainset_changed.connect(_on_vehicle_trainset_changed)
    # the player is made with the game, before any scenery
    _write_person(PlayerServer.player_get_person(), "present")


## For a scenery being left
func stop() -> void:
    _close("scenery", "left")


## The game quit with a scenery running - before the scenery goes down, whose freeing would follow
func _exit_tree() -> void:
    if _recording:
        _close("game", "closed")


## The last line, `subject` `what` ended the recording; nothing is written after it
func _close(subject: String, what: String) -> void:
    VehicleServer.vehicle_command_received.disconnect(_on_vehicle_command_received)
    ScenarioEventServer.event_launched.disconnect(_on_event_launched)
    PersonServer.person_created.disconnect(_on_person_created)
    PersonServer.person_name_changed.disconnect(_on_person_name_changed)
    PersonServer.person_freed.disconnect(_on_person_freed)
    DriverSystem.driver_attached.disconnect(_on_driver_attached)
    VehicleServer.cabin_person_entered.disconnect(_on_cabin_person_entered)
    VehicleServer.cabin_person_left.disconnect(_on_cabin_person_left)
    VehicleServer.cabin_person_moved.disconnect(_on_cabin_person_moved)
    VehicleServer.cabin_person_role_changed.disconnect(_on_cabin_person_role_changed)
    VehicleServer.vehicle_configured.disconnect(_on_vehicle_configured)
    VehicleServer.vehicle_freed.disconnect(_on_vehicle_freed)
    RailVehicleServer.vehicle_trainset_changed.disconnect(_on_vehicle_trainset_changed)
    _vehicle_names.clear()
    _last_trainset = ""
    _repeat_timer.stop()
    _write_repeated()
    _gameplay_log.info(_entry(subject, what, ""))
    _recording = false


func _on_vehicle_command_received(vehicle_rid: RID, command: String, p1: Variant, p2: Variant) -> void:
    var key: String = "%s %s" % [_named(vehicle_rid, VehicleServer.vehicle_get_name(vehicle_rid)), command]
    var now: int = Time.get_ticks_msec()
    if key == _repeated_key and now - _repeated_msec <= REPEAT_GAP_MSEC:
        _repeats += 1
        _repeated_last = "until=%.3f last=%s,%s" % [SimulationServer.simulation_get_time(), p1, p2]
    else:
        _write_repeated()
        _repeated_log = _gameplay_log if vehicle_rid == PlayerServer.player_get_vehicle() else _ai_log
        _repeated_key = key
        _repeated_line = _entry("command", _named(vehicle_rid, VehicleServer.vehicle_get_name(vehicle_rid)),
                "%s p1=%s p2=%s" % [command, p1, p2])
        _repeats = 1
    _repeated_msec = now
    _repeat_timer.start(REPEAT_GAP_MSEC / 1000.0)


func _on_event_launched(event: RID, activator: RID) -> void:
    _write_repeated()
    _scenario_log.info(_entry("event", ScenarioEventServer.event_get_name(event),
            "activator=%s" % VehicleServer.vehicle_get_name(activator) if activator.is_valid() else ""))


func _on_person_created(person: RID) -> void:
    _write_person(person, "created")


func _on_person_name_changed(person: RID, previous: String) -> void:
    _write_person(person, "renamed previous=%s" % previous)


## The person drives as an AI from here on
func _on_driver_attached(driver: RID) -> void:
    _write_person(driver, "attached")


func _on_person_freed(person: RID) -> void:
    _write_person(person, "freed")


func _on_cabin_person_entered(cabin: RID, person: RID, role: VehiclePersonRole.Role) -> void:
    _write_person(person, "entered %s role=%s" % [_cabin_text(cabin), _role_names.get(role, str(role))])


func _on_cabin_person_left(cabin: RID, person: RID) -> void:
    _write_person(person, "left %s" % _cabin_text(cabin))


func _on_cabin_person_moved(person: RID, cabin: RID, previous: RID) -> void:
    _write_person(person, "moved %s from_%s" % [_cabin_text(cabin), _cabin_text(previous).replace(" ", " from_")])


func _on_cabin_person_role_changed(cabin: RID, person: RID, role: VehiclePersonRole.Role) -> void:
    _write_person(person, "role=%s %s" % [_role_names.get(role, str(role)), _cabin_text(cabin)])


## The vehicle has its simulation: built, named - it counts as made from here
func _on_vehicle_configured(vehicle: RID) -> void:
    if _vehicle_names.has(vehicle):
        return
    _vehicle_names[vehicle] = VehicleServer.vehicle_get_name(vehicle)
    _write_repeated()
    _gameplay_log.info(_entry("vehicle", _named(vehicle, _vehicle_names[vehicle]), "created"))


func _on_vehicle_freed(vehicle: RID) -> void:
    if not _vehicle_names.has(vehicle):
        return
    _write_repeated()
    _gameplay_log.info(_entry("vehicle", _named(vehicle, _vehicle_names[vehicle]), "freed"))
    _vehicle_names.erase(vehicle)


func _on_vehicle_trainset_changed(vehicle: RID) -> void:
    var members: Array[String] = []
    for coupled: RID in RailVehicleServer.vehicle_get_coupled(
            vehicle, RailVehicleController.COUPLER_END_FRONT, RailVehicleController.COUPLING_FLAG_COUPLER):
        members.append(_named(coupled, VehicleServer.vehicle_get_name(coupled)))
    var trainset: String = ",".join(members)
    # the same trainset from its other vehicles, or read from its other end
    var reversed_members: Array[String] = members.duplicate()
    reversed_members.reverse()
    if trainset == _last_trainset or ",".join(reversed_members) == _last_trainset:
        return
    _last_trainset = trainset
    _write_repeated()
    _gameplay_log.info(_entry("trainset", members[0], "vehicles=%s" % trainset))


## A person's line, its kind the player, an AI driver or a person who is neither (yet)
func _write_person(person: RID, what: String) -> void:
    var kind: String = "person"
    if person == PlayerServer.player_get_person():
        kind = "player"
    elif DriverSystem.driver_get_rids().has(person):
        kind = "ai"
    _write_repeated()
    _gameplay_log.info(_entry(kind, _named(person, PersonServer.person_get_name(person)), what))


## "vehicle=<name#rid> cab=<kind>"
func _cabin_text(cabin: RID) -> String:
    var vehicle: RID = VehicleServer.cabin_get_vehicle(cabin)
    return "vehicle=%s cab=%s" % [
        _named(vehicle, VehicleServer.vehicle_get_name(vehicle)),
        _cabin_kind_names.get(RailVehicleServer.cabin_get_kind(cabin), "-"),
    ]


## "<name>#<rid>"
func _named(rid: RID, name: String) -> String:
    return "%s#%d" % [name, rid.get_id()]


## A line: "<time> <simulation time> <kind> <subject> <details>", one space between
func _entry(kind: String, subject: String, details: String) -> String:
    return ("%s %.3f %s %s %s" % [
        Time.get_time_string_from_system(), SimulationServer.simulation_get_time(), kind, subject, details
    ]).strip_edges()


## The command being counted goes to the log, and the next one starts a line of its own
func _write_repeated() -> void:
    if not _repeated_line:
        return
    _repeated_log.info(_repeated_text())
    _repeated_line = ""
    _repeated_key = ""


func _repeated_text() -> String:
    if _repeats == 1:
        return _repeated_line
    return "%s repeats=%d %s" % [_repeated_line, _repeats, _repeated_last]

