class_name SettingFolder
extends SettingRow

## Where the game keeps something (the log files): the folder, not editable, and a button - its
## icon alone, its name a tooltip - that shows it in the system's file manager; Enter does the same.
## The setting is a project setting naming a file in that folder (debug/file_logging/log_path).

var _folder: String = ""


func is_edited_by_steps() -> bool:
    return false


func _show(value: Variant) -> void:
    _folder = ProjectSettings.globalize_path(String(value).get_base_dir())
    %Path.text = _folder
    %Path.tooltip_text = _folder


func _activate() -> void:
    reveal()


## The folder in the system's file manager - made first, if nothing has been written there yet
func reveal() -> void:
    DirAccess.make_dir_recursive_absolute(_folder)
    OS.shell_show_in_file_manager(_folder)
