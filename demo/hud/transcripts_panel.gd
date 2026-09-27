extends PanelContainer

## What the sounds heard say, at the bottom of the screen while there is anything to read and the
## transcripts are shown - the original's transcripts panel (transcripts_panel::update(),
## driveruipanels.cpp:1594; gui.showtranscripts, Globals.cpp:775)

var _shown:bool = true


func _ready() -> void:
    TranscriptSystem.lines_changed.connect(_show_lines)


func _exit_tree() -> void:
    TranscriptSystem.lines_changed.disconnect(_show_lines)


func set_shown(shown:bool) -> void:
    _shown = shown
    _show_lines()


func _show_lines() -> void:
    var lines:PackedStringArray = TranscriptSystem.get_lines()
    %Lines.text = "\n".join(lines)
    visible = _shown and lines.size() > 0
    reset_size()
