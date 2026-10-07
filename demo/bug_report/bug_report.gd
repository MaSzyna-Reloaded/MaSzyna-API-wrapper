extends CanvasLayer

## Reporting a problem or a suggestion from the scenery selector or a running scenery, as a GitHub
## issue through the reporting endpoint (BugReportSender). The top bar's entry and the button at
## the right edge both open it; the game shows that button wherever a report can be made
## (demo_scenery_loading.gd). The report pauses the simulation while its form is open and resumes
## it on closing, unless it was paused already. It is the one owner of that state - the form and
## its buttons only ask (bug_report.tscn). What happened before the report was opened is in the
## logs it attaches: the engine's (app.log) and the game's log files of the running scenery
## (attach_world()).

## Characters of the description that go into the issue's title
const TITLE_DESCRIPTION_LENGTH: int = 80
## The engine's file log of this session, and how much of its end goes with a report
const FILE_LOGGING_SETTING: String = "debug/file_logging/enable_file_logging"
const LOG_PATH_SETTING: String = "debug/file_logging/log_path"
const LOG_TAIL_BYTES: int = 4 * 1024 * 1024
## How much of the end of each of the game's log files goes with a report - they compress well
const GAME_LOG_TAIL_BYTES: int = 50 * 1024 * 1024

## The running scenery, null in the menu
var _world: SceneryWorld = null
## When the scenery started, Time.get_ticks_msec()
var _started_msec: int = 0
## The game's log files of the running scenery (GameLogFileHandler), empty in the menu
var _log_files: PackedStringArray = []
## The simulation was running when the report opened, so closing it resumes it
var _paused_by_report: bool = false
## The state read when the report opened
var _snapshot: Dictionary = {}
## The issue the last sent report became, opened from the confirmation
var _issue_url: String = ""


## The scenery being started and its log files, null and none when it is left
func attach_world(world: SceneryWorld, log_files: PackedStringArray) -> void:
    _world = world
    _log_files = log_files
    _started_msec = Time.get_ticks_msec()


## With no reporting endpoint (BugReportSender.is_available()) a notice says so instead of a form
## that would only pretend to send
func open_report() -> void:
    if not BugReportSender.is_available():
        %UnavailableDialog.ask()
        return
    _paused_by_report = not SimulationServer.simulation_is_paused()
    SimulationServer.simulation_pause()
    # the screenshot is the frame the player saw, without the edge button that opened the report
    %EdgeButton.retract()
    await RenderingServer.frame_post_draw
    var screenshot: Image = get_viewport().get_texture().get_image()
    _snapshot = BugReportSnapshot.collect(_world, _started_msec)
    %ReportDialog.begin(screenshot)


## The form and the confirmation of a sent report are gone, the simulation resumes
func close_report() -> void:
    %ReportDialog.hide()
    if _paused_by_report:
        SimulationServer.simulation_unpause()
    _paused_by_report = false


func send_report() -> void:
    var scenario: Dictionary = _snapshot["scenario"]
    var description: String = %ReportDialog.get_description()
    var vehicle: String = scenario["vehicle"]
    # "[build] scenery / vehicle: the description's first line", without what the selector lacks
    var subject: PackedStringArray = []
    for part: String in [scenario.get("title", ""), vehicle]:
        if part:
            subject.append(part)
    var title: String = "[%s] " % GameDataServer.build_get_number()
    if subject:
        title += " / ".join(subject) + ": "
    var report: Dictionary = {
        "title": title + description.get_slice("\n", 0).left(TITLE_DESCRIPTION_LENGTH),
        "description": description,
        "build": GameDataServer.build_get_number(),
        "scenery": scenario.get("filename", ""),
        "vehicle": vehicle,
    }
    # the log of this session, its last part - only while it is written: with file logging off the
    # file is an earlier session's (EngineOverrides, the Debug settings), and there are no game logs
    var logs: Dictionary[String, PackedByteArray] = {}
    if ProjectSettings.get_setting(FILE_LOGGING_SETTING, false):
        var session_log: String = ProjectSettings.get_setting(LOG_PATH_SETTING)
        logs[session_log.get_file()] = _read_tail(session_log, LOG_TAIL_BYTES)
    for log_file: String in _log_files:
        logs[log_file.get_file()] = _read_tail(log_file, GAME_LOG_TAIL_BYTES)
    %ReportDialog.show_sending()
    %Sender.send(report, JSON.stringify(_snapshot, "\t"), %ReportDialog.render_screenshot(), logs)


## The last `bytes` of a file, nothing when it cannot be read
func _read_tail(path: String, bytes: int) -> PackedByteArray:
    var file: FileAccess = FileAccess.open(path, FileAccess.READ)
    if not file:
        return PackedByteArray()
    var length: int = file.get_length()
    file.seek(maxi(length - bytes, 0))
    return file.get_buffer(mini(length, bytes))


## Sent: a confirmation that can open the issue, and the simulation stays paused until it is closed;
## an endpoint that gives no link just closes the form
func _on_report_sent(issue_url: String) -> void:
    if not issue_url:
        close_report()
        return
    %ReportDialog.hide()
    _issue_url = issue_url
    %SentDialog.ask()


## The button at the right edge, in the selector and in a running scenery
func show_edge_button() -> void:
    %EdgeButton.visible = true


## ...and none while a scenery loads or unloads
func hide_edge_button() -> void:
    %EdgeButton.retract()
    %EdgeButton.visible = false


func _on_sent_dialog_alternative_chosen() -> void:
    OS.shell_open(_issue_url)
    close_report()
