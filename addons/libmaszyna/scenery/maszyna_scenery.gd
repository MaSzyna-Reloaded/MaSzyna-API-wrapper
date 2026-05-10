@tool
extends MaszynaIncludeNode
class_name MaszynaSceneryNode


@export var title:String = ""
@export var category:String = ""
@export_multiline var description:String = ""
@export var use_cache:bool = false


var cache = ResourceCache.create("scenery")


func _enter_tree() -> void:
    loaded.connect(save_cache)
    

func load() -> void:
    if filename:
        if use_cache and SceneryInstancer.scenery_exists(filename):
            var scene = cache.get(filename) as PackedScene
            if scene:
                var cached_node = scene.instantiate()
                var children: Array[Node] = []
                for child: Node in cached_node.get_children():
                    children.append(child)
                for child: Node in children:
                    cached_node.remove_child(child)
                    _clear_owner_recursive(child)
                    add_child(child)
                return
        super.load()

    
func save_cache():
    if SceneryInstancer.scenery_exists(filename):
        var copy := Node3D.new()
        copy.position = position
        copy.name = name
        for child in get_children():
            copy.add_child(child.duplicate())
        _set_owner_recursive(copy, copy)
        var scene = PackedScene.new()
        scene.pack(copy)
        cache.set(filename, scene)


func _set_owner_recursive(node: Node, scene_owner: Node) -> void:
    for child in node.get_children():
        child.owner = scene_owner
        _set_owner_recursive(child, scene_owner)


func _clear_owner_recursive(node: Node) -> void:
    node.owner = null
    for child: Node in node.get_children():
        _clear_owner_recursive(child)
