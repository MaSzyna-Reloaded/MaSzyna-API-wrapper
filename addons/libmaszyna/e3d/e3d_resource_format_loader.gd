@tool
extends ResourceFormatLoader
class_name E3DResourceFormatLoader

var parser = load("res://addons/libmaszyna/e3d/e3d_parser.gd").new()

func _get_recognized_extensions():
    return ["e3d"]

func _handles_type(type):
    return type == "Resource"

func _get_resource_type(path: String):
    return "Resource"

func _load(path: String, original_path: String = "", use_sub_threads: bool = false, cache_mode: int = 0) -> Resource:
    var file = FileAccess.open(path, FileAccess.READ)
    if not file:
        push_error("Failed to open E3D file: %s" % path)
        return null

    var model = parser.parse(file)
    return model
