@tool
extends ResourceFormatLoader
class_name FIZResourceLoader

## Makes `.fiz` files directly `load()`-able as a VehicleModel - the vehicle's properties
## and the components it is made of - the same way `.e3d` files are directly loadable as an
## E3DModel via E3DResourceFormatLoader, with no import step. Registered in libmaszyna.gd.
##
## Not a PackedScene any more: a component belongs to a vehicle rather than to a scene, so a .fiz
## describes a vehicle, not a tree of nodes.


func _get_recognized_extensions() -> PackedStringArray:
    return PackedStringArray(["fiz"])


func _handles_type(type: StringName) -> bool:
    return type == &"Resource"


func _get_resource_type(path: String) -> String:
    return "Resource" if path.get_extension().to_lower() == "fiz" else ""


func _load(path: String, original_path: String, use_sub_threads: bool, cache_mode: int) -> Variant:
    var model: VehicleModel = FizVehicleBuilder.build_model_at(path)
    if model == null:
        return ERR_CANT_OPEN
    return model
