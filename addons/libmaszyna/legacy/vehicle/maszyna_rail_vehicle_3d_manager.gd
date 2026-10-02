@tool
extends Node

## Builds MaSzyna vehicles into their RailVehicle3D nodes from a cached structure, mirroring E3DModelManager's own
## ResourceCache pattern (addons/libmaszyna/legacy/e3d/e3d_model_manager.gd) one layer up.
##
## Reading a vehicle's .mmd is several passes over the same file - the body/lowpoly/passengers
## model names, the cab and the sound bank each do their own - and a scenery routinely places
## many vehicles sharing one data_path/file_name/skin (every wagon of one type in a trainset,
## every signal of one type, ...). This turns an O(vehicle count) MMD parse into O(distinct
## data_path+file_name+skin combinations).
##
## What is cached is a MaszynaVehicleStructure - what the MMD says the vehicle is built from - and not a
## node tree: a vehicle is a model plus a cab plus a physics handle, which is cheap to assemble
## and costly to pack. vehicle_id/initial_velocity/driver_type/load/head_display_material are not in it
## at all, because they say which *instance* a vehicle is, and two wagons of the same type are
## still two vehicles.

## Time the vehicle builds may take per frame; a vehicle that started is finished, so a frame
## builds at least one. Built all in one frame, a scenery's vehicles stalled the loading screen; at
## 8 ms - less than one vehicle takes - every frame built one, and a frame of the loading screen
## cost the scenery's drawing on top of each: Wrzosy's 561 vehicles took 370 frames
## (docs/findings-archive.md, 2026-10-03 hundreds of vehicles). A frame at 30 fps.
const BUILD_BUDGET_MSEC:int = 33

var _cache = ResourceCache.create("rail_vehicle")
## Vehicles waiting for their build, in the order they asked; processed while it is not empty
var _build_queue:Array[MaszynaRailVehicle3D] = []


## The vehicle is built in its turn, within the per-frame budget
func build_request(vehicle:MaszynaRailVehicle3D) -> void:
    if _build_queue.has(vehicle):
        return
    _build_queue.append(vehicle)
    if _build_queue.size() == 1:
        get_tree().process_frame.connect(_on_process_frame)


## A vehicle leaving the tree is not built
func build_cancel(vehicle:MaszynaRailVehicle3D) -> void:
    if not _build_queue.has(vehicle):
        return
    _build_queue.erase(vehicle)
    if not _build_queue:
        get_tree().process_frame.disconnect(_on_process_frame)


func _on_process_frame() -> void:
    var deadline:int = Time.get_ticks_msec() + BUILD_BUDGET_MSEC
    while _build_queue and Time.get_ticks_msec() < deadline:
        _build_queue.pop_front().build()
    if not _build_queue:
        get_tree().process_frame.disconnect(_on_process_frame)


func clear_cache() -> void:
    _cache.clear()


## The structure of a vehicle type and skin; of the vehicle itself when its MMD names (p1), the
## vehicle's own name (DynObj.cpp:5263)
func _make_cache_path(
        normalized_data_path:String, file_name:String, skin:String, vehicle_name:String,
        abs_mmd_path:String) -> String:
    var variant:String = skin
    if MmdCabinInstancer.names_vehicle(abs_mmd_path):
        variant += "|" + vehicle_name
    return normalized_data_path.path_join("%s_%s.res" % [file_name, variant.md5_text()])


func _make_cache_hash(abs_mmd_path:String) -> String:
    # Only the .mmd's own mtime is checked - not every .e3d/.fiz file it transitively
    # references - matching FizVehicleBuilder._make_cache_hash()'s same simplification
    # for FIZ `include`s.
    # The hash cannot see changes to MaszynaRailVehicle3DInstancer's own code - bump this tag
    # whenever that code changes the cached structure. v6: MaSzyna->Godot vehicle-frame
    # conversion applied to every vehicle model and the cab. v12: LowPolyInterior carries no
    # instancer in the template (RailVehicle3D switches it with the distance). v13: the skins
    # of the models are resolved into texture-only slots too (MmdCabinInstancer.resolve_skins).
    # v14: bumped on request together with the E186 cab work, the structure itself is unchanged.
    # v18: the cache holds a MaszynaVehicleStructure - what the MMD says the vehicle is built from -
    # instead of a PackedScene of the vehicle's nodes. v19: it carries the whole `loads:` block,
    # so a vehicle can be drawn with the cargo the scenery gave it. v19: spring brake, line breaker and cab gauge
    # fixes of 2026-09-24 (catalog entries, component defaults) - bumped on request. v21: the
    # mirror submodels of `animmirrorprefix:` and the cab's mirrors_sw. v23: the models and the
    # submodels that move are a RailVehicleAppearance, the cab carries the vehicle frame. v24: the
    # MMD is read with the vehicle's (p1)-(p3), SN61's body model was "none"; attachments.
    return ("structure-v24:%s:%s" % [FileAccess.get_modified_time(abs_mmd_path), abs_mmd_path]).md5_text()


## Builds the vehicle of data_path/file_name/skin into `vehicle` (in the tree), returning its
## parts; none when data_path/file_name are missing. The only way in: every vehicle of a scenery
## comes through here, so every one of them shares the cache.
func build_into(
        vehicle:RailVehicle3D, data_path:String, file_name:String, skin:String, vehicle_id:String,
        initial_velocity:float, driver_type:VehicleController.DriverType = VehicleController.DRIVER_NOBODY,
        load_name:String = "", load_amount:float = 0.0) -> Array[Node]:
    var parts:Array[Node] = []
    if not data_path or not file_name:
        return parts

    var normalized_data_path:String = data_path if data_path.begins_with("/") else "/" + data_path
    var game_dir:String = UserSettings.get_maszyna_game_dir()
    var relative_path:String = normalized_data_path.trim_prefix("/").path_join(file_name + ".mmd")
    var abs_mmd_path:String = game_dir.path_join(MaszynaDataPath.resolve(game_dir, relative_path))
    var cache_path:String = _make_cache_path(normalized_data_path, file_name, skin, vehicle_id, abs_mmd_path)
    var cache_hash:String = _make_cache_hash(abs_mmd_path)

    var structure:MaszynaVehicleStructure = _cache.get(cache_path, cache_hash) as MaszynaVehicleStructure
    if not structure:
        structure = MaszynaRailVehicle3DInstancer.read_structure(data_path, file_name, skin, vehicle_id)
        if not structure:
            return parts
        _cache.set(cache_path, structure, cache_hash)

    return MaszynaRailVehicle3DInstancer.build_into(
            vehicle, structure, skin, vehicle_id, initial_velocity, driver_type, load_name, load_amount)
