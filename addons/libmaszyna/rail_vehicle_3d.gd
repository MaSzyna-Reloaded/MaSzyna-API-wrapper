@tool
extends Node3D
class_name RailVehicle3D

# FIXME: Head Display implementation is experimental and only for demo purposes

@export var controller: RailVehicle:
    set(value):
        if controller == value:
            return
        controller = value
        _runtime_controller = null
        _controller_signals_bound = false
        _dirty = true

@export var cabin_scene: PackedScene
@export var cabin_rotate_180deg: bool = false
@export_node_path("E3DModelInstance") var low_poly_cabin_path: NodePath = NodePath("")

@export_node_path("E3DModelInstance") var head_display_e3d_path: NodePath = NodePath(""):
    set(value):
        if head_display_e3d_path == value:
            return
        head_display_e3d_path = value
        _head_display_e3d = null
        _dirty = true

@export var head_display_material: Material:
    set(value):
        if head_display_material == value:
            return
        head_display_material = value
        _needs_head_display_update = true

@export_node_path("MeshInstance3D") var head_display_node_path = NodePath(""):
    set(value):
        if head_display_node_path == value:
            return
        head_display_node_path = value
        _needs_head_display_update = true

var state: Dictionary:
    get:
        return get_state()

var config: Dictionary:
    get:
        return get_config()

var _dirty := true
var _needs_head_display_update := false
var _head_display_e3d: E3DModelInstance
var _cabin: Cabin3D
var _camera: FreeCamera3D
var _runtime_controller: LegacyRailVehicle
var _t := 0.0
var _invalid_velocity_reported := false
var _controller_signals_bound := false


func enter_cabin(player: MaszynaPlayer):
    _camera = player.get_camera()
    _cabin = cabin_scene.instantiate() as Cabin3D
    if not _cabin:
        push_error("Root node of cabin scene must be a Cabin3D")
        return

    _cabin.visible = false

    var jump_into_cabin := func():
        if low_poly_cabin_path:
            var low_poly_cabin = get_node_or_null(low_poly_cabin_path)
            if low_poly_cabin:
                low_poly_cabin.visible = false

        var cabin_enter_camera_transform = _cabin.get_camera_transform()

        player.remove_child(_camera)
        _cabin.add_child(_camera)
        _camera.bound_enabled = _cabin.camera_bound_enabled
        _camera.bound_min = _cabin.camera_bound_min
        _camera.bound_max = _cabin.camera_bound_max
        _camera.bound_min.y += 0.5
        _camera.bound_max.y += 1.8

        if cabin_enter_camera_transform:
            _camera.global_transform = cabin_enter_camera_transform
            if cabin_rotate_180deg:
                _camera.global_basis = global_basis
            else:
                _camera.global_basis = global_basis.rotated(Vector3.UP, deg_to_rad(180))
        else:
            _camera.position = Vector3(0, 3, 0)

        _camera.velocity_multiplier = 0.2

    _cabin.cabin_ready.connect(jump_into_cabin)
    add_child(_cabin)
    _cabin.controller_path = _cabin.get_path_to(self)

    _cabin.global_transform = global_transform
    if cabin_rotate_180deg:
        _cabin.rotate_y(deg_to_rad(180))

    await get_tree().process_frame
    await get_tree().process_frame
    _cabin.visible = true


func leave_cabin(player: Node):
    if low_poly_cabin_path:
        var low_poly_cabin = get_node(low_poly_cabin_path)
        if low_poly_cabin:
            low_poly_cabin.visible = true
    var cam_trn = _camera.global_transform
    _cabin.remove_child(_camera)
    player.add_child(_camera)
    _camera.bound_enabled = false
    _camera.global_transform = cam_trn
    _camera.global_transform.origin = global_transform.origin + Vector3(5, 0, 0)
    _camera.global_transform.origin.y += 1.75
    _camera.look_at(global_position + Vector3(0, 1.75, -5))
    _camera.velocity_multiplier = 1.0
    _cabin.get_parent().remove_child(_cabin)
    _cabin.queue_free()
    _cabin = null


func get_controller() -> LegacyRailVehicle:
    if Engine.is_editor_hint():
        return controller as LegacyRailVehicle
    return _runtime_controller if _runtime_controller else controller as LegacyRailVehicle


func get_state() -> Dictionary:
    var active_controller := get_controller()
    return active_controller.get_state() if active_controller else {}


func get_config() -> Dictionary:
    var active_controller := get_controller()
    return active_controller.get_config() if active_controller else {}


func get_rail_vehicle_modules() -> Array:
    var active_controller := get_controller()
    return active_controller.get_rail_vehicle_modules() if active_controller else []


func find_module(module_name: String):
    for module in get_rail_vehicle_modules():
        if module and module.get_name() == module_name:
            return module
    return null


func _instantiate_runtime_controller() -> void:
    if Engine.is_editor_hint() or _runtime_controller or controller == null:
        return

    _runtime_controller = controller.duplicate(true) as LegacyRailVehicle
    if _runtime_controller == null:
        push_error("RailVehicle3D '%s': controller resource must inherit LegacyRailVehicle." % name)
        return

    _runtime_controller.initialize()


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
        if not Engine.is_editor_hint():
            _instantiate_runtime_controller()

    if not Engine.is_editor_hint() and _runtime_controller:
        _runtime_controller.update(delta)

    _t += delta
    if _t > 0.25 and _needs_head_display_update:
        _t = 0.0
        _update_head_display()


func _physics_process(delta: float) -> void:
    if Engine.is_editor_hint():
        return

    if _runtime_controller:
        var track_rid := _runtime_controller.get_track_rid()
        var track_offset := float(_runtime_controller.get_track_offset())
        var movement_delta := float(get_state().get("movement_delta", 0.0))
        var track := TrackManager.get_track(track_rid)
        if track != null and is_finite(track_offset) and is_finite(movement_delta):
            _invalid_velocity_reported = false
            global_position = TrackManager.get_track_position(track_rid, track_offset) + track.get_axis() * movement_delta
            _runtime_controller.set_track_offset(track_offset + movement_delta)
            _runtime_controller.set_mover_location(global_position)
        elif track_rid != RID() and not _invalid_velocity_reported:
            _invalid_velocity_reported = true
            push_error(
                "RailVehicle3D '%s': controller '%s' produced invalid track state." %
                [name, _runtime_controller.get_name()]
            )



func _ready() -> void:
    set_physics_process(true)
    _needs_head_display_update = true
    _dirty = true
    E3DModelInstanceManager.instances_reloaded.connect(func(): _needs_head_display_update = true)


func _enter_tree() -> void:
    if not Engine.is_editor_hint():
        _instantiate_runtime_controller()


func _exit_tree() -> void:
    if _runtime_controller:
        _runtime_controller.deinitialize()
        _runtime_controller = null
        _controller_signals_bound = false
