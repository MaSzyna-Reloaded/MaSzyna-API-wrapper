extends MaszynaGutTest

## A screenshot of this size, a mark inside it of this rectangle
const IMAGE_SIZE: Vector2i = Vector2i(64, 48)
const MARK: Rect2 = Rect2(10.0, 10.0, 30.0, 20.0)
const BOUNDARY: String = "TestBoundary"
## Where the test saves its reports
const REPORTS_DIRECTORY: String = "user://test_bug_reports"
## A gameplay log line's fields (BugReportRecorder): time, simulation time, kind, subject, details
const COLUMN_KIND: int = 2
const COLUMN_DETAILS: int = 4

var _endpoint: Variant = null


func before_each() -> void:
    _endpoint = ProjectSettings.get_setting(BugReportSender.ENDPOINT_SETTING)


func after_each() -> void:
    ProjectSettings.set_setting(BugReportSender.ENDPOINT_SETTING, _endpoint)
    # only the tests that write files make the directory
    if not DirAccess.dir_exists_absolute(REPORTS_DIRECTORY):
        return
    for directory: String in DirAccess.get_directories_at(REPORTS_DIRECTORY):
        var path: String = REPORTS_DIRECTORY.path_join(directory)
        for file: String in DirAccess.get_files_at(path):
            DirAccess.remove_absolute(path.path_join(file))
        DirAccess.remove_absolute(path)
    DirAccess.remove_absolute(REPORTS_DIRECTORY)


func test_reporting_is_unavailable_without_an_endpoint() -> void:
    ProjectSettings.set_setting(BugReportSender.ENDPOINT_SETTING, "")
    assert_false(BugReportSender.is_available(), "an empty endpoint should leave reporting unavailable")

    ProjectSettings.set_setting(BugReportSender.ENDPOINT_SETTING, REPORTS_DIRECTORY)
    assert_true(BugReportSender.is_available(), "a directory should make reporting available")


func test_hardware_names_the_build_and_the_graphics_adapter() -> void:
    var hardware: Dictionary = BugReportSnapshot.hardware()

    assert_eq(hardware["build"], GameDataServer.build_get_number())
    for key: String in ["os", "cpu", "cpu_threads", "memory", "gpu", "gpu_driver", "rendering_method"]:
        assert_has(hardware, key, "the hardware should name its %s" % key)


func test_a_snapshot_without_a_scenery_is_valid_json() -> void:
    var snapshot: Dictionary = BugReportSnapshot.collect(null, Time.get_ticks_msec())

    var parsed: Variant = JSON.parse_string(JSON.stringify(snapshot))
    assert_true(parsed is Dictionary, "the snapshot should survive JSON")
    for section: String in ["hardware", "scenario", "simulation", "camera", "streaming", "vehicles",
            "signal_heads", "events"]:
        assert_has(parsed, section, "the snapshot should have its %s" % section)


func test_the_body_has_every_field_and_the_archive_and_ends_with_the_boundary() -> void:
    var fields: Dictionary = BugReportSender.fields_of({"title": "a title", "build": 42})
    var archive: PackedByteArray = PackedByteArray([1, 2, 3])

    var body: String = BugReportSender.build_body(BOUNDARY, fields, archive).get_string_from_utf8()

    assert_string_contains(body, "--%s\r\nContent-Disposition: form-data; name=\"api_version\"" % BOUNDARY)
    assert_string_contains(body, "name=\"title\"\r\nContent-Type: text/plain; charset=utf-8\r\n\r\na title\r\n")
    assert_string_contains(body, "name=\"build\"\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n42\r\n")
    assert_string_contains(body, "name=\"attachments\"; filename=\"report.zip\"\r\nContent-Type: application/zip")
    assert_true(body.ends_with("--%s--\r\n" % BOUNDARY), "the body should be closed by its boundary")


func test_the_fields_start_with_the_api_version() -> void:
    var fields: Dictionary = BugReportSender.fields_of({"title": "a title"})

    assert_eq(fields["api_version"], str(BugReportSender.API_VERSION))
    assert_eq(fields["title"], "a title")


func test_the_archive_holds_the_files_it_is_given() -> void:
    var files: Dictionary[String, PackedByteArray] = {
        BugReportSender.SNAPSHOT_FILE: "{}".to_utf8_buffer(),
        BugReportSender.LOG_FILE: "a log line".to_utf8_buffer(),
    }
    DirAccess.make_dir_recursive_absolute(REPORTS_DIRECTORY.path_join("archive"))
    var path: String = REPORTS_DIRECTORY.path_join("archive").path_join(BugReportSender.ATTACHMENTS_FILE)

    assert_eq(BugReportSender.pack(files, path), OK)

    var reader: ZIPReader = ZIPReader.new()
    assert_eq(reader.open(path), OK)
    assert_eq(Array(reader.get_files()), [BugReportSender.SNAPSHOT_FILE, BugReportSender.LOG_FILE])
    assert_eq(reader.read_file(BugReportSender.LOG_FILE).get_string_from_utf8(), "a log line")
    reader.close()


func test_a_directory_endpoint_saves_the_report_instead_of_sending_it() -> void:
    ProjectSettings.set_setting(BugReportSender.ENDPOINT_SETTING, REPORTS_DIRECTORY)
    var sender: BugReportSender = autofree(BugReportSender.new())
    watch_signals(sender)
    var report: Dictionary = {"title": "a title"}

    sender.send(report, "{}", PackedByteArray([1, 2, 3]), "a log line".to_utf8_buffer(),
            "a gameplay line".to_utf8_buffer())

    assert_signal_emitted(sender, "report_saved")
    assert_signal_not_emitted(sender, "report_failed")
    var directories: PackedStringArray = DirAccess.get_directories_at(REPORTS_DIRECTORY)
    assert_eq(directories.size(), 1, "the report should have a directory of its own")
    var directory: String = REPORTS_DIRECTORY.path_join(directories[0])
    assert_eq(Array(DirAccess.get_files_at(directory)), [BugReportSender.REPORT_FILE, BugReportSender.ATTACHMENTS_FILE])
    var reader: ZIPReader = ZIPReader.new()
    reader.open(directory.path_join(BugReportSender.ATTACHMENTS_FILE))
    assert_eq(Array(reader.get_files()),
            [BugReportSender.SNAPSHOT_FILE, BugReportSender.SCREENSHOT_FILE, BugReportSender.LOG_FILE,
                BugReportSender.GAMEPLAY_LOG_FILE])
    reader.close()
    var saved: Variant = JSON.parse_string(FileAccess.get_file_as_string(directory.path_join(BugReportSender.REPORT_FILE)))
    assert_eq(saved["title"], "a title")
    assert_eq(saved["api_version"], str(BugReportSender.API_VERSION))


func test_the_gameplay_log_counts_a_repeated_command_on_one_line() -> void:
    var recorder: BugReportRecorder = add_child_autofree(BugReportRecorder.new())
    recorder.log_path = REPORTS_DIRECTORY.path_join("gameplay.log")
    recorder.start()
    # no player's vehicle here: the commands to no vehicle are its commands
    var player_vehicle: RID = PlayerServer.player_get_vehicle()
    for level: float in [0.1, 0.2, 0.3]:
        VehicleServer.vehicle_command_received.emit(player_vehicle, "brake_level_set", level, null)
    VehicleServer.vehicle_command_received.emit(player_vehicle, "converter", true, null)

    var lines: PackedStringArray = recorder.get_log().get_string_from_utf8().strip_edges().split("\n")
    recorder.stop()
    DirAccess.remove_absolute(recorder.log_path)

    # the first line is the player, present from the start
    assert_eq(lines.size(), 3, "a dragged lever should be one line, the next command another")
    var dragged: PackedStringArray = _fields(lines[1])
    assert_eq(dragged[COLUMN_KIND], "command")
    assert_eq(Array(dragged.slice(COLUMN_DETAILS, COLUMN_DETAILS + 4)), ["brake_level_set", "p1=0.1", "p2=<null>", "repeats=3"])
    assert_eq(dragged[-1], "last=0.3,<null>", "the line should end with the last values")
    assert_eq(Array(_fields(lines[2]).slice(COLUMN_DETAILS)), ["converter", "p1=true", "p2=<null>"],
            "a single command should stand as it came")


func test_the_gameplay_log_names_the_persons_made_renamed_and_freed() -> void:
    var recorder: BugReportRecorder = add_child_autofree(BugReportRecorder.new())
    recorder.log_path = REPORTS_DIRECTORY.path_join("gameplay.log")
    recorder.start()
    var person: RID = PersonServer.person_create("SN61-02")
    PersonServer.person_set_name(person, "SN61-03")
    PersonServer.person_free(person)

    var lines: PackedStringArray = recorder.get_log().get_string_from_utf8().strip_edges().split("\n")
    recorder.stop()
    DirAccess.remove_absolute(recorder.log_path)

    var player: RID = PlayerServer.player_get_person()
    assert_eq(lines.size(), 4, "the player, then the person made, renamed and freed: %s" % lines)
    assert_eq(Array(_fields(lines[0]).slice(COLUMN_KIND)),
            ["player", "%s#%d" % [PersonServer.person_get_name(player), player.get_id()], "present"],
            "the player is named from the start: %s" % lines[0])
    assert_eq(Array(_fields(lines[1]).slice(COLUMN_KIND)), ["person", "SN61-02#%d" % person.get_id(), "created"])
    assert_eq(Array(_fields(lines[2]).slice(COLUMN_KIND)),
            ["person", "SN61-03#%d" % person.get_id(), "renamed", "previous=SN61-02"])
    assert_eq(Array(_fields(lines[3]).slice(COLUMN_KIND)), ["person", "SN61-03#%d" % person.get_id(), "freed"],
            "a freed person still has its name: %s" % lines[3])


## Linux, macOS and Windows set one of each pair; the test sets them all, to a home and a login of
## its own, and puts back what there was
func test_the_home_directory_and_the_login_leave_no_trace_in_what_is_sent() -> void:
    var variables: PackedStringArray = BugReportSender.HOME_VARIABLES + BugReportSender.USER_VARIABLES
    var kept: Dictionary[String, String] = {}
    for variable: String in variables:
        kept[variable] = OS.get_environment(variable)
    OS.set_environment("HOME", "/home/jan.kowalski")
    OS.set_environment("USERPROFILE", "C:\\Users\\jan.kowalski")
    OS.set_environment("USER", "jan.kowalski")
    OS.set_environment("USERNAME", "jan.kowalski")

    var masked: String = BugReportSender.mask_personal("\n".join([
        "ERROR: Cannot load /home/jan.kowalski/.local/share/Steam/scenery/drewno.inc",
        "WARNING: C:\\Users\\jan.kowalski\\Games\\x.e3d and C:/Users/jan.kowalski/Games/y.e3d",
        "07:16:42 0.000 person 3 jan.kowalski present",
        "D:\\Steam\\jan.kowalski\\MaSzyna and jan.kowalskiego stays",
    ]))
    for variable: String in variables:
        OS.set_environment(variable, kept[variable])

    assert_eq(masked, "\n".join([
        "ERROR: Cannot load ~/.local/share/Steam/scenery/drewno.inc",
        "WARNING: ~\\Games\\x.e3d and ~/Games/y.e3d",
        "07:16:42 0.000 person 3 <user> present",
        "D:\\Steam\\<user>\\MaSzyna and jan.kowalskiego stays",
    ]))


func test_a_gameplay_line_is_on_the_disk_at_once() -> void:
    var recorder: BugReportRecorder = add_child_autofree(BugReportRecorder.new())
    recorder.log_path = REPORTS_DIRECTORY.path_join("gameplay.log")
    recorder.start()
    # read past the recorder, as a crash leaves the file - nothing flushed it on the way
    var on_disk: String = FileAccess.get_file_as_string(recorder.log_path)
    recorder.stop()
    DirAccess.remove_absolute(recorder.log_path)

    assert_string_contains(on_disk, " present", "the line written at the start is in the file")


func test_leaving_the_scenery_is_the_last_line() -> void:
    var recorder: BugReportRecorder = add_child_autofree(BugReportRecorder.new())
    recorder.log_path = REPORTS_DIRECTORY.path_join("gameplay.log")
    recorder.start()
    recorder.stop()
    var lines: PackedStringArray = FileAccess.get_file_as_string(recorder.log_path).strip_edges().split("\n")
    DirAccess.remove_absolute(recorder.log_path)

    assert_true(lines[-1].ends_with(" scenery left"), "the log ends where the scenery was left: %s" % lines[-1])


func test_a_mark_is_burnt_into_the_screenshot_along_its_edges() -> void:
    var annotator: ScreenshotAnnotator = autofree(ScreenshotAnnotator.new())
    var image: Image = Image.create_empty(IMAGE_SIZE.x, IMAGE_SIZE.y, false, Image.FORMAT_RGBA8)
    image.fill(Color.BLACK)
    annotator.set_image(image)

    annotator.add_mark(MARK)
    var rendered: Image = annotator.render_annotated_image()

    var corner: Vector2i = Vector2i(MARK.position)
    assert_eq(rendered.get_pixelv(corner), ScreenshotAnnotator.MARK_COLOR, "the mark's edge should be red")
    assert_eq(rendered.get_pixelv(Vector2i(MARK.get_center())), Color.BLACK, "the inside of the mark should stay")
    assert_eq(image.get_pixelv(corner), Color.BLACK, "the screenshot itself should stay unmarked")


func test_the_last_mark_can_be_taken_away() -> void:
    var annotator: ScreenshotAnnotator = autofree(ScreenshotAnnotator.new())
    var image: Image = Image.create_empty(IMAGE_SIZE.x, IMAGE_SIZE.y, false, Image.FORMAT_RGBA8)
    annotator.set_image(image)
    annotator.add_mark(MARK)

    annotator.remove_last_mark()

    assert_eq(annotator.render_annotated_image().get_pixelv(Vector2i(MARK.position)), Color(0, 0, 0, 0))


## A gameplay log line's fields
func _fields(line: String) -> PackedStringArray:
    return line.split(" ")
