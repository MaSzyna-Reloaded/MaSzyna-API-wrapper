@tool
extends Node

var _cache:ResourceCache = ResourceCache.create("mmd")
var _cabin_definitions:Dictionary = {}
var _random_choices:Dictionary = {}


func clear_cache() -> void:
    _cache.clear()
    _cabin_definitions.clear()
    _random_choices.clear()


func load_vehicle(data_path:String, file_name:String) -> MmdVehicleDefinition:
    var relative_path:String = data_path.path_join(file_name + ".mmd")
    var source_path:String = UserSettings.get_maszyna_game_dir().path_join(relative_path)
    var cache_hash:String = ("%s:%s" % [FileAccess.get_modified_time(source_path), source_path]).md5_text()
    var definition:MmdVehicleDefinition = _cache.get(relative_path + ".res", cache_hash) as MmdVehicleDefinition
    if definition:
        return definition
    if not FileAccess.file_exists(source_path):
        push_error("MMD file does not exist: %s" % source_path)
        return null

    definition = MmdVehicleDefinition.new()
    definition.body_model = MmdCabinInstancer.parse_body_model(source_path)
    definition.lowpoly_interior_model = MmdCabinInstancer.parse_lowpoly_interior_model(source_path)
    definition.passengers_model = MmdCabinInstancer.parse_passengers_model(source_path)
    definition.cabin_count = MmdCabinInstancer.parse_cabin_count(source_path)
    _cache.set(relative_path + ".res", definition, cache_hash)
    return _cache.get(relative_path + ".res") as MmdVehicleDefinition


func get_source_path(data_path:String, file_name:String) -> String:
    return UserSettings.get_maszyna_game_dir().path_join(data_path).path_join(file_name + ".mmd")


func load_cabin(data_path:String, file_name:String, cabin_number:int) -> MmdCabinDefinition:
    var source_path:String = get_source_path(data_path, file_name)
    var modified_time:int = FileAccess.get_modified_time(source_path)
    var cache_key:String = "%s:%d:%d" % [source_path, modified_time, cabin_number]
    var definition:MmdCabinDefinition = _cabin_definitions.get(cache_key)
    if definition:
        return definition
    if not FileAccess.file_exists(source_path):
        push_error("MMD file does not exist: %s" % source_path)
        return null

    var choices:Dictionary = _random_choices.get_or_add(source_path, {})
    definition = MmdCabinInstancer.parse(source_path, cabin_number, choices)
    _cabin_definitions[cache_key] = definition
    return definition
