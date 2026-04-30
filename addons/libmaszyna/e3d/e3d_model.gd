@tool
extends Resource
class_name E3DModel

@export var name:String = ""
@export var submodels:Array = []

func add_child(submodel: E3DSubModel) -> void:
    submodels.append(submodel)


func get_aabb() -> AABB:
    var merged: AABB = AABB()
    var has_aabb: bool = false
    _accumulate_aabb(submodels, Transform3D.IDENTITY, merged, has_aabb)
    return merged


func _accumulate_aabb(
    current_submodels: Array, current_transform: Transform3D, merged: AABB, has_aabb: bool
) -> Dictionary:
    for submodel: E3DSubModel in current_submodels:
        if submodel == null:
            continue

        var local_transform: Transform3D = current_transform * submodel.transform
        var mesh: ArrayMesh = submodel.mesh
        if mesh != null:
            var mesh_aabb: AABB = local_transform * mesh.get_aabb()
            if mesh_aabb.has_surface():
                if has_aabb:
                    merged = merged.merge(mesh_aabb)
                else:
                    merged = mesh_aabb
                    has_aabb = true

        if not submodel.submodels.is_empty():
            var result: Dictionary = _accumulate_aabb(
                submodel.submodels,
                local_transform,
                merged,
                has_aabb
            )
            merged = result["merged"] as AABB
            has_aabb = result["has_aabb"] as bool

    return {
        "merged": merged,
        "has_aabb": has_aabb,
    }
