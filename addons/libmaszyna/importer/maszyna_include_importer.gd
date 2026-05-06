@tool
extends RefCounted

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
        var obj := MaszynaIncludeNode.new()
        obj.name = filename
        obj.filename = filename
        obj.parameters = parameters
        obj.context_rotate = context.rotate
        obj.context_origin = context.origin
        #SceneryInstancer.instantiate(obj, parameters, context)
        #return obj.get_children(true)
        return [obj]
        
    else:
        push_error("Cannot load include file: " + final_path)
        return []
