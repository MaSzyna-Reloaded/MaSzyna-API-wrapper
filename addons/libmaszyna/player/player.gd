extends Node3D
class_name MaszynaPlayer

signal controlled_vehicle_changed
signal controlled_vehicle_path_changed

@export_node_path("RailVehicle3D") var controlled_vehicle_path:NodePath = NodePath(""):
    set(x):
        if not controlled_vehicle_path == x:
            controlled_vehicle_path = x
            _dirty = true

@export var physics_pick_enabled: bool = true
@export var physics_pick_only_free_camera: bool = true
@export var physics_pick_distance: float = 80.0
@export var physics_pick_spring: float = 90000.0
@export var physics_pick_damping: float = 12000.0
@export var physics_throw_multiplier: float = 2800.0

var last_controlled_vehicle_path:NodePath = NodePath()
var controlled_vehicle:RailVehicle3D
var _dirty: bool = false
var _picked_body: RigidBody3D
var _picked_local_point := Vector3.ZERO
var _picked_distance: float = 0.0
var _last_pick_target := Vector3.ZERO
var _pick_target_velocity := Vector3.ZERO

func _ready() -> void:
    pass

func _process(delta):
    if _dirty:
        _dirty = false
        var _changed:bool = false

        if controlled_vehicle:
            controlled_vehicle.leave_cabin(self)
            controlled_vehicle = null
            _changed = true

        if controlled_vehicle_path:
            controlled_vehicle = get_node(controlled_vehicle_path)
            if controlled_vehicle:
                controlled_vehicle.enter_cabin(self)
                last_controlled_vehicle_path = controlled_vehicle_path
                _changed = true

        if _changed:
            controlled_vehicle_changed.emit()

func _physics_process(delta: float) -> void:
    if _picked_body == null:
        return
    if not is_instance_valid(_picked_body):
        _clear_physics_pick()
        return

    var camera := _get_physics_pick_camera()
    if camera == null:
        _clear_physics_pick()
        return

    var target := _get_mouse_pick_target(camera, _picked_distance)
    if delta > 0.0:
        _pick_target_velocity = (target - _last_pick_target) / delta
    _last_pick_target = target

    var grabbed_point := _picked_body.to_global(_picked_local_point)
    var point_velocity := _picked_body.linear_velocity + _picked_body.angular_velocity.cross(grabbed_point - _picked_body.global_position)
    var force := (target - grabbed_point) * physics_pick_spring - point_velocity * physics_pick_damping
    _picked_body.apply_force(force, grabbed_point - _picked_body.global_position)


func _input(event):
    if _handle_physics_pick_input(event):
        return

    if event.is_action_pressed("change_vehicle") or event.is_action_pressed("cabin_mode_toggle"):
        if not controlled_vehicle and $Camera3D/RailVehicleDetector.is_colliding():
            var coll:Area3D = $Camera3D/RailVehicleDetector.get_collider(0)
            if coll:
                var _tmp = coll.get_parent()
                while _tmp and not _tmp is RailVehicle3D:
                    _tmp = _tmp.get_parent()
                if event.is_action_pressed("change_vehicle") or not last_controlled_vehicle_path:
                    controlled_vehicle_path = get_path_to(_tmp)

    if event.is_action_pressed("cabin_mode_toggle"):
        if not controlled_vehicle_path:
            if last_controlled_vehicle_path:
                controlled_vehicle_path = last_controlled_vehicle_path
        else:
            controlled_vehicle_path = NodePath("")

func get_camera():
    return $Camera3D


func _handle_physics_pick_input(event: InputEvent) -> bool:
    if not physics_pick_enabled:
        return false
    if not event is InputEventMouseButton:
        return false

    var mouse_event := event as InputEventMouseButton
    if mouse_event.button_index != MOUSE_BUTTON_LEFT:
        return false

    if not mouse_event.pressed and _picked_body != null:
        _finish_physics_pick()
        return true

    if get_viewport().gui_get_hovered_control():
        return false

    if mouse_event.pressed:
        return _try_start_physics_pick(mouse_event.position)

    return false


func _try_start_physics_pick(screen_position: Vector2) -> bool:
    var camera := _get_physics_pick_camera()
    if camera == null:
        return false

    var ray_origin := camera.project_ray_origin(screen_position)
    var ray_end := ray_origin + camera.project_ray_normal(screen_position) * physics_pick_distance
    var query := PhysicsRayQueryParameters3D.create(ray_origin, ray_end)
    query.collide_with_areas = false
    query.collide_with_bodies = true

    var hit := camera.get_world_3d().direct_space_state.intersect_ray(query)
    if hit.is_empty():
        return false

    var body := hit.get("collider") as RigidBody3D
    if body == null or not body.is_in_group(&"rail_vehicle_runtime_physics_body"):
        return false

    _picked_body = body
    _picked_local_point = body.to_local(hit.get("position", body.global_position))
    _picked_distance = clampf(ray_origin.distance_to(hit.get("position", body.global_position)), 1.0, physics_pick_distance)
    _last_pick_target = _get_mouse_pick_target(camera, _picked_distance)
    _pick_target_velocity = Vector3.ZERO
    return true


func _finish_physics_pick() -> void:
    if _picked_body != null and is_instance_valid(_picked_body):
        var impulse := _pick_target_velocity * physics_throw_multiplier
        _picked_body.apply_impulse(impulse, _picked_body.to_global(_picked_local_point) - _picked_body.global_position)
    _clear_physics_pick()


func _clear_physics_pick() -> void:
    _picked_body = null
    _picked_local_point = Vector3.ZERO
    _picked_distance = 0.0
    _last_pick_target = Vector3.ZERO
    _pick_target_velocity = Vector3.ZERO


func _get_physics_pick_camera() -> Camera3D:
    if physics_pick_only_free_camera and controlled_vehicle != null:
        return null
    return get_node_or_null("Camera3D") as Camera3D


func _get_mouse_pick_target(camera: Camera3D, distance: float) -> Vector3:
    var mouse_position := get_viewport().get_mouse_position()
    return camera.project_ray_origin(mouse_position) + camera.project_ray_normal(mouse_position) * distance
