@tool
extends Node3D
class_name TrainSet3D

@export var auto_couple_on_ready: bool = true
@export var start_track_id: String = "":
    set(value):
        if value == start_track_id:
            return
        start_track_id = value
        _queue_editor_layout()
@export var start_track_offset: float = 0.0:
    set(value):
        if is_equal_approx(value, start_track_offset):
            return
        start_track_offset = value
        _queue_editor_layout()

var _vehicles: Array[RailVehicle3D] = []
var _controllers: Array[LegacyRailVehicle] = []
var _editor_layout_queued := false
var _missing_preview_track_warning_id := ""


func _enter_tree() -> void:
    if Engine.is_editor_hint():
        add_to_group(&"train_set_3d_editor_preview")


func _ready() -> void:
    if Engine.is_editor_hint():
        _queue_editor_layout()
        return

    if not auto_couple_on_ready:
        return

    _collect_consist_members()
    _place_consist_on_track(false)
    _couple_consist()
    _layout_consist()


func _collect_consist_members() -> void:
    _vehicles.clear()
    _controllers.clear()

    for child in get_children():
        if not child is RailVehicle3D:
            continue

        var vehicle_3d := child as RailVehicle3D
        var controller := vehicle_3d.get_controller()

        if controller == null:
            push_error("TrainSet3D: missing controller for child '%s'." % vehicle_3d.name)
            continue

        _vehicles.append(vehicle_3d)
        _controllers.append(controller)


func _couple_consist() -> void:
    var previous_controller: LegacyRailVehicle = null

    for controller in _controllers:
        if previous_controller != null:
            if previous_controller.get_back_vehicle() == controller and controller.get_front_vehicle() == previous_controller:
                previous_controller = controller
                continue
            print(
                "TrainSet3D: coupling %s(BACK) -> %s(FRONT)" %
                [previous_controller.get_name(), controller.get_name()]
            )
            previous_controller.couple_back(controller, RailVehicle.FRONT)

        previous_controller = controller


func _layout_consist() -> void:
    if _vehicles.is_empty():
        return

    if Engine.is_editor_hint():
        _layout_consist_preview()
        return

    _place_consist_on_track(true)


func _place_consist_on_track(require_coupled_chain: bool) -> void:
    if _vehicles.is_empty():
        return

    if start_track_id.is_empty():
        push_error("TrainSet3D: start_track_id is required for consist placement.")
        return

    var track := TrackManager.get_track_by_name(start_track_id)
    

    if track == null:
        push_error("TrainSet3D: track '%s' is not registered in TrackManager." % start_track_id)
        return

    var track_rid := track.get_rid()
    var current_offset := start_track_offset
    var previous_controller: LegacyRailVehicle = null

    for index in range(_vehicles.size()):
        var current_vehicle := _vehicles[index]
        var current_controller := _controllers[index]

        if previous_controller != null:
            if (
                require_coupled_chain
                and (
                    previous_controller.get_back_vehicle() != current_controller
                    or current_controller.get_front_vehicle() != previous_controller
                )
            ):
                break
            current_offset -= _get_spacing(previous_controller, current_controller)

        current_controller.assign_track_rid(track_rid, start_track_id)
        current_controller.set_track_offset(current_offset)
        current_vehicle.global_position = TrackManager.get_track_position(track_rid, current_offset)
        current_controller.set_mover_location(current_vehicle.global_position)

        previous_controller = current_controller


func _get_spacing(previous_controller: LegacyRailVehicle, current_controller: LegacyRailVehicle) -> float:
    return 0.5 * (previous_controller.length + current_controller.length)


func _layout_consist_preview() -> void:
    if start_track_id.is_empty():
        return

    var track := TrackManager.get_track_by_name(start_track_id)
    if track == null:
        if _missing_preview_track_warning_id != start_track_id:
            _missing_preview_track_warning_id = start_track_id
            push_warning("TrainSet3D: preview track '%s' is not registered in TrackManager." % start_track_id)
        return

    _missing_preview_track_warning_id = ""
    var track_rid := track.get_rid()
    var current_offset := start_track_offset

    for index in range(_vehicles.size()):
        var current_vehicle := _vehicles[index]

        if index > 0:
            current_offset -= _get_spacing(_controllers[index - 1], _controllers[index])

        current_vehicle.global_position = TrackManager.get_track_position(track_rid, current_offset)


func _queue_editor_layout() -> void:
    if not Engine.is_editor_hint() or not is_inside_tree() or _editor_layout_queued:
        return

    _editor_layout_queued = true
    _apply_editor_layout.call_deferred()


func _apply_editor_layout() -> void:
    _editor_layout_queued = false

    if not Engine.is_editor_hint() or not is_inside_tree():
        return

    _collect_consist_members()
    _layout_consist()


func _exit_tree() -> void:
    if Engine.is_editor_hint():
        remove_from_group(&"train_set_3d_editor_preview")
