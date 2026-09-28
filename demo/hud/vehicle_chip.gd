class_name VehicleChip
extends PanelContainer

## A floating button of one vehicle, in the HUD's bottom right corner: its icon, the vehicle's name
## and, under it, how it is going and how fast. The whole chip is the button; an action icon, when
## the chip has one, is a second button beside it. The chip shows while it has a vehicle, and what
## it shows is refreshed by one Timer meanwhile.

## The chip was clicked
signal pressed
## The action button was clicked
signal action_pressed

## What the chip stands for
@export var icon:Texture2D = null
## The second button's icon; no action button without one
@export var action_icon:Texture2D = null
@export var action_tooltip:String = ""

var vehicle:RID = RID()


func _ready() -> void:
    %Icon.texture = icon
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
    if not RailVehicleServer.vehicle_exists(vehicle):
        return
    %Name.text = RailVehicleServer.vehicle_get_name(vehicle)
    %Status.text = "%s · %d km/h" % [VehicleSelectorRow.motion_label(vehicle),
            roundi(absf(RailVehicleServer.vehicle_get_speed(vehicle)))]


func _on_gui_input(event:InputEvent) -> void:
    var click:InputEventMouseButton = event as InputEventMouseButton
    if click and click.pressed and click.button_index == MOUSE_BUTTON_LEFT:
        pressed.emit()


func _on_mouse_entered() -> void:
    theme_type_variation = &"FloatingChipHover"


func _on_mouse_exited() -> void:
    theme_type_variation = &"FloatingChip"


func _on_action_button_pressed() -> void:
    action_pressed.emit()
