extends SceneTree

var _instance:E3DModelInstance
var _frames:int = 0


func _initialize() -> void:
    _instance = E3DModelInstance.new()
    _instance.instancer = E3DModelInstance.Instancer.NODES
    _instance.data_path = "dynamic/pkp/sm42_v1"
    _instance.model_filename = "6d_kabina"
    root.add_child(_instance)
    print("instance=", _instance, " children=", root.get_child_count())


func _process(_delta:float) -> bool:
    _frames += 1
    var target:Node3D = _find(_instance, "nastawnik")
    if not target:
        if _frames > 600:
            print("NOT FOUND after ", _frames, " frames")
            return true
        return false

    var chain:Array[String] = []
    var node:Node = target
    while node and not node == _instance:
        chain.push_front(node.name)
        node = node.get_parent()
    print("path: ", "/".join(chain))

    node = target
    while node and not node == _instance:
        print("  ", node.name, " basis=", (node as Node3D).transform.basis, " origin=", (node as Node3D).transform.origin)
        node = node.get_parent()

    var model_from_node:Transform3D = _instance.global_transform.affine_inverse() * target.global_transform
    print("model<-node: ", model_from_node)
    print("rotation axis (local +Y) in model space: ", model_from_node.basis.y.normalized())

    # CabinSwitch: mesh.basis = original_basis * Basis(UP, deg); probe = top of the wheel
    # (local +Z, since the submodel's local frame has +Z pointing up in model space)
    var probe:Vector3 = Vector3(0.0, 0.0, 0.2)
    var original_basis:Basis = target.transform.basis
    var parent_to_model:Transform3D = _instance.global_transform.affine_inverse() \
            * (target.get_parent() as Node3D).global_transform
    for degrees:float in [0.0, -64.8, 64.8]:
        var rotated:Basis = original_basis * Basis(Vector3.UP, deg_to_rad(degrees))
        var point:Vector3 = parent_to_model * (Transform3D(rotated, target.transform.origin) * probe)
        print("  probe at %+6.1f deg -> model space %s" % [degrees, point])
    return true


func _find(node:Node, target_name:String) -> Node3D:
    if node.name.to_lower() == target_name:
        return node as Node3D
    for child:Node in node.get_children(true):
        var found:Node3D = _find(child, target_name)
        if found:
            return found
    return null
