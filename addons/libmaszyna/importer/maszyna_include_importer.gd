@tool
extends RefCounted

const SceneryInstancer = preload("res://addons/libmaszyna/scenery/scenery_instancer.gd")

func import(p: MaszynaParser, context: MaszynaImporterContext):
    var tokens = p.get_tokens_until("end")
    tokens.pop_back()
    var filename = tokens.pop_front()
    var data_dir = UserSettings.get_maszyna_game_dir()
    var final_path = data_dir.path_join("scenery").path_join(filename)

    var file = FileAccess.open(final_path, FileAccess.READ)
    var parameters = {}
    for i in range(tokens.size()):
        parameters["p%s" % (i+1)] = tokens[i]
    if file:
        context.push_state()
        context.include_depth += 1
        var objects := SceneryInstancer.parse_file(filename, parameters, context)
        context.pop_state()
        return objects
    else:
        push_error("Cannot load include file: " + final_path)
        return []
