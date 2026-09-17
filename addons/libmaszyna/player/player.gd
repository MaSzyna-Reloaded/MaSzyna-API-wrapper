extends Node3D
class_name MaszynaPlayer

signal controlled_vehicle_changed

@export var start_train_id:String = "":
    set(x):
        if not start_train_id == x:
            start_train_id = x
            if x:
                _auto_start_pending = false
            _dirty = true

var last_controlled_train_id:String = ""
var controlled_vehicle:RailVehicle3D
var _camera:FreeCamera3D
@onready var train_sound_listener:TrainSoundListener3D = $TrainSoundListener3D
var _dirty: bool = true
var _auto_start_pending:bool = true

func _ready() -> void:
    pass

func _process(_delta:float) -> void:
    if _dirty:
        _dirty = false
        var _changed:bool = false

        if controlled_vehicle:
            controlled_vehicle.leave_cabin(self)
            controlled_vehicle = null
            _changed = true

        var target_vehicle:RailVehicle3D = _find_start_vehicle()
        if target_vehicle:
            controlled_vehicle = target_vehicle
            controlled_vehicle.enter_cabin(self)
            last_controlled_train_id = start_train_id
            _dirty = false
            _changed = true
        elif start_train_id or _auto_start_pending:
            _dirty = true

        if _changed:
            controlled_vehicle_changed.emit()

    var camera:FreeCamera3D = get_camera()
    var cabin:Cabin3D = camera.get_parent() as Cabin3D
    if cabin:
        camera.h_offset = cabin.get_camera_shake_offset().x * 1.5
        camera.rotation.z = cabin.get_camera_shake_roll()
    else:
        camera.h_offset = 0.0
        camera.rotation.z = 0.0

func _input(event):
    if event.is_action_pressed("change_vehicle") or event.is_action_pressed("cabin_mode_toggle"):
        var detector:ShapeCast3D = get_camera().get_node("RailVehicleDetector")
        if not controlled_vehicle and detector.is_colliding():
            var coll:Area3D = detector.get_collider(0)
            if coll:
                var _tmp = coll.get_parent()
                while _tmp and not _tmp is RailVehicle3D:
                    _tmp = _tmp.get_parent()
                if _tmp and _tmp.cabin_scene:
                    var candidate:RailVehicle3D = _tmp as RailVehicle3D
                    var train_id:String = _get_vehicle_train_id(candidate)
                    if train_id and (event.is_action_pressed("change_vehicle") or not last_controlled_train_id):
                        start_train_id = train_id

    if event.is_action_pressed("cabin_mode_toggle"):
        if not controlled_vehicle:
            if last_controlled_train_id:
                start_train_id = last_controlled_train_id
        else:
            start_train_id = ""
            _auto_start_pending = false

func _find_start_vehicle() -> RailVehicle3D:
    var vehicles:Array[Node] = get_tree().get_root().find_children("", "RailVehicle3D", true, false)
    if start_train_id:
        for node:Node in vehicles:
            var vehicle:RailVehicle3D = node as RailVehicle3D
            if vehicle and _get_vehicle_train_id(vehicle) == start_train_id:
                return vehicle
        return null

    if _auto_start_pending:
        for node:Node in vehicles:
            var vehicle:RailVehicle3D = node as RailVehicle3D
            var train_id:String = _get_vehicle_train_id(vehicle) if vehicle else ""
            if vehicle and train_id:
                start_train_id = train_id
                _auto_start_pending = false
                return vehicle
    return null

func _get_vehicle_train_id(vehicle:RailVehicle3D) -> String:
    var controller:TrainController = vehicle.get_controller()
    return controller.train_id if controller else ""

func get_camera() -> FreeCamera3D:
    if not _camera:
        _camera = get_node("Camera3D") as FreeCamera3D
    return _camera
