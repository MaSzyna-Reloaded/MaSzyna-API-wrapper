@tool
extends ResourceFormatLoader
class_name E3DResourceFormatLoader

func _get_recognized_extensions() -> PackedStringArray:
    return ["e3d"]

func _handles_type(type: StringName) -> bool:
    return type == &"E3DModel" or type == &"Resource"

func _get_resource_type(path: String) -> String:
    if path.get_extension().to_lower() == "e3d":
        return "E3DModel"
    return ""

func _load(path: String, original_path: String = "", use_sub_threads: bool = false, cache_mode: int = 0) -> Resource:
    var file: FileAccess = FileAccess.open(path, FileAccess.READ)
    if not file:
        push_error("Failed to open E3D file: %s" % path)
        return null

    var model: E3DModel = E3DParser.parse(file)
    return model
