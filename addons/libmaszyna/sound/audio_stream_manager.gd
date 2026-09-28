@tool
extends Node

## A missing sound plays this instead: a vehicle whose data lacks a file stays quiet there rather
## than failing, as the original only logs it (audio.cpp:147). A tenth of a second of 16-bit mono.
const SILENCE_MIX_RATE:int = 44100
const SILENCE_BYTES:int = 8820

var _silence:AudioStreamWAV = AudioStreamWAV.new()


func _init() -> void:
    _silence.format = AudioStreamWAV.FORMAT_16_BITS
    _silence.mix_rate = SILENCE_MIX_RATE
    var data:PackedByteArray = PackedByteArray()
    data.resize(SILENCE_BYTES)
    _silence.data = data


func get_stream(name:String, loop:bool = false) -> AudioStream:
    var project_data_dir = UserSettings.get_maszyna_game_dir()
    var full_path = "%s/sounds/%s.ogg" % [project_data_dir, name.to_lower()]
    if not ResourceLoader.exists(full_path):
        push_warning("[%s] file does not exist: %s" % [self, full_path])
        return _silence
    var stream:AudioStreamOggVorbis = load(full_path)  # uses godot's builtin resource cache
    if stream and not stream.loop == loop:
        stream = stream.duplicate(0)
        stream.loop = loop
    return stream
