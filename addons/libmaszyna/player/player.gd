extends Node3D
class_name MaszynaPlayer

signal controlled_vehicle_changed
signal controlled_vehicle_path_changed

@export_node_path("RailVehicle3D") var controlled_vehicle_path:NodePath = NodePath(""):
    set(x):
        if not controlled_vehicle_path == x:
            controlled_vehicle_path = x
            _dirty = true

var last_controlled_vehicle_path:NodePath = NodePath()
var controlled_vehicle:RailVehicle3D
var _dirty: bool = false

func _ready() -> void:
    pass

func _process(delta):
    if _dirty:
        _dirty = false
        var _changed:bool = false

        if controlled_vehicle:
            controlled_vehicle.leave_cabin(self)
            controlled_vehicle = null
            _changed = true

        if controlled_vehicle_path:
            controlled_vehicle = get_node(controlled_vehicle_path)
            if controlled_vehicle:
                controlled_vehicle.enter_cabin(self)
                last_controlled_vehicle_path = controlled_vehicle_path
                _changed = true

        if _changed:
            controlled_vehicle_changed.emit()

func _input(event):
    if event.is_action_pressed("change_vehicle") or event.is_action_pressed("cabin_mode_toggle"):
        if not controlled_vehicle and $Camera3D/RailVehicleDetector.is_colliding():
            var coll:Area3D = $Camera3D/RailVehicleDetector.get_collider(0)
            if coll:
                var _tmp = coll.get_parent()
                while _tmp and not _tmp is RailVehicle3D:
                    _tmp = _tmp.get_parent()
                if event.is_action_pressed("change_vehicle") or not last_controlled_vehicle_path:
                    controlled_vehicle_path = get_path_to(_tmp)

    if event.is_action_pressed("cabin_mode_toggle"):
        if not controlled_vehicle_path:
            if last_controlled_vehicle_path:
                controlled_vehicle_path = last_controlled_vehicle_path
        else:
            controlled_vehicle_path = NodePath("")

func get_camera():
    return $Camera3D
