@tool
extends ResourceFormatLoader
class_name FIZResourceLoader

## Makes `.fiz` files directly `load()`-able as a PackedScene (a TrainController + typed
## TrainPart children tree), the same way `.e3d` files are directly loadable as an E3DModel
## via E3DResourceFormatLoader - no import step. Registered in libmaszyna.gd.


func _get_recognized_extensions() -> PackedStringArray:
    return PackedStringArray(["fiz"])


func _handles_type(type: StringName) -> bool:
    return type == &"PackedScene"


func _get_resource_type(path: String) -> String:
    return "PackedScene" if path.get_extension().to_lower() == "fiz" else ""


func _load(path: String, original_path: String, use_sub_threads: bool, cache_mode: int) -> Variant:
    var scene: PackedScene = FizTrainControllerInstancer.build_scene(path)
    if scene == null:
        return ERR_CANT_OPEN
    return scene
