class_name VehicleCard
extends PanelContainer

## The card of a driven vehicle - what the original's vehicle parameters panel shows of it
## (vehicleparams.cpp) laid out anew: its name, the side views of its trainset
## as the scenery selector previews them, its data, how it is going now, its trainset, its driver
## and its timetable. The data is filled once; what changes - the timetable's delay counting on
## too - is refreshed by one Timer while the card is open.
## The driver section takes the vehicle over (Take over), puts the player in its cab with its
## driver driving on (Enter cabin) and switches its AI.

## The close button asks the owner to free the card
signal close_requested
## The player confirmed that the vehicle's trainset is to be removed
signal remove_trainset_requested(vehicle:RID)

const KILOGRAMS_PER_TONNE:float = 1000.0
## A driver's speed below zero is no limit (VelNext = -1, Driver.h)
const NO_VELOCITY:float = 0.0
## What the train's dispatch at a stop is doing (StationServer)
const DISPATCH_STEP_NAMES:Dictionary[StationServer.DispatchStep, String] = {
    StationServer.DISPATCH_STEP_EXCHANGE: "Passenger exchange",
    StationServer.DISPATCH_STEP_WAIT_DEPARTURE: "Waiting for departure",
    StationServer.DISPATCH_STEP_CLOSE_DOORS: "Closing doors",
}

## The vehicle the card was opened for
var vehicle:RID = RID()

## The vehicle of the trainset whose data the card shows - the one it was opened for, or the one
## clicked in the trainset's side views
var _shown:RID = RID()
## The trainset's vehicles, in the order of their side views
var _trainset:Array[RID] = []
## Every vehicle coupled to the one shown, the side views' and the rest - a coupling change of any
## of them changes the trainset
var _coupled:Array[RID] = []

## The caption and value labels of each caption, per grid, made when the caption first appears
var _values:Dictionary[GridContainer, Dictionary] = {}


## The card of the vehicle: its trainset's side views, and its own data
func show_vehicle(p_vehicle:RID) -> void:
    vehicle = p_vehicle
    _show_trainset()
    _show_trainset_vehicle(vehicle)


## The follow button is on while the camera follows the vehicle shown
func _on_camera_changed() -> void:
    %FollowButton.set_pressed_no_signal(PlayerCameraServer.camera_get_mode() == PlayerCameraServer.CAMERA_MODE_FOLLOW
            and PlayerCameraServer.camera_get_target() == _shown)


func _on_player_vehicle_changed(_vehicle:RID, _previous:RID) -> void:
    _on_refresh_timer_timeout()


## A coupling changed in the trainset: its side views again, and the vehicle shown kept while it is
## still in it
func _on_vehicle_trainset_changed(p_vehicle:RID) -> void:
    if not _coupled.has(p_vehicle):
        return
    _show_trainset()
    _show_trainset_vehicle(_shown if _trainset.has(_shown) else vehicle)


## The trainset as the scenery selector previews it, and the vehicles of it the next coupling
## change is listened for
func _show_trainset() -> void:
    var tiles:Array[TileGrid.Tile] = []
    _trainset.clear()
    _coupled.assign(RailVehicleServer.vehicle_get_coupled(
            vehicle, RailVehicleController.COUPLER_END_FRONT, RailVehicleController.COUPLING_FLAG_COUPLER))
    for trainset_vehicle:RID in _coupled:
        # a vehicle of MaszynaLegacyVehicleSystem keeps the data path, file and skin it was made of
        if not MaszynaLegacyVehicleSystem.vehicle_exists(trainset_vehicle):
            continue
        var dynamic:MaszynaDynamicData = MaszynaLegacyVehicleSystem.vehicle_get_dynamic(trainset_vehicle)
        _trainset.append(trainset_vehicle)
        var vehicle_name:String = VehicleServer.vehicle_get_name(trainset_vehicle)
        tiles.append(TileGrid.Tile.new(dynamic.data_path, dynamic.file_name, dynamic.skin,
                vehicle_name, "%s (%s)" % [vehicle_name, dynamic.data_path.get_file()]))
    %Trainset.set_tiles(tiles)


## The data of one vehicle of the trainset, marked in the side views: filled once, what changes
## shown as it is now
func _show_trainset_vehicle(p_vehicle:RID) -> void:
    _shown = p_vehicle
    var index:int = _trainset.find(_shown)
    # the grid starts on its first tile; the selection goes where the mark is
    if index >= 0:
        %Trainset.select(index)
    %Trainset.set_marked(index)
    %ActionsButton.vehicle = _shown
    _on_camera_changed()
    var state:Dictionary = VehicleServer.vehicle_dump_state(_shown)
    var config:Dictionary = VehicleServer.vehicle_dump_config(_shown)
    %Title.text = VehicleServer.vehicle_get_name(_shown)
    %TypeName.text = RailVehicleServer.vehicle_get_type_name(_shown)
    var brake:Object = RailVehicleServer.vehicle_component_get(_shown, RailVehicleComponentType.COMPONENT_BRAKES)
    var data:Dictionary[String, String] = {}
    if MaszynaLegacyVehicleSystem.vehicle_exists(_shown):
        var dynamic:MaszynaDynamicData = MaszynaLegacyVehicleSystem.vehicle_get_dynamic(_shown)
        data[tr("File")] = "%s/%s" % [dynamic.data_path, dynamic.file_name]
        data[tr("Skin")] = dynamic.skin
    data[tr("Length")] = "%.2f m" % config.get("length", 0.0)
    data[tr("Mass")] = "%.1f t" % (state.get("mass_total", 0.0) / KILOGRAMS_PER_TONNE)
    data[tr("Axle arrangement")] = config.get("axle_arrangement", "")
    data[tr("Axles")] = "%d" % config.get("axles_count", 0)
    data[tr("Track width")] = "%.3f m" % config.get("track_width", 0.0)
    data[tr("Maximum speed")] = "%d km/h" % config.get("max_speed", 0.0)
    if config.get("power", 0.0) > 0.0:
        data[tr("Power")] = "%d kW" % config.get("power", 0.0)
    if brake:
        data[tr("Brake system")] = _enum_label(brake, &"cntrl_brake_system")
        data[tr("Brake valve")] = _enum_label(brake, &"valve_type")
    _show_values(%BasicData, data)
    _on_refresh_timer_timeout()


## A side view clicked: the card shows that vehicle's data
func _on_trainset_activated() -> void:
    _show_trainset_vehicle(_trainset[%Trainset.get_selected()])


func _ready() -> void:
    PlayerCameraServer.camera_changed.connect(_on_camera_changed)
    PlayerServer.player_vehicle_changed.connect(_on_player_vehicle_changed)
    RailVehicleServer.vehicle_trainset_changed.connect(_on_vehicle_trainset_changed)


func _exit_tree() -> void:
    PlayerCameraServer.camera_changed.disconnect(_on_camera_changed)
    PlayerServer.player_vehicle_changed.disconnect(_on_player_vehicle_changed)
    RailVehicleServer.vehicle_trainset_changed.disconnect(_on_vehicle_trainset_changed)


## The label of an enum property's value, from the property's own hint ("Name" or "Name:value")
static func _enum_label(object:Object, property:StringName) -> String:
    var value:int = object.get(property)
    for info:Dictionary in object.get_property_list():
        if not info.name == property:
            continue
        var names:PackedStringArray = (info.hint_string as String).split(",")
        for index:int in names.size():
            var parts:PackedStringArray = names[index].split(":")
            if (parts[1].to_int() if parts.size() > 1 else index) == value:
                return parts[0]
    return "%d" % value


## Each caption with its value in the grid, in the order given; a caption missing from values is
## hidden
func _show_values(grid:GridContainer, values:Dictionary[String, String]) -> void:
    var rows:Dictionary = _values.get_or_add(grid, {})
    for caption:String in rows:
        for label:Label in rows[caption]:
            label.visible = values.has(caption)
    for caption:String in values:
        if not rows.has(caption):
            var caption_label:Label = Label.new()
            caption_label.text = caption
            caption_label.theme_type_variation = &"TimetableMuted"
            var value_label:Label = Label.new()
            value_label.theme_type_variation = &"TimetableDigits"
            value_label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
            value_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
            grid.add_child(caption_label)
            grid.add_child(value_label)
            rows[caption] = [caption_label, value_label]
        rows[caption][1].text = values[caption]


## How the vehicle and its trainset are going, and what its driver is doing
func _on_refresh_timer_timeout() -> void:
    if not VehicleServer.vehicle_exists(_shown):
        return
    var state:Dictionary = VehicleServer.vehicle_dump_state(_shown)
    var motion:Dictionary[String, String] = {
        tr("Speed"): "%.1f km/h" % absf(state.get("speed", 0.0)),
        tr("Acceleration"): "%.2f m/s²" % state.get("acceleration", 0.0),
        tr("Direction"): [tr("Backward"), tr("Neutral"), tr("Forward")][signi(state.get("direction", 0)) + 1],
        tr("Distance"): "%.2f km" % state.get("total_distance", 0.0),
    }
    _show_values(%MotionData, motion)

    var brakes:Dictionary[String, String] = {}
    if state.has("brake_pipe_pressure"):
        brakes[tr("Brake pipe")] = "%.2f bar" % state["brake_pipe_pressure"]
    if state.has("brake_air_pressure"):
        brakes[tr("Brake cylinder")] = "%.2f bar" % state["brake_air_pressure"]
    if state.has("compressor_pressure"):
        brakes[tr("Main reservoir")] = "%.2f bar" % state["compressor_pressure"]
    if state.has("feed_pipe_pressure"):
        brakes[tr("Feed pipe")] = "%.2f bar" % state["feed_pipe_pressure"]
    if state.has("brake_controller_position"):
        brakes[tr("Brake handle")] = "%.1f" % state["brake_controller_position"]
    if state.has("brake_releaser_active"):
        brakes[tr("Releaser")] = tr("Enabled") if state["brake_releaser_active"] else tr("Disabled")
    _show_values(%BrakeData, brakes)
    %BrakesTitle.visible = brakes.size() > 0

    var drive:Dictionary[String, String] = {}
    if state.has("current_collector/voltage"):
        drive[tr("Line voltage")] = "%d V" % absf(state["current_collector/voltage"])
    if state.has("battery_voltage"):
        drive[tr("Battery")] = "%.1f V" % state["battery_voltage"]
    if state.has("current0"):
        drive[tr("Current")] = "%d A" % absf(state["current0"])
    if state.has("engine_power"):
        drive[tr("Power")] = "%d kW" % state["engine_power"]
    if state.has("engine_rpm"):
        drive[tr("Engine speed")] = "%d rpm" % state["engine_rpm"]
    if state.has("master_controller_position"):
        drive[tr("Master controller")] = "%d" % state["master_controller_position"]
    if state.has("main_switch_enabled"):
        drive[tr("Main switch")] = tr("Enabled") if state["main_switch_enabled"] else tr("Disabled")
    if state.has("converter_enabled"):
        drive[tr("Converter")] = tr("Enabled") if state["converter_enabled"] else tr("Disabled")
    _show_values(%DriveData, drive)
    %DriveTitle.visible = drive.size() > 0

    # the whole trainset, as the original's electricity usage (vehicleparams.cpp:240-262)
    var mass:float = 0.0
    var drawn:float = 0.0
    var returned:float = 0.0
    for trainset_vehicle:RID in _coupled:
        mass += VehicleServer.vehicle_get_controller(trainset_vehicle).get_mass_total()
        var trainset_power_source:RailVehicleEnginePowerSource = RailVehicleServer.vehicle_component_get(
                trainset_vehicle, RailVehicleComponentType.COMPONENT_ENGINE_POWER_SOURCE) as RailVehicleEnginePowerSource
        if trainset_power_source:
            drawn += trainset_power_source.get_energy_drawn()
            returned += trainset_power_source.get_energy_returned()
    var trainset_data:Dictionary[String, String] = {
        tr("Vehicles"): "%d" % _coupled.size(),
        tr("Mass"): "%.1f t" % (mass / KILOGRAMS_PER_TONNE),
    }
    if drawn or returned:
        trainset_data[tr("Energy drawn")] = "%.1f kWh" % drawn
        trainset_data[tr("Energy returned")] = "%.1f kWh" % -returned
        trainset_data[tr("Energy balance")] = "%.1f kWh" % (drawn + returned)
    _show_values(%TrainsetData, trainset_data)

    var driver:RID = DriverSystem.vehicle_get_driver(_shown)
    var driving:bool = DriverSystem.vehicle_is_control_active(_shown)
    var driver_kind:VehicleSelectorRow.Driver = VehicleSelectorRow.driver_of(_shown, _shown == PlayerServer.player_get_vehicle())
    %Driver.text = VehicleSelectorRow.driver_label(driver_kind)
    %TakeOverButton.visible = not driver_kind == VehicleSelectorRow.Driver.PLAYER
    %EnterCabinButton.visible = not (_shown == PlayerServer.player_get_vehicle()
            and PlayerCameraServer.camera_get_mode() == PlayerCameraServer.CAMERA_MODE_CABIN)
    # the AI is switched only on the player's own vehicle - the player gives it the driver's role or
    # takes it back
    %AIButton.visible = driver.is_valid() and _shown == PlayerServer.player_get_vehicle()
    %AIButton.text = tr("Disable AI") if driving else tr("Enable AI")
    var driver_state:Dictionary = DriverSystem.driver_get_state(driver) if driver.is_valid() else {}
    var driver_data:Dictionary[String, String] = {}
    if driver_state:
        driver_data[tr("Order")] = driver_state.get("order_text", "")
        driver_data[tr("Direction")] = tr("Forward") if driver_state.get("direction", 1) > 0 else tr("Backward")
        driver_data[tr("Speed allowed")] = _format_velocity(driver_state.get("velocity", NO_VELOCITY))
        driver_data[tr("Speed ahead")] = _format_velocity(driver_state.get("velocity_next", NO_VELOCITY))
        if driver_state.get("next_stop", ""):
            driver_data[tr("Next stop")] = (driver_state["next_stop"] as String).replace("_", " ")
        if driver_state.get("at_passenger_stop", false):
            driver_data[tr("At the platform")] = tr("Yes")
        if driver_state.get("stop_time", 0.0) > 0.0:
            driver_data[tr("Stop time")] = "%d s" % driver_state["stop_time"]
        var dispatch_step:StationServer.DispatchStep = StationServer.dispatch_get_step(VehicleServer.person_get_vehicle(driver))
        if DISPATCH_STEP_NAMES.has(dispatch_step):
            driver_data[tr("Dispatch")] = tr(DISPATCH_STEP_NAMES[dispatch_step])
    _show_values(%DriverData, driver_data)

    # the timetable of the driver: the train, its relation, the delay and the station passed last,
    # the one it drives to and the one after
    var timetable_state:Dictionary = DriverSystem.driver_get_timetable_state(driver) if driver.is_valid() else {}
    var timetable:Timetable = timetable_state.get("timetable")
    %TrainLabel.text = "%s %s" % [timetable.train_category, timetable.train_name] if timetable else ""
    %TrainLabel.visible = not timetable == null
    var data:Dictionary[String, String] = {}
    if timetable:
        data[tr("Train")] = "%s %s" % [timetable.train_category, timetable.train_name]
        if timetable.relation_from or timetable.relation_to:
            data[tr("Relation")] = "%s  →  %s" % [
                timetable.relation_from.replace("_", " "), timetable.relation_to.replace("_", " ")]
        var late_minutes:int = TimetablePanel.delay_minutes(timetable_state, SimulationServer.time_of_day)
        data[tr("Delay")] = tr("On time") if late_minutes == 0 else "%+d min" % late_minutes
        var entries:Array = timetable.entries
        var index:int = timetable_state.get("station_index", 0)
        if index > 0 and index - 1 < entries.size():
            data[tr("Last station")] = (entries[index - 1] as TimetableEntry).station_name.replace("_", " ")
        if index < entries.size():
            data[tr("Next station")] = (entries[index] as TimetableEntry).station_name.replace("_", " ")
        if index + 1 < entries.size():
            data[tr("Then")] = (entries[index + 1] as TimetableEntry).station_name.replace("_", " ")
    else:
        data[tr("Train")] = tr("No timetable")
    _show_values(%TimetableData, data)


## A driver's speed; one below zero is no limit
func _format_velocity(velocity:float) -> String:
    return "%d km/h" % velocity if velocity >= NO_VELOCITY else tr("No limit")


## The player takes the vehicle over, as when picking the vehicle in the world
func _on_take_over_button_pressed() -> void:
    PlayerServer.player_take_over_vehicle(_shown)


## The player sits in the cab, its driver drives on
func _on_enter_cabin_button_pressed() -> void:
    PlayerServer.player_enter_vehicle(_shown)


## Q / Shift+Q for the player's vehicle (Train.cpp:1088-1118): the player takes it over from its AI,
## or hands it over to the AI
func _on_ai_button_pressed() -> void:
    if DriverSystem.vehicle_is_control_active(_shown):
        PlayerServer.player_take_back_vehicle()
    else:
        PlayerServer.player_hand_over_vehicle()
    _on_refresh_timer_timeout()


func _on_actions_button_remove_confirmed(p_vehicle:RID) -> void:
    remove_trainset_requested.emit(p_vehicle)


## The camera follows the vehicle shown, the player keeping the cab; switched off, the free camera
## goes on from where the following one was
func _on_follow_button_toggled(toggled_on:bool) -> void:
    if toggled_on:
        PlayerCameraServer.camera_set_target(_shown)
        PlayerCameraServer.camera_set_mode(PlayerCameraServer.CAMERA_MODE_FOLLOW)
    else:
        PlayerCameraServer.camera_set_mode(PlayerCameraServer.CAMERA_MODE_FREE)


## The free camera beside the vehicle shown, looking at it
func _on_show_button_pressed() -> void:
    PlayerCameraServer.camera_show_vehicle(_shown)


func _on_close_button_pressed() -> void:
    close_requested.emit()
