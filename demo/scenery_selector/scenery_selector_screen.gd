extends Control

## Full screen scenario selector: <game_dir>/scenery/*.scn on the left, details of the selected
## one (MaszynaSceneryInfo) on the right, with the consists it declares. After "Wczytaj" the background dissolves into the
## loading screen below. Escape asks to quit (quit_requested).

signal scenery_selected(filename: String, train_id: String, skin_overrides: Dictionary)
## Escape - the game fades out and quits
signal quit_requested

const DISSOLVE_TIME: float = 1.0

## UI feedback of the startup screens. Events are named after what happened, not after the
## sample - what each one sounds like is the bank's decision, not this screen's.
const UI_SOUNDS: SfxBank = preload("res://startup/ui_sounds.tres")

## Height of a vehicle side view in the consist preview - twice the 30 px the images have
const VEHICLE_PREVIEW_HEIGHT: float = 60.0
const VehicleViewer = preload("res://scenery_selector/vehicle_viewer.gd")
## Vehicle under the mouse pointer gets a lit background, the same as the skins of the viewer
const VEHICLE_GLOW_SHADER: Shader = preload("res://scenery_selector/skin_glow.gdshader")
const VEHICLE_HOVER_COLOR: Color = Color(1.0, 1.0, 1.0)
## The tile is taller than the vehicle, so its background shows around it
const VEHICLE_TILE_PADDING: float = 1.35

const VEHICLE_SELECTED_COLOR: Color = Color(0.35, 1.0, 0.45)
## Sections the keyboard walks through with Tab. The focus is virtual - the search field keeps the
## Godot focus, so typing filters the list whichever section is current.
enum Section { SCENERY, CONSISTS, VEHICLES, SKINS }

var _files: PackedStringArray = []
## Title of each scenery and its item on the list, in the order of _files
var _titles: PackedStringArray = []
var _ui_sounds: SfxPlayer
var _info: MaszynaSceneryInfo = null
## Vehicles of the shown consist and their tiles, in the order they run
var _vehicles: Array[MaszynaSceneryInfo.Vehicle] = []
var _vehicle_tiles: Array[Control] = []
## Vehicle whose viewer is open
var _shown_index: int = -1
## Vehicle the keyboard is on - the viewer opens on Enter, so this is not _shown_index
var _highlighted_vehicle: int = -1
## Fade of the scenery list under the viewer, killed when the other fade starts
var _list_fade_tween: Tween = null
## Section Tab left the keyboard on - one of Section, kept as int so no enum cast is needed
var _section: int = Section.SCENERY



func _ready() -> void:
    _ui_sounds = SfxPlayer.new()
    _ui_sounds.bank = UI_SOUNDS
    add_child(_ui_sounds)
    var files: PackedStringArray = DirAccess.get_files_at(UserSettings.get_maszyna_game_dir().path_join("scenery"))
    files.sort()
    for file: String in files:
        # "$" files are not scenarios to start (e.g. $stary_jawor_eszelon.scn)
        if not file.get_extension().to_lower() == "scn" or file.begins_with("$"):
            continue
        _files.append(file)
        _titles.append(MaszynaSceneryInfo.read_display_name(file))
    var notes: PackedStringArray = []
    for file: String in _files:
        notes.append(file.get_basename().to_upper())
    # the list selects its first row and reports it back, so the details follow from here on
    %SceneryList.set_rows(_titles, notes)
    %BuildLabel.text = "Pre-Alpha Demo Release %s (build %s)" % [
        ProjectSettings.get_setting("application/config/version"), _build_number()
    ]


## The stamp of the build that is actually loaded; a checkout that was never built has none.
func _build_number() -> String:
    var stamp:String = MaszynaRuntime.get_build_number()
    return stamp if stamp else "unbuilt"


func open() -> void:
    (%Background.material as ShaderMaterial).set_shader_parameter("dissolve", 0.0)
    %Content.visible = true
    visible = true
    # activating the list puts the keyboard in its search field
    _set_section(Section.SCENERY)


func _unhandled_input(event: InputEvent) -> void:
    if not visible or not %Content.visible or not event.is_action_pressed("ui_cancel"):
        return
    get_viewport().set_input_as_handled()
    if %VehicleViewer.visible:
        %VehicleViewer.close()
        return
    quit_requested.emit()


## Tab and the keys of the two tile sections. The two SelectorLists take their own keys while they
## have the focus, so nothing here touches them. Escape stays in _unhandled_input, uncontested.
func _input(event: InputEvent) -> void:
    if not visible or not %Content.visible:
        return
    if event.is_action_pressed("ui_focus_next"):
        _change_section(1)
    elif event.is_action_pressed("ui_focus_prev"):
        _change_section(-1)
    elif event.is_action_pressed("ui_down", true):
        _move_selection(1)
    elif event.is_action_pressed("ui_up", true):
        _move_selection(-1)
    elif event.is_action_pressed("ui_page_down", true):
        _move_selection(SelectorList.PAGE_STEP)
    elif event.is_action_pressed("ui_page_up", true):
        _move_selection(-SelectorList.PAGE_STEP)
    elif event.is_action_pressed("ui_end"):
        _move_selection(SelectorList.LIST_END_STEP)
    elif event.is_action_pressed("ui_home"):
        _move_selection(-SelectorList.LIST_END_STEP)
    # ui_text_submit and not ui_accept: that one is Space as well, and Space belongs to the search
    elif event.is_action_pressed("ui_text_submit"):
        _activate_selection()
    else:
        return
    get_viewport().set_input_as_handled()


## Sections that have something to walk right now: the scenery list is gone under the viewer, the
## consists need a scenery, the vehicles a consist, the skins an open viewer
func _available_sections() -> Array[int]:
    var sections: Array[int] = []
    if %ListPanel.visible:
        sections.append(Section.SCENERY)
    if %ConsistsList.visible:
        sections.append(Section.CONSISTS)
    if _vehicle_tiles:
        sections.append(Section.VEHICLES)
    if %VehicleViewer.visible:
        sections.append(Section.SKINS)
    return sections


## Tab: the next section that is there, wrapping around. A section change never moves a selection.
func _change_section(step: int) -> void:
    var sections: Array[int] = _available_sections()
    if not sections:
        return
    # a section that went away leaves find() at -1, which lands on the first one
    _set_section(sections[wrapi(sections.find(_section) + step, 0, sections.size())])
    _ui_sounds.play(&"change_focus")


func _set_section(section: int) -> void:
    _section = section
    %SceneryList.set_section_focused(section == Section.SCENERY)
    %ConsistsList.set_section_focused(section == Section.CONSISTS)
    %VehiclesFocus.focused = section == Section.VEHICLES
    %VehicleViewer.set_section_focused(section == Section.SKINS)


## Up/Down: the item change a click would have made, under the keyboard's own sound. An end of the
## list changes nothing, and moves nothing - reselecting a scenery re-reads its .scn and reselecting
## a consist rebuilds its vehicles.
func _move_selection(step: int) -> void:
    match _section:
        Section.VEHICLES:
            if not _vehicle_tiles:
                return
            var index: int = clampi(_highlighted_vehicle + step, 0, _vehicle_tiles.size() - 1)
            if index == _highlighted_vehicle:
                return
            _ui_sounds.play(&"keystroke")
            _highlight_vehicle(index)
            FocusSection.scroll_to_item_in_row(%ConsistPreview, _vehicle_tiles[index])
        Section.SKINS:
            %VehicleViewer.move_skin_selection(step)


## Enter: the gesture the mouse would have made in this section, sound included - on a scenery that
## gesture is "Wczytaj" and not the row, which the arrows already select
func _activate_selection() -> void:
    match _section:
        Section.VEHICLES:
            if _highlighted_vehicle >= 0:
                _on_vehicle_preview_pressed(_highlighted_vehicle)
        Section.SKINS:
            %VehicleViewer.activate_skin_selection()


## A scenery came up on the list - by key, by click or as the first result of a search
func _on_scenery_list_item_selected(index: int) -> void:
    _show_details(index)


## Enter on a scenery is the "Wczytaj" button, disabled state included
func _on_scenery_list_item_activated(_index: int) -> void:
    if not %LoadButton.disabled:
        _on_load_button_pressed()


## Right goes into the consists of the scenery; there is nothing to the left of the list
func _on_scenery_list_navigate_out(step: int) -> void:
    if step < 0 or not %ConsistsList.visible:
        return
    _set_section(Section.CONSISTS)
    _ui_sounds.play(&"change_focus")


func _on_scenery_list_focus_requested() -> void:
    _set_section(Section.SCENERY)


func _on_consists_list_item_selected(index: int) -> void:
    _show_consist(index)


## The consist is already the selected one - Enter only says so out loud
func _on_consists_list_item_activated(_index: int) -> void:
    _ui_sounds.play(&"list_item_click")


## Left goes back to the scenery list, unless the viewer is standing on it
func _on_consists_list_navigate_out(step: int) -> void:
    if step > 0 or not %ListPanel.visible:
        return
    _set_section(Section.SCENERY)
    _ui_sounds.play(&"change_focus")


func _on_consists_list_focus_requested() -> void:
    _set_section(Section.CONSISTS)


func _on_load_button_pressed() -> void:
    _ui_sounds.play(&"load_scenery")
    %Content.visible = false
    scenery_selected.emit(
        _files[%SceneryList.get_selected()], _get_selected_train_id(), _get_skin_overrides()
    )
    var tween: Tween = create_tween()
    tween.tween_property(%Background.material, "shader_parameter/dissolve", 1.0, DISSOLVE_TIME)
    tween.tween_callback(hide)


## The player starts in the headdriver vehicle of the selected consist
func _get_selected_train_id() -> String:
    var index: int = %ConsistsList.get_selected()
    if not _info or index < 0:
        return ""
    return _info.trainsets[index].get_driver_train_id()


func _show_details(index: int) -> void:
    %LoadButton.disabled = index < 0
    %Image.texture = null
    %Image.visible = false
    _info = null
    if index < 0:
        %Title.text = ""
        %FileName.text = ""
        %Description.text = ""
        %ConsistsHeader.visible = false
        %ConsistsList.visible = false
        # an empty list reports no selection, which takes the vehicles down with it
        %ConsistsList.set_rows([], [])
        return
    _info = MaszynaSceneryInfo.read(_files[index])
    %Title.text = _titles[index]
    %FileName.text = _files[index].get_basename().to_upper()
    %Description.text = _info.description
    if _info.image_path:
        var image: Image = Image.load_from_file(_info.image_path)
        if image:
            %Image.texture = ImageTexture.create_from_image(image)
            %Image.visible = true
    var names: PackedStringArray = []
    var notes: PackedStringArray = []
    for trainset: MaszynaSceneryInfo.Trainset in _info.trainsets:
        names.append(_get_consist_name(trainset))
        notes.append(_format_consist_note(trainset))
    var has_consists: bool = names.size() > 0
    %ConsistsHeader.visible = has_consists
    %ConsistsList.visible = has_consists
    # the list selects its first consist and reports it back, so the vehicles follow from here on
    %ConsistsList.set_rows(names, notes)


## The vehicles of the consist as their skin textures, and its mission description
func _show_consist(index: int) -> void:
    # the viewer shows a vehicle of the consist that is going away
    if %VehicleViewer.visible:
        %VehicleViewer.close()
    for child: Node in %Vehicles.get_children():
        child.queue_free()
    _vehicles.clear()
    _vehicle_tiles.clear()
    _shown_index = -1
    _highlighted_vehicle = -1
    %VehiclesFocus.visible = index >= 0
    if not _info or index < 0:
        return
    var trainset: MaszynaSceneryInfo.Trainset = _info.trainsets[index]
    %Description.text = (
        "%s\n\n%s" % [trainset.description, _info.description]
        if trainset.description
        else _info.description
    )
    _vehicles.assign(trainset.vehicles)
    for vehicle_index: int in _vehicles.size():
        var tile: Control = _create_vehicle_preview(vehicle_index)
        _vehicle_tiles.append(tile)
        %Vehicles.add_child(tile)
    if _vehicle_tiles:
        _highlight_vehicle(0)


## Side view of the vehicle from VehicleProfileManager (rendered on the spot when the data has
## no image of it)
func _create_vehicle_preview(index: int) -> Control:
    var vehicle: MaszynaSceneryInfo.Vehicle = _vehicles[index]
    var tile := Control.new()
    tile.custom_minimum_size = Vector2(
        VEHICLE_PREVIEW_HEIGHT, VEHICLE_PREVIEW_HEIGHT * VEHICLE_TILE_PADDING
    )


    var background := ColorRect.new()
    background.name = "Background"
    background.set_anchors_preset(Control.PRESET_FULL_RECT)
    background.mouse_filter = Control.MOUSE_FILTER_IGNORE
    background.material = ShaderMaterial.new()
    (background.material as ShaderMaterial).shader = VEHICLE_GLOW_SHADER
    tile.add_child(background)

    var preview := TextureButton.new()
    preview.ignore_texture_size = true
    preview.stretch_mode = TextureButton.STRETCH_KEEP_ASPECT_CENTERED
    preview.set_anchors_preset(Control.PRESET_FULL_RECT)
    # the profiles are small, linear filtering turns them into a blur when scaled up
    preview.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
    preview.tooltip_text = "%s (%s)" % [vehicle.train_id, vehicle.data_path.get_file()]
    preview.name = "Preview"
    preview.pressed.connect(_on_vehicle_preview_pressed.bind(index))
    preview.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
    preview.mouse_entered.connect(_on_vehicle_preview_hovered.bind(index, true))
    preview.mouse_exited.connect(_on_vehicle_preview_hovered.bind(index, false))
    tile.add_child(preview)
    _load_vehicle_profile(preview, vehicle)
    return tile


## Side view of the vehicle at index, reloaded after its skin changes
func _refresh_vehicle_profile(index: int) -> void:
    var tile: Control = _vehicle_tiles[index]
    _load_vehicle_profile(tile.get_node("Preview") as TextureButton, _vehicles[index])


func _load_vehicle_profile(preview: TextureButton, vehicle: MaszynaSceneryInfo.Vehicle) -> void:
    # dimmed until the side view of the new skin is there
    preview.modulate.a = 0.3
    var profile: Texture2D = await VehicleProfileManager.get_profile(
        vehicle.data_path, vehicle.file_name, vehicle.skin
    )
    if not is_instance_valid(preview):
        return
    preview.modulate.a = 1.0
    if not profile:
        return
    preview.texture_normal = profile
    var tile: Control = preview.get_parent() as Control
    # as wide as the vehicle is long, so the vehicles of a consist line up without gaps
    var tile_size: Vector2 = Vector2(
        VEHICLE_PREVIEW_HEIGHT * float(profile.get_width()) / float(profile.get_height()),
        VEHICLE_PREVIEW_HEIGHT * VEHICLE_TILE_PADDING
    )
    tile.custom_minimum_size = tile_size
    var background: ColorRect = tile.get_node("Background") as ColorRect
    (background.material as ShaderMaterial).set_shader_parameter("rect_size", tile_size)


func _on_vehicle_preview_pressed(index: int) -> void:
    _ui_sounds.play(&"vehicle_click")
    _set_section(Section.VEHICLES)
    _highlighted_vehicle = index
    _show_vehicle(index)


func _on_vehicle_preview_hovered(index: int, hovered: bool) -> void:
    if index == _shown_index:
        return
    if hovered:
        _ui_sounds.play(&"vehicle_hover")
    _set_vehicle_glow(index, VEHICLE_HOVER_COLOR if hovered else Color.TRANSPARENT)


## Vehicle the arrows are on: lit like one under the pointer, green while its viewer is open
func _highlight_vehicle(index: int) -> void:
    _set_vehicle_glow(_highlighted_vehicle, Color.TRANSPARENT)
    _highlighted_vehicle = index
    _set_vehicle_glow(index, VEHICLE_SELECTED_COLOR if index == _shown_index else VEHICLE_HOVER_COLOR)


## The vehicle whose viewer is open keeps a green background
func _set_vehicle_glow(index: int, color: Color) -> void:
    if index < 0 or index >= _vehicle_tiles.size():
        return
    var tile: Control = _vehicle_tiles[index]
    var material: ShaderMaterial = (tile.get_node("Background") as ColorRect).material as ShaderMaterial
    material.set_shader_parameter("glow_color", color)
    material.set_shader_parameter("glow_strength", 0.0 if color == Color.TRANSPARENT else 1.0)


func _on_vehicle_viewer_skin_clicked() -> void:
    _set_section(Section.SKINS)


## A skin accepted in the viewer: the consist shows it and the scenery is loaded with it
func _on_vehicle_viewer_skin_applied(skin: String) -> void:
    _ui_sounds.play(&"apply_skin")
    if _shown_index < 0:
        return
    _vehicles[_shown_index].skin = skin
    _refresh_vehicle_profile(_shown_index)


## The scenery is loaded with the skins of the consist as they are shown here
func _get_skin_overrides() -> Dictionary[String, String]:
    var overrides: Dictionary[String, String] = {}
    for vehicle: MaszynaSceneryInfo.Vehicle in _vehicles:
        overrides[vehicle.train_id] = vehicle.skin
    return overrides


## The vehicle viewer takes the place of the scenery list, the two cross-fade
func _show_vehicle(index: int) -> void:
    _set_vehicle_glow(_shown_index, Color.TRANSPARENT)
    _shown_index = index
    _set_vehicle_glow(index, VEHICLE_SELECTED_COLOR)
    if _list_fade_tween:
        _list_fade_tween.kill()
    _list_fade_tween = create_tween()
    _list_fade_tween.tween_property(%ListPanel, "modulate:a", 0.0, VehicleViewer.FADE_TIME)
    _list_fade_tween.tween_callback(%ListPanel.hide)
    %VehicleViewer.show_vehicle(_vehicles[index])


## Emitted when the viewer starts fading out
func _on_vehicle_viewer_closed(by_back_button: bool) -> void:
    if by_back_button:
        _ui_sounds.play(&"back_button")
    if _section == Section.SKINS:
        _set_section(Section.VEHICLES)
    _set_vehicle_glow(_shown_index, Color.TRANSPARENT)
    _shown_index = -1
    _highlight_vehicle(_highlighted_vehicle)
    %ListPanel.modulate.a = 0.0
    %ListPanel.visible = true
    if _list_fade_tween:
        _list_fade_tween.kill()
    _list_fade_tween = create_tween()
    _list_fade_tween.tween_property(%ListPanel, "modulate:a", 1.0, VehicleViewer.FADE_TIME)


## A consist is named by the vehicle the player starts in - the "trainset" line carries the
## starting track, not a name; the vehicles are named in "node <x> <y> <name> dynamic"
static func _get_consist_name(trainset: MaszynaSceneryInfo.Trainset) -> String:
    return trainset.get_driver_train_id()


static func _format_consist_note(trainset: MaszynaSceneryInfo.Trainset) -> String:
    return "%d POJAZDÓW" % trainset.vehicles.size()
