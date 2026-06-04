extends RefCounted
class_name SpatialIndex

var _cell_size: float = 1.0
var _cells: Dictionary[Vector2i, Array] = {}
var _rid_hashes: Dictionary[RID, Array] = {}


func _init(cell_size: float) -> void:
    _cell_size = cell_size


func add(item_rid: RID, aabb: Rect2) -> void:
    for hash: Vector2i in _get_hashes_for_aabb(aabb):
        if not hash in _cells:
            _cells[hash] = []
        _cells[hash].append(item_rid)
        if not item_rid in _rid_hashes:
            _rid_hashes[item_rid] = []
        _rid_hashes[item_rid].append(hash)


func remove(item_rid: RID) -> void:
    for hash: Vector2i in _rid_hashes.get(item_rid, []):
        var hash_rids: Array = _cells.get(hash, [])
        hash_rids.erase(item_rid)
    _rid_hashes.erase(item_rid)


func query(aabb: Rect2) -> Array[RID]:
    var result: Array[RID] = []
    var hashes: Array[Vector2i] = _get_hashes_for_aabb(aabb)
    for hash: Vector2i in hashes:
        if hash in _cells:
            result.append_array(_cells[hash])
    var unique: Dictionary[RID, bool] = {}
    for rid in result:
        unique[rid] = true
    return unique.keys()    

func clear() -> void:
    _cells.clear()
    _rid_hashes.clear()


func _get_hashes_for_aabb(aabb: Rect2) -> Array[Vector2i]:
    var min_cell: Vector2 = (aabb.position / _cell_size).floor()
    var max_cell: Vector2 = ((aabb.position + aabb.size) / _cell_size).floor()
    var hashes: Array[Vector2i] = []

    for x: int in range(int(min_cell.x), int(max_cell.x) + 1):
        for y: int in range(int(min_cell.y), int(max_cell.y) + 1):
            hashes.append(Vector2i(x, y))

    return hashes
