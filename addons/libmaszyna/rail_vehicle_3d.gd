@tool
extends Node3D
class_name RailVehicle3D

# FIXME: Head Display implementation is experimental and only for demo purposes

## Fixed original-engine submodel naming convention for exterior lights (confirmed against
## MaSzyna's own vehicle/DynObj.cpp:2379-2418, m_headlampNN/m_endsignalNN .Init() calls) - every
## .e3d model converted from the original engine names these submodels the same way, so this is a
## constant mapping, not a per-vehicle setting. Maps the e3d model's own light name to the
## matching TrainController.state key (populated by TrainLighting, see TrainLighting.cpp).
const LIGHT_STATE_BINDINGS:Dictionary[String, String] = {
    "headlamp11": "lights/front_headlight_upper_enabled",
    "headlamp12": "lights/front_headlight_right_enabled",
    "headlamp13": "lights/front_headlight_left_enabled",
    "headlamp21": "lights/rear_headlight_upper_enabled",
    "headlamp22": "lights/rear_headlight_right_enabled",
    "headlamp23": "lights/rear_headlight_left_enabled",
    "endsignal12": "lights/front_redmarker_right_enabled",
    "endsignal13": "lights/front_redmarker_left_enabled",
    "endsignal22": "lights/rear_redmarker_right_enabled",
    "endsignal23": "lights/rear_redmarker_left_enabled",
}

@export_node_path("E3DModelInstance") var model_instance_path:NodePath = NodePath(""):
    set(x):
        if not x == model_instance_path:
            model_instance_path = x
            _dirty = true

@export var lights:Dictionary[String, bool] = {}:
    set(x):
        if not x == lights:
            lights = x
            _sync_model_lights()

@export_node_path("TrainController") var controller_path:NodePath = NodePath(""):
    set(x):
        if not x == controller_path:
            controller_path = x
            _dirty = true

@export_node_path("Node3D") var front_bogie_path:NodePath = NodePath(""):
    set(x):
        if not x == front_bogie_path:
            front_bogie_path = x
            _animation_bindings_dirty = true

@export_node_path("Node3D") var rear_bogie_path:NodePath = NodePath(""):
    set(x):
        if not x == rear_bogie_path:
            rear_bogie_path = x
            _animation_bindings_dirty = true

@export var front_rolling_wheel_paths:Array[NodePath] = []:
    set(x):
        if not x == front_rolling_wheel_paths:
            front_rolling_wheel_paths = x
            _animation_bindings_dirty = true

@export var powered_wheel_paths:Array[NodePath] = []:
    set(x):
        if not x == powered_wheel_paths:
            powered_wheel_paths = x
            _animation_bindings_dirty = true

@export var rear_rolling_wheel_paths:Array[NodePath] = []:
    set(x):
        if not x == rear_rolling_wheel_paths:
            rear_rolling_wheel_paths = x
            _animation_bindings_dirty = true

@export var start_track_name:String = "":
    set(x):
        if not x == start_track_name:
            start_track_name = x
            _pending_start_track_retry = true if start_track_name else false
            _dirty = true

@export var start_track_offset:float = 0.0:
    set(x):
        if not is_equal_approx(x, start_track_offset):
            start_track_offset = x
            _pending_start_track_retry = true if start_track_name else false
            _dirty = true

@export_enum("NORMAL", "REVERSED") var start_direction:int = TrackManager.Direction.DIRECTION_NORMAL:
    set(x):
        if not x == start_direction:
            start_direction = x
            _pending_start_track_retry = true if start_track_name else false
            _dirty = true

@export var cabin_scene:PackedScene
@export var cabin_rotate_180deg:bool = false
@export_node_path("E3DModelInstance") var low_poly_cabin_path:NodePath = NodePath("")
## Interior cab glow (window self-illumination, see e3d_instancer.gd's emission_enabled shader
## parameter) energy while roof_light_enabled is true - the imported .e3d selfillum data is
## authored for the full-detail cabin's own real Light3D-lit interior, not for this simplified
## stand-in seen only from outside, so it needs its own explicit value here rather than reusing
## the imported one.
@export var low_poly_cabin_emission_energy:float = 0.2
@export var low_poly_cabin_emission_fade_time:float = 0.2

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

var _dirty:bool = true
var _needs_head_display_update: bool = false
var _head_display_e3d:E3DModelInstance
var _cabin:Cabin3D
var _camera:FreeCamera3D
var _controller:TrainController
var _fiz_controller:FIZTrainController
var _model_node:E3DModelInstance
var _detection_area:Area3D
var _low_poly_cabin:E3DModelInstance
var _low_poly_emissive_materials:Array[ShaderMaterial] = []
var _low_poly_emission_tween:Tween
var _rid:RID = RID()
var _pending_start_track_retry:bool = false
var _t:float = 0.0
var _animation_bindings_dirty:bool = true
var _front_bogie_node:Node3D
var _rear_bogie_node:Node3D
var _front_rolling_wheel_nodes:Array[Node3D] = []
var _powered_wheel_nodes:Array[Node3D] = []
var _rear_rolling_wheel_nodes:Array[Node3D] = []
var _node_rest_bases:Dictionary = {}
var _bogie_rest_global_bases:Dictionary = {}
var _bogie_configuration_warned:bool = false


func enter_cabin(player:MaszynaPlayer):
    if not cabin_scene:
        push_warning("%s has no cabin_scene; cabin entry not yet supported" % name)
        return

    _camera = player.get_camera()
    var cabin:Cabin3D = cabin_scene.instantiate() as Cabin3D
    if not cabin:
        push_error("Root node of cabin scene must be a Cabin3D")
        return
    _cabin = cabin

    if controller_path:
        var controller = _resolve_controller(controller_path)
        if controller:
            cabin.controller_path = controller.get_path()

    # The sequence of adding, removing, hiding, showing nodes is very important
    # to reduce visual artifacts

    # first, mark cabin invisible and add it to the scene
    cabin.visible = false

    var _jump_into_cabin = func():
        # leave_cabin() may have already torn this same cabin down while this signal was still
        # in flight (e.g. the player exits again immediately after entering) - _cabin no longer
        # pointing at `cabin` means that already happened, so there's nothing left to jump into.
        if not is_instance_valid(cabin) or not _cabin == cabin:
            return

        # this subsequence must be called after cabin meshes were loaded
        # hide low poly cab
        if low_poly_cabin_path:
            var low_poly_cabin = get_node_or_null(low_poly_cabin_path)
            if low_poly_cabin:
                low_poly_cabin.visible = false

        # now remove the cam from the player
        player.remove_child(_camera)

        # and add the cam to the cabin
        cabin.add_child(_camera)
        _apply_cabin_camera_configuration()

        # and tune the camera settings
        _camera.velocity_multiplier = 0.2

    # cabin is not ready, because it is not added to the tree yet
    cabin.cabin_ready.connect(_jump_into_cabin, CONNECT_ONE_SHOT)
    cabin.camera_configuration_changed.connect(_apply_cabin_camera_configuration)

    # cabin_ready can fire synchronously in add_child(), so configure the transform first.
    cabin.transform = Transform3D.IDENTITY
    if cabin_rotate_180deg:
        cabin.rotate_y(deg_to_rad(180))
    add_child(cabin)

    # wait for apply transforms
    await get_tree().process_frame
    await get_tree().process_frame

    # leave_cabin() may have already torn this same cabin down while we were waiting (e.g. the
    # player exits again immediately after entering) - _cabin no longer pointing at `cabin` means
    # that already happened, and it (or whatever replaced it) is no longer ours to show.
    if not is_instance_valid(cabin) or not _cabin == cabin:
        return

    # show the cabin mesh
    cabin.visible = true

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
    _cabin.camera_configuration_changed.disconnect(_apply_cabin_camera_configuration)
    _cabin.get_parent().remove_child(_cabin)
    _cabin.queue_free()
    _cabin = null


func _apply_cabin_camera_configuration() -> void:
    if not _cabin or not _camera.get_parent() == _cabin:
        return
    _camera.bound_enabled = _cabin.camera_bound_enabled
    _camera.bound_min = _cabin.camera_bound_min
    _camera.bound_max = _cabin.camera_bound_max

    # Original engine camera bounds include the driver's height allowance.
    _camera.bound_min.y += 0.5
    _camera.bound_max.y += 1.8
    _camera.global_transform = _cabin.get_camera_transform()
    if cabin_rotate_180deg:
        _camera.global_basis = global_basis
    else:
        _camera.global_basis = global_basis.rotated(Vector3.UP, deg_to_rad(180))


func _on_controller_changed(controller:TrainController) -> void:
    if _controller == controller:
        return
    if _controller:
        _controller.roof_light_changed.disconnect(_on_roof_light_changed)
    _controller = controller
    if _controller:
        _controller.roof_light_changed.connect(_on_roof_light_changed)
    if _rid.is_valid():
        RailVehiclePhysicsServer.vehicle_bind_controller(
            _rid,
            _controller.get_rid() if _controller else RID(),
        )
    if _cabin:
        _cabin.set_train_controller(_controller)
    _on_roof_light_changed(_controller and _controller.state.get("roof_light_enabled", false))


func _exit_tree() -> void:
    TrackManager.tracks_changed.disconnect(_on_track_manager_tracks_changed)
    if _model_node:
        _model_node.e3d_loaded.disconnect(_on_model_node_e3d_loaded)
        _model_node = null
    if _rid.is_valid():
        RailVehiclePhysicsServer.vehicle_free(_rid)
        _rid = RID()
    if _fiz_controller:
        _fiz_controller.controller_changed.disconnect(_on_controller_changed)
        _fiz_controller = null
    if _controller:
        _controller.roof_light_changed.disconnect(_on_roof_light_changed)
        _controller = null


func get_controller() -> TrainController:
    if controller_path:
        return _resolve_controller(controller_path)
    else:
        return null

## Resolves controller_path to a TrainController, transparently unwrapping a FIZTrainController
## wrapper if that's what the path points at (mirrors how model_instance_path always points at
## an E3DModelInstance rather than a live mesh node directly).
func _resolve_controller(node_path:NodePath) -> TrainController:
    var node = get_node_or_null(node_path)
    if node is FIZTrainController:
        return node.get_controller()
    return node as TrainController

func _update_head_display():
    if is_inside_tree():
        if head_display_node_path:
            var node:MeshInstance3D = get_node_or_null(head_display_node_path)
            if node:
                node.material_override = head_display_material
                _needs_head_display_update = false
            else:
                _needs_head_display_update = false
        else:
            _needs_head_display_update = false


func _process(delta):
    if _dirty:
        _process_dirty()
    if _animation_bindings_dirty:
        _animation_bindings_dirty = false
        _cache_animation_bindings()
        _update_track_transform()

    _t += delta
    if _t > 0.25 and _needs_head_display_update:
        _t = 0.0
        _update_head_display()

    if not Engine.is_editor_hint():
        if _rid.is_valid() and start_track_name and not _pending_start_track_retry:
            RailVehiclePhysicsServer.process_movement(_rid, delta)
            _update_track_transform()
        elif _controller and not start_track_name:
            position += Vector3.FORWARD * delta * _controller.state.get("velocity", 0.0)
            _update_wheel_animation_state()
        if _controller:
            _sync_lights_from_controller()

func _schedule_head_display_update():
    _needs_head_display_update = true


func _process_dirty() -> void:
    if _dirty:
        _dirty = false
        if head_display_e3d_path:
            _head_display_e3d = get_node_or_null(head_display_e3d_path)
            if _head_display_e3d:
                _head_display_e3d.e3d_loaded.connect(func(): _needs_head_display_update = true)

        if is_inside_tree():
            var controller_node:Node = get_node_or_null(controller_path) if controller_path else null
            var fiz_controller:FIZTrainController = controller_node as FIZTrainController
            if not fiz_controller and controller_path:
                var parent_path:NodePath = NodePath(String(controller_path).get_base_dir())
                fiz_controller = get_node_or_null(parent_path) as FIZTrainController if parent_path else null
            if not _fiz_controller == fiz_controller:
                if _fiz_controller:
                    _fiz_controller.controller_changed.disconnect(_on_controller_changed)
                _fiz_controller = fiz_controller
                if _fiz_controller:
                    _fiz_controller.controller_changed.connect(_on_controller_changed)
            _on_controller_changed(get_controller())

            var model_node: E3DModelInstance = null
            if model_instance_path:
                model_node = get_node_or_null(model_instance_path)
            if _model_node:
                _model_node.e3d_loaded.disconnect(_on_model_node_e3d_loaded)
            _model_node = model_node
            if _model_node:
                _model_node.e3d_loaded.connect(_on_model_node_e3d_loaded)
            _sync_model_lights()
            _update_detection_area()

            if _low_poly_cabin:
                _low_poly_cabin.e3d_loaded.disconnect(_on_low_poly_cabin_e3d_loaded)
            _low_poly_cabin = get_node_or_null(low_poly_cabin_path) if low_poly_cabin_path else null
            if _low_poly_cabin:
                _low_poly_cabin.e3d_loaded.connect(_on_low_poly_cabin_e3d_loaded)
                if _low_poly_cabin.is_e3d_loaded():
                    _on_low_poly_cabin_e3d_loaded()

            if _pending_start_track_retry:
                _apply_start_track()


func _sync_model_lights() -> void:
    if not _model_node or not _model_node.is_e3d_loaded():
        return
    lights.merge(_model_node.lights_state, false)
    for light_name: String in lights.keys():
        if not _model_node.lights_state.has(light_name):
            lights.erase(light_name)
    lights.sort()
    _model_node.lights_state = lights


## Applies TrainLighting's live on/off state (TrainController.state) to whichever of this
## model's own registered lights match LIGHT_STATE_BINDINGS's fixed submodel names - lights not
## in that table (e.g. cab interior lights) are left as they are, untouched.
func _sync_lights_from_controller() -> void:
    if not _model_node or not _model_node.is_e3d_loaded():
        return
    var changed:bool = false
    for light_name:String in lights.keys():
        if LIGHT_STATE_BINDINGS.has(light_name):
            var new_value:bool = _controller.state.get(LIGHT_STATE_BINDINGS[light_name], false)
            if not lights[light_name] == new_value:
                lights[light_name] = new_value
                changed = true
    if changed:
        _model_node.lights_state = lights


func _on_model_node_e3d_loaded() -> void:
    _sync_model_lights()
    _update_detection_area()
    _animation_bindings_dirty = true


## Collects the low-poly stand-in interior's self-illuminated submodels (window glow etc.) so
## their emission_energy can be driven by roof_light_enabled below. Duplicated per-instance first
## since _get_material_override() returns a MaterialManager-cached ShaderMaterial shared by every
## vehicle using the same model+skin, which an in-place shader parameter edit would otherwise
## desync between vehicle instances.
func _on_low_poly_cabin_e3d_loaded() -> void:
    _low_poly_emissive_materials.clear()
    for mesh_instance:MeshInstance3D in _low_poly_cabin.find_children("", "MeshInstance3D", true, false):
        var material:Material = mesh_instance.material_override
        if material is ShaderMaterial and material.get_shader_parameter("emission_enabled"):
            material = material.duplicate()
            mesh_instance.material_override = material
            _low_poly_emissive_materials.append(material)

    var roof_light_enabled:bool = _controller and _controller.state.get("roof_light_enabled", false)
    _set_low_poly_emission_energy(low_poly_cabin_emission_energy if roof_light_enabled else 0.0)


## Fades the low-poly stand-in interior's window glow in/out with the roof light switch - driven
## by TrainController's own roof_light_changed signal (emitted only on actual state changes, see
## TrainController.cpp's _handle_mover_update()) rather than polled every _process, since this
## only needs to react on the rare toggle, not track a continuously-changing value.
func _on_roof_light_changed(enabled:bool) -> void:
    if _low_poly_emission_tween:
        _low_poly_emission_tween.kill()
    var target_energy:float = low_poly_cabin_emission_energy if enabled else 0.0
    var current_energy:float = (
            _low_poly_emissive_materials[0].get_shader_parameter("emission_energy")
            if _low_poly_emissive_materials
            else target_energy)
    _low_poly_emission_tween = create_tween()
    _low_poly_emission_tween.tween_method(
            _set_low_poly_emission_energy, current_energy, target_energy, low_poly_cabin_emission_fade_time)


func _set_low_poly_emission_energy(value:float) -> void:
    for material:ShaderMaterial in _low_poly_emissive_materials:
        material.set_shader_parameter("emission_energy", value)


## Creates (once) and keeps in sync an Area3D/CollisionShape3D under this RailVehicle3D,
## sized from the resolved model's AABB, so the player's raycaster can detect this vehicle
## without requiring it to be hand-authored per vehicle scene.
func _update_detection_area() -> void:
    if Engine.is_editor_hint():
        return
    if not _model_node or not _model_node.is_e3d_loaded():
        return
    var aabb:AABB = _model_node.get_aabb()
    if aabb.size == Vector3.ZERO:
        return

    if not _detection_area:
        _detection_area = Area3D.new()
        _detection_area.name = "RailVehicleDetectionArea"
        _detection_area.monitoring = false
        var shape_node := CollisionShape3D.new()
        shape_node.shape = BoxShape3D.new()
        _detection_area.add_child(shape_node)
        add_child(_detection_area)

    _detection_area.transform = _model_node.transform
    var shape_node:CollisionShape3D = _detection_area.get_child(0)
    var box:BoxShape3D = shape_node.shape
    box.size = aabb.size
    shape_node.position = aabb.get_center()


func _ready() -> void:
    _schedule_head_display_update()
    _dirty = true

    for instance:E3DModelInstance in find_children("", "E3DModelInstance", true, false):
        instance.e3d_loaded.connect(_schedule_head_display_update)


func _enter_tree() -> void:
    TrackManager.tracks_changed.connect(_on_track_manager_tracks_changed)
    _rid = RailVehiclePhysicsServer.vehicle_create()
    _pending_start_track_retry = true if start_track_name else false
    _dirty = true


func move_on_track(distance:float) -> void:
    if not _rid.is_valid():
        return
    RailVehiclePhysicsServer.vehicle_move(_rid, distance)
    _update_track_transform()


func _on_track_manager_tracks_changed() -> void:
    if _pending_start_track_retry:
        _apply_start_track()


func _apply_start_track() -> void:
    if not start_track_name:
        return
    var track_rid:RID = TrackManager.track_get_rid_by_name(start_track_name)
    if not track_rid.is_valid():
        return
    _pending_start_track_retry = false
    RailVehiclePhysicsServer.vehicle_set_track(
        _rid,
        track_rid,
        start_track_offset,
        start_direction,
    )
    _update_track_transform()


func _resolve_animation_nodes(paths:Array[NodePath]) -> Array[Node3D]:
    var nodes:Array[Node3D] = []
    for path:NodePath in paths:
        if not path == NodePath(""):
            var node:Node3D = get_node_or_null(path) as Node3D
            if node:
                nodes.append(node)
    return nodes


func _capture_rest_basis(node:Node3D) -> void:
    if node and not _node_rest_bases.has(node):
        _node_rest_bases[node] = node.transform.basis.orthonormalized()


func _cache_animation_bindings() -> void:
    _front_bogie_node = get_node_or_null(front_bogie_path) as Node3D
    _rear_bogie_node = get_node_or_null(rear_bogie_path) as Node3D
    _front_rolling_wheel_nodes = _resolve_animation_nodes(front_rolling_wheel_paths)
    _powered_wheel_nodes = _resolve_animation_nodes(powered_wheel_paths)
    _rear_rolling_wheel_nodes = _resolve_animation_nodes(rear_rolling_wheel_paths)
    _node_rest_bases.clear()
    _bogie_rest_global_bases.clear()

    for bogie_node:Node3D in [_front_bogie_node, _rear_bogie_node]:
        if bogie_node:
            _capture_rest_basis(bogie_node)
            _bogie_rest_global_bases[bogie_node] = global_basis.inverse() * bogie_node.global_basis
    for wheel_node:Node3D in _front_rolling_wheel_nodes:
        _capture_rest_basis(wheel_node)
    for wheel_node:Node3D in _powered_wheel_nodes:
        _capture_rest_basis(wheel_node)
    for wheel_node:Node3D in _rear_rolling_wheel_nodes:
        _capture_rest_basis(wheel_node)


func _apply_wheel_rotation(nodes:Array[Node3D], angle_degrees:float) -> void:
    var angle_radians:float = deg_to_rad(angle_degrees)
    for node:Node3D in nodes:
        var rest_basis:Variant = _node_rest_bases.get(node)
        if rest_basis is Basis:
            node.transform.basis = (rest_basis as Basis) * Basis(Vector3.RIGHT, angle_radians)


func _update_wheel_animation_state() -> void:
    if not _controller:
        return
    _apply_wheel_rotation(
        _front_rolling_wheel_nodes,
        float(_controller.state.get("wheel_angle_front_deg", 0.0)),
    )
    _apply_wheel_rotation(
        _powered_wheel_nodes,
        float(_controller.state.get("wheel_angle_powered_deg", 0.0)),
    )
    _apply_wheel_rotation(
        _rear_rolling_wheel_nodes,
        float(_controller.state.get("wheel_angle_rear_deg", 0.0)),
    )


func _update_track_transform() -> void:
    if not _rid.is_valid() or not start_track_name or _pending_start_track_retry:
        return

    var center_transform:Transform3D = RailVehiclePhysicsServer.vehicle_get_transform(_rid)
    if not _front_bogie_node and not _rear_bogie_node:
        global_transform = center_transform
        _update_wheel_animation_state()
        return
    if not _front_bogie_node or not _rear_bogie_node:
        global_transform = center_transform
        if not _bogie_configuration_warned:
            _bogie_configuration_warned = true
            push_warning(
                "RailVehicle3D '%s': front and rear bogie paths must be set together." % name
            )
        _update_wheel_animation_state()
        return

    _bogie_configuration_warned = false
    var bogie_pivot_spacing:float = (
        float(_controller.config.get("bogie_pivot_spacing", 0.0)) if _controller else 0.0
    )
    if bogie_pivot_spacing <= 0.0:
        global_transform = center_transform
        _update_wheel_animation_state()
        return

    var front_transform:Transform3D = RailVehiclePhysicsServer.vehicle_get_transform_at_distance(
        _rid,
        bogie_pivot_spacing * 0.5,
    )
    var rear_transform:Transform3D = RailVehiclePhysicsServer.vehicle_get_transform_at_distance(
        _rid,
        bogie_pivot_spacing * -0.5,
    )
    var body_forward:Vector3 = front_transform.origin - rear_transform.origin
    if body_forward.is_zero_approx():
        global_transform = center_transform
        _update_wheel_animation_state()
        return

    body_forward = body_forward.normalized()
    var average_up:Vector3 = (front_transform.basis.y + rear_transform.basis.y).normalized()
    var z_axis:Vector3 = -body_forward
    var x_axis:Vector3 = average_up.cross(z_axis).normalized()
    var y_axis:Vector3 = z_axis.cross(x_axis).normalized()
    global_transform = Transform3D(
        Basis(x_axis, y_axis, z_axis).orthonormalized(),
        (front_transform.origin + rear_transform.origin) * 0.5,
    )

    var body_yaw:float = atan2(-body_forward.x, body_forward.z)
    var bogie_nodes:Array[Node3D] = [_front_bogie_node, _rear_bogie_node]
    var bogie_transforms:Array[Transform3D] = [front_transform, rear_transform]
    for index:int in range(bogie_nodes.size()):
        var bogie_node:Node3D = bogie_nodes[index]
        var bogie_forward:Vector3 = -bogie_transforms[index].basis.z.normalized()
        var bogie_yaw:float = atan2(-bogie_forward.x, bogie_forward.z)
        var rest_global_basis:Variant = _bogie_rest_global_bases.get(bogie_node)
        if rest_global_basis is Basis:
            var yaw_delta:float = -(bogie_yaw - body_yaw)
            bogie_node.global_basis = (
                global_basis
                * Basis(Vector3.UP, yaw_delta)
                * (rest_global_basis as Basis)
            )

    _update_wheel_animation_state()
