@tool
extends Node

func get_stream(name:String, loop:bool = false) -> AudioStream:
    var project_data_dir = UserSettings.get_maszyna_game_dir()
    var full_path = "%s/sounds/%s.ogg" % [project_data_dir, name.to_lower()]
    var stream:AudioStreamOggVorbis = load(full_path)  # uses godot's builtin resource cache
    if stream:
        if not stream.loop == loop:
            stream = stream.duplicate(0)
            stream.loop = loop
    else:
        push_error("[%s] file does not exists: %s" % [self, full_path])
    return stream
