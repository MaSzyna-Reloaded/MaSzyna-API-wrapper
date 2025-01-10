@tool
extends AudioStream
class_name MaszynaAudioStream

@export var file_path:String = "":
    set(x):
        if not file_path == x:
            file_path = x
            _real_stream = null

@export var loop:bool = false:
    set(x):
        if not loop == x:
            loop = x
            _real_stream = null

var _real_stream:AudioStream

func _get_stream_name() -> String:
    return file_path

func _get_length() -> float:
    return _real_stream.get_length() if _real_stream else 0.0

func _instantiate_playback() -> AudioStreamPlayback:
    if file_path and not _real_stream:
        _real_stream = AudioStreamManager.get_stream(file_path, loop)

    if _real_stream:
        return _real_stream.instantiate_playback()
    else:
        return null
