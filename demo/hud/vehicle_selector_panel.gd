extends PanelContainer

## The vehicle selector: every vehicle with a driver, AI or player, as the original's vehicle list
## (vehiclelist.cpp:28-44), and the player's own vehicle when it has none - of a scenario, only the
## vehicles of the trainsets the player can drive (MaszynaSceneryInfo.Trainset.is_drivable()) until
## the switch shows all of them, as the scenario selector does. Each row shows how its
## train is going and opens its card and the operator's actions. The rows follow the persons
## getting on and off and the drivers freed; what they show is refreshed by one Timer while the
## panel is open.

## The close button asks the owner of the View menu to hide the panel and untick its entry
signal close_requested
## A row was clicked: its card is to be the one the selector has open, or closed if it is already
signal vehicle_activated(vehicle:RID)
## The player confirmed that the vehicle's trainset is to be removed
signal remove_trainset_requested(vehicle:RID)

const ROW:PackedScene = preload("vehicle_selector_row.tscn")

## The rows, by the vehicle each shows
var _rows:Dictionary[RID, VehicleSelectorRow] = {}
## The vehicle the player drives or last drove, listed with or without a driver
var _player_vehicle:RID = RID()
## The vehicle of the active row, whose card the selector has open; lit
var _active_vehicle:RID = RID()
## The scenery's names of the vehicles of the trainsets the player can drive
var _drivable_vehicle_names:Dictionary[String, bool] = {}
## Without a scenario (a scene put together by hand) every vehicle is listed
var _has_scenario:bool = false


func _ready() -> void:
    VehicleServer.cabin_person_entered.connect(_on_cabin_person_entered)
    VehicleServer.cabin_person_left.connect(_on_cabin_person_left)
    DriverSystem.driver_attached.connect(_on_driver_changed)
    DriverSystem.driver_freed.connect(_on_driver_changed)
    VehicleServer.vehicle_freed.connect(_on_vehicle_freed)
    _update_rows()


func _exit_tree() -> void:
    VehicleServer.cabin_person_entered.disconnect(_on_cabin_person_entered)
    VehicleServer.cabin_person_left.disconnect(_on_cabin_person_left)
    DriverSystem.driver_attached.disconnect(_on_driver_changed)
    DriverSystem.driver_freed.disconnect(_on_driver_changed)
    VehicleServer.vehicle_freed.disconnect(_on_vehicle_freed)


## The vehicle of the active row, lit alone; an invalid RID leaves none lit
func show_active_vehicle(vehicle:RID) -> void:
    _active_vehicle = vehicle
    for row_vehicle:RID in _rows:
        _rows[row_vehicle].show_active(row_vehicle == _active_vehicle)


## The scenario played: its trainsets the player cannot drive are listed only with the switch on
func show_scenario(info:MaszynaSceneryInfo) -> void:
    _drivable_vehicle_names.clear()
    _has_scenario = info != null
    if info:
        for trainset:MaszynaSceneryInfo.Trainset in info.trainsets:
            if not trainset.is_drivable():
                continue
            for vehicle:MaszynaSceneryInfo.Vehicle in trainset.vehicles:
                _drivable_vehicle_names[vehicle.train_id] = true
    %AllTrainsetsSwitch.visible = _has_scenario
    _update_rows()


## The vehicle the player drives or last drove, or none
func follow_player_vehicle(vehicle:RID) -> void:
    var previous:RID = _player_vehicle
    _player_vehicle = vehicle
    _update_row(previous)
    _update_row(vehicle)


## Whoever gets on may be a driver; the row decides from the vehicle (_update_row())
func _on_cabin_person_entered(cabin:RID, _person:RID, _role:VehiclePersonRole.Role) -> void:
    _update_row(VehicleServer.cabin_get_vehicle(cabin))


func _on_cabin_person_left(cabin:RID, _person:RID) -> void:
    _update_row(VehicleServer.cabin_get_vehicle(cabin))


## A person aboard became a driver, or a driver was freed while still aboard - one freed off the
## vehicle has left it already
func _on_driver_changed(driver:RID) -> void:
    _update_row(VehicleServer.person_get_vehicle(driver))


func _on_vehicle_freed(vehicle:RID) -> void:
    if vehicle == _player_vehicle:
        _player_vehicle = RID()
    _update_row(vehicle)


## Every vehicle that may be listed: those with a driver and the player's
func _update_rows() -> void:
    for driver:RID in DriverSystem.driver_get_rids():
        _update_row(VehicleServer.person_get_vehicle(driver))
    _update_row(_player_vehicle)


## The vehicle is listed while it has a driver or is the player's - of a scenario, one the player
## can drive unless the switch shows all; a new row goes in by name
func _update_row(vehicle:RID) -> void:
    if not vehicle.is_valid():
        return
    var listed:bool = VehicleServer.vehicle_exists(vehicle) and (
            DriverSystem.vehicle_get_driver(vehicle).is_valid() or vehicle == _player_vehicle) and (
            vehicle == _player_vehicle or not _has_scenario or %AllTrainsetsSwitch.button_pressed
            or _drivable_vehicle_names.has(VehicleServer.vehicle_get_name(vehicle)))
    var row:VehicleSelectorRow = _rows.get(vehicle)
    if row and not listed:
        _rows.erase(vehicle)
        row.queue_free()
    elif listed and not row:
        row = ROW.instantiate()
        row.vehicle = vehicle
        row.activated.connect(vehicle_activated.emit)
        row.remove_requested.connect(remove_trainset_requested.emit)
        row.show_active(vehicle == _active_vehicle)
        var name:String = VehicleServer.vehicle_get_name(vehicle)
        var index:int = 0
        while index < %Vehicles.get_child_count() and \
                VehicleServer.vehicle_get_name((%Vehicles.get_child(index) as VehicleSelectorRow).vehicle) < name:
            index += 1
        %Vehicles.add_child(row)
        %Vehicles.move_child(row, index)
        _rows[vehicle] = row
        row.refresh()
    if listed:
        row.show_player_vehicle(vehicle == _player_vehicle)
    %Empty.visible = not _rows


func _on_visibility_changed() -> void:
    if not visible:
        %RefreshTimer.stop()
        return
    _on_refresh_timer_timeout()
    %RefreshTimer.start()


func _on_refresh_timer_timeout() -> void:
    for row:VehicleSelectorRow in _rows.values():
        row.refresh()


func _on_all_trainsets_switch_toggled(_toggled_on:bool) -> void:
    _update_rows()


func _on_close_button_pressed() -> void:
    close_requested.emit()
