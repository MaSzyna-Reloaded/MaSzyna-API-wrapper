class_name DrivingAid
extends Control

## The original's driving aid (drivingaid_panel, driveruipanels.cpp:42-205) as floating tiles: the
## controllers, the brakes, the brake pipe, the speed, the speed allowed with the next limit and
## how far it is, and the vigilance (CA) and cab signal (SHP) lamps. The speed limit stands on its
## own in the top right corner, the rest in a row at the top left - two children of a full-screen
## root that lets the mouse through, each anchored to its corner. What changes is refreshed by one Timer while the tiles are shown and the player has a
## vehicle. The values are padded to a fixed width (the digits font is monospaced), as the
## original's "%3d" (driveruipanels.cpp:137, 170), so a tile does not change its width; the limit's
## sign is its tile's last item, so it stays put while the next limit comes and goes on its left.

## How the tiles look: cards (a framed panel each), or their text on a faint black background
enum Style { CARDS, TEXT }
## Project Setting choosing the Style
const STYLE_SETTING:String = "hud/driving_aid/style"
## Laid over each tile for Style.TEXT: no frame, a faint black background
const TEXT_THEME:Theme = preload("driving_aid_text_theme.tres")
## ... and its background: darker over a bright scene
const CONTRAST_SHADER:Shader = preload("driving_aid_contrast.gdshader")
## A dark lamp greyed out over a dark scene
const LAMP_SHADER:Shader = preload("driving_aid_lamp.gdshader")
const METRES_PER_KILOMETRE:float = 1000.0
## The positions of a local brake (LocalBrakePosNo, hamulce.h:45)
const LOCAL_BRAKE_POSITIONS:float = 10.0
## The reverser of the vehicle that pulls (DirActive, "direction"): forward, neutral, backward
const DIRECTION_SYMBOLS:Dictionary[int, String] = {1: "▲", 0: "—", -1: "▼"}
## A train keeps under its timetable's speed; any other order under the shunting speed
## (driveruipanels.cpp:78-81)
const TRAIN_ORDERS:int = MaszynaLegacyAIDriver.Order.OBEY_TRAIN | MaszynaLegacyAIDriver.Order.BANK
const NO_LIMIT:float = MaszynaLegacyDriverSpeed.NO_LIMIT
## The vigilance lamp is lit and dark this long in turn while it blinks [s] (fCzuwakBlink, Train.h:27)
const VIGILANCE_BLINK:float = 0.15
## What keeps the vehicle from being ready, as the stop reason's detail
const ENGINE_CHECK_NAMES:Dictionary[int, String] = {
    MaszynaLegacyAIDriver.EngineCheck.CONVERTER_OVERLOAD: "converter overload",
    MaszynaLegacyAIDriver.EngineCheck.LINE_BREAKER: "line breaker",
    MaszynaLegacyAIDriver.EngineCheck.DIRECTION: "reverser",
    MaszynaLegacyAIDriver.EngineCheck.CONVERTER: "converter",
    MaszynaLegacyAIDriver.EngineCheck.AIR: "main reservoir below %.1f bar",
}

## The vehicle the player drives; an invalid RID while none
var vehicle:RID = RID()
## The dark lamps' material, off while a lamp is lit
var _dark_lamp:ShaderMaterial = ShaderMaterial.new()


func _ready() -> void:
    %BlinkTimer.wait_time = VIGILANCE_BLINK
    _dark_lamp.shader = LAMP_SHADER
    _light_lamp(%VigilanceLamp, false)
    _light_lamp(%CabSignalLamp, false)
    for tile:Control in _tiles():
        apply_style(tile)


## A tile laid out in the Style the project chooses - Style.TEXT: the text theme, over a background
## darker where the scene behind is bright. For every tile of the HUD alike (SimulationSpeedPanel).
static func apply_style(tile:Control) -> void:
    if not ProjectSettings.get_setting(STYLE_SETTING, Style.CARDS) == Style.TEXT:
        return
    var contrast:ShaderMaterial = ShaderMaterial.new()
    contrast.shader = CONTRAST_SHADER
    tile.theme = TEXT_THEME
    tile.material = contrast


## The vehicle whose aid is shown; an invalid RID hides the tiles
func show_vehicle(p_vehicle:RID) -> void:
    vehicle = p_vehicle
    for tile:Control in _tiles():
        tile.visible = vehicle.is_valid()
    _on_visibility_changed()


## The tiles: the row's and the speed limit's
func _tiles() -> Array[Control]:
    var tiles:Array[Control] = [%LimitTile]
    for child:Node in %Tiles.get_children():
        tiles.append(child as Control)
    return tiles


func _on_visibility_changed() -> void:
    if is_visible_in_tree() and vehicle.is_valid():
        %RefreshTimer.start()
        _on_refresh_timer_timeout()
    else:
        %RefreshTimer.stop()
        %BlinkTimer.stop()
        _light_lamp(%VigilanceLamp, false)


func _on_refresh_timer_timeout() -> void:
    if not VehicleServer.vehicle_exists(vehicle):
        return
    var state:Dictionary = VehicleServer.vehicle_dump_state(vehicle)
    # the controllers of the vehicle that pulls (Controlling(), driveruipanels.cpp:139-140)
    var powered:Dictionary = VehicleServer.vehicle_dump_state(RailVehicleServer.vehicle_find_powered(vehicle))
    %DirectionValue.text = DIRECTION_SYMBOLS[signi(int(powered.get("direction", 0)))]
    %ControllerValue.text = "%2d + %-2d" % [
            powered.get("controller_main_position", 0), powered.get("controller_second_position", 0)]
    %BrakesValue.text = "%4.1f + %-2d" % [
            state.get("brake_controller_position", 0.0),
            roundi(state.get("brake_local_position_normalized", 0.0) * LOCAL_BRAKE_POSITIONS)]
    # the train pipe (PipePress, driveruipanels.cpp:160), as the cab's gauge
    %BrakePipeValue.text = "%4.2f" % state.get("pipe_pressure", 0.0)
    %SpeedValue.text = "%3d" % floori(absf(state.get("speed", 0.0)))
    # the vigilance lamp blinks while the alerter asks, the cab signal lamp is lit while it is
    # active (Train.cpp:9024-9039)
    var vigilance:bool = state.get("vigilance_blinking", false)
    if vigilance and %BlinkTimer.is_stopped():
        %BlinkTimer.start()
        _light_lamp(%VigilanceLamp, true)
    elif not vigilance and not %BlinkTimer.is_stopped():
        %BlinkTimer.stop()
        _light_lamp(%VigilanceLamp, false)
    _light_lamp(%CabSignalLamp, state.get("cabsignal_blinking", false))

    var driver:RID = DriverSystem.vehicle_get_driver(vehicle)
    %LimitTile.visible = driver.is_valid()
    if not driver.is_valid():
        return
    # the speed allowed and the next limit (driveruipanels.cpp:73-122)
    var driver_state:Dictionary = DriverSystem.driver_get_state(driver)
    var velocity_desired:float = driver_state.get("velocity_desired", 0.0)
    var limit:float = floorf(velocity_desired)
    var next_limit:float = limit
    var next_distance:float = INF
    # not allowed to move, any next limit is irrelevant
    if not limit == 0.0:
        var order:int = driver_state.get("order", 0)
        var timetable_velocity:float = driver_state.get("timetable_velocity", NO_LIMIT)
        var schedule_limit:float = NO_LIMIT
        if order & TRAIN_ORDERS and timetable_velocity > 0.0:
            schedule_limit = floorf(timetable_velocity)
        elif not order & TRAIN_ORDERS:
            schedule_limit = floorf(driver_state.get("shunt_velocity", NO_LIMIT))
        # first the change after the current limit is passed
        var last_distance:float = driver_state.get("velocity_limit_last_distance", MaszynaLegacyDriverRoute.NO_DISTANCE)
        if last_distance > 0.0:
            next_limit = MaszynaLegacyDriverSpeed.min_speed(
                    schedule_limit, floorf(driver_state.get("velocity_limit_last", NO_LIMIT)))
            next_distance = last_distance
        # then the change ahead: the lower of the two, else the current limit lasts until it is passed
        var no_active_limit:bool = last_distance < 0.0
        var proximity:float = driver_state.get("proximity_distance", 0.0)
        var proximity_limit:float = MaszynaLegacyDriverSpeed.min_speed(
                schedule_limit, floorf(driver_state.get("speed_velocity_next", NO_LIMIT)))
        var take_proximity:bool = no_active_limit
        if proximity_limit < next_limit:
            take_proximity = proximity_limit < velocity_desired or proximity > next_distance
        # nothing read ahead (VelNext -1, the proximity the reach) is no new limit
        if proximity_limit < 0.0:
            take_proximity = false
        if take_proximity:
            next_limit = proximity_limit
            next_distance = proximity
        # a limit reaching past the reading tells nothing of its length, and a vehicle ahead is no
        # limit - the proximity is its distance, less the gap a train keeps (speed.gd, Driver.cpp:7371)
        var obstacle:float = driver_state.get("obstacle_distance", NO_LIMIT)
        if next_distance >= MaszynaLegacyDriverRoute.LIMIT_BEYOND_RANGE:
            next_limit = limit
        elif obstacle >= 0.0 and (proximity == obstacle
                or proximity == obstacle - MaszynaLegacyDriverSpeed.DRIVER_DISTANCE):
            next_limit = limit
    # told to stand (VelDesired 0 - not ready, waiting for orders or for the way out): a stop sign,
    # not a limit of 0; standing, there is no next limit (driveruipanels.cpp:76)
    var stop:bool = limit == 0.0
    %LimitSign.theme_type_variation = &"SpeedStopSign" if stop else &"SpeedLimitSign"
    %LimitValue.theme_type_variation = &"SpeedStopSignText" if stop else &"SpeedLimitSignDigits"
    %LimitValue.text = tr("STOP") if stop else "%d" % limit
    # why it stands, on the left of the sign where the next limit is otherwise
    var stop_reason:MaszynaLegacyDriverSpeed.StopReason = driver_state.get(
            "stop_reason", MaszynaLegacyDriverSpeed.StopReason.NONE)
    # a stop ahead needs no words - the sign says it
    %StopReason.visible = stop and not stop_reason in [
            MaszynaLegacyDriverSpeed.StopReason.NONE, MaszynaLegacyDriverSpeed.StopReason.AHEAD]
    if stop:
        var detail:PackedStringArray = []
        match stop_reason:
            MaszynaLegacyDriverSpeed.StopReason.NOT_READY:
                %StopReasonValue.text = tr("Vehicle not ready")
                var missing:int = driver_state.get("engine_missing", 0)
                for check:int in ENGINE_CHECK_NAMES:
                    if missing & check:
                        var name:String = tr(ENGINE_CHECK_NAMES[check])
                        detail.append(name % MaszynaLegacyAIDriver.MIN_MAIN_RESERVOIR_PRESSURE
                                if check == MaszynaLegacyAIDriver.EngineCheck.AIR else name)
            MaszynaLegacyDriverSpeed.StopReason.WAITING_FOR_ORDERS:
                %StopReasonValue.text = tr("Waiting for orders")
            MaszynaLegacyDriverSpeed.StopReason.SIGNAL:
                %StopReasonValue.text = tr("Signal at stop")
        %StopReasonDetail.text = ", ".join(detail)
        %StopReasonDetail.visible = detail.size() > 0
    # the next limit only when there is a change; the sign stays put, the tile narrows on its left
    %NextLimit.visible = not next_limit == limit
    if next_limit == limit:
        return
    # no limit ahead is a sign, not a word - a translation would change the width
    %NextLimitValue.text = "%3d km/h" % next_limit if next_limit >= 0.0 else "  ∞ km/h"
    %NextLimitDistance.text = tr("in %4.1f km") % (next_distance / METRES_PER_KILOMETRE)


func _on_blink_timer_timeout() -> void:
    _light_lamp(%VigilanceLamp, not %VigilanceLamp.theme_type_variation == &"DrivingAidLampLit")


## A lamp of the security tile lit or dark: its body and its caption
func _light_lamp(lamp:PanelContainer, lit:bool) -> void:
    lamp.theme_type_variation = &"DrivingAidLampLit" if lit else &"DrivingAidLamp"
    lamp.material = null if lit else _dark_lamp
    (lamp.get_child(0) as Label).theme_type_variation = &"DrivingAidLampLabelLit" if lit else &"DrivingAidLampLabel"
