@tool
extends RefCounted

static var track_importer = preload("res://addons/libmaszyna/importer/maszyna_node_track_importer.gd").new()
static var traction_importer = preload("res://addons/libmaszyna/importer/maszyna_node_traction_importer.gd").new()
static var model_importer = preload("res://addons/libmaszyna/importer/maszyna_node_model_importer.gd").new()
static var triangles_importer = preload("res://addons/libmaszyna/importer/maszyna_node_triangles_importer.gd").new()
static var dynamic_importer = preload("res://addons/libmaszyna/importer/maszyna_node_dynamic_importer.gd").new()
static var power_source_importer = preload("res://addons/libmaszyna/importer/maszyna_node_tractionpowersource_importer.gd").new()

# FIXME: _current_rotation, _origin change to parent nodes
func import(p:MaszynaParser, context: MaszynaImporterContext):
    var range_max = float(p.next_token())
    var range_min = float(p.next_token())
    var name = p.next_token()
    # "none" is this format's sentinel for "no value given" everywhere (skin, load type, ...),
    # not a real name - treating it as one makes every "none"-named track/vehicle after the
    # first collide (TrackManager.track_update() rejects the duplicate and returns early,
    # silently skipping that track's width/type/AABB setup entirely; DynamicRailVehicle3D.train_id
    # collisions break TrainSystem lookups the same way).
    if name.to_lower() == "none":
        name = ""

    var type = p.next_token().to_lower()
    var node_name = name if name else "node_%s" % type
    var obj

    match type:
        "eventlauncher":
            p.get_tokens_until("end")
            #push_warning("Eventlauncher node is not supported yet")

        "triangles":
            var triangles = triangles_importer.import(p, context, range_min, range_max)
            if triangles:
                context.triangles.append(triangles)

        "sound":
            p.get_tokens_until("endsound")
            #push_warning("Sound node is not supported yet")

        "traction":
            var traction = traction_importer.import(p, context)
            if traction:
                context.traction.append(traction)

        "tractionpowersource":
            var power_source = power_source_importer.import(p, context)
            if power_source:
                power_source.name = name
                context.power_sources.append(power_source)

        "model":
            obj = model_importer.import(p, context)
            #if obj and context.rotate:
            #    obj.rotation += Vector3(context.rotate)

        "track":
            var track = track_importer.import(p, context)
            if track:
                track.track_name = name
                context.tracks.append(track)

        "dynamic":
            obj = dynamic_importer.import(p, context)
            if obj:
                (obj as DynamicRailVehicle3D).train_id = name

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
        # Matches the original engine's transform() (simulationstateserializer.cpp): the node's
        # own LOCAL offset is rotated by the enclosing rotate: context before the origin: offset
        # is added - not just translated by context.origin directly. Skipping the rotation step
        # placed every node inside a rotated origin/rotate block (e.g. a traction pole's bracket
        # arm, offset from its pole by tra/sb165-3d.inc's own "rotate 0 (p5) 0") on the wrong side.
        obj.position = (obj.position as Vector3).rotated(Vector3.UP, context.rotate.y) + context.origin
        obj.rotation += Vector3(context.rotate)

    if range_max > 0 and obj and obj is GeometryInstance3D:
        obj.visibility_range_begin = range_min
        obj.visibility_range_end = range_max
    #if obj is MaSzyna_Node:
    #    obj.node_name = node_name
    #    obj.range_min = range_min
    #    obj.range_max = range_max
    return [obj]
