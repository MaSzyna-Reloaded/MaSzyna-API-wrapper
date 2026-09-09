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
var _t:float = 0.0


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
    if _cabin:
        _cabin.set_train_controller(_controller)
    _on_roof_light_changed(_controller and _controller.state.get("roof_light_enabled", false))


func _exit_tree() -> void:
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

    _t += delta
    if _t > 0.25 and _needs_head_display_update:
        _t = 0.0
        _update_head_display()

    if not Engine.is_editor_hint():
        if _controller:
            position += Vector3.FORWARD * delta * _controller.state.get("velocity", 0.0)
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
