@tool
extends Node

## Loader/cache for fully-built RailVehicle3D vehicles, mirroring E3DModelManager's own
## ResourceCache pattern (addons/libmaszyna/e3d/e3d_model_manager.gd) one layer up.
##
## Reading a vehicle's .mmd is several passes over the same file - the body/lowpoly/passengers
## model names, the cab and the sound bank each do their own - and a scenery routinely places
## many vehicles sharing one data_path/file_name/skin (every wagon of one type in a consist,
## every signal of one type, ...). This turns an O(vehicle count) MMD parse into O(distinct
## data_path+file_name+skin combinations).
##
## What is cached is a VehicleStructure - what the MMD says the vehicle is built from - and not a
## node tree: a vehicle is a model plus a cab plus a physics handle, which is cheap to assemble
## and costly to pack. train_id/initial_velocity/driver_type/load/head_display_material are not in it
## at all, because they say which *instance* a vehicle is, and two wagons of the same type are
## still two vehicles.

var _cache = ResourceCache.create("rail_vehicle")


func clear_cache() -> void:
    _cache.clear()


func _make_cache_path(normalized_data_path:String, file_name:String, skin:String) -> String:
    return normalized_data_path.path_join("%s_%s.res" % [file_name, skin.md5_text()])


func _make_cache_hash(normalized_data_path:String, file_name:String) -> String:
    # Only the .mmd's own mtime is checked - not every .e3d/.fiz file it transitively
    # references - matching FizVehicleBuilder._make_cache_hash()'s same simplification
    # for FIZ `include`s.
    var abs_mmd_path:String = (
            UserSettings.get_maszyna_game_dir().path_join(normalized_data_path).path_join(file_name + ".mmd"))
    # The hash cannot see changes to MaszynaRailVehicle3DInstancer's own code - bump this tag
    # whenever that code changes the cached structure. v6: MaSzyna->Godot vehicle-frame
    # conversion applied to every vehicle model and the cab. v12: LowPolyInterior carries no
    # instancer in the template (RailVehicle3D switches it with the distance). v13: the skins
    # of the models are resolved into texture-only slots too (MmdCabinInstancer.resolve_skins).
    # v14: bumped on request together with the E186 cab work, the structure itself is unchanged.
    # v18: the cache holds a VehicleStructure - what the MMD says the vehicle is built from -
    # instead of a PackedScene of the vehicle's nodes. v19: it carries the whole `loads:` block,
    # so a vehicle can be drawn with the cargo the scenery gave it. v19: spring brake, line breaker and cab gauge
    # fixes of 2026-09-24 (catalog entries, component defaults) - bumped on request.
    return ("structure-v19:%s:%s" % [FileAccess.get_modified_time(abs_mmd_path), abs_mmd_path]).md5_text()


## Loads a fully wired RailVehicle3D (not yet track-placed, not yet parented under a
## DynamicRailVehicle3D). Returns null if data_path/file_name are missing. The only way in:
## every vehicle of a scenery comes through here, so every one of them shares the cache.
func load(
        data_path:String, file_name:String, skin:String, train_id:String,
        initial_velocity:float, head_display_material:Material,
        driver_type:VehicleController.DriverType = VehicleController.DRIVER_NOBODY,
        load_name:String = "", load_amount:float = 0.0) -> RailVehicle3D:
    if not data_path or not file_name:
        return null

    var normalized_data_path:String = data_path if data_path.begins_with("/") else "/" + data_path
    var cache_path:String = _make_cache_path(normalized_data_path, file_name, skin)
    var cache_hash:String = _make_cache_hash(normalized_data_path, file_name)

    var structure:VehicleStructure = _cache.get(cache_path, cache_hash) as VehicleStructure
    if not structure:
        structure = MaszynaRailVehicle3DInstancer.read_structure(data_path, file_name, skin)
        if not structure:
            return null
        _cache.set(cache_path, structure, cache_hash)

    var vehicle:RailVehicle3D = MaszynaRailVehicle3DInstancer.build_from_structure(
            structure, train_id, initial_velocity, driver_type, load_name, load_amount)
    MaszynaRailVehicle3DInstancer.initialize_instance(vehicle, structure, head_display_material)
    return vehicle
