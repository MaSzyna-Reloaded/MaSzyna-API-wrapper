extends Control

func log_to_godot(level, line):
    if level == LogSystem.INFO or level == LogSystem.DEBUG:
        print(line)
    elif level == LogSystem.WARNING:
        push_warning(line)
    elif level == LogSystem.ERROR:
        push_error(line)

func _ready():
    LogSystem.log_updated.connect(log_to_godot)
