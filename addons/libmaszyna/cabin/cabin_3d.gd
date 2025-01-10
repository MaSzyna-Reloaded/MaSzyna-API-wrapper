extends Node3D
class_name Cabin3D

signal cabin_ready

var _dirty = true
var _cabin_ready:bool = false
var _e3d_instances:Array[E3DModelInstance] = []
var _e3d_loaded_count:int = 0

@export_node_path("TrainController") var controller_path:NodePath = NodePath(""):
    set(x):
        if not x == controller_path:
            _dirty = true
            controller_path = x

@export var camera_bound_min = Vector3.ZERO
@export var camera_bound_max = Vector3.ZERO
@export var camera_bound_enabled:bool = false
@export var driver_position = Vector3.ZERO

func get_camera_transform():
    return global_transform.translated_local(driver_position)

func _propagate_train_controller(node: Node, controller: TrainController):
    for child in node.get_children():
        _propagate_train_controller(child, controller)
        if "controller_path" in child:
            if controller:
                child.controller_path = child.get_path_to(controller)
            else:
                child.controller_path = NodePath("")

func set_train_controller(controller:TrainController):
    _propagate_train_controller(self, controller)

func _process(delta):
    if _dirty:
        _dirty = false
        if controller_path:
            var controller:TrainController = get_node(controller_path)
            set_train_controller(controller)

# NOTE: EXPERIMENTAL: Automatic handle of `cabin_ready` signal.
#
# Finds all E3DModelInstances and emits a signal after all of them
# are fully created. E3DModelInstances are lazy-initialized, loaded and created
# at the runtime. This approach introduces artifacts like not fully loaded
# meshes, which will be invisible for a while. Switching to the indoor (cabin)
# is based on the `cabin_ready` signal to address these issues.
#
# NOTE: Will not re-emit signal after removing and adding the node
# to the scene tree. To achieve that a NOTIFICATION_ENTER_TREE should be handled.

# NOTE: Will not handle E3D hot reloading. The signal will be emitted just once.

# FIXME: In case of E3D loading/instancing error, the signal WILL NEVER be emitted.
# Consider adding a fallback timer.

func _find_e3d_instances(node:Node, found: Array):
    for obj in node.get_children(false):
        if obj is E3DModelInstance:
            found.append(obj)
        _find_e3d_instances(obj, found)

func _on_e3d_loaded():
    if not _cabin_ready:
        _e3d_loaded_count += 1
        if _e3d_loaded_count == _e3d_instances.size():
            _cabin_ready = true
            cabin_ready.emit()

func _notification(what: int) -> void:
    match what:
        NOTIFICATION_READY:
            _e3d_instances = []
            _find_e3d_instances(self, _e3d_instances)
            for obj in _e3d_instances:
                obj.e3d_loaded.connect(_on_e3d_loaded)
