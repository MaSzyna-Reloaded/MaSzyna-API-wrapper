extends GenericTrainPart
class_name TrainSoundContext

var soundproofing:Array[PackedFloat32Array] = []:
    set(value):
        soundproofing = value
        mark_dirty()

var _listener_context:int = 5


func _process_train_part(_delta:float) -> void:
    _listener_context = _resolve_listener_context()


func _get_train_part_state() -> Dictionary:
    return {"sound/listener_context": _listener_context}


func _get_train_part_config() -> Dictionary:
    return {"sound/soundproofing": soundproofing}


func _resolve_listener_context() -> int:
    var camera:Camera3D = get_viewport().get_camera_3d()
    if not camera:
        return 5
    var source_vehicle:Node = get_train_controller_node().get_parent().get_parent()
    var child:Node = camera
    var node:Node = camera.get_parent()
    while node:
        if node == source_vehicle:
            if child.has_method("get_sound_listener_context"):
                return int(child.call("get_sound_listener_context"))
            return 4
        child = node
        node = node.get_parent()
    return 5
