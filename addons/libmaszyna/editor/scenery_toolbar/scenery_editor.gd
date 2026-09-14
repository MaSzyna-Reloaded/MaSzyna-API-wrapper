@tool
extends RefCounted


static func update_owners(root:Node3D) -> void:
    var scene_owner:Node = null
    if root.editable_in_editor:
        scene_owner = root.owner if root.owner else root
    for child:Node in root.get_children(true):
        _set_owner_recursive(child, scene_owner)


static func _set_owner_recursive(node:Node, scene_owner:Node) -> void:
    node.owner = scene_owner
    if node is SfxPlayer3D:
        return
    for child:Node in node.get_children(true):
        _set_owner_recursive(child, scene_owner)
