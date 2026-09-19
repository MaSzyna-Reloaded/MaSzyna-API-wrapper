@tool
extends RefCounted
class_name MaszynaSceneryInfo

## Scenario description from the header comments of a .scn file (read by the original starter):
## [code]//$n[/code] title, [code]//$d[/code] description lines, [code]//$i[/code] image in
## scenery/images/, plus the consists of its [code]trainset[/code] blocks. Files are in cp1250.
##
## Read line by line: the tokens of a "trainset"/"node ... dynamic" are written on one line in
## every scenery, and the mission description of a consist is in [code]//$o[/code] comments,
## which a tokenizer would drop.

## One vehicle of a consist ("node ... dynamic ... enddynamic")
class Vehicle:
    ## Node name of the vehicle - DynamicRailVehicle3D.train_id
    var train_id:String = ""
    ## e.g. "dynamic/pkp/su42_v1"
    var data_path:String = ""
    var skin:String = ""
    ## Base name of the vehicle's .mmd/.fiz files
    var file_name:String = ""
    ## "headdriver" for the vehicle the player starts in, "reardriver", "passenger", ...
    var driver_type:String = ""


## One "trainset ... endtrainset" block
class Trainset:
    var name:String = ""
    var track:String = ""
    ## Mission description of the consist (//$o lines)
    var description:String = ""
    var vehicles:Array[Vehicle] = []

    ## Vehicle the player starts in: the one with a headdriver, else any with a driver
    func get_driver_train_id() -> String:
        for vehicle:Vehicle in vehicles:
            if vehicle.driver_type == "headdriver":
                return vehicle.train_id
        for vehicle:Vehicle in vehicles:
            if vehicle.driver_type == "reardriver":
                return vehicle.train_id
        return vehicles[0].train_id if vehicles else ""

## Only the beginning of a scenery is read - the header and the consists are there
const MAX_BYTES:int = 262144
## The title alone is in the first lines of the file
const MAX_HEADER_BYTES:int = 8192
## Unicode code points of cp1250 bytes 0x80-0xFF (U+FFFD for undefined bytes)
const CP1250_HIGH:PackedInt32Array = [
    0x20AC, 0xFFFD, 0x201A, 0xFFFD, 0x201E, 0x2026, 0x2020, 0x2021,
    0xFFFD, 0x2030, 0x0160, 0x2039, 0x015A, 0x0164, 0x017D, 0x0179,
    0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0xFFFD, 0x2122, 0x0161, 0x203A, 0x015B, 0x0165, 0x017E, 0x017A,
    0x00A0, 0x02C7, 0x02D8, 0x0141, 0x00A4, 0x0104, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x015E, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x017B,
    0x00B0, 0x00B1, 0x02DB, 0x0142, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x0105, 0x015F, 0x00BB, 0x013D, 0x02DD, 0x013E, 0x017C,
    0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7,
    0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
    0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7,
    0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
    0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7,
    0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
    0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7,
    0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9,
]

var title:String = ""
var description:String = ""
## Absolute path of the scenario image, empty when it does not exist
var image_path:String = ""
var trainsets:Array[Trainset] = []


## Name of scenery/<filename> for a list of sceneries: the scenery of its "//$n" line and the
## file name, which is what tells apart the dozen scenarios sharing one scenery
## ("stary_jawor_eszelon.scn" with "//$n Stary Jawor" -> "Stary Jawor - Eszelon").
static func read_display_name(filename:String) -> String:
    var scenery_name:String = _read_scenery_name(filename)
    var file_name:String = humanize_file_name(filename.get_basename())
    if not scenery_name:
        return file_name

    # what the file name adds to the name of the scenery: "braniewo_szeroki" of "Wojskowy Rejon
    # Przeladunkowy - Braniewo" is its "Szeroki" variant, the rest is already in the name
    var known_words:PackedStringArray = _simplify(scenery_name).split(" ", false)
    var scenario_words:PackedStringArray = []
    for word:String in file_name.split(" ", false):
        if not known_words.has(_simplify(word)):
            scenario_words.append(word)
    if not scenario_words:
        return scenery_name
    return "%s - %s" % [scenery_name, " ".join(scenario_words)]


## Lowercased and without the Polish letters, for comparing words of a name with a file name
static func _simplify(text:String) -> String:
    var simplified:String = text.to_lower()
    for replacement:Array in [
        ["ą", "a"], ["ć", "c"], ["ę", "e"], ["ł", "l"], ["ń", "n"],
        ["ó", "o"], ["ś", "s"], ["ź", "z"], ["ż", "z"], ["-", " "],
    ]:
        simplified = simplified.replace(replacement[0], replacement[1])
    return simplified


## "stary_jawor_noc" -> "Stary Jawor Noc", "calkowo_sm42_v2" -> "Calkowo SM42 V2" (a word with
## a digit in it is a vehicle or a version, those are written in capitals)
static func humanize_file_name(file_name:String) -> String:
    var words:PackedStringArray = []
    for word:String in file_name.replace("_", " ").replace("-", " ").split(" ", false):
        words.append(word.to_upper() if _has_digit(word) else word.capitalize())
    return " ".join(words)


static func _has_digit(word:String) -> bool:
    for character:String in word:
        if character.is_valid_int():
            return true
    return false


## The "//$n" line of the header, empty when the scenery has none
static func _read_scenery_name(filename:String) -> String:
    var scenery_dir:String = UserSettings.get_maszyna_game_dir().path_join("scenery")
    var file:FileAccess = FileAccess.open(scenery_dir.path_join(filename), FileAccess.READ)
    if not file:
        return ""
    var bytes:PackedByteArray = file.get_buffer(mini(file.get_length(), MAX_HEADER_BYTES))
    var start:int = 0
    while start < bytes.size():
        var end:int = bytes.find(10, start)  # "\n"
        if end < 0:
            end = bytes.size()
        var line:String = decode_cp1250(bytes.slice(start, end)).strip_edges()
        start = end + 1
        if line.begins_with("//$n"):
            return line.substr(4).strip_edges()
        if line and not line.begins_with("//"):
            break
    return ""


## Reads the header of scenery/<filename> and the consists declared in it
static func read(filename:String) -> MaszynaSceneryInfo:
    var info := MaszynaSceneryInfo.new()
    var scenery_dir:String = UserSettings.get_maszyna_game_dir().path_join("scenery")
    var file:FileAccess = FileAccess.open(scenery_dir.path_join(filename), FileAccess.READ)
    if not file:
        return info
    # raw bytes - FileAccess.get_line() decodes UTF-8 and would lose the cp1250 characters
    var bytes:PackedByteArray = file.get_buffer(mini(file.get_length(), MAX_BYTES))
    var description_lines:PackedStringArray = []
    var mission_lines:PackedStringArray = []
    var in_header:bool = true
    var trainset:Trainset = null
    var start:int = 0
    while start < bytes.size():
        var end:int = bytes.find(10, start)  # "\n"
        if end < 0:
            end = bytes.size()
        var line:String = decode_cp1250(bytes.slice(start, end)).strip_edges()
        start = end + 1

        if line.begins_with("//$o"):
            mission_lines.append(line.substr(4).strip_edges())
            continue
        if in_header:
            if line and not line.begins_with("//"):
                in_header = false
            elif line.begins_with("//$n"):
                info.title = line.substr(4).strip_edges()
                continue
            elif line.begins_with("//$d"):
                description_lines.append(line.substr(4).strip_edges())
                continue
            elif line.begins_with("//$i"):
                var image_path:String = scenery_dir.path_join("images").path_join(line.substr(4).strip_edges())
                if FileAccess.file_exists(image_path):
                    info.image_path = image_path
                continue
            else:
                continue

        var tokens:PackedStringArray = line.get_slice("//", 0).split(" ", false)
        if not tokens:
            continue
        match tokens[0].to_lower():
            "trainset":
                trainset = Trainset.new()
                trainset.name = tokens[1] if tokens.size() > 1 else ""
                trainset.track = tokens[2] if tokens.size() > 2 else ""
                mission_lines.clear()
                info.trainsets.append(trainset)
            "endtrainset":
                if trainset:
                    trainset.description = "\n".join(mission_lines)
                    mission_lines.clear()
                trainset = null
            "node":
                if trainset and tokens.size() > 9 and tokens[4].to_lower() == "dynamic":
                    var vehicle := Vehicle.new()
                    vehicle.train_id = tokens[3]
                    vehicle.data_path = _resolve_data_path(tokens[5])
                    # lowercased like maszyna_node_dynamic_importer.gd does - the data files are
                    # lowercase, the .scn spells them however it likes
                    vehicle.skin = tokens[6].to_lower()
                    vehicle.file_name = tokens[7].to_lower()
                    vehicle.driver_type = tokens[9].to_lower()
                    trainset.vehicles.append(vehicle)
    info.description = "\n".join(description_lines)
    return info


## "PKP\\SU42_V1" -> "dynamic/pkp/su42_v1", same as maszyna_node_dynamic_importer.gd
static func _resolve_data_path(data_folder:String) -> String:
    var segments:PackedStringArray = data_folder.replace("\\", "/").to_lower().split("/", false)
    if not segments or not segments[0] == "dynamic":
        segments.insert(0, "dynamic")
    return "/".join(segments)


static func decode_cp1250(bytes:PackedByteArray) -> String:
    var text:String = ""
    for byte:int in bytes:
        text += String.chr(byte if byte < 0x80 else CP1250_HIGH[byte - 0x80])
    return text
