@tool
extends Node

const E3D_CACHE_PATH = "user://cache/e3d/res"

func _ready():
    UserSettings.cache_clear_requested.connect(clear_cache)

func clear_cache():
    if DirAccess.dir_exists_absolute(E3D_CACHE_PATH):
        var err = FileUtils.remove_dir_recursively(E3D_CACHE_PATH)
        if err == OK:
            print("E3D/RES cache cleared")
        else:
            push_error("E3D/RES cache error: ", err)

func _set_owner_recursive(node, new_owner):
    if not node == new_owner:
        node.owner = new_owner
    if node.get_child_count():
        for kid in node.get_children():
            _set_owner_recursive(kid, new_owner)

func load_model(data_path:String, filename: String) -> E3DModel:
    var output:E3DModel
    var path = UserSettings.get_maszyna_game_dir() + "" + data_path + "/" + filename + ".e3d"

    # check users cache
    var cached_path = E3D_CACHE_PATH+"/"+data_path+"__"+filename+".res"
    var cached_meta_path = cached_path+".meta"

    var is_cache_valid = false

    if FileAccess.file_exists(path):
        var orig_hash = str(FileAccess.get_modified_time(path))
        if FileAccess.file_exists(cached_path) and FileAccess.file_exists(cached_meta_path):
            var cached_hash = FileAccess.get_file_as_string(cached_meta_path)
            is_cache_valid = (cached_hash == orig_hash)

        if is_cache_valid:
            output = load(cached_path)
        else:
            var res = load(path)  # load external e3d
            if res:
                output = res
                var cached_dir = cached_path.get_base_dir()
                DirAccess.make_dir_recursive_absolute(cached_dir)
                ResourceSaver.save(res, cached_path)
                var meta = FileAccess.open(cached_meta_path, FileAccess.WRITE)
                if meta:
                    meta.store_string(orig_hash)
                    meta.close()
    else:
        push_error("File does not exist: %s" % path)
    return output
