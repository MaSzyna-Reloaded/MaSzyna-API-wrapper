extends Object
class_name FileUtils

static func remove_dir_recursively(dir: String):
    var subdirs = DirAccess.get_directories_at(dir)
    for subdir in subdirs:
        var err = remove_dir_recursively(dir + "/" + subdir)
        if not err == OK:
            return err
    for file in DirAccess.get_files_at(dir):
        var err = DirAccess.remove_absolute(dir + "/" + file)
        if not err == OK:
            return err
    return DirAccess.remove_absolute(dir)
