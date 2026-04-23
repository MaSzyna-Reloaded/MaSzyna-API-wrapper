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

var _dirty:bool = true
var _needs_head_display_update: bool = false
var _head_display_e3d:E3DModelInstance
var _cabin:Cabin3D
var _camera:FreeCamera3D
var _controller:TrainController
var _t:float = 0.0
var _invalid_velocity_reported: bool = false

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
    if controller_path and is_inside_tree():
        _controller = get_node_or_null(controller_path) as TrainController
    else:
        _controller = null


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
        if _controller:
            var track_rid := _controller.get_track_rid()
            var track_offset := float(_controller.get_track_offset())
            var movement_delta := float(_controller.state.get("movement_delta", 0.0))
            var track := TrackManager.get_track(track_rid)
            if track != null and is_finite(track_offset) and is_finite(movement_delta):
                _invalid_velocity_reported = false
                global_position = TrackManager.get_track_position(track_rid, track_offset) + track.get_axis() * movement_delta
                _controller.set_track_offset(track_offset + movement_delta)
                _controller.set_mover_location(global_position)
            elif track_rid != RID() and not _invalid_velocity_reported:
                _invalid_velocity_reported = true
                push_error(
                    "RailVehicle3D '%s': controller '%s' produced invalid track state." %
                    [name, _controller.name]
                )

func _ready() -> void:
    set_physics_process(true)
    _needs_head_display_update = true
    _dirty = true
    _resolve_controller()
    _register_train()
    E3DModelInstanceManager.instances_reloaded.connect(func(): _needs_head_display_update = true)


func _exit_tree() -> void:
    _unregister_train()
