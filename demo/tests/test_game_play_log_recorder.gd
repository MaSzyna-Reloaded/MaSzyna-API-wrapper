extends MaszynaGutTest

## The gameplay log, written through a file handler of the "gameplay" logger
const LOG_PATH: String = "user://test_game_play_log/gameplay.log"
const GAMEPLAY_LOGGER: String = "gameplay"
## A gameplay log line's fields: time, simulation time, kind, subject, details
const COLUMN_KIND: int = 2
const COLUMN_DETAILS: int = 4

var _handler: GameLogFileHandler = null
var _recorder: GamePlayLogRecorder = null


func before_each() -> void:
    _handler = GameLogFileHandler.open(LOG_PATH)
    GameLog.create_handler(GAMEPLAY_LOGGER, _handler)
    _recorder = add_child_autofree(GamePlayLogRecorder.new())
    _recorder.start()


func after_each() -> void:
    GameLog.remove_handler(GAMEPLAY_LOGGER, _handler)
    _handler = null
    DirAccess.remove_absolute(LOG_PATH)
    DirAccess.remove_absolute(LOG_PATH.get_base_dir())


func test_a_repeated_command_is_counted_on_one_line() -> void:
    # no player's vehicle here: the commands to no vehicle are its commands
    var player_vehicle: RID = PlayerServer.player_get_vehicle()
    for level: float in [0.1, 0.2, 0.3]:
        VehicleServer.vehicle_command_received.emit(player_vehicle, "brake_level_set", level, null)
    VehicleServer.vehicle_command_received.emit(player_vehicle, "converter", true, null)

    _recorder.stop()
    var lines: PackedStringArray = _read_lines()

    # the first line is the player, present from the start, the last one the scenery left
    assert_eq(lines.size(), 4, "a dragged lever should be one line, the next command another: %s" % lines)
    var dragged: PackedStringArray = _fields(lines[1])
    assert_eq(dragged[COLUMN_KIND], "command")
    assert_eq(Array(dragged.slice(COLUMN_DETAILS, COLUMN_DETAILS + 4)), ["brake_level_set", "p1=0.1", "p2=<null>", "repeats=3"])
    assert_eq(dragged[-1], "last=0.3,<null>", "the line should end with the last values")
    assert_eq(Array(_fields(lines[2]).slice(COLUMN_DETAILS)), ["converter", "p1=true", "p2=<null>"],
            "a single command should stand as it came")


func test_the_persons_made_renamed_and_freed_are_named() -> void:
    var person: RID = PersonServer.person_create("SN61-02")
    PersonServer.person_set_name(person, "SN61-03")
    PersonServer.person_free(person)

    _recorder.stop()
    var lines: PackedStringArray = _read_lines()

    var player: RID = PlayerServer.player_get_person()
    assert_eq(lines.size(), 5, "the player, the person made, renamed and freed, the scenery left: %s" % lines)
    assert_eq(Array(_fields(lines[0]).slice(COLUMN_KIND)),
            ["player", "%s#%d" % [PersonServer.person_get_name(player), player.get_id()], "present"],
            "the player is named from the start: %s" % lines[0])
    assert_eq(Array(_fields(lines[1]).slice(COLUMN_KIND)), ["person", "SN61-02#%d" % person.get_id(), "created"])
    assert_eq(Array(_fields(lines[2]).slice(COLUMN_KIND)),
            ["person", "SN61-03#%d" % person.get_id(), "renamed", "previous=SN61-02"])
    assert_eq(Array(_fields(lines[3]).slice(COLUMN_KIND)), ["person", "SN61-03#%d" % person.get_id(), "freed"],
            "a freed person still has its name: %s" % lines[3])


func test_a_line_is_on_the_disk_at_once() -> void:
    # read past the recorder, as a crash leaves the file - nothing flushed it on the way
    var on_disk: String = FileAccess.get_file_as_string(LOG_PATH)
    _recorder.stop()

    assert_string_contains(on_disk, " present", "the line written at the start is in the file")


func test_leaving_the_scenery_is_the_last_line() -> void:
    _recorder.stop()
    var lines: PackedStringArray = _read_lines()

    assert_true(lines[-1].ends_with(" scenery left"), "the log ends where the scenery was left: %s" % lines[-1])


func test_a_line_below_the_handlers_level_is_left_out() -> void:
    _handler.min_level = GameLog.LogLevel.WARNING

    GameLog.get_logger(GAMEPLAY_LOGGER).info("an info line")
    GameLog.get_logger(GAMEPLAY_LOGGER).warning("a warning line")
    _recorder.stop()

    # the player's line came before the level was raised, the scenery left after it
    var lines: PackedStringArray = _read_lines()
    assert_eq(lines[-1], "a warning line", "the warning should reach the file, not what came after it")
    assert_does_not_have(lines, "an info line", "the info line should be left out")


func _read_lines() -> PackedStringArray:
    return FileAccess.get_file_as_string(LOG_PATH).strip_edges().split("\n")


## A gameplay log line's fields
func _fields(line: String) -> PackedStringArray:
    return line.split(" ")
