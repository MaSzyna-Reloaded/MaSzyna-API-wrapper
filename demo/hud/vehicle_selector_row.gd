class_name VehicleSelectorRow
extends PanelContainer

## One driven vehicle in the vehicle selector: the vehicle, its train and how it is going, and what
## the original's vehicle list and vehicle parameters panel offer for it (vehiclelist.cpp:28-44,
## vehicleparams.cpp:264-303) - its card, opened by a click on the row, and the cog's operator
## actions.

## The row was clicked: its card is to be the one the selector has open, or closed if it is already
signal activated(vehicle:RID)
## The player confirmed that the vehicle's trainset is to be removed
signal remove_requested(vehicle:RID)

## How the vehicle is going, as the row shows it
enum Motion { STANDING, RUNNING, BRAKING }
## Who drives the vehicle
enum Driver { AI, PLAYER, UNMANNED }

## The badge of each Driver: its pill and its label
const DRIVER_PILLS:Array[StringName] = [&"AIPill", &"PlayerPill", &"UnmannedPill"]
const DRIVER_BADGES:Array[StringName] = [&"AIBadge", &"PlayerBadge", &"UnmannedBadge"]

## The trainset moves only while standing: V < 0.01 m/s (vehicleparams.cpp:295)
const STANDING_VELOCITY:float = 0.01
const ON_TIME_COLOR:Color = Color(0.4, 0.85, 0.55)
const LATE_COLOR:Color = Color(1.0, 0.45, 0.4)
const EARLY_COLOR:Color = Color(0.45, 0.72, 1.0)

var vehicle:RID = RID()
## The vehicle is the one the player drives or last drove
var _player_vehicle:bool = false


func _ready() -> void:
    %ActionsButton.vehicle = vehicle


## The vehicle's name, its train, speed, driver, motion and delay as they are now
func refresh() -> void:
    var driver:RID = DriverSystem.vehicle_get_driver(vehicle)
    var timetable_state:Dictionary = DriverSystem.driver_get_timetable_state(driver) if driver.is_valid() else {}
    var timetable:Timetable = timetable_state.get("timetable")
    %Name.text = VehicleServer.vehicle_get_name(vehicle)
    %Train.text = "%s %s" % [timetable.train_category, timetable.train_name] if timetable else tr("No timetable")
    %Relation.text = "%s  →  %s" % [
        timetable.relation_from.replace("_", " "), timetable.relation_to.replace("_", " ")
    ] if timetable and (timetable.relation_from or timetable.relation_to) else ""
    %Relation.visible = not %Relation.text == ""
    %Speed.text = "%d km/h" % roundi(absf(VehicleServer.vehicle_get_speed(vehicle)))
    var driver_kind:Driver = driver_of(vehicle, _player_vehicle)
    %DriverBadge.text = driver_label(driver_kind)
    %DriverPill.theme_type_variation = DRIVER_PILLS[driver_kind]
    %DriverBadge.theme_type_variation = DRIVER_BADGES[driver_kind]
    %Motion.text = motion_label(vehicle)
    %Delay.visible = not timetable == null
    if timetable:
        var late_minutes:int = TimetablePanel.delay_minutes(timetable_state, SimulationServer.time_of_day)
        %Delay.text = tr("On time") if late_minutes == 0 else "%+d min" % late_minutes
        %Delay.add_theme_color_override(&"font_color",
                ON_TIME_COLOR if late_minutes == 0 else (LATE_COLOR if late_minutes > 0 else EARLY_COLOR))


## Whether the vehicle is the player's; its badge says so rather than that nobody drives it
func show_player_vehicle(p_player_vehicle:bool) -> void:
    _player_vehicle = p_player_vehicle
    refresh()


## Who drives the vehicle, by the roles aboard: its AI in the driver's role, the player in it, or
## nobody - the player only looking from the cab (OBSERVER) does not drive it
static func driver_of(p_vehicle:RID, p_player_vehicle:bool) -> Driver:
    if DriverSystem.vehicle_is_control_active(p_vehicle):
        return Driver.AI
    return Driver.PLAYER if p_player_vehicle and VehicleServer.person_get_role(PlayerServer.player_get_person()) \
            == VehiclePersonRole.VEHICLE_PERSON_ROLE_DRIVER else Driver.UNMANNED


static func driver_label(p_driver:Driver) -> String:
    return [TranslationServer.translate("AI"), TranslationServer.translate("Player"),
            TranslationServer.translate("Unmanned")][p_driver]


## How the vehicle is going now, as the selector and the floating buttons show it
static func motion_label(p_vehicle:RID) -> String:
    var motion:Motion = Motion.RUNNING
    var brakes:RailVehicleBrake = RailVehicleServer.vehicle_component_get(
            p_vehicle, RailVehicleComponentType.COMPONENT_BRAKES) as RailVehicleBrake
    if absf(VehicleServer.vehicle_get_velocity(p_vehicle)) < STANDING_VELOCITY:
        motion = Motion.STANDING
    elif brakes and brakes.is_braking():
        motion = Motion.BRAKING
    return [TranslationServer.translate("Standing"), TranslationServer.translate("Running"),
            TranslationServer.translate("Braking")][motion]


## The row is lit while it is the active one - the selector has its card open
func show_active(active:bool) -> void:
    theme_type_variation = &"TimetableRowCurrent" if active else &"VehicleSelectorRow"


func _on_gui_input(event:InputEvent) -> void:
    var click:InputEventMouseButton = event as InputEventMouseButton
    if click and click.pressed and click.button_index == MOUSE_BUTTON_LEFT:
        activated.emit(vehicle)


func _on_actions_button_remove_confirmed(p_vehicle:RID) -> void:
    remove_requested.emit(p_vehicle)
