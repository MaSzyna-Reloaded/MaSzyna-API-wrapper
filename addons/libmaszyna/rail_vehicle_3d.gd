extends Node3D
class_name RailVehicle3D

@export_node_path("TrainController") var controller:NodePath = NodePath("")
@export var cabin_scene:PackedScene
@export_node_path("E3DModelInstance") var low_poly_cabin_path:NodePath = NodePath("")

var _cabin:Cabin3D
var _camera:FreeCamera3D

func enter_cabin(camera:FreeCamera3D):
    _camera = camera
    _cabin = cabin_scene.instantiate()
    _cabin.controller_path = get_node(controller).get_path()
    add_child(_cabin)
    if low_poly_cabin_path:
        var low_poly_cabin = get_node(low_poly_cabin_path)
        if low_poly_cabin:
            low_poly_cabin.visible = false
    _cabin.global_transform = self.global_transform
    _cabin.rotate_y(deg_to_rad(180))  # ???
    camera.velocity_multiplier = 0.2
    camera.get_parent().remove_child(camera)
    _cabin.add_child(camera)
    camera.global_transform = _cabin.get_camera_transform()

func leave_cabin(scene:Node):
    if low_poly_cabin_path:
        var low_poly_cabin = get_node(low_poly_cabin_path)
        if low_poly_cabin:
            low_poly_cabin.visible = true
    _cabin.remove_child(_camera)
    scene.add_child(_camera)
    _camera.global_transform = self.global_transform
    _camera.transform.origin += Vector3(3, 1.75, 0)
    _camera.velocity_multiplier = 1.0
    _cabin.get_parent().remove_child(_cabin)
    _cabin.queue_free()
    _cabin = null
