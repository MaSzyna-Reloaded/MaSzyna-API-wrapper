extends Control

## Full screen scenario selector: <game_dir>/scenery/*.scn on the left, details of the selected
## one (MaszynaSceneryInfo) on the right, with the consists it declares. After "Wczytaj" the background dissolves into the
## loading screen below. Escape asks to quit (quit_requested).

signal scenery_selected(filename: String, train_id: String)
## Escape - the game fades out and quits
signal quit_requested

const DISSOLVE_TIME: float = 1.0

## Height of a vehicle side view in the consist preview - twice the 30 px the images have
const VEHICLE_PREVIEW_HEIGHT: float = 60.0

var _files: PackedStringArray = []
var _info: MaszynaSceneryInfo = null


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
    scenery_selected.emit(_files[selected[0]], _get_selected_train_id())
    var tween: Tween = create_tween()
    tween.tween_property(%Background.material, "shader_parameter/dissolve", 1.0, DISSOLVE_TIME)
    tween.tween_callback(hide)


func _on_consists_item_selected(index: int) -> void:
    _show_consist(index)


## The player starts in the headdriver vehicle of the selected consist
func _get_selected_train_id() -> String:
    var selected: PackedInt32Array = %Consists.get_selected_items()
    if not _info or not selected:
        return ""
    return _info.trainsets[selected[0]].get_driver_train_id()


func _show_details(index: int) -> void:
    %LoadButton.disabled = index < 0
    %Image.texture = null
    %Image.visible = false
    %Consists.clear()
    _info = null
    if index < 0:
        %Title.text = ""
        %Description.text = ""
        %ConsistsHeader.visible = false
        %Consists.visible = false
        _show_consist(-1)
        return
    _info = MaszynaSceneryInfo.read(_files[index])
    %Title.text = _info.title if _info.title else _files[index].get_basename()
    %Description.text = _info.description
    if _info.image_path:
        var image: Image = Image.load_from_file(_info.image_path)
        if image:
            %Image.texture = ImageTexture.create_from_image(image)
            %Image.visible = true
    for trainset: MaszynaSceneryInfo.Trainset in _info.trainsets:
        %Consists.add_item(_format_consist(trainset))
    var has_consists: bool = _info.trainsets.size() > 0
    %ConsistsHeader.visible = has_consists
    %Consists.visible = has_consists
    if has_consists:
        %Consists.select(0)
    _show_consist(0 if has_consists else -1)


## The vehicles of the consist as their skin textures, and its mission description
func _show_consist(index: int) -> void:
    for child: Node in %Vehicles.get_children():
        child.queue_free()
    %ConsistPreview.visible = index >= 0
    if not _info or index < 0:
        return
    var trainset: MaszynaSceneryInfo.Trainset = _info.trainsets[index]
    %Description.text = (
        "%s\n\n%s" % [trainset.description, _info.description]
        if trainset.description
        else _info.description
    )
    for vehicle: MaszynaSceneryInfo.Vehicle in trainset.vehicles:
        %Vehicles.add_child(_create_vehicle_preview(vehicle))


## Side view of the vehicle, its type when the data has no image for it
func _create_vehicle_preview(vehicle: MaszynaSceneryInfo.Vehicle) -> Control:
    var tooltip: String = "%s (%s)" % [vehicle.train_id, vehicle.data_path.get_file()]
    var image: Image = Image.load_from_file(vehicle.image_path) if vehicle.image_path else null
    if not image:
        var label := Label.new()
        label.text = vehicle.data_path.get_file()
        label.tooltip_text = tooltip
        label.custom_minimum_size = Vector2(0, VEHICLE_PREVIEW_HEIGHT)
        label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
        return label

    var preview := TextureRect.new()
    preview.texture = ImageTexture.create_from_image(image)
    # in a container the width has to be given, the height alone leaves the control 0 px wide
    preview.custom_minimum_size = Vector2(
        VEHICLE_PREVIEW_HEIGHT * float(image.get_width()) / float(image.get_height()),
        VEHICLE_PREVIEW_HEIGHT
    )
    preview.stretch_mode = TextureRect.STRETCH_SCALE
    # the images are tiny, linear filtering turns them into a blur when scaled up
    preview.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
    preview.tooltip_text = tooltip
    return preview


static func _format_consist(trainset: MaszynaSceneryInfo.Trainset) -> String:
    var driver_train_id: String = trainset.get_driver_train_id()
    var consist_name: String = (
        trainset.name if trainset.name and not trainset.name.to_lower() == "none" else driver_train_id
    )
    return "%s - %d pojazdów, start: %s" % [consist_name, trainset.vehicles.size(), driver_train_id]
