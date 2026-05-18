@tool
extends EditorPlugin

var _dragging := false
var _drag_data: NodebankGridItem
var _drag_camera: Camera3D
var _drag_position: Vector2 = Vector2.ZERO


var nodebank_panel_scene = preload("./nodebank_panel.tscn")
var nodebank_panel_instance
var nodebank_bottom_button


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

func _ready() -> void:
    set_process(true)

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

func _process(_delta: float) -> void:
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
        drag_end()
        drag_reset()

func _forward_3d_gui_input(viewport_camera: Camera3D, event: InputEvent) -> int:
    if not _dragging:
        return AFTER_GUI_INPUT_PASS

    if not _drag_data:
        return AFTER_GUI_INPUT_PASS

    if event is InputEventMouseMotion:
        var mouse_motion: InputEventMouseMotion = event as InputEventMouseMotion
        _drag_camera = viewport_camera
        _drag_position = mouse_motion.position

    return AFTER_GUI_INPUT_PASS

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
