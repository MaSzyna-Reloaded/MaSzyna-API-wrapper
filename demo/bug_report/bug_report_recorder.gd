class_name BugReportRecorder
extends Node

## What a problem report cannot read back at the moment it is made: the commands the player's
## vehicle received and the scenario's events that ran, from start() to stop(). They go to the
## gameplay log as they come, not into memory, so a session of any length goes with the report. A
## command carries no sender, so the scenario's commands to the player's vehicle are written as
## well as the player's own. A command repeated without a pause - a lever dragged with the mouse, a
## key held - is one line: the first one, its count and the last one's time and values, written
## once the command has not come again for REPEAT_GAP_MSEC.

## A line went to the gameplay log
signal line_written(line: String)

## How much of the gameplay log's end goes with a report - it compresses well
const LOG_TAIL_BYTES: int = 50 * 1024 * 1024
## A command coming this soon after the same one [ms of real time] is counted on its line
const REPEAT_GAP_MSEC: int = 1000

## The gameplay log of a run without a display - a test, a probe: it shares the game's user
## directory, and starting its own scenery would cut the log of a game running beside it
const HEADLESS_LOG_PATH: String = "user://logs/headless/gameplay.log"

## The gameplay log, started afresh with every scenery
@export var log_path: String = "user://logs/gameplay.log"

## Lines: "<time> <simulation time> <vehicle> <command> <p1> <p2>", for a repeated command
## followed by "x<count> until <time> <simulation time> <p1> <p2>"; "<time> <simulation time>
## <event> <activator>" for an event
var _log: FileAccess = null
## The last command, not written yet while it may repeat: its line, what tells a repeat (vehicle
## and command), how many times it came, the last one's time and values and when it came
var _repeated_line: String = ""
var _repeated_key: String = ""
var _repeats: int = 0
var _repeated_last: String = ""
var _repeated_msec: int = 0
## Writes the command being counted once it has not come again for REPEAT_GAP_MSEC - the file
## reads the same as when the next command wrote it, but the line is out at once
var _repeat_timer: Timer = null


func _ready() -> void:
    if DisplayServer.get_name() == "headless":
        log_path = HEADLESS_LOG_PATH
    _repeat_timer = Timer.new()
    _repeat_timer.one_shot = true
    add_child(_repeat_timer)
    _repeat_timer.timeout.connect(_write_repeated)


## For a scenery being started
func start() -> void:
    DirAccess.make_dir_recursive_absolute(log_path.get_base_dir())
    _log = FileAccess.open(log_path, FileAccess.WRITE)
    VehicleServer.vehicle_command_received.connect(_on_vehicle_command_received)
    ScenarioEventServer.event_launched.connect(_on_event_launched)


## For a scenery being left: the log is closed, and stays until the next scenery starts
func stop() -> void:
    VehicleServer.vehicle_command_received.disconnect(_on_vehicle_command_received)
    ScenarioEventServer.event_launched.disconnect(_on_event_launched)
    _repeat_timer.stop()
    _write_repeated()
    _log = null


## The gameplay log's last LOG_TAIL_BYTES, with the command still being counted
func get_log() -> PackedByteArray:
    if not _log:
        return PackedByteArray()
    _log.flush()
    var file: FileAccess = FileAccess.open(log_path, FileAccess.READ)
    if not file:
        return PackedByteArray()
    var length: int = file.get_length()
    file.seek(maxi(length - LOG_TAIL_BYTES, 0))
    var tail: PackedByteArray = file.get_buffer(mini(length, LOG_TAIL_BYTES))
    if _repeated_line:
        tail.append_array((_repeated_text() + "\n").to_utf8_buffer())
    return tail


func _on_vehicle_command_received(vehicle_rid: RID, command: String, p1: Variant, p2: Variant) -> void:
    if not vehicle_rid == PlayerServer.player_get_vehicle():
        return
    var key: String = "%s %s" % [VehicleServer.vehicle_get_name(vehicle_rid), command]
    var now: int = Time.get_ticks_msec()
    if key == _repeated_key and now - _repeated_msec <= REPEAT_GAP_MSEC:
        _repeats += 1
        _repeated_last = "%s %s %s" % [_time_text(), p1, p2]
    else:
        _write_repeated()
        _repeated_key = key
        _repeated_line = "%s %s %s %s" % [_time_text(), key, p1, p2]
        _repeats = 1
    _repeated_msec = now
    _repeat_timer.start(REPEAT_GAP_MSEC / 1000.0)


func _on_event_launched(event: RID, activator: RID) -> void:
    _write_repeated()
    _write_line("%s %s %s" % [
        _time_text(), ScenarioEventServer.event_get_name(event), VehicleServer.vehicle_get_name(activator)
    ])


## The command being counted goes to the log, and the next one starts a line of its own
func _write_repeated() -> void:
    if not _repeated_line:
        return
    _write_line(_repeated_text())
    _repeated_line = ""
    _repeated_key = ""


func _write_line(line: String) -> void:
    _log.store_line(line)
    line_written.emit(line)


func _repeated_text() -> String:
    if _repeats == 1:
        return _repeated_line
    return "%s x%d until %s" % [_repeated_line, _repeats, _repeated_last]


## "<time> <simulation time>" of a line
func _time_text() -> String:
    return "%s %.3f" % [Time.get_time_string_from_system(), SimulationServer.simulation_get_time()]
