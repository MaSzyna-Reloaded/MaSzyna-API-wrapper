class_name VehicleBrowserWindow
extends Control

## Every vehicle of the game (MaszynaVehiclesBank) in a window of the game (UIWindow) over the
## screen that opened it: a search field over the side views of the vehicles found, PAGE_SIZE of
## them at a time, of the category chosen in the sidebar (MaszynaVehiclesBank.Category). Enter or
## "Select vehicle" chooses the selected one, Escape or the close button leaves without; what the
## choice means is its owner's. The categories, the grid and the buttons are its three sections,
## Tab goes round them.

## A vehicle was chosen - its directory as a scenery names it ("dynamic/pkp/en57_v1"), its file
## name and its first skin
signal vehicle_chosen(data_path: String, file_name: String, skin: String)
## The window went, with a vehicle chosen or without
signal closed

## The part of the game's window it takes, each way
const WINDOW_RATIO: float = 0.8
## Vehicles on one page of the grid - as many as the editor's vehicles panel shows
const PAGE_SIZE: int = 50
## The sidebar's first row, every category together
const ALL_CATEGORIES: int = -1
## The categories' names on the sidebar, as msgids
const CATEGORY_NAMES: Dictionary[MaszynaVehiclesBank.Category, String] = {
    MaszynaVehiclesBank.Category.ELECTRIC_LOCOS: "Electric locos",
    MaszynaVehiclesBank.Category.DIESEL_LOCOS: "Diesel locos",
    MaszynaVehiclesBank.Category.STEAM_LOCOS: "Steam locos",
    MaszynaVehiclesBank.Category.RAILCARS: "Railcars",
    MaszynaVehiclesBank.Category.EMU: "EMU",
    MaszynaVehiclesBank.Category.UTILITY: "Utility",
    MaszynaVehiclesBank.Category.DRAISINES: "Draisines",
    MaszynaVehiclesBank.Category.TRAMS: "Trams",
    MaszynaVehiclesBank.Category.TRUCKS: "Trucks",
    MaszynaVehiclesBank.Category.BUSES: "Buses",
    MaszynaVehiclesBank.Category.CARS: "Cars",
    MaszynaVehiclesBank.Category.PEOPLE: "People",
    MaszynaVehiclesBank.Category.ANIMALS: "Animals",
    MaszynaVehiclesBank.Category.CARRIAGES: "Carriages",
    MaszynaVehiclesBank.Category.UNKNOWN: "Unknown",
}

## Every vehicle of the game directory they were read from - read once per directory, a scan of
## the whole dynamic/ is no work for every opening
var _vehicles: Array[MaszynaVehiclesBank.Vehicle] = []
var _vehicles_game_dir: String = ""
## The vehicles the search found, and the page of them the grid shows
var _found: Array[MaszynaVehiclesBank.Vehicle] = []
var _page: int = 0
## The category of each sidebar row - ALL_CATEGORIES first, then those the vehicles have - and the
## one chosen
var _listed_categories: Array[int] = []
var _category: int = ALL_CATEGORIES


## Shown as it was left - the search, the page, the selected vehicle and the scroll stay; only
## another game directory has other vehicles to read and search anew
func open() -> void:
    var game_dir: String = UserSettings.get_maszyna_game_dir()
    if not game_dir == _vehicles_game_dir:
        _vehicles = MaszynaVehiclesBank.scan(game_dir)
        _vehicles_game_dir = game_dir
        _list_categories()
    visible = true
    %Window.show_part(WINDOW_RATIO)
    %SearchInput.grab_typing_focus()
    focus_vehicles()


func close() -> void:
    %CategoryList.release_section_focus()
    %VehicleGrid.release_section_focus()
    %ActionsSection.release_section_focus()
    %SearchInput.release_typing_focus()
    %Window.hide()
    visible = false
    closed.emit()


## The categories the vehicles have, in their order; the list selects the first row - every
## vehicle - and reports it back, so the search follows from there and counts them
func _list_categories() -> void:
    var categories: Dictionary[int, bool] = {}
    for vehicle: MaszynaVehiclesBank.Vehicle in _vehicles:
        categories[vehicle.category] = true
    _listed_categories.assign([ALL_CATEGORIES])
    var names: PackedStringArray = [tr("All vehicles")]
    for category: MaszynaVehiclesBank.Category in CATEGORY_NAMES:
        if categories.has(category):
            _listed_categories.append(category)
            names.append(tr(CATEGORY_NAMES[category]))
    var notes: PackedStringArray = []
    notes.resize(names.size())
    %CategoryList.set_rows(names, notes)


func _on_category_list_item_selected(index: int) -> void:
    _category = _listed_categories[index] if index >= 0 else ALL_CATEGORIES
    search_vehicles()


## The vehicles of the chosen category whose name, directory or a skin holds the search text, from
## their first page. Every category counts what the search found in it, so the sidebar tells where
## else the text is.
func search_vehicles() -> void:
    var matching: Array[MaszynaVehiclesBank.Vehicle] = MaszynaVehiclesBank.filter(_vehicles, %SearchInput.get_text())
    var counts: Dictionary[int, int] = {ALL_CATEGORIES: matching.size()}
    for vehicle: MaszynaVehiclesBank.Vehicle in matching:
        counts[vehicle.category] = counts.get(vehicle.category, 0) + 1
    for row: int in _listed_categories.size():
        %CategoryList.set_row_note(row, str(counts.get(_listed_categories[row], 0)))
    _found = matching
    if not _category == ALL_CATEGORIES:
        _found = matching.filter(
            func(vehicle: MaszynaVehiclesBank.Vehicle) -> bool: return vehicle.category == _category
        )
    show_page(0)


func show_page(page: int) -> void:
    var page_count: int = maxi(ceili(float(_found.size()) / PAGE_SIZE), 1)
    _page = clampi(page, 0, page_count - 1)
    %PageLabel.text = "%d/%d" % [_page + 1, page_count]
    %PreviousPageButton.disabled = _page <= 0
    %NextPageButton.disabled = _page >= page_count - 1
    %SelectButton.disabled = not _found
    var tiles: Array[TileGrid.Tile] = []
    for vehicle: MaszynaVehiclesBank.Vehicle in _found.slice(_page * PAGE_SIZE, (_page + 1) * PAGE_SIZE):
        tiles.append(TileGrid.Tile.new(
            vehicle.data_path, vehicle.file_name, vehicle.skins[0] if vehicle.skins else "", "",
            vehicle.data_path.path_join(vehicle.file_name), vehicle.file_name
        ))
    # an empty grid hides itself; the column keeps its room and says so, with the way back from a
    # search that found nothing
    %VehicleGrid.set_tiles(tiles)
    %NoneFound.visible = not tiles
    %ClearSearchButton.visible = not tiles and not %SearchInput.get_text() == ""


## The next page; past the last one the keyboard steps down to the buttons
func show_next_page() -> void:
    if _page * PAGE_SIZE + PAGE_SIZE >= _found.size():
        focus_actions()
        return
    show_page(_page + 1)


func show_previous_page() -> void:
    show_page(_page - 1)


## Up from the first row of a page: the page before, its last vehicle selected - the one right
## above where the keyboard was
func _on_vehicle_grid_navigate_up() -> void:
    if _page <= 0:
        return
    show_previous_page()
    %VehicleGrid.select(PAGE_SIZE - 1)


## The vehicle the grid has selected goes to the owner, and the window closes
func choose_selected_vehicle() -> void:
    var index: int = %VehicleGrid.get_selected()
    if index < 0:
        return
    var vehicle: MaszynaVehiclesBank.Vehicle = _found[_page * PAGE_SIZE + index]
    # the bank names the directory as a "dynamic" line does, with a slash first; a scenery does not
    vehicle_chosen.emit(
        vehicle.data_path.trim_prefix("/"), vehicle.file_name, vehicle.skins[0] if vehicle.skins else ""
    )
    close()


## The categories, the grid and the buttons are exclusive; every way between them ends in one of
## these three
func focus_categories() -> void:
    %VehicleGrid.release_section_focus()
    %ActionsSection.release_section_focus()
    %CategoryList.grab_section_focus()


func focus_vehicles() -> void:
    if not %VehicleGrid.visible:
        focus_actions()
        return
    %CategoryList.release_section_focus()
    %ActionsSection.release_section_focus()
    %VehicleGrid.grab_section_focus()


func focus_actions() -> void:
    %CategoryList.release_section_focus()
    %VehicleGrid.release_section_focus()
    %ActionsSection.grab_section_focus()


## The search waits for the typing to stop - a page of side views is no work for every key
func _on_search_input_typed(_text: String) -> void:
    %SearchDebounce.start()


## Tab and Shift+Tab: the next or the previous of the three sections
func _on_window_window_input(event: InputEvent) -> void:
    var step: int = (
        1 if event.is_action_pressed("menu_next_section", true, true)
        else -1 if event.is_action_pressed("menu_previous_section", true, true)
        else 0
    )
    if step == 0:
        return
    %Window.set_input_as_handled()
    var sections: Array[FocusSection] = [%CategoryList, %VehicleGrid, %ActionsSection]
    var at: int = sections.find_custom(func(section: FocusSection) -> bool: return section.focused)
    [focus_categories, focus_vehicles, focus_actions][wrapi(at + step, 0, sections.size())].call()
