extends PanelContainer

## The game's logs since the HUD came up, each in a tab of its own: one for every logger of GameLog,
## named by its id, as it comes ("game" - the game's messages, "gameplay", "scenario" and "ai" - the
## game's log files, GamePlayLogRecorder), and "App" - the engine's own log, what the engine's
## file log (app.log) holds.

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


## A logger's tab
const LOG_LINES_SCENE: PackedScene = preload("log_lines.tscn")


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
## The tabs of GameLog's loggers, by logger id
var _logger_tabs: Dictionary[String, LogLines] = {}


func _ready() -> void:
    for logger_id: String in GameLog.get_loggers():
        _on_logger_created(logger_id)
    GameLog.logger_created.connect(_on_logger_created)
    GameLog.message_logged.connect(_on_message_logged)
    _app_logger = AppLogger.new(%App)
    OS.add_logger(_app_logger)


func _exit_tree() -> void:
    GameLog.logger_created.disconnect(_on_logger_created)
    GameLog.message_logged.disconnect(_on_message_logged)
    OS.remove_logger(_app_logger)


## A logger's tab, before the engine's one
func _on_logger_created(logger_id: String) -> void:
    var tab: LogLines = LOG_LINES_SCENE.instantiate()
    tab.name = logger_id
    %Tabs.add_child(tab)
    %Tabs.move_child(tab, %App.get_index())
    _logger_tabs[logger_id] = tab


func _on_message_logged(logger_id: String, level: GameLog.LogLevel, line: String) -> void:
    _logger_tabs[logger_id].add_line(line, LEVEL_COLORS[level])


func _on_close_button_pressed() -> void:
    close_requested.emit()
