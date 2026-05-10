@tool
extends RefCounted


func import(p: MaszynaParser, _context: MaszynaImporterContext) -> Array:
    var tokens: Array = p.get_tokens_until("endsky")
    if tokens.is_empty():
        return []

    var model_path: String = str(tokens[0]).to_lower()
    var sky: MaszynaEnvironmentNode = MaszynaEnvironmentNode.new()
    sky.name = "Sky"
    sky.model_filename = model_path.get_file().get_basename()
    sky.data_path = _resolve_data_path(model_path)
    return [sky]


func _resolve_data_path(model_path: String) -> String:
    var data_path: String = model_path.get_base_dir()
    if data_path.is_empty():
        return "models"

    var path_parts: PackedStringArray = data_path.split("/")
    if path_parts.is_empty() or path_parts[0] != "models":
        path_parts.insert(0, "models")
    return "/".join(path_parts)
