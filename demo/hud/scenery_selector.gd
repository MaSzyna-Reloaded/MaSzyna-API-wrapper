extends ScrollContainer
class_name ScenerySelector

## Lists <game_dir>/scenery/*.scn (flat, no subdirectories) with a "Wczytaj" button per entry.

signal scenery_selected(filename: String)


func _ready() -> void:
    var files: PackedStringArray = DirAccess.get_files_at(UserSettings.get_maszyna_game_dir().path_join("scenery"))
    files.sort()
    for file: String in files:
        if not file.get_extension().to_lower() == "scn":
            continue
        var row: HBoxContainer = HBoxContainer.new()
        var label: Label = Label.new()
        label.text = file.get_basename()
        label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
        var button: Button = Button.new()
        button.text = "Wczytaj"
        button.pressed.connect(scenery_selected.emit.bind(file))
        row.add_child(label)
        row.add_child(button)
        %List.add_child(row)
