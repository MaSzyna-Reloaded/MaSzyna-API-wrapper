extends BaseCabinTool3D
class_name CabinHasler

@export var sound_event : StringName = ""
@export var speed_parameter : StringName = "speed"

var _cabin:Cabin3D
var _sfx:SfxPlayer3D

func _ready() -> void:
    _cabin = get_cabin()
    if not _cabin:
        return
    _sfx = SfxPlayer3D.new()
    add_child(_sfx)
    _sfx.bank = _cabin.sound_bank
        
func _exit_tree() -> void:
    if _sfx:
        remove_child(_sfx)
        _sfx = null
    
func _process_tool(delta):
    if not _cabin or not _sfx or not _controller:
        return
    var speed = _controller.state.get("speed", 0.0)
    if speed > 0.1 and not _sfx.is_playing(sound_event):
        _sfx.play(sound_event, 0.0, {speed_parameter: speed})
    elif speed <= 0.1 and _sfx.is_playing(sound_event):
        _sfx.stop(sound_event, 0.0)
    elif _sfx.is_playing(sound_event):
        _sfx.modulate(sound_event, {speed_parameter: speed})
