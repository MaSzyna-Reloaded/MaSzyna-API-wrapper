@tool
extends Node


## Returns the first mesh from the E3D submodel tree using depth-first preorder.
func find_first_mesh(model: E3DModel) -> ArrayMesh:
    if not model:
        return null

    return _find_first_mesh_in_submodels(model.submodels)


## Returns null when the current branch does not contain any mesh.
func _find_first_mesh_in_submodels(submodels: Array) -> ArrayMesh:
    for item: Variant in submodels:
        if not item is E3DSubModel:
            continue

        var submodel: E3DSubModel = item as E3DSubModel
        if submodel.mesh:
            return submodel.mesh

        var mesh: ArrayMesh = _find_first_mesh_in_submodels(submodel.submodels)
        if mesh:
            return mesh

    return null
