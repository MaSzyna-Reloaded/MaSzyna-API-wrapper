class_name VehicleActionsButton
extends Button

## The cog of a vehicle - the operator's actions the original's vehicle parameters panel offers
## for its trainset (vehicleparams.cpp:264-303): release the brakes, stop, move or remove it. The
## same menu stands in the trainset list's rows and on the vehicle card; the trainset is removed
## only once the player confirms it, and whoever owns the scene does the removing.

## The player confirmed that the vehicle's trainset is to be removed
signal remove_confirmed(vehicle:RID)


## The vehicle the actions are for
var vehicle:RID = RID()


## The moves are offered only to a standing trainset (vehicleparams.cpp:295)
func _on_pressed() -> void:
    var standing:bool = absf(VehicleServer.vehicle_get_velocity(vehicle)) < VehicleSelectorRow.STANDING_VELOCITY
    for button:Button in %Moves.get_children():
        button.disabled = not standing
    var anchor:Rect2 = get_global_rect()
    %ActionsPopup.popup(Rect2i(Vector2i(anchor.position + Vector2(0.0, anchor.size.y)), Vector2i.ZERO))


## consistreleaser (simulation.cpp:184): every vehicle of the trainset releases its brake while the
## button is held (vehicleparams.cpp:289-293)
func _on_release_brakes_button_down() -> void:
    _release_trainset_brakes(true)


func _on_release_brakes_button_up() -> void:
    _release_trainset_brakes(false)
    %ActionsPopup.hide()


func _release_trainset_brakes(active:bool) -> void:
    for trainset_vehicle:RID in RailVehicleServer.vehicle_get_coupled(
            vehicle, RailVehicleController.COUPLER_END_FRONT, RailVehicleController.COUPLING_FLAG_COUPLER):
        VehicleServer.vehicle_send_command(trainset_vehicle, "consist_releaser", active)


func _on_emergency_stop_pressed() -> void:
    %ActionsPopup.hide()
    VehicleServer.vehicle_send_command(vehicle, "brake_level_set_position", "emergency")


## The trainset moved [m], towards the vehicle's front when positive
func _on_move_pressed(distance:float) -> void:
    %ActionsPopup.hide()
    RailVehicleServer.trainset_move(vehicle, distance)


func _on_remove_pressed() -> void:
    %ActionsPopup.hide()
    %RemoveDialog.message = tr("Remove the trainset of %s?") % VehicleServer.vehicle_get_name(vehicle)
    %RemoveDialog.ask()


func _on_remove_dialog_confirmed() -> void:
    remove_confirmed.emit(vehicle)
