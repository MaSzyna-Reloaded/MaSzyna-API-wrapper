extends Control

## Full screen scenario selector: <game_dir>/scenery/*.scn on the left, details of the selected
## one (MaszynaSceneryInfo) on the right, with the trainsets it declares. After "Load" the background dissolves into the
## loading screen below. Escape asks to quit (quit_requested).

signal scenery_selected(filename: String, train_id: String, skin_overrides: Dictionary)
## Escape - the game fades out and quits
signal quit_requested

const DISSOLVE_TIME: float = 1.0

## UI feedback of the startup screens. Events are named after what happened, not after the
## sample - what each one sounds like is the bank's decision, not this screen's.
const UI_SOUNDS: SfxBank = preload("res://startup/ui_sounds.tres")

const VehicleViewer = preload("res://scenery_selector/vehicle_viewer.gd")
## Sections the keyboard walks through with Tab. The focus is virtual - the search field keeps the
## Godot focus, so typing filters the list whichever section is current.
enum Section { SCENERY, TRAINSETS, VEHICLES, SKINS, ACTIONS }

## Where "back" from the scenery list leads while there is nothing better to come back to: one step
## deeper, the trainsets of the scenery
const DEFAULT_PREVIOUS_SECTION: int = Section.TRAINSETS

var _files: PackedStringArray = []
## Title of each scenery and its item on the list, in the order of _files
var _titles: PackedStringArray = []
var _ui_sounds: SfxPlayer
var _info: MaszynaSceneryInfo = null
## Vehicles of the shown trainset, in the order they run
var _vehicles: Array[MaszynaSceneryInfo.Vehicle] = []
## Vehicle whose viewer is open
var _shown_index: int = -1
## Fade of the scenery list under the viewer, killed when the other fade starts
var _list_fade_tween: Tween = null
## Section the keyboard is on - one of Section, kept as int so no enum cast is needed, and -1 until
## the screen is opened
var _section: int = -1
## Section the focus came from, so a jump to another column can be stepped out of the way it was
## entered: left from the vehicles and right from the sceneries land back on each other
var _previous_section: int = DEFAULT_PREVIOUS_SECTION
## The four sections in the order of Section, so one takes the focus and the rest give it up
var _sections: Array[FocusSection] = []



func _ready() -> void:
    _ui_sounds = SfxPlayer.new()
    _ui_sounds.bank = UI_SOUNDS
    add_child(_ui_sounds)
    _sections.assign([
        %SceneryList, %TrainsetList, %TrainsetGrid, %VehicleViewer.get_skins_section(),
        %ActionsSection
    ])
    # the skins section lives in another scene, so the screen wires it here, and the buttons row
    # has no player of its own to play its focus with
    %VehicleViewer.get_skins_section().focus_requested.connect(focus_skins)
    %ActionsSection.focus_taken.connect(_ui_sounds.play.bind(&"change_focus"))
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


## The build caption is composed, not a msgid a Label translates itself; the tree sends this on
## entering it as well as on every change of the language
func _notification(what:int) -> void:
    if not what == NOTIFICATION_TRANSLATION_CHANGED:
        return
    # Both halves come from the file the build writes (cmake/write_build_number.cmake, stamped
    # "%Y%m%d%H%M%S"), so the label names the library that is actually loaded. A date typed into
    # project.godot cannot do that - it kept showing 2026-09-19 through every build after it.
    var stamp:String = MaszynaRuntime.get_build_number()
    %BuildLabel.text = (tr("Pre-Alpha Demo Release %s-%s-%s (build %s)") % [
        stamp.substr(0, 4), stamp.substr(4, 2), stamp.substr(6, 2), stamp
    ]) if stamp else tr("Pre-Alpha Demo Release (unbuilt)")


func open() -> void:
    (%Background.material as ShaderMaterial).set_shader_parameter("dissolve", 0.0)
    %Content.visible = true
    visible = true
    # activating the list puts the keyboard in its search field
    activate_section(Section.SCENERY)


## Escape on the scenery list is wired straight to this - the list is the first section, so there
## is nothing left to step back to
func request_quit() -> void:
    quit_requested.emit()


## Tab alone: every section takes its own keys, Escape included, while it has the focus.
func _input(event: InputEvent) -> void:
    if not visible or not %Content.visible:
        return
    if event.is_action_pressed("ui_focus_next"):
        _change_section(1)
    elif event.is_action_pressed("ui_focus_prev"):
        _change_section(-1)
    else:
        return
    get_viewport().set_input_as_handled()


## Sections that have something to walk right now: the scenery list is gone under the viewer, the
## trainsets need a scenery, the vehicles a trainset, the skins an open viewer, and "Load" needs
## a scenery to load
func _available_sections() -> Array[int]:
    var sections: Array[int] = []
    if %ListPanel.visible:
        sections.append(Section.SCENERY)
    if %TrainsetList.visible:
        sections.append(Section.TRAINSETS)
    if %TrainsetGrid.visible:
        sections.append(Section.VEHICLES)
    if %VehicleViewer.visible:
        sections.append(Section.SKINS)
    if not %LoadButton.disabled:
        sections.append(Section.ACTIONS)
    return sections


## Tab: the next section that is there, wrapping around. A section change never moves a selection.
func _change_section(step: int) -> void:
    var sections: Array[int] = _available_sections()
    if not sections:
        return
    # a section that went away leaves find() at -1, which lands on the first one
    activate_section(sections[wrapi(sections.find(_section) + step, 0, sections.size())])


## The one way the focus moves on this screen. The sections are exclusive, so none of them takes
## the focus for itself: a click asks with focus_requested, a key leaves with navigate_*, and every
## one of those is wired to one of the focus_* methods below, which all end up here. A section that
## has nothing to walk right now is not activated at all.
func activate_section(section: int) -> void:
    if section == _section or not _available_sections().has(section):
        return
    # -1 is "nothing focused yet", which is no section to come back to
    if _section >= 0:
        _previous_section = _section
    _section = section
    for index: int in _sections.size():
        if index == section:
            _sections[index].grab_section_focus()
        else:
            _sections[index].release_section_focus()


## Where the focus can be asked to go - one per section, so a scene can wire a signal straight to
## the place it means
func focus_scenery_list() -> void:
    activate_section(Section.SCENERY)


func focus_trainset_list() -> void:
    activate_section(Section.TRAINSETS)


func focus_trainset_vehicles() -> void:
    activate_section(Section.VEHICLES)


func focus_skins() -> void:
    activate_section(Section.SKINS)


func focus_actions() -> void:
    activate_section(Section.ACTIONS)


## Back where the focus came from: the scenery list is left to whichever section jumped to it, so
## coming from the vehicles goes back to the vehicles and not to the trainsets above them. A section
## that has gone away since - a trainset change takes its vehicles with it - falls back to the
## trainsets, which is the step deeper from the sceneries anyway.
func focus_previous_section() -> void:
    if _available_sections().has(_previous_section):
        activate_section(_previous_section)
        return
    activate_section(DEFAULT_PREVIOUS_SECTION)


## The sections under the scenery list were rebuilt, so what the focus came from is gone with them
func reset_focus_history() -> void:
    _previous_section = DEFAULT_PREVIOUS_SECTION


## A scenery came up on the list - by key, by click or as the first result of a search
func _on_scenery_list_item_selected(index: int) -> void:
    _show_details(index)


func _on_trainset_list_item_selected(index: int) -> void:
    _show_trainset(index)


## Loads the scenery the list has selected, and does nothing while a search has left none selected.
## The "Load" button and Enter on a row are both wired straight to this, in the scene.
func load_selected_scenery() -> void:
    var index: int = %SceneryList.get_selected()
    if index < 0:
        return
    _ui_sounds.play(&"load_scenery")
    %Content.visible = false
    scenery_selected.emit(
        _files[index], _get_selected_train_id(), _get_skin_overrides()
    )
    var tween: Tween = create_tween()
    tween.tween_property(%Background.material, "shader_parameter/dissolve", 1.0, DISSOLVE_TIME)
    tween.tween_callback(hide)


## The scenery is loaded with the skins of the trainset as they are shown in the grid
func _get_skin_overrides() -> Dictionary[String, String]:
    var overrides: Dictionary[String, String] = {}
    for vehicle: MaszynaSceneryInfo.Vehicle in _vehicles:
        overrides[vehicle.train_id] = vehicle.skin
    return overrides


## The player starts in the headdriver vehicle of the selected trainset
func _get_selected_train_id() -> String:
    var index: int = %TrainsetList.get_selected()
    if not _info or index < 0:
        return ""
    return _info.trainsets[index].get_driver_train_id()


func _show_details(index: int) -> void:
    # another scenery brings other trainsets and other vehicles
    reset_focus_history()
    %LoadButton.disabled = index < 0
    %Image.texture = null
    %Image.visible = false
    _info = null
    if index < 0:
        %Title.text = ""
        %FileName.text = ""
        %Description.text = ""
        %TrainsetsHeader.visible = false
        %TrainsetList.visible = false
        # an empty list reports no selection, which takes the vehicles down with it
        %TrainsetList.set_rows(PackedStringArray(), PackedStringArray())
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
        names.append(_get_trainset_name(trainset))
        notes.append(_format_trainset_note(trainset))
    var has_trainsets: bool = names.size() > 0
    %TrainsetsHeader.visible = has_trainsets
    %TrainsetList.visible = has_trainsets
    # the list selects its first trainset and reports it back, so the vehicles follow from here on
    %TrainsetList.set_rows(names, notes)


## The vehicles of the trainset go to the preview, its mission description to the details
func _show_trainset(index: int) -> void:
    # the viewer shows a vehicle of the trainset that is going away
    if %VehicleViewer.visible:
        %VehicleViewer.close()
    _shown_index = -1
    _vehicles.clear()
    # one exit, and the tiles are built typed: an untyped [] is refused by a typed parameter, and
    # the refusal is a runtime error - the grid would keep the vehicles of the scenery before
    var tiles: Array[TileGrid.Tile] = []
    if _info and index >= 0:
        var trainset: MaszynaSceneryInfo.Trainset = _info.trainsets[index]
        %Description.text = (
            "%s\n\n%s" % [trainset.description, _info.description]
            if trainset.description
            else _info.description
        )
        _vehicles.assign(trainset.vehicles)
        for vehicle: MaszynaSceneryInfo.Vehicle in _vehicles:
            tiles.append(TileGrid.Tile.new(
                vehicle.data_path, vehicle.file_name, vehicle.skin,
                "%s (%s)" % [vehicle.train_id, vehicle.data_path.get_file()]
            ))
    %TrainsetGrid.set_tiles(tiles)


## A skin accepted in the viewer: the trainset shows it and the scenery is loaded with it
func _on_vehicle_viewer_skin_applied(skin: String) -> void:
    _ui_sounds.play(&"apply_skin")
    if _shown_index < 0:
        return
    _vehicles[_shown_index].skin = skin
    %TrainsetGrid.reload_tile(_shown_index, skin)


## A vehicle of the preview was opened - the viewer takes the place of the scenery list, the two
## cross-fade
func _on_trainset_grid_item_activated() -> void:
    var index: int = %TrainsetGrid.get_selected()
    # Enter again on the vehicle already open only walks into its skins: building its model anew
    # would restart the turntable it is rotating on
    if not index == _shown_index:
        _shown_index = index
        %TrainsetGrid.set_marked(index)
        _fade_list_panel(0.0)
        %VehicleViewer.show_vehicle(_vehicles[index])
    activate_section(Section.SKINS)


## Emitted when the viewer starts fading out
func _on_vehicle_viewer_closed() -> void:
    # Escape on the skins closed the viewer, so the keyboard steps back to the vehicle it was opened
    # from - the same way in as the way out
    if _section == Section.SKINS:
        activate_section(Section.VEHICLES)
    _shown_index = -1
    %TrainsetGrid.set_marked(-1)
    _fade_list_panel(1.0)


## The scenery list and the viewer cross-fade in the same slot. Always from the alpha the panel has
## right now and with the tween of the other direction killed first: setting the alpha back to 0 on
## every open, while a fade-in was still running, is what made the two flap.
func _fade_list_panel(to_alpha: float) -> void:
    if _list_fade_tween:
        _list_fade_tween.kill()
    %ListPanel.visible = true
    _list_fade_tween = create_tween()
    _list_fade_tween.tween_property(%ListPanel, "modulate:a", to_alpha, VehicleViewer.FADE_TIME)
    if to_alpha == 0.0:
        _list_fade_tween.tween_callback(%ListPanel.hide)


## A trainset is named by the vehicle the player starts in - the "trainset" line carries the
## starting track, not a name; the vehicles are named in "node <x> <y> <name> dynamic"
static func _get_trainset_name(trainset: MaszynaSceneryInfo.Trainset) -> String:
    return trainset.get_driver_train_id()


static func _format_trainset_note(trainset: MaszynaSceneryInfo.Trainset) -> String:
    return "%d POJAZDÓW" % trainset.vehicles.size()
