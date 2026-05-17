@tool
extends EditorPlugin

var _dragging: bool = false
var _drag_data: NodebankGridItem
var _drag_camera: Camera3D
var _drag_position: Vector2 = Vector2.ZERO
var _drag_preview_instance: E3DModelInstance

var nodebank_panel_scene = preload("./nodebank_panel.tscn")
var nodebank_panel_instance
var nodebank_bottom_button

func drag_start(item_data: NodebankGridItem) -> void:
    _dragging = true
    if item_data:
        _drag_data = item_data
        var scene_root: Node = EditorInterface.get_edited_scene_root()
        _drag_preview_instance = _drag_data.model.duplicate() as E3DModelInstance
        scene_root.add_child(_drag_preview_instance, false, Node.InternalMode.INTERNAL_MODE_BACK)
        _drag_preview_instance.owner = null

func drag_reset() -> void:
    if _drag_preview_instance:
        _drag_preview_instance.queue_free()
    _drag_preview_instance = null
    _drag_data = null
    _drag_camera = null
    _dragging = false
    _drag_position = Vector2.ZERO

func drag_end() -> void:
    var scene_root: Node = EditorInterface.get_edited_scene_root()
    if not scene_root or not scene_root is Node3D:
        push_warning("Nodebank item can only be added to a 3D scene root.")
        return

    if not _drag_data:
        push_warning("Nodebank drag data not set. Plugin is not working correctly.")
        return

    var world_position: Vector3 = _get_drop_world_position(_drag_camera, _drag_position)
    var parent: Node3D = scene_root as Node3D
    var instance: E3DModelInstance = _drag_data.model.duplicate()
    instance.position = parent.global_transform.affine_inverse() * world_position

    var undo_redo: EditorUndoRedoManager = get_undo_redo()
    undo_redo.create_action("Add nodebank E3D model")
    undo_redo.add_do_method(parent, "add_child", instance, true)
    undo_redo.add_do_method(instance, "propagate_call", "set_owner", [scene_root])
    undo_redo.add_undo_method(parent, "remove_child", instance)
    undo_redo.add_do_reference(instance)
    undo_redo.commit_action()

    EditorInterface.get_selection().clear()
    EditorInterface.get_selection().add_node(instance)

func _enter_tree() -> void:
    set_input_event_forwarding_always_enabled()
        
    nodebank_panel_instance = nodebank_panel_scene.instantiate()
    nodebank_panel_instance.set_plugin_runtime()
    nodebank_panel_instance.item_drag_started.connect(drag_start)
    nodebank_bottom_button = add_control_to_bottom_panel(nodebank_panel_instance, "Nodebank")

func _exit_tree() -> void:
    drag_reset()
    nodebank_panel_instance.item_drag_started.disconnect(drag_start)
    remove_control_from_bottom_panel(nodebank_panel_instance)
    nodebank_panel_instance.free()
    nodebank_panel_instance = null

func _process(_detla: float) -> void:
    if _dragging:
        var viewport = EditorInterface.get_editor_viewport_3d(0)
        var position = viewport.get_mouse_position()
        if viewport.get_visible_rect().has_point(position):
            # mouse is over editor viewport
            _drag_position = position
            _drag_camera = viewport.get_camera_3d()
        else:
            # mouse is outside editor viewport
            _drag_camera = null
            
        if _drag_preview_instance:
            # handle 3d preview
            _drag_preview_instance.visible = not _drag_camera == null
            if _drag_camera:        
                var scene_root: Node = EditorInterface.get_edited_scene_root()
                var world_position: Vector3 = _get_drop_world_position(_drag_camera, _drag_position)
                _drag_preview_instance.position = scene_root.global_transform.affine_inverse() * world_position
            
        # handle drop
        var mouse_pressed = Input.is_mouse_button_pressed(MOUSE_BUTTON_LEFT)
        if not mouse_pressed:
            if _drag_camera:
                # mouse is over the editor viewport
                drag_end()
            # mouse was released, so stop dragging
            drag_reset()

func _get_drop_world_position(viewport_camera: Camera3D, screen_position: Vector2) -> Vector3:
    var ray_origin: Vector3 = viewport_camera.project_ray_origin(screen_position)
    var ray_direction: Vector3 = viewport_camera.project_ray_normal(screen_position)
    var ray_end: Vector3 = ray_origin + ray_direction * 100000.0
    var world: World3D = viewport_camera.get_world_3d()

    if world:
        var query: PhysicsRayQueryParameters3D = PhysicsRayQueryParameters3D.create(ray_origin, ray_end)
        var hit: Dictionary = world.direct_space_state.intersect_ray(query)
        if hit.has("position"):
            return hit["position"]

    var ground_plane: Plane = Plane(Vector3.UP, 0.0)
    var ground_hit: Variant = ground_plane.intersects_ray(ray_origin, ray_direction)
    if ground_hit is Vector3:
        return ground_hit

    return ray_origin + ray_direction * 10.0
