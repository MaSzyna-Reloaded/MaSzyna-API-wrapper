extends Control

## Full screen scenario selector: <game_dir>/scenery/*.scn on the left, details of the selected
## one (MaszynaSceneryInfo) on the right, with the trainsets it declares. The vehicles of the shown
## trainset can be changed, taken out, added (VehicleBrowserWindow), moved (Ctrl+Left/Right, a
## drag) and turned round (Ctrl+R), and their skins picked - kept while the game runs, until the
## trainset is restored. After "Load" the background dissolves into the
## loading screen below. Escape asks to quit (quit_requested).

## trainset: the shown trainset as arranged here (MaszynaIncludeNode.trainset_override)
signal scenery_selected(filename: String, train_id: String, trainset: Array[MaszynaDynamicData])
## Escape - the game fades out and quits
signal quit_requested
## The gear - the settings are the game's, not this screen's
signal settings_requested

const DISSOLVE_TIME: float = 1.0
## Project Setting: sceneries listed in folded groups by the first part of their title
const GROUP_SETTING: String = "maszyna/starter/group_sceneries"

## UI feedback of the startup screens. Events are named after what happened, not after the
## sample - what each one sounds like is the bank's decision, not this screen's.
const UI_SOUNDS: SfxBank = preload("res://startup/ui_sounds.tres")

const VehicleViewer = preload("res://scenery_selector/vehicle_viewer.gd")
## Sections the keyboard walks through with Tab. The focus is virtual - the search field keeps the
## Godot focus, so typing filters the list whichever section is current.
enum Section { SCENERY, TRAINSETS, VEHICLES, TRAINSET_EDIT, VEHICLE_ACTIONS, SKINS, ACTIONS }
## What the vehicle chosen in the browser is for: the place of the vehicle open in the viewer, or
## a new one after the selected vehicle
enum VehicleChoice { CHANGE, ADD }

## Where "back" from the scenery list leads while there is nothing better to come back to: one step
## deeper, the trainsets of the scenery
const DEFAULT_PREVIOUS_SECTION: int = Section.TRAINSETS

var _files: PackedStringArray = []
## Title of each scenery and its item on the list, in the order of _files
var _titles: PackedStringArray = []
var _ui_sounds: SfxPlayer
var _info: MaszynaSceneryInfo = null
## The trainsets of the scenery the list shows, in its order - the occupied ones, or all of them
## while the check box asks for those that cannot be driven as well
var _listed_trainsets: Array[MaszynaSceneryInfo.Trainset] = []
## Vehicles of the shown trainset, in the order they run - the arranged ones of _arranged_trainsets
var _vehicles: Array[MaszynaSceneryInfo.Vehicle] = []
## The trainsets as the player arranged them - copies of the scenery's vehicles, by scenery file and
## trainset (_get_trainset_key()), kept while the game runs, so going to another scenery and back
## finds them as they were left
var _arranged_trainsets: Dictionary[String, Array] = {}
## Vehicle whose viewer is open
var _shown_index: int = -1
## What the vehicle browser was opened for, and for CHANGE the vehicle whose place it fills
var _vehicle_choice: VehicleChoice = VehicleChoice.CHANGE
var _changed_index: int = -1
## The vehicle the remove question asks about
var _removed_index: int = -1
## Fade of the scenery list under the viewer, killed when the other fade starts
var _list_fade_tween: Tween = null
## Section the keyboard is on - one of Section, kept as int so no enum cast is needed, and -1 until
## the screen is opened
var _section: int = -1
## Section the focus came from, so a jump to another column can be stepped out of the way it was
## entered: left from the vehicles and right from the sceneries land back on each other
var _previous_section: int = DEFAULT_PREVIOUS_SECTION
## The development notice was asked this launch - it comes once, after the game directory problem
var _development_notice_shown: bool = false
## The four sections in the order of Section, so one takes the focus and the rest give it up
var _sections: Array[FocusSection] = []



func _ready() -> void:
    _ui_sounds = SfxPlayer.new()
    _ui_sounds.bank = UI_SOUNDS
    add_child(_ui_sounds)
    _sections.assign([
        %SceneryList, %TrainsetList, %TrainsetGrid, %TrainsetEditSection,
        %VehicleViewer.get_vehicle_actions_section(), %VehicleViewer.get_skins_section(), %ActionsSection
    ])
    # the viewer's sections live in another scene, so the screen wires them here, and the rows of
    # buttons have no player of their own to play their focus with
    %VehicleViewer.get_skins_section().focus_requested.connect(focus_skins)
    %VehicleViewer.get_skins_section().navigate_up.connect(focus_vehicle_actions)
    %VehicleViewer.get_vehicle_actions_section().navigate_down.connect(focus_skins)
    %VehicleViewer.get_vehicle_actions_section().navigate_up.connect(focus_trainset_vehicles)
    %VehicleViewer.get_vehicle_actions_section().focus_taken.connect(_ui_sounds.play.bind(&"change_focus"))
    %ActionsSection.focus_taken.connect(_ui_sounds.play.bind(&"change_focus"))
    %TrainsetEditSection.focus_taken.connect(_ui_sounds.play.bind(&"change_focus"))
    UserSettings.game_dir_changed.connect(list_sceneries)
    UserSettings.game_dir_changed.connect(update_game_dir_warning)
    update_game_dir_warning()
    # without the game's data the development notice says nothing yet - the problem goes first
    if UserSettings.is_maszyna_game_dir_valid():
        show_development_notice()
    else:
        %GameDirProblem.ask()


## The sceneries of the game directory - read again whenever it changes, the screen opens or the
## refresh button asks. The search typed and the scenery selected stay, while it is still there.
func list_sceneries() -> void:
    var selected: int = %SceneryList.get_selected()
    var selected_file: String = _files[selected] if selected >= 0 else ""
    _files.clear()
    _titles.clear()
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
    var groups: PackedStringArray = []
    if ProjectSettings.get_setting(GROUP_SETTING, true):
        # "Bałtyk · SKM1" goes under "Bałtyk"
        var names: PackedStringArray = []
        for title: String in _titles:
            names.append(title.get_slice(MaszynaSceneryInfo.PART_SEPARATOR, 0))
        # and one scenery written as "Całkowo V2 Towarowe" goes under "Całkowo V2" when there is
        # one - the shortest name of the others its own begins with, as whole words
        for scenery_name: String in names:
            var group_name: String = scenery_name
            for other: String in names:
                if scenery_name.begins_with(other + " ") and other.length() < group_name.length():
                    group_name = other
            groups.append(group_name)
    %SceneryList.title_separator = MaszynaSceneryInfo.PART_SEPARATOR
    # the list selects the scenery and reports it back, so the details follow from here on
    %SceneryList.set_rows(_titles, notes, groups, _files.find(selected_file))


## The build caption is composed, not a msgid a Label translates itself; the tree sends this on
## entering it as well as on every change of the language
func _notification(what:int) -> void:
    if not what == NOTIFICATION_TRANSLATION_CHANGED:
        return
    # Both halves come from the file the build writes (cmake/write_build_number.cmake, stamped
    # "%Y%m%d%H%M%S"), so the label names the library that is actually loaded. A date typed into
    # project.godot cannot do that - it kept showing 2026-09-19 through every build after it.
    var stamp:String = GameDataServer.build_get_number()
    %BuildLabel.text = (tr("Pre-Alpha Demo Release %s-%s-%s (build %s)") % [
        stamp.substr(0, 4), stamp.substr(4, 2), stamp.substr(6, 2), stamp
    ]) if stamp else tr("Pre-Alpha Demo Release (unbuilt)")


## Shown at the start and on every way back from a game - with the sceneries read again, as files
## may have come or changed meanwhile
func open() -> void:
    list_sceneries()
    (%Background.material as ShaderMaterial).set_shader_parameter("dissolve", 0.0)
    %Content.visible = true
    visible = true
    # activating the list puts the keyboard in its search field
    activate_section(Section.SCENERY)


## Escape on the scenery list is wired straight to this - the list is the first section, so there
## is nothing left to step back to
func request_quit() -> void:
    quit_requested.emit()


func request_settings() -> void:
    settings_requested.emit()


## Tab alone: every section takes its own keys, Escape included, while it has the focus. F11 opens
## the settings, as the gear does.
func _input(event: InputEvent) -> void:
    if not visible or not %Content.visible:
        return
    if event.is_action_pressed("settings_open", false, true):
        request_settings()
    elif event.is_action_pressed("menu_next_section", false, true):
        _change_section(1)
    elif event.is_action_pressed("menu_previous_section", false, true):
        _change_section(-1)
    else:
        return
    get_viewport().set_input_as_handled()


## Sections that have something to walk right now: the scenery list is gone under the viewer, the
## trainsets need a scenery, the vehicles and "+" a trainset, the vehicle's buttons and skins an
## open viewer, and "Load" needs a scenery to load
func _available_sections() -> Array[int]:
    var sections: Array[int] = []
    if %ListPanel.visible:
        sections.append(Section.SCENERY)
    if %TrainsetList.visible:
        sections.append(Section.TRAINSETS)
    if not %AddVehicleButton.disabled:
        sections.append(Section.TRAINSET_EDIT)
    if %TrainsetGrid.visible:
        sections.append(Section.VEHICLES)
    if %VehicleViewer.visible:
        sections.append(Section.VEHICLE_ACTIONS)
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


func focus_trainset_edit() -> void:
    activate_section(Section.TRAINSET_EDIT)


func focus_trainset_vehicles() -> void:
    activate_section(Section.VEHICLES)


func focus_vehicle_actions() -> void:
    activate_section(Section.VEHICLE_ACTIONS)


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


## Loads the scenery the list has selected, and does nothing while "Load" is disabled - no scenery
## selected, or a trainset nobody can drive. The "Load" button and Enter on a row are both wired
## straight to this, in the scene.
func load_selected_scenery() -> void:
    if %LoadButton.disabled:
        return
    var index: int = %SceneryList.get_selected()
    _ui_sounds.play(&"load_scenery")
    %Content.visible = false
    scenery_selected.emit(_files[index], _get_selected_train_id(), _get_arranged_trainset())
    var tween: Tween = create_tween()
    tween.tween_property(%Background.material, "shader_parameter/dissolve", 1.0, DISSOLVE_TIME)
    tween.tween_callback(hide)


## The scenery is loaded with the trainset as the grid shows it - its vehicles, their order and
## their skins; an added vehicle has no name of the scenery
func _get_arranged_trainset() -> Array[MaszynaDynamicData]:
    var trainset: Array[MaszynaDynamicData] = []
    for vehicle: MaszynaSceneryInfo.Vehicle in _vehicles:
        var dynamic: MaszynaDynamicData = MaszynaDynamicData.new()
        dynamic.name = vehicle.train_id
        dynamic.data_path = vehicle.data_path
        dynamic.file_name = vehicle.file_name
        dynamic.skin = vehicle.skin
        dynamic.direction = TrackServer.DIRECTION_REVERSED if vehicle.reversed else TrackServer.DIRECTION_NORMAL
        trainset.append(dynamic)
    return trainset


## The player starts in the headdriver vehicle of the selected trainset
func _get_selected_train_id() -> String:
    var index: int = %TrainsetList.get_selected()
    if not _info or index < 0:
        return ""
    return _listed_trainsets[index].get_driver_train_id()


func _show_details(index: int) -> void:
    # another scenery brings other trainsets and other vehicles
    reset_focus_history()
    %Image.texture = null
    %Image.visible = false
    _info = null
    if index < 0:
        %Title.text = ""
        %FileName.text = ""
        %Description.text = ""
        %TrainsetsHeader.visible = false
        %AllTrainsetsSwitch.visible = false
        %TrainsetList.visible = false
        _listed_trainsets.clear()
        # an empty list reports no selection, which takes the vehicles and "Load" down with it
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
    var has_trainsets: bool = _info.trainsets.size() > 0
    %TrainsetsHeader.visible = has_trainsets
    %AllTrainsetsSwitch.visible = has_trainsets
    _list_trainsets()


## The trainsets of the scenery on the list: the occupied ones the scenario offers, and while the
## check box is on all the others - its AI and decoration trainsets, and those that cannot be
## driven, which can be looked at and reskinned, not loaded
func _list_trainsets() -> void:
    _listed_trainsets.clear()
    var names: PackedStringArray = []
    var notes: PackedStringArray = []
    for trainset: MaszynaSceneryInfo.Trainset in _info.trainsets:
        if not trainset.is_drivable() and not %AllTrainsetsSwitch.button_pressed:
            continue
        _listed_trainsets.append(trainset)
        names.append(_get_trainset_name(trainset))
        notes.append(_format_trainset_note(trainset))
    %TrainsetList.visible = names.size() > 0
    # the list selects its first trainset and reports it back, so the vehicles follow from here on
    %TrainsetList.set_rows(names, notes)


func _on_all_trainsets_switch_toggled(_toggled_on: bool) -> void:
    _list_trainsets()


## The vehicles of the trainset go to the preview, its mission description to the details; "Load"
## takes a scenery, and refuses a trainset nobody can drive - a scenery without a trainset listed
## is loaded to walk around it
func _show_trainset(index: int) -> void:
    %LoadButton.disabled = not _info or (index >= 0 and not _listed_trainsets[index].is_occupied())
    # the viewer shows a vehicle of the trainset that is going away
    if %VehicleViewer.visible:
        %VehicleViewer.close()
    _shown_index = -1
    var vehicles: Array[MaszynaSceneryInfo.Vehicle] = []
    _vehicles = vehicles
    if _info and index >= 0:
        var trainset: MaszynaSceneryInfo.Trainset = _listed_trainsets[index]
        %Description.text = (
            "%s\n\n%s" % [trainset.description, _info.description]
            if trainset.description
            else _info.description
        )
        # copies: the scenery's own vehicles are read again with every scenery shown
        var key: String = _get_trainset_key(trainset)
        if not _arranged_trainsets.has(key):
            for vehicle: MaszynaSceneryInfo.Vehicle in trainset.vehicles:
                vehicles.append(vehicle.copy())
            _arranged_trainsets[key] = vehicles
        _vehicles = _arranged_trainsets[key]
    _show_vehicles(0)


## The vehicles of the trainset in the grid, the one at `selected` selected and the open one
## marked, and "+" with them
func _show_vehicles(selected: int) -> void:
    # the tiles are built typed: an untyped [] is refused by a typed parameter, and the refusal is
    # a runtime error - the grid would keep the vehicles of the scenery before
    var tiles: Array[TileGrid.Tile] = []
    for vehicle: MaszynaSceneryInfo.Vehicle in _vehicles:
        var tile: TileGrid.Tile = TileGrid.Tile.new(
            vehicle.data_path, vehicle.file_name, vehicle.skin, vehicle.train_id,
            "%s (%s)" % [vehicle.train_id, vehicle.data_path.get_file()]
        )
        tile.flipped = vehicle.reversed
        tile.removable = _is_removable(tiles.size())
        tiles.append(tile)
    %TrainsetGrid.set_tiles(tiles)
    # the buttons at the end of the row keep their room while they cannot be used, so nothing moves
    %AddVehicleButton.disabled = not tiles
    %TrainsetEditSection.modulate.a = 1.0 if tiles else 0.0
    _show_trainset_changes()
    if not tiles:
        return
    %TrainsetGrid.select(selected)
    %TrainsetGrid.set_marked(_shown_index)
    if _shown_index >= 0:
        %VehicleViewer.set_removable(_is_removable(_shown_index))


## Whether the trainset is arranged otherwise than the scenario gives it: its note on the list says
## so, and the restore button can take it back
func _show_trainset_changes() -> void:
    var index: int = %TrainsetList.get_selected()
    var modified: bool = index >= 0 and _is_trainset_modified(_listed_trainsets[index])
    %RestoreTrainsetButton.disabled = not modified
    if index >= 0:
        %TrainsetList.set_row_note(index, _format_trainset_note(_listed_trainsets[index]))


## The trainset's own vehicles and the arranged ones differ in their number, order, files or skins
func _is_trainset_modified(trainset: MaszynaSceneryInfo.Trainset) -> bool:
    var arranged: Array = _arranged_trainsets.get(_get_trainset_key(trainset), trainset.vehicles)
    if not arranged.size() == trainset.vehicles.size():
        return true
    for index: int in arranged.size():
        if not (arranged[index] as MaszynaSceneryInfo.Vehicle).is_same(trainset.vehicles[index]):
            return true
    return false


## A trainset of the scenery shown, as _arranged_trainsets knows it: the scenery file and its place
## among the scenery's trainsets
func _get_trainset_key(trainset: MaszynaSceneryInfo.Trainset) -> String:
    return "%s/%d" % [_files[%SceneryList.get_selected()], _info.trainsets.find(trainset)]


## The restore button asks first - what was arranged goes for good
func ask_restore_trainset() -> void:
    %RestoreTrainsetQuestion.ask()


## The trainset as the scenario gives it, the arranged one dropped
func restore_trainset() -> void:
    var index: int = %TrainsetList.get_selected()
    if index < 0:
        return
    _arranged_trainsets.erase(_get_trainset_key(_listed_trainsets[index]))
    _show_trainset(index)


## The trash takes neither the vehicle with a driver - the player's - nor the last one
func _is_removable(index: int) -> bool:
    return _vehicles.size() > 1 and not _vehicles[index].has_driver()


## "Change vehicle" in the viewer or on a tile: the browser chooses another vehicle for its place
func open_vehicle_browser_to_change(index: int) -> void:
    _vehicle_choice = VehicleChoice.CHANGE
    _changed_index = index
    %VehicleBrowser.open()


func _on_vehicle_viewer_change_requested() -> void:
    open_vehicle_browser_to_change(_shown_index)


func _on_vehicle_viewer_remove_requested() -> void:
    ask_remove_vehicle(_shown_index)


## The trash, in the viewer or on a tile, asks first - the vehicle is named in the question
func ask_remove_vehicle(index: int) -> void:
    if index < 0 or not _is_removable(index):
        return
    _removed_index = index
    var vehicle: MaszynaSceneryInfo.Vehicle = _vehicles[index]
    %RemoveVehicleQuestion.message = tr("%s leaves the trainset.") % (
        vehicle.train_id if vehicle.train_id else vehicle.file_name
    )
    %RemoveVehicleQuestion.ask()


func _on_remove_vehicle_question_confirmed() -> void:
    remove_vehicle(_removed_index)


func _on_vehicle_viewer_reverse_requested() -> void:
    reverse_vehicle(_shown_index)


## "+": the browser chooses a vehicle to go after the selected one
func open_vehicle_browser_to_add() -> void:
    _vehicle_choice = VehicleChoice.ADD
    %VehicleBrowser.open()


func _on_vehicle_browser_vehicle_chosen(data_path: String, file_name: String, skin: String) -> void:
    match _vehicle_choice:
        VehicleChoice.CHANGE:
            change_vehicle(data_path, file_name, skin)
        VehicleChoice.ADD:
            add_vehicle(data_path, file_name, skin)


## The vehicle the browser was opened for becomes another one; its place, name, direction and
## crew stay, and the viewer shows the new one when it was open on it
func change_vehicle(data_path: String, file_name: String, skin: String) -> void:
    if _changed_index < 0:
        return
    var vehicle: MaszynaSceneryInfo.Vehicle = _vehicles[_changed_index]
    vehicle.data_path = data_path
    vehicle.file_name = file_name
    vehicle.skin = skin
    _show_vehicles(_changed_index)
    if _changed_index == _shown_index:
        %VehicleViewer.show_vehicle(vehicle)


## A vehicle after the selected one, with nobody aboard and no name of the scenery
func add_vehicle(data_path: String, file_name: String, skin: String) -> void:
    var index: int = %TrainsetGrid.get_selected() + 1
    var vehicle: MaszynaSceneryInfo.Vehicle = MaszynaSceneryInfo.Vehicle.new()
    vehicle.data_path = data_path
    vehicle.file_name = file_name
    vehicle.skin = skin
    _vehicles.insert(index, vehicle)
    if _shown_index >= index:
        _shown_index += 1
    _show_vehicles(index)


## The vehicle leaves the trainset - asked and answered (ask_remove_vehicle()); the viewer closes
## when it was open on it
func remove_vehicle(index: int) -> void:
    if index < 0 or not _is_removable(index):
        return
    _vehicles.remove_at(index)
    if index == _shown_index:
        %VehicleViewer.close()
    elif _shown_index > index:
        _shown_index -= 1
    _show_vehicles(mini(index, _vehicles.size() - 1))


## Ctrl+R or the tile's button: the vehicle stands the other way round, and its side view with it
func reverse_vehicle(index: int) -> void:
    if index < 0:
        return
    _vehicles[index].reversed = not _vehicles[index].reversed
    %TrainsetGrid.set_tile_flipped(index, _vehicles[index].reversed)
    _show_trainset_changes()


## Ctrl+Left and Ctrl+Right, or the tile's buttons: the vehicle one place that way
func move_vehicle_left(index: int) -> void:
    move_vehicle(index, index - 1)


func move_vehicle_right(index: int) -> void:
    move_vehicle(index, index + 1)


## The vehicle at `from` goes to the place `to` - a key, or a tile dragged onto another - and the
## ones between close up; the open one stays open wherever it went
func move_vehicle(from: int, to: int) -> void:
    if from < 0 or to < 0 or to >= _vehicles.size() or from == to:
        return
    var shown: MaszynaSceneryInfo.Vehicle = _vehicles[_shown_index] if _shown_index >= 0 else null
    _vehicles.insert(to, _vehicles.pop_at(from))
    _shown_index = _vehicles.find(shown) if shown else -1
    # the tile moves as it is - a rebuilt row would render and scroll anew
    %TrainsetGrid.move_tile(from, to)
    %TrainsetGrid.select(to)
    _show_trainset_changes()


## A skin accepted in the viewer: the trainset shows it and the scenery is loaded with it
func _on_vehicle_viewer_skin_applied(skin: String) -> void:
    _ui_sounds.play(&"apply_skin")
    if _shown_index < 0:
        return
    _vehicles[_shown_index].skin = skin
    %TrainsetGrid.reload_tile(_shown_index, skin)
    _show_trainset_changes()


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
        %VehicleViewer.set_removable(_is_removable(index))
    activate_section(Section.SKINS)


## Emitted when the viewer starts fading out
func _on_vehicle_viewer_closed() -> void:
    # Escape in the viewer closed it, so the keyboard steps back to the vehicle it was opened from -
    # the same way in as the way out
    if _section == Section.SKINS or _section == Section.VEHICLE_ACTIONS:
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


## Its vehicles as arranged, and whether that is not as the scenario gives them
func _format_trainset_note(trainset: MaszynaSceneryInfo.Trainset) -> String:
    var arranged: Array = _arranged_trainsets.get(_get_trainset_key(trainset), trainset.vehicles)
    var note: String = "%d POJAZDÓW" % arranged.size()
    if _is_trainset_modified(trainset):
        note += " · " + tr("modified").to_upper()
    return note


## The warning stays over the screen while the game directory holds no game data
func update_game_dir_warning() -> void:
    %GameDirWarning.visible = not UserSettings.is_maszyna_game_dir_valid()
    %WarningDir.text = UserSettings.get_maszyna_game_dir()


## The directory was changed in a game, which ended for it - the data is the new one's now
func show_game_dir_changed() -> void:
    %GameDirChangedNotice.message = (
        tr("The game's data is now read from %s.") % UserSettings.get_maszyna_game_dir()
    )
    %GameDirChangedNotice.ask()


## Chosen here, the directory is the game's at once - saved, as the settings' "Save" would
func _on_game_dir_window_game_dir_chosen(path: String) -> void:
    UserSettings.save_maszyna_game_dir(path)


## Shown over the screen once on every launch, until the player acknowledges it - after the game
## directory problem, when there is one
func show_development_notice() -> void:
    if _development_notice_shown:
        return
    _development_notice_shown = true
    %DevelopmentNotice.ask()


## The development notice is acknowledged - its button, Enter or Escape
func _on_development_notice_acknowledged() -> void:
    _ui_sounds.play(&"notice_acknowledged")
