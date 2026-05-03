@tool
extends ResourceFormatLoader
class_name OggVorbisFormatLoader

func _get_recognized_extensions():
    return ["ogg"]

func _handles_type(type):
    return type == "Resource"

func _get_resource_type(path: String):
    return "Resource"

func _load(path: String, original_path: String = "", use_sub_threads: bool = false, cache_mode: int = 0) -> Resource:
    var file = FileAccess.open(path, FileAccess.READ)
    if not file:
        push_error("Failed to open OGG file: %s" % path)
        return null

    var ogg:AudioStreamOggVorbis = AudioStreamOggVorbis.load_from_file(path)
    return ogg
