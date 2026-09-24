@tool
extends Node3D
class_name RailVehicle3D

# FIXME: Head Display implementation is experimental and only for demo purposes

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
            _controller = null
            _dirty = true

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
var _model_node:E3DModelInstance
var _t:float = 0.0


func enter_cabin(player:MaszynaPlayer):
    _camera = player.get_camera()
    _cabin = cabin_scene.instantiate() as Cabin3D
    if not _cabin:
        push_error("Root node of cabin scene must be a Cabin3D")
        return

    if controller_path:
        var controller = get_node(controller_path)
        if controller:
            _cabin.controller_path = controller.get_path()

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

    # cabin is not ready, because it is not added to the tree yet
    _cabin.cabin_ready.connect(_jump_into_cabin, CONNECT_ONE_SHOT)

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
    if controller_path:
        return get_node(controller_path)
    else:
        return null

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


func _physics_process(delta: float) -> void:
    if not Engine.is_editor_hint():
        if _controller:
            global_position += global_basis.z * delta * _controller.state.get("velocity", 0.0)

func _process(delta: float) -> void:
    if _dirty:
        _process_dirty()

    _t += delta
    if _t > 0.25 and _needs_head_display_update:
        _t = 0.0
        _update_head_display()

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
            if controller_path:
                _controller = get_node_or_null(controller_path)

            var model_node: E3DModelInstance = null
            if model_instance_path:
                model_node = get_node_or_null(model_instance_path)
            if _model_node:
                _model_node.e3d_loaded.disconnect(_on_model_node_e3d_loaded)
            _model_node = model_node
            if _model_node:
                _model_node.e3d_loaded.connect(_on_model_node_e3d_loaded)
            _sync_model_lights()


func _sync_model_lights() -> void:
    if not _model_node or not _model_node.is_e3d_loaded():
        return
    lights.merge(_model_node.lights_state, false)
    for light_name: String in lights.keys():
        if not _model_node.lights_state.has(light_name):
            lights.erase(light_name)
    lights.sort()
    _model_node.lights_state = lights


func _on_model_node_e3d_loaded() -> void:
    _sync_model_lights()


func _ready() -> void:
    _schedule_head_display_update()
    _dirty = true

    for instance:E3DModelInstance in find_children("", "E3DModelInstance", true, false):
        instance.e3d_loaded.connect(_schedule_head_display_update)
