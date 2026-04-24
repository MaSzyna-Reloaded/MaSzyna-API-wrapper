@tool
extends Node3D
class_name RailVehicle3D

# FIXME: Head Display implementation is experimental and only for demo purposes

@export var train_id: StringName = &"":
    set(x):
        if x == train_id:
            return
        if not Engine.is_editor_hint():
            _unregister_train()
        train_id = x
        if not Engine.is_editor_hint():
            _register_train()

@export_node_path("TrainController") var controller_path:NodePath = NodePath(""):
    set(x):
        if not x == controller_path:
            if not Engine.is_editor_hint():
                _unregister_train()
            controller_path = x
            _controller = null
            _dirty = true
            if not Engine.is_editor_hint():
                _resolve_controller()
                _register_train()

@export var cabin_scene:PackedScene
@export var cabin_rotate_180deg:bool = false
@export_node_path("E3DModelInstance") var low_poly_cabin_path:NodePath = NodePath("")

@export_node_path("E3DModelInstance") var head_display_e3d_path:NodePath = NodePath(""):
    set(x):
        if not x == head_display_e3d_path:
            head_display_e3d_path = x
            _head_display_e3d = null
            _dirty = true

@export var head_display_material:Material:
    set(x):
        if not x == head_display_material:
            head_display_material = x
            _needs_head_display_update = true

@export_node_path("MeshInstance3D") var head_display_node_path = NodePath(""):
    set(x):
        if not x == head_display_node_path:
            head_display_node_path = x
            _needs_head_display_update = true

@export var runtime_physics_enabled: bool = false:
    set(x):
        if x == runtime_physics_enabled:
            return
        if not x and _is_derailed_physics:
            runtime_physics_enabled = true
            if not _runtime_physics_disable_warned:
                _runtime_physics_disable_warned = true
                push_warning("RailVehicle3D '%s': runtime physics cannot be disabled once activated." % name)
            return

        runtime_physics_enabled = x
        if runtime_physics_enabled and is_inside_tree() and not Engine.is_editor_hint():
            _enter_runtime_physics()

var _dirty:bool = true
var _needs_head_display_update: bool = false
var _head_display_e3d:E3DModelInstance
var _cabin:Cabin3D
var _camera:FreeCamera3D
var _controller:TrainController
var _derail_body: RigidBody3D
var _t:float = 0.0
var _invalid_velocity_reported: bool = false
var _is_derailed_physics: bool = false
var _runtime_physics_disable_warned: bool = false
var _runtime_physics_impact_origin: Vector3 = Vector3.INF
var _runtime_physics_relative_speed_kmh: float = 0.0

var state: Dictionary:
    get:
        if _controller:
            return _controller.state
        return {}

var config: Dictionary:
    get:
        if _controller:
            return _controller.config
        return {}

var type_name: String:
    get:
        if _controller:
            return _controller.type_name
        return ""


func _resolve_controller() -> void:
    if _controller and _controller.is_connected("derailed", Callable(self, "_on_controller_derailed")):
        _controller.disconnect("derailed", Callable(self, "_on_controller_derailed"))

    if controller_path and is_inside_tree():
        _controller = get_node_or_null(controller_path) as TrainController
    else:
        _controller = null

    if _controller and not _controller.is_connected("derailed", Callable(self, "_on_controller_derailed")):
        _controller.connect("derailed", Callable(self, "_on_controller_derailed"))


func _register_train() -> void:
    if Engine.is_editor_hint():
        return
    if not is_inside_tree():
        return
    if train_id.is_empty():
        return
    if not _controller:
        return
    TrainSystem.register_train(train_id, _controller)


func _unregister_train() -> void:
    if Engine.is_editor_hint():
        return
    if train_id.is_empty():
        return
    TrainSystem.unregister_train(train_id)


func enter_cabin(player:MaszynaPlayer):
    _camera = player.get_camera()
    _cabin = cabin_scene.instantiate() as Cabin3D
    if not _cabin:
        push_error("Root node of cabin scene must be a Cabin3D")
        return

    if controller_path:
        var controller = get_node(controller_path)
        if controller:
            _cabin.controller_path = get_path()

    # The sequence of adding, removing, hiding, showing nodes is very important
    # to reduce visual artifacts

    # first, mark cabin invisible and add it to the scene
    _cabin.visible = false

    var _jump_into_cabin = func():
        # this subsequence must be called after cabin meshes were loaded
        # hide low poly cab
        if low_poly_cabin_path:
            var low_poly_cabin = get_node_or_null(low_poly_cabin_path)
            if low_poly_cabin:
                low_poly_cabin.visible = false

        var cabin_enter_camera_transform = _cabin.get_camera_transform()

        # now remove the cam from the player
        player.remove_child(_camera)

        # and add the cam to the cabin
        _cabin.add_child(_camera)
        _camera.bound_enabled = _cabin.camera_bound_enabled
        _camera.bound_min = _cabin.camera_bound_min
        _camera.bound_max = _cabin.camera_bound_max

        # https://github.com/eu07/maszyna/blob/d187ce6b12fab1825c0c92c1346e7ecda401a440/Train.cpp#L9204
        _camera.bound_min.y += 0.5  # these "magic" values comes from the original source
        _camera.bound_max.y += 1.8  # (see link above)

        # then apply camera transforms
        if cabin_enter_camera_transform:
            _camera.global_transform = cabin_enter_camera_transform

            # FIXME: there is something strange with some cabin's rotation
            if cabin_rotate_180deg:
                _camera.global_basis = global_basis
            else:
                _camera.global_basis = global_basis.rotated(Vector3.UP, deg_to_rad(180))

        else:
            _camera.position = Vector3(0, 3, 0)

        # and tune the camera settings
        _camera.velocity_multiplier = 0.2

    # connect callback
    _cabin.cabin_ready.connect(_jump_into_cabin)

    # then add it to the scene
    add_child(_cabin)

    # then apply transforms
    _cabin.global_transform = self.global_transform
    if cabin_rotate_180deg:
        _cabin.rotate_y(deg_to_rad(180))

    # wait for apply transforms
    await get_tree().process_frame
    await get_tree().process_frame

    # show the cabin mesh
    _cabin.visible = true

func leave_cabin(player:Node):
    if low_poly_cabin_path:
        var low_poly_cabin = get_node(low_poly_cabin_path)
        if low_poly_cabin:
            low_poly_cabin.visible = true
    var cam_trn = _camera.global_transform
    _cabin.remove_child(_camera)
    player.add_child(_camera)
    _camera.bound_enabled = false
    _camera.global_transform = cam_trn
    _camera.global_transform.origin = self.global_transform.origin + Vector3(5, 0, 0)
    _camera.global_transform.origin.y += 1.75
    _camera.look_at(self.global_position+Vector3(0, 1.75, -5))
    _camera.velocity_multiplier = 1.0
    _cabin.get_parent().remove_child(_cabin)
    _cabin.queue_free()
    _cabin = null

func get_controller() -> TrainController:
    if not _controller:
        _resolve_controller()
    return _controller


func get_runtime_physics_body() -> RigidBody3D:
    return _derail_body


func send_command(command: StringName, p1: Variant = null, p2: Variant = null) -> void:
    if train_id.is_empty():
        push_error("RailVehicle3D '%s': cannot send command without train_id." % name)
        return
    TrainSystem.send_command(train_id, String(command), p1, p2)


func broadcast_command(command: String, p1: Variant = null, p2: Variant = null) -> void:
    TrainSystem.broadcast_command(command, p1, p2)


func log(level: int, line: String) -> void:
    if train_id.is_empty():
        push_error("RailVehicle3D '%s': cannot log without train_id: %s" % [name, line])
        return
    TrainSystem.log(train_id, level, line)


func log_debug(line: String) -> void:
    self.log(GameLog.LogLevel.DEBUG, line)


func log_info(line: String) -> void:
    self.log(GameLog.LogLevel.INFO, line)


func log_warning(line: String) -> void:
    self.log(GameLog.LogLevel.WARNING, line)


func log_error(line: String) -> void:
    self.log(GameLog.LogLevel.ERROR, line)


func _controller_to_vehicle(controller: TrainController) -> RailVehicle3D:
    if controller == null:
        return null

    var node := controller.get_parent()
    while node != null:
        if node is RailVehicle3D:
            return node as RailVehicle3D
        node = node.get_parent()

    return null


func _get_trainset_vehicles(controller: TrainController) -> Array[RailVehicle3D]:
    var vehicles: Array[RailVehicle3D] = []
    if controller == null:
        return vehicles

    var trainset := controller.get_trainset()
    if trainset == null:
        return vehicles

    for entry in trainset.to_array():
        var vehicle_controller := entry as TrainController
        var vehicle := _controller_to_vehicle(vehicle_controller)
        if vehicle != null:
            vehicles.append(vehicle)

    return vehicles


func _find_nearest_collision_vehicle() -> RailVehicle3D:
    if _controller == null:
        return null

    var track_id := String(_controller.state.get("track_id", ""))
    if track_id.is_empty():
        return null

    var own_offset := float(_controller.state.get("track_offset", 0.0))
    var nearest_vehicle: RailVehicle3D = null
    var nearest_distance := INF

    for node in get_tree().get_nodes_in_group(&"rail_vehicle_runtime"):
        if node == self or not node is RailVehicle3D:
            continue

        var vehicle := node as RailVehicle3D
        var other_controller := vehicle.get_controller()
        if other_controller == null:
            continue
        if String(other_controller.state.get("track_id", "")) != track_id:
            continue

        var other_offset := float(other_controller.state.get("track_offset", 0.0))
        var distance := abs(other_offset - own_offset)
        if distance < nearest_distance:
            nearest_distance = distance
            nearest_vehicle = vehicle

    return nearest_vehicle


func _find_primary_collision_shape() -> CollisionShape3D:
    for node in find_children("", "CollisionShape3D", true, false):
        var collision_shape := node as CollisionShape3D
        if collision_shape != null and collision_shape.shape != null:
            return collision_shape
    return null


func _create_derail_body_shape() -> CollisionShape3D:
    var source_shape := _find_primary_collision_shape()
    if source_shape == null:
        return null

    var body_shape := CollisionShape3D.new()
    body_shape.shape = source_shape.shape
    body_shape.transform = global_transform.affine_inverse() * source_shape.global_transform
    return body_shape


func _detach_from_consist() -> void:
    if _controller == null:
        return

    _controller.uncouple_front()
    _controller.uncouple_back()


func _activate_derail_body(derail_body: RigidBody3D, body_shape: CollisionShape3D, impact_origin: Vector3, relative_speed_kmh: float) -> void:
    if derail_body == null or body_shape == null:
        return

    await get_tree().physics_frame
    if not is_instance_valid(derail_body) or not is_instance_valid(body_shape):
        return

    body_shape.disabled = false

    if impact_origin != Vector3.INF:
        var rel_speed_ms := clamp(relative_speed_kmh / 3.6, 2.0, 12.0)
        var push_dir := global_position - impact_origin
        push_dir.y = 0.0
        if push_dir.is_zero_approx():
            push_dir = global_basis.x
        push_dir = push_dir.normalized()

        var torque_axis := global_basis.z.cross(push_dir)
        if torque_axis.is_zero_approx():
            torque_axis = global_basis.x
        torque_axis = torque_axis.normalized()
        var effective_mass := clamp(derail_body.mass, 5000.0, 12000.0)
        var torque_impulse = torque_axis * min(effective_mass * rel_speed_ms * 0.02, 80000.0)

        derail_body.apply_torque_impulse(torque_impulse)


func _wrap_in_runtime_physics_body(impact_origin: Vector3 = Vector3.INF, relative_speed_kmh: float = 0.0) -> void:
    if _is_derailed_physics or _controller == null:
        return

    var body_shape := _create_derail_body_shape()
    if body_shape == null:
        push_warning("RailVehicle3D '%s': missing CollisionShape3D for runtime physics." % name)
        return

    var previous_parent := get_parent()
    if previous_parent == null:
        return

    var previous_index := get_index()
    var track_rid := _controller.get_track_rid()
    var track := TrackManager.get_track(track_rid)
    var velocity := float(_controller.state.get("velocity", 0.0))
    var mass_total := float(_controller.state.get("mass_total", 0.0))
    var derail_body := RigidBody3D.new()
    derail_body.name = "%sDerailBody" % name
    derail_body.global_transform = global_transform
    derail_body.mass = mass_total if mass_total > 0.0 else _controller.mass
    derail_body.contact_monitor = true
    derail_body.max_contacts_reported = 4
    derail_body.linear_damp = 0.05
    derail_body.angular_damp = 0.05
    derail_body.continuous_cd = true
    derail_body.add_to_group(&"rail_vehicle_runtime_physics_body")
    derail_body.add_child(body_shape)
    body_shape.disabled = true

    previous_parent.add_child(derail_body)
    previous_parent.move_child(derail_body, previous_index)

    _detach_from_consist()
    _controller.assign_track_rid(RID(), "")
    _is_derailed_physics = true
    runtime_physics_enabled = true
    _derail_body = derail_body

    var preserved_global_transform := global_transform
    previous_parent.remove_child(self)
    derail_body.add_child(self)
    global_transform = preserved_global_transform
    if _cabin:
        _cabin.controller_path = get_path()

    var linear_velocity := Vector3.ZERO
    if track != null:
        linear_velocity = TrackManager.get_track_axis(track_rid, float(_controller.get_track_offset())) * velocity
    else:
        linear_velocity = -global_basis.z.normalized() * velocity

    var roll_axis := global_basis.z.normalized()
    if roll_axis.is_zero_approx():
        roll_axis = Vector3.FORWARD

    derail_body.linear_velocity = linear_velocity
    derail_body.angular_velocity = roll_axis * 0.28
    _activate_derail_body(derail_body, body_shape, impact_origin, relative_speed_kmh)


func _enter_runtime_physics() -> void:
    if _is_derailed_physics:
        runtime_physics_enabled = true
        return

    _wrap_in_runtime_physics_body(_runtime_physics_impact_origin, _runtime_physics_relative_speed_kmh)


func _request_runtime_physics(impact_origin: Vector3 = Vector3.INF, relative_speed_kmh: float = 0.0) -> void:
    _runtime_physics_impact_origin = impact_origin
    _runtime_physics_relative_speed_kmh = relative_speed_kmh
    runtime_physics_enabled = true


func _enter_derail_physics() -> void:
    if _is_derailed_physics or _controller == null:
        return

    var collision_vehicle := _find_nearest_collision_vehicle()
    var impacted_vehicles: Array[RailVehicle3D] = []
    var collision_origin := Vector3.INF
    var relative_speed_kmh := max(
        float(_controller.state.get("front_relative_speed_kmh", 0.0)),
        float(_controller.state.get("rear_relative_speed_kmh", 0.0))
    )
    if collision_vehicle != null:
        impacted_vehicles = _get_trainset_vehicles(collision_vehicle.get_controller())
        collision_origin = collision_vehicle.global_position

    _request_runtime_physics(collision_origin, relative_speed_kmh)

    for vehicle in impacted_vehicles:
        if vehicle == null or vehicle == self:
            continue
        vehicle._request_runtime_physics(global_position, relative_speed_kmh)


func _on_controller_derailed(_damage_flag: int, _derail_reason: int) -> void:
    _enter_derail_physics()

func _update_head_display():
    if not is_inside_tree():
        return

    if head_display_node_path:
        var node: MeshInstance3D = get_node_or_null(head_display_node_path)
        if node:
            node.material_override = head_display_material
            _needs_head_display_update = false
            return

    if _head_display_e3d and head_display_node_path and head_display_material:
        var path_text := String(head_display_node_path)
        var path_parts := path_text.split("/")
        if not path_parts.is_empty():
            var submodel_name := path_parts[path_parts.size() - 1]
            _head_display_e3d.set_submodel_material_override(submodel_name, head_display_material)
    _needs_head_display_update = false


func _process(delta):
    if _dirty:
        _dirty = false
        if head_display_e3d_path:
            _head_display_e3d = get_node_or_null(head_display_e3d_path)
            if _head_display_e3d:
                _head_display_e3d.e3d_loaded.connect(func(): _needs_head_display_update = true)

        _resolve_controller()

    _t += delta
    if _t > 0.25 and _needs_head_display_update:
        _t = 0.0
        _update_head_display()


func _physics_process(delta: float) -> void:
    if not Engine.is_editor_hint():
        if runtime_physics_enabled and not _is_derailed_physics:
            _enter_runtime_physics()
        elif _controller and bool(_controller.state.get("is_derailed", false)) and not _is_derailed_physics:
            _enter_derail_physics()

        if _is_derailed_physics:
            if _controller:
                _controller.set_mover_location(global_position)
            return

        if _controller:
            var track_rid := _controller.get_track_rid()
            var track_offset := float(_controller.get_track_offset())
            var track := TrackManager.get_track(track_rid)
            if track != null and is_finite(track_offset):
                _invalid_velocity_reported = false
                global_position = TrackManager.get_track_position(track_rid, track_offset)
                _controller.set_mover_location(global_position)
            elif track_rid != RID() and not _invalid_velocity_reported:
                _invalid_velocity_reported = true
                push_error(
                    "RailVehicle3D '%s': controller '%s' produced invalid track state." %
                    [name, _controller.name]
                )

func _ready() -> void:
    set_physics_process(true)
    add_to_group(&"rail_vehicle_runtime")
    _needs_head_display_update = true
    _dirty = true
    _resolve_controller()
    _register_train()
    E3DModelInstanceManager.instances_reloaded.connect(func(): _needs_head_display_update = true)
    if runtime_physics_enabled:
        _enter_runtime_physics()
    elif _controller and bool(_controller.state.get("is_derailed", false)):
        _enter_derail_physics()


func _exit_tree() -> void:
    if _controller and _controller.is_connected("derailed", Callable(self, "_on_controller_derailed")):
        _controller.disconnect("derailed", Callable(self, "_on_controller_derailed"))
    remove_from_group(&"rail_vehicle_runtime")
    _unregister_train()
