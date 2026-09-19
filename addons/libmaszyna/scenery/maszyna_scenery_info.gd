@tool
extends RefCounted
class_name MaszynaSceneryInfo

## Scenario description from the header comments of a .scn file (read by the original starter):
## [code]//$n[/code] title, [code]//$d[/code] description lines, [code]//$i[/code] image in
## scenery/images/. Files are in cp1250.

## The header is at the top of the file
const MAX_HEADER_BYTES:int = 65536
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


## Reads the header of scenery/<filename>: comment and empty lines up to the first statement
static func read(filename:String) -> MaszynaSceneryInfo:
    var info := MaszynaSceneryInfo.new()
    var scenery_dir:String = UserSettings.get_maszyna_game_dir().path_join("scenery")
    var file:FileAccess = FileAccess.open(scenery_dir.path_join(filename), FileAccess.READ)
    if not file:
        return info
    # raw bytes - FileAccess.get_line() decodes UTF-8 and would lose the cp1250 characters
    var bytes:PackedByteArray = file.get_buffer(mini(file.get_length(), MAX_HEADER_BYTES))
    var description_lines:PackedStringArray = []
    var start:int = 0
    while start < bytes.size():
        var end:int = bytes.find(10, start)  # "\n"
        if end < 0:
            end = bytes.size()
        var line:String = decode_cp1250(bytes.slice(start, end)).strip_edges()
        start = end + 1
        if line and not line.begins_with("//"):
            break
        if line.begins_with("//$n"):
            info.title = line.substr(4).strip_edges()
        elif line.begins_with("//$d"):
            description_lines.append(line.substr(4).strip_edges())
        elif line.begins_with("//$i"):
            var image_path:String = scenery_dir.path_join("images").path_join(line.substr(4).strip_edges())
            if FileAccess.file_exists(image_path):
                info.image_path = image_path
    info.description = "\n".join(description_lines)
    return info


static func decode_cp1250(bytes:PackedByteArray) -> String:
    var text:String = ""
    for byte:int in bytes:
        text += String.chr(byte if byte < 0x80 else CP1250_HIGH[byte - 0x80])
    return text
