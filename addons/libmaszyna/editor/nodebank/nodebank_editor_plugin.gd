extends MaszynaEditorPluginDelegate
class_name NodebankEditorPluginDelegate

var _dragging := false
var _drag_data: NodebankGridItem
var _drag_camera: Camera3D
var _drag_position: Vector2 = Vector2.ZERO


func drag_start(item_data: NodebankGridItem) -> void:
    if item_data:
        _dragging = true
        _drag_data = item_data
        _drag_camera = null
        _drag_position = Vector2.ZERO

func drag_reset() -> void:
    _drag_data = null
    _drag_camera = null
    _dragging = false
    _drag_position = Vector2.ZERO

func drag_end(editor_plugin:EditorPlugin) -> void:
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

    var undo_redo: EditorUndoRedoManager = editor_plugin.get_undo_redo()
    undo_redo.create_action("Add nodebank E3D model")
    undo_redo.add_do_method(parent, "add_child", instance, true)
    undo_redo.add_do_method(self, "_set_node_owner", instance, scene_root)
    undo_redo.add_undo_method(parent, "remove_child", instance)
    undo_redo.add_do_reference(instance)
    undo_redo.commit_action()

    EditorInterface.get_selection().clear()
    EditorInterface.get_selection().add_node(instance)

func _set_node_owner(node: Node, scene_root: Node) -> void:
    node.owner = scene_root
    for child: Node in node.get_children():
        _set_node_owner(child, scene_root)

func _editor_plugin_enter_tree(editor_plugin: EditorPlugin) -> void:
    pass

func _editor_plugin_exit_tree(editor_plugin: EditorPlugin) -> void:
    drag_reset()

func _editor_plugin_process(editor_plugin: EditorPlugin, _delta: float) -> void:
    if not _dragging or Input.is_mouse_button_pressed(MOUSE_BUTTON_LEFT):
        return
        
    # handle drop
    if not is_instance_valid(_drag_camera):
        var editor_viewport: SubViewport = EditorInterface.get_editor_viewport_3d(0)
        if editor_viewport:
            var editor_position: Vector2 = editor_viewport.get_mouse_position()
            if editor_viewport.get_visible_rect().has_point(editor_position):
                _drag_camera = editor_viewport.get_camera_3d()
                _drag_position = editor_position

    if _drag_camera:
        drag_end(editor_plugin)
        drag_reset()

func _editor_plugin_forward_3d_gui_input(editor_plugin: EditorPlugin, viewport_camera: Camera3D, event: InputEvent) -> int:
    if not _dragging:
        return editor_plugin.AFTER_GUI_INPUT_PASS
        
    if not _drag_data:
        return editor_plugin.AFTER_GUI_INPUT_PASS

    if event is InputEventMouseMotion:
        var mouse_motion: InputEventMouseMotion = event as InputEventMouseMotion
        _drag_camera = viewport_camera
        _drag_position = mouse_motion.position

    return editor_plugin.AFTER_GUI_INPUT_PASS

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
