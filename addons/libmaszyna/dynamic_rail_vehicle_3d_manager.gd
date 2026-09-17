@tool
extends Node

## Loader/cache for fully-built RailVehicle3D vehicles, mirroring E3DModelManager's own
## ResourceCache pattern (addons/libmaszyna/e3d/e3d_model_manager.gd) one layer up.
##
## MaszynaRailVehicle3DInstancer.build() re-reads and re-parses the vehicle's .mmd (multiple
## times - body/lowpoly/passengers model names and the sound bank each do their own pass) and
## .fiz files from scratch every time it runs. A scenery routinely places many vehicles that
## share the same data_path/file_name/skin (every wagon of one type in a consist, every signal
## of one type, ...), so this turns an O(vehicle count) MMD/FIZ parse into O(distinct
## data_path+file_name+skin combinations): build() below caches the structural result (exterior
## model + FIZ controller + cabin scene + sound bank) as a PackedScene and instantiate()s it -
## the same "cache a Resource, not the live Node" approach FizTrainControllerInstancer.build()
## already uses for the FIZTrainController layer.
##
## train_id/initial_velocity/head_display_material are deliberately NOT part of the cached
## template (built with neutral placeholder values instead) - they vary per vehicle instance
## even when data_path/file_name/skin are identical (two wagons of the same type still need
## distinct TrainSystem ids), so they're re-applied on every instantiate()'d copy instead.

var _cache = ResourceCache.create("rail_vehicle")


func _make_cache_path(normalized_data_path:String, file_name:String, skin:String) -> String:
    return normalized_data_path.path_join("%s_%s.res" % [file_name, skin.md5_text()])


func _make_cache_hash(normalized_data_path:String, file_name:String) -> String:
    # Only the .mmd's own mtime is checked - not every .e3d/.fiz file it transitively
    # references - matching FizTrainControllerInstancer._make_cache_hash()'s same simplification
    # for FIZ `include`s.
    var abs_mmd_path:String = (
            UserSettings.get_maszyna_game_dir().path_join(normalized_data_path).path_join(file_name + ".mmd"))
    # Invalidate templates created before vehicle SFX players received 16 voices.
    return ("structure-v4:%s:%s" % [FileAccess.get_modified_time(abs_mmd_path), abs_mmd_path]).md5_text()


## Loads a fully wired RailVehicle3D (not yet track-placed, not yet parented under a
## DynamicRailVehicle3D). Returns null if data_path/file_name are missing. Drop-in replacement
## for MaszynaRailVehicle3DInstancer.build() - same signature, cached.
func load(
        data_path:String, file_name:String, skin:String, train_id:String,
        initial_velocity:float, head_display_material:Material) -> RailVehicle3D:
    if not data_path or not file_name:
        return null

    var normalized_data_path:String = data_path if data_path.begins_with("/") else "/" + data_path
    var cache_path:String = _make_cache_path(normalized_data_path, file_name, skin)
    var cache_hash:String = _make_cache_hash(normalized_data_path, file_name)

    var scene:PackedScene = _cache.get(cache_path, cache_hash) as PackedScene
    if not scene:
        var template:RailVehicle3D = MaszynaRailVehicle3DInstancer._build_structure(
                data_path, file_name, skin, "", 0.0)
        if not template:
            return null
        # PackedScene.pack() only includes nodes whose `owner` is set (see
        # MaszynaRailVehicle3DInstancer._build_cabin_scene()'s own comment on the same trick) -
        # build() parents model/fiz_controller/etc. under `template` without ever setting their
        # owner, since the un-cached code path returns this live tree directly and never packs it.
        _set_owner_recursive(template, template)
        scene = PackedScene.new()
        var err:Error = scene.pack(template)
        template.free()
        if err != OK:
            push_error("DynamicRailVehicle3DManager: could not pack vehicle scene for %s/%s" % [normalized_data_path, file_name])
            return null
        _cache.set(cache_path, scene, cache_hash)

    var vehicle:RailVehicle3D = scene.instantiate() as RailVehicle3D
    var fiz_controller:FIZTrainController = vehicle.get_node("FIZTrainController") as FIZTrainController
    if fiz_controller:
        fiz_controller.train_id = train_id
        fiz_controller.initial_velocity = initial_velocity
    MaszynaRailVehicle3DInstancer._initialize_instance(vehicle, file_name, head_display_material)
    return vehicle


func _set_owner_recursive(node:Node, new_owner:Node) -> void:
    for child:Node in node.get_children(true):
        child.owner = new_owner
        _set_owner_recursive(child, new_owner)
