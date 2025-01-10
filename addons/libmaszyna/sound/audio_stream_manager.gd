@tool
extends Node

var _streams = {}

func clear_cache():
    _streams.clear()

func _ready():
    UserSettings.config_changed.connect(clear_cache)
    UserSettings.cache_clear_requested.connect(clear_cache)

func get_stream(name:String, loop:bool = false) -> AudioStream:
    var stream_key = "%s:%s" % [name, loop]
    if not stream_key in _streams:
        var project_data_dir = UserSettings.get_maszyna_game_dir()
        var full_path = "%s/sounds/%s.ogg" % [project_data_dir, name.to_lower()]
        if FileAccess.file_exists(full_path):
            var stream:AudioStreamOggVorbis = AudioStreamOggVorbis.load_from_file(full_path)
            if stream:
                stream.loop = loop
                _streams[stream_key] = stream
        else:
            push_error("[%s] file does not exists: %s" % [self, full_path])
    return _streams.get(stream_key)
