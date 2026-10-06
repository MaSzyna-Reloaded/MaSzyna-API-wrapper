@tool
extends RefCounted
class_name MaszynaVehiclesBank

## Every vehicle of the game's dynamic/ directory with its skins, as the launcher of the original
## gathers them: from the textures.txt of each vehicle directory (vehicles_bank::scan_textures(),
## launcher/textures_scanner.cpp:5). A vehicle is a directory and the name of its .fiz/.mmd, both
## lower case as the launcher keeps them (get_vehicle(), :247); a directory without a textures.txt
## has no vehicles here, as it has none in the launcher.

const DYNAMIC_DIR: String = "dynamic"
const INDEX_FILE: String = "textures.txt"


class Vehicle:
    ## The vehicle's directory as a `dynamic` names it ("/dynamic/pkp/en57_v1")
    var data_path: String
    var file_name: String
    ## In the order of the index
    var skins: Array[String] = []

    func _init(p_data_path: String, p_file_name: String) -> void:
        data_path = p_data_path
        file_name = p_file_name


## The vehicles under game_dir (an absolute path), by directory and then by name
static func scan(game_dir: String) -> Array[Vehicle]:
    var by_path: Dictionary[String, Vehicle] = {}
    var dynamic_dir: String = MaszynaDataPath.resolve(game_dir, DYNAMIC_DIR)
    var directories: Array[String] = [dynamic_dir]
    while directories:
        var directory: String = directories.pop_back()
        var absolute_dir: String = game_dir.path_join(directory)
        for subdirectory: String in DirAccess.get_directories_at(absolute_dir):
            directories.append(directory.path_join(subdirectory))
        for file: String in DirAccess.get_files_at(absolute_dir):
            if not file.to_lower() == INDEX_FILE:
                continue
            var data_path: String = "/" + DYNAMIC_DIR.path_join(directory.trim_prefix(dynamic_dir).trim_prefix("/"))
            # the descriptions are cp1250; file and vehicle names, all that is read here, are ASCII
            var lines: PackedStringArray = FileAccess.get_file_as_bytes(
                    absolute_dir.path_join(file)).get_string_from_ascii().split("\n")
            for line: String in lines:
                var entry: PackedStringArray = MaszynaVehicleSkins.parse_skin_line(line)
                if not entry:
                    continue
                var key: String = data_path.path_join(entry[0].to_lower())
                if not by_path.has(key):
                    by_path[key] = Vehicle.new(data_path, entry[0].to_lower())
                if not entry[1] in by_path[key].skins:
                    by_path[key].skins.append(entry[1])
    var keys: Array[String] = []
    keys.assign(by_path.keys())
    keys.sort()
    var vehicles: Array[Vehicle] = []
    for key: String in keys:
        vehicles.append(by_path[key])
    return vehicles
