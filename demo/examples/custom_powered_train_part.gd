extends Control

func log_to_godot(level, line):
    if level == GameLog.INFO or level == GameLog.DEBUG:
        print(line)
    elif level == GameLog.WARNING:
        push_warning(line)
    elif level == GameLog.ERROR:
        push_error(line)

func _ready():
    GameLog.log_updated.connect(log_to_godot)
