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


## A resource has no tree to leave: the connection goes with the stream when it is freed
func _init() -> void:
    GameDataServer.data_unload_requested.connect(_on_data_unload_requested)


## The sound file is read again, from the game directory set now, when the stream is next played
func _on_data_unload_requested() -> void:
    _real_stream = null

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
