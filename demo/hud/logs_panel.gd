extends PanelContainer

## The game's logs since the HUD came up, each in a tab of its own: "Console" - the game's messages
## (GameLog), "Gameplay" - the player's vehicle's commands and the scenario's events (the problem
## report's gameplay log, BugReportRecorder), "App" - the engine's own log, what the engine's file
## log (app.log) holds.

## The close button asks the owner of the View menu to hide the panel and untick its entry
signal close_requested

## The colours of the levels that stand out: debug faint, a warning in the HUD's orange
## (DrivingAidNextLimit), an error red
const DEBUG_COLOR: Color = Color(0.6, 0.66, 0.75, 1)
const WARNING_COLOR: Color = Color(1, 0.78, 0.35, 1)
const ERROR_COLOR: Color = Color(1, 0.45, 0.42, 1)
const LEVEL_COLORS: Dictionary[GameLog.LogLevel, Color] = {
    GameLog.LogLevel.DEBUG: DEBUG_COLOR,
    GameLog.LogLevel.INFO: LogLines.TEXT_COLOR,
    GameLog.LogLevel.WARNING: WARNING_COLOR,
    GameLog.LogLevel.ERROR: ERROR_COLOR,
}


## The engine's messages, handed to the "App" tab. The engine logs from any thread, so a line
## reaches the tab through the main thread's deferred call.
class AppLogger extends Logger:
    var _lines: LogLines


    func _init(lines: LogLines) -> void:
        _lines = lines


    func _log_message(message: String, error: bool) -> void:
        _lines.add_line.call_deferred(message.strip_edges(false, true), ERROR_COLOR if error else LogLines.TEXT_COLOR)


    func _log_error(function: String, file: String, line: int, code: String, rationale: String,
            _editor_notify: bool, error_type: int, _script_backtraces: Array[ScriptBacktrace]) -> void:
        var text: String = "%s (%s:%d, %s)" % [rationale if rationale else code, file, line, function]
        _lines.add_line.call_deferred(text, WARNING_COLOR if error_type == ERROR_TYPE_WARNING else ERROR_COLOR)


var _app_logger: AppLogger = null


func _ready() -> void:
    GameLog.log_updated.connect(_on_game_log_updated)
    _app_logger = AppLogger.new(%App)
    OS.add_logger(_app_logger)


func _exit_tree() -> void:
    GameLog.log_updated.disconnect(_on_game_log_updated)
    OS.remove_logger(_app_logger)


## A line the gameplay log has just written
func add_gameplay_line(line: String) -> void:
    %Gameplay.add_line(line)


func _on_game_log_updated(level: GameLog.LogLevel, line: String) -> void:
    %Console.add_line(line, LEVEL_COLORS[level])


func _on_close_button_pressed() -> void:
    close_requested.emit()
