extends Control

## Full screen scenario selector: <game_dir>/scenery/*.scn on the left, details of the selected
## one (MaszynaSceneryInfo) on the right. After "Wczytaj" the background dissolves into the
## loading screen below. Escape asks to quit (quit_requested).

signal scenery_selected(filename: String)
## Escape - the game fades out and quits
signal quit_requested

const DISSOLVE_TIME: float = 1.0

var _files: PackedStringArray = []


func _ready() -> void:
    var files: PackedStringArray = DirAccess.get_files_at(UserSettings.get_maszyna_game_dir().path_join("scenery"))
    files.sort()
    for file: String in files:
        # "$" files are not scenarios to start (e.g. $stary_jawor_eszelon.scn)
        if file.get_extension().to_lower() == "scn" and not file.begins_with("$"):
            _files.append(file)
            %List.add_item(file.get_basename())
    _show_details(-1)
    %BuildLabel.text = "Pre-Alpha Demo Release %s" % ProjectSettings.get_setting("application/config/version")


func open() -> void:
    (%Background.material as ShaderMaterial).set_shader_parameter("dissolve", 0.0)
    %Content.visible = true
    visible = true
    %List.grab_focus()


func _unhandled_input(event: InputEvent) -> void:
    if visible and %Content.visible and event.is_action_pressed("ui_cancel"):
        get_viewport().set_input_as_handled()
        quit_requested.emit()


func _on_list_item_selected(index: int) -> void:
    _show_details(index)


func _on_load_button_pressed() -> void:
    var selected: PackedInt32Array = %List.get_selected_items()
    %Content.visible = false
    scenery_selected.emit(_files[selected[0]])
    var tween: Tween = create_tween()
    tween.tween_property(%Background.material, "shader_parameter/dissolve", 1.0, DISSOLVE_TIME)
    tween.tween_callback(hide)


func _show_details(index: int) -> void:
    %LoadButton.disabled = index < 0
    %Image.texture = null
    %Image.visible = false
    if index < 0:
        %Title.text = ""
        %Description.text = ""
        return
    var info: MaszynaSceneryInfo = MaszynaSceneryInfo.read(_files[index])
    %Title.text = info.title if info.title else _files[index].get_basename()
    %Description.text = info.description
    if info.image_path:
        var image: Image = Image.load_from_file(info.image_path)
        if image:
            %Image.texture = ImageTexture.create_from_image(image)
            %Image.visible = true
