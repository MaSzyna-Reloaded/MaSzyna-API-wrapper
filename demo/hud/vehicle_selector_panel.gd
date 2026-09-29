extends PanelContainer

## The vehicle selector: every vehicle with a driver, AI or player, as the original's vehicle list
## (vehiclelist.cpp:28-44), and the player's own vehicle when it has none. Each row shows how its
## train is going and opens its card and the operator's actions. The rows follow the drivers as
## they come and go; what they show is refreshed by one Timer while the panel is open.

## The close button asks the owner of the View menu to hide the panel and untick its entry
signal close_requested
## A row was clicked: its card is to be the one the selector has open, or closed if it is already
signal vehicle_activated(vehicle:RID)
## The player confirmed that the vehicle's trainset is to be removed
signal remove_trainset_requested(vehicle:RID)

const ROW:PackedScene = preload("vehicle_selector_row.tscn")

## The rows, by the vehicle each shows
var _rows:Dictionary[RID, VehicleSelectorRow] = {}
## The vehicle each driver drives, to find the row a driver leaves
var _driver_vehicles:Dictionary[RID, RID] = {}
## The vehicle the player drives or last drove, listed with or without a driver
var _player_vehicle:RID = RID()
## The vehicle of the active row, whose card the selector has open; lit
var _active_vehicle:RID = RID()


func _ready() -> void:
    DriverSystem.driver_vehicle_attached.connect(_on_driver_vehicle_attached)
    DriverSystem.driver_freed.connect(_on_driver_freed)
    RailVehicleServer.vehicle_freed.connect(_on_vehicle_freed)
    for driver:RID in DriverSystem.driver_get_rids():
        _on_driver_vehicle_attached(driver, DriverSystem.driver_get_vehicle(driver))


func _exit_tree() -> void:
    DriverSystem.driver_vehicle_attached.disconnect(_on_driver_vehicle_attached)
    DriverSystem.driver_freed.disconnect(_on_driver_freed)
    RailVehicleServer.vehicle_freed.disconnect(_on_vehicle_freed)


## The vehicle of the active row, lit alone; an invalid RID leaves none lit
func show_active_vehicle(vehicle:RID) -> void:
    _active_vehicle = vehicle
    for row_vehicle:RID in _rows:
        _rows[row_vehicle].show_active(row_vehicle == _active_vehicle)


## The vehicle the player drives or last drove, or none
func follow_player_vehicle(vehicle:RID) -> void:
    var previous:RID = _player_vehicle
    _player_vehicle = vehicle
    _update_row(previous)
    _update_row(vehicle)


func _on_driver_vehicle_attached(driver:RID, vehicle:RID) -> void:
    var previous:RID = _driver_vehicles.get(driver, RID())
    _driver_vehicles[driver] = vehicle
    _update_row(previous)
    _update_row(vehicle)


func _on_driver_freed(driver:RID) -> void:
    var vehicle:RID = _driver_vehicles.get(driver, RID())
    _driver_vehicles.erase(driver)
    _update_row(vehicle)


func _on_vehicle_freed(vehicle:RID) -> void:
    if vehicle == _player_vehicle:
        _player_vehicle = RID()
    _update_row(vehicle)


## The vehicle is listed while it has a driver or is the player's; a new row goes in by name
func _update_row(vehicle:RID) -> void:
    if not vehicle.is_valid():
        return
    var listed:bool = RailVehicleServer.vehicle_exists(vehicle) and (
            DriverSystem.vehicle_get_driver(vehicle).is_valid() or vehicle == _player_vehicle)
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
        var name:String = RailVehicleServer.vehicle_get_name(vehicle)
        var index:int = 0
        while index < %Vehicles.get_child_count() and \
                RailVehicleServer.vehicle_get_name((%Vehicles.get_child(index) as VehicleSelectorRow).vehicle) < name:
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


func _on_close_button_pressed() -> void:
    close_requested.emit()
