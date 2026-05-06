@tool
extends Object

static var track_importer = preload("res://addons/libmaszyna/importer/maszyna_node_track_importer.gd").new()
static var traction_importer = preload("res://addons/libmaszyna/importer/maszyna_node_traction_importer.gd").new()
static var model_importer = preload("res://addons/libmaszyna/importer/maszyna_node_model_importer.gd").new()
static var triangles_importer = preload("res://addons/libmaszyna/importer/maszyna_node_triangles_importer.gd").new()

# FIXME: _current_rotation, _origin change to parent nodes
func import(p:MaszynaParser, context: MaszynaImporterContext):
    var range_max = float(p.next_token())
    var range_min = float(p.next_token())
    var name = p.next_token()

    var type = p.next_token().to_lower()
    var node_name = name if name else "node_%s" % type
    var obj

    match type:
        "eventlauncher":
            p.get_tokens_until("end")
            #push_warning("Eventlauncher node is not supported yet")

        "triangles":
            obj = triangles_importer.import(p, context)

        "sound":
            p.get_tokens_until("endsound")
            #push_warning("Sound node is not supported yet")

        "traction":
            obj = traction_importer.import(p, context)
            #push_warning("Traction node is not supported yet")

        "tractionpowersource":
            p.get_tokens_until("end")
            #push_warning("Traction power source node is not supported yet")

        "model":
            obj = model_importer.import(p, context)
            if obj and context.rotate:
                obj.rotation = Vector3(context.rotate)

        "track":
            var track = track_importer.import(p, context)
            if track:
                track.name = name
                #track.range_max = range_max
                #track.range_min = range_min
                context.tracks.append(track)

        "memcell":
            p.get_tokens_until("endmemcell")
            #push_warning("Memcell node is not supported yet")
        "lines":
            p.get_tokens_until("endline")
            #push_warning("Lines node is not supported yet")
        _:
            #push_error("Unhandled node type: "+type)
            pass
    if obj:
        obj.name = node_name
    if obj is Node3D:
        obj.position += context.origin
    if range_max > 0 and obj and obj is GeometryInstance3D:
        obj.visibility_range_begin = range_min
        obj.visibility_range_end = range_max
    #if obj is MaSzyna_Node:
    #    obj.node_name = node_name
    #    obj.range_min = range_min
    #    obj.range_max = range_max
    return [obj]
