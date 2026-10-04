class_name VehicleChip
extends UIChip

## A floating button of one vehicle, in the HUD's bottom right corner: its icon, the vehicle's name
## and, under it, how it is going and how fast. The whole chip is the button; an action icon, when
## the chip has one, is a second button beside it. The chip shows while it has a vehicle, and what
## it shows is refreshed by one Timer meanwhile.
## A chip with the DRIVER badge shows, in place of its icon, who drives the vehicle - the player,
## the AI or nobody, as the vehicle card says it - on a button that asks to switch the driver.

## What the chip shows on its left
enum Badge { ICON, DRIVER }

## Each VehicleSelectorRow.Driver's icon and button style
const DRIVER_ICONS:Array[Texture2D] = [
    preload("icons/automaton.svg"), preload("icons/joystick.svg"), preload("icons/joystick.svg")]
const DRIVER_BUTTONS:Array[StringName] = [
    &"DriverAIButton", &"DriverPlayerButton", &"DriverUnmannedButton"]

## The chip was clicked
signal pressed
## The action button was clicked
signal action_pressed
## The driver button was clicked
signal driver_pressed

@export var badge:Badge = Badge.ICON
## What the chip stands for
@export var icon:Texture2D = null
## The second button's icon; no action button without one
@export var action_icon:Texture2D = null
@export var action_tooltip:String = ""

var vehicle:RID = RID()


func _ready() -> void:
    super()
    %Icon.texture = icon
    %Icon.visible = badge == Badge.ICON
    %DriverButton.visible = badge == Badge.DRIVER
    %ActionButton.icon = action_icon
    %ActionButton.tooltip_text = action_tooltip
    %ActionButton.visible = not action_icon == null


## The vehicle the chip shows; an invalid RID hides the chip
func show_vehicle(p_vehicle:RID) -> void:
    vehicle = p_vehicle
    visible = vehicle.is_valid()


func _on_visibility_changed() -> void:
    if not visible:
        %RefreshTimer.stop()
        return
    _on_refresh_timer_timeout()
    %RefreshTimer.start()


func _on_refresh_timer_timeout() -> void:
    if not VehicleServer.vehicle_exists(vehicle):
        return
    %Name.text = VehicleServer.vehicle_get_name(vehicle)
    %Status.text = "%s · %d km/h" % [VehicleSelectorRow.motion_label(vehicle),
            roundi(absf(VehicleServer.vehicle_get_speed(vehicle)))]
    if badge == Badge.DRIVER:
        var driver:VehicleSelectorRow.Driver = VehicleSelectorRow.driver_of(
                vehicle, vehicle == PlayerServer.player_get_vehicle())
        %DriverButton.icon = DRIVER_ICONS[driver]
        %DriverButton.theme_type_variation = DRIVER_BUTTONS[driver]
        %DriverButton.tooltip_text = VehicleSelectorRow.driver_label(driver)


func _on_gui_input(event:InputEvent) -> void:
    var click:InputEventMouseButton = event as InputEventMouseButton
    if click and click.pressed and click.button_index == MOUSE_BUTTON_LEFT:
        pressed.emit()


func _on_mouse_entered() -> void:
    set_hovered(true)


func _on_mouse_exited() -> void:
    set_hovered(false)


func _on_action_button_pressed() -> void:
    action_pressed.emit()


func _on_driver_button_pressed() -> void:
    driver_pressed.emit()
    _on_refresh_timer_timeout()
