@tool
extends Node

var _cache = ResourceCache.create("e3d")

func clear_cache():
    _cache.clear()

func _set_owner_recursive(node, new_owner):
    if not node == new_owner:
        node.owner = new_owner
    if node.get_child_count():
        for kid in node.get_children():
            _set_owner_recursive(kid, new_owner)

func _make_cache_path(data_path:String, filename:String, source_abs_path:String):
    return data_path.path_join(filename+".e3d.res")


func _make_cache_hash(source_abs_path:String) -> String:
    return ("%s:%s:%s" % [
        FileAccess.get_modified_time(source_abs_path),
        E3DModel.FORMAT_VERSION,
        source_abs_path
    ]).md5_text()

func load_model(data_path:String, filename: String) -> E3DModel:
    var output:E3DModel
    var relpath = data_path.path_join(filename+".e3d")
    var path = UserSettings.get_maszyna_game_dir().path_join(relpath)

    # check users cache
    
    var cached_path = _make_cache_path(data_path, filename, path)
    var cache_hash = _make_cache_hash(path)

    output = _cache.get(cached_path, cache_hash) as E3DModel
    if output:
        return output
    
    if FileAccess.file_exists(path):
        output = load(path) as E3DModel # load external e3d
        if output:
            _cache.set(cached_path, output, cache_hash)
            return _cache.get(cached_path)  # force use proper resource ref
        else:
            push_warning("File is not an E3DModel: "+path)                        
    else:
        push_error("File does not exist: %s" % path)
    return output
