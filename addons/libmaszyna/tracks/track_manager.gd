@tool
extends Node

enum SwitchState {COMMON, DIVERGING}
enum TrackType {NORMAL, SWITCH, ROAD, CROSS, RIVER, TRIBUTARY, TURN, TABLE}
enum Direction {OPPOSITE_TO_CONSIST_FRONT, ALIGNED_WITH_CONSIST_FRONT}

signal switch_state_changed(track_rid: RID, state: int)
signal switch_offset_updated(track_rid: RID, offset: float)
signal switching_started(track_rid: RID, from_state: int, to_state: int)
signal switching_finished(track_rid: RID, state: int)
signal tracks_changed
signal topology_changed

const SWITCH_MAX_OFFSET: float = 0.1 # MaSzyna Track.cpp:35 fMaxOffset.
const SWITCH_OFFSET_DELAY: float = 0.05
const INVALID_NODE_ID: int = -1
const _ENDPOINT_EPSILON: float = 0.25
const _GRID_CELL_SIZE: float = 500.0
const _ENDPOINT_GRID_CELL_SIZE: float = _ENDPOINT_EPSILON
const _TRACK_GROUP_NONE: int = 0
const _TRACK_GROUP_RAIL: int = 1
const _TRACK_GROUP_ROAD: int = 2
const _TRACK_GROUP_WATER: int = 4

const UNDEFINED_TRACK = RID()


class TrackSegment:
    var track_rid: RID = UNDEFINED_TRACK
    var type: int = 0
    var curve1: MaszynaTrackCurve:
        set(x):
            curve1 = x
            _cached_endpoints = []
            update_length()
    var curve2: MaszynaTrackCurve:
        set(x):
            curve2 = x
            _cached_endpoints = []
            update_length()
    var width: float = 1.6
    var length: float = 0.0
    var length1: float = 0.0
    var length2: float = 0.0
    var graph_id: int = -1
    var switch_state: int = SwitchState.COMMON
    var switch_f_offset_delay: float = TrackManager.SWITCH_OFFSET_DELAY
    var switch_desired_offset: float = -TrackManager.SWITCH_OFFSET_DELAY
    # MaSzyna Track.cpp:57-58 initial common offset.
    var switch_f_offset: float = -TrackManager.SWITCH_OFFSET_DELAY: # fOffset in MaSzyna
        set(value):
            switch_f_offset = value
            # MaSzyna Track.cpp:1933-1941 clamps blade offsets.
            switch_f_offset1 = minf(value, TrackManager.SWITCH_MAX_OFFSET)
            switch_f_offset2 = maxf(value, 0.0)
            TrackManager.switch_offset_updated.emit(track_rid, value)
    var switch_f_offset1: float = -0.05 # fOffset1
    var switch_f_offset2: float = 0.0   # fOffset2
    var switch_common_endpoint_index: int = -1
    var aabb: Rect2 = Rect2()
    var grid_cells: Array[Vector2i] = []
    var node_ids: PackedInt32Array = [-1, -1, -1, -1]
    var _cached_endpoints: Array[Vector3]

    func get_active_length() -> float:
        if curve2 and switch_state == SwitchState.DIVERGING:
            return length2
        return length1

    func get_active_curve() -> MaszynaTrackCurve:
        if curve2 and switch_state == SwitchState.DIVERGING:
            return curve2
        return curve1
        
    func update_length() -> void:
        length = 0.0
        length1 = 0.0
        length2 = 0.0
        if curve1:
            var c1 = TrackManager._build_domain_curve(curve1)
            length1 = c1.get_baked_length()
            length += length1
        if curve2:
            var c2 = TrackManager._build_domain_curve(curve2)
            length2 = c2.get_baked_length()
            length += length2

    func get_endpoints() -> Array[Vector3]:
        if _cached_endpoints:
            return _cached_endpoints
        var result: Array[Vector3] = []
        if curve1:
            result.append(curve1.p1)
            result.append(curve1.p2)
        if curve2:
            result.append(curve2.p1)
            result.append(curve2.p2)
        _cached_endpoints = result
        return result
        


class EndpointRef:
    var track_rid: RID = UNDEFINED_TRACK
    var endpoint_index: int = -1

    func _init(p_track_rid: RID, p_endpoint_index: int) -> void:
        track_rid = p_track_rid
        endpoint_index = p_endpoint_index


class TrackNode:
    var id: int = INVALID_NODE_ID
    var world_position: Vector3 = Vector3.ZERO
    var endpoint_refs: Array[EndpointRef] = []


class BranchNeighbors:
    var previous_track_rid: RID = UNDEFINED_TRACK
    var previous_endpoint_index: int = -1
    var next_track_rid: RID = UNDEFINED_TRACK
    var next_endpoint_index: int = -1


class TrackMotionResult:
    var track_rid: RID = UNDEFINED_TRACK
    var offset: float = 0.0
    var direction: Direction = Direction.ALIGNED_WITH_CONSIST_FRONT


var _tracks: Dictionary[RID, TrackSegment] = {}
var _named_tracks: Dictionary[String, RID] = {}
var _spatial_grid: Dictionary = {} # Vector2i -> Array
var _next_track_id: int = 0
var _next_graph_id: int = 0
var _graph_members: Dictionary[int, Array] = {}
var _nodes: Array[TrackNode] = []
var _switch_tweens: Dictionary[RID, Tween] = {}
var is_topology_changed: bool = false


func register_track() -> RID:
    _next_track_id += 1
    var track_rid: RID = rid_from_int64(_next_track_id)
    var track: TrackSegment = TrackSegment.new()
    track.track_rid = track_rid
    _tracks[track_rid] = track
    _mark_topology_changed()
    tracks_changed.emit()
    return track_rid


func unregister_track(track_rid: RID) -> void:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return
    var existing_tween: Tween = _switch_tweens.get(track_rid)
    if existing_tween:
        existing_tween.kill()
        _switch_tweens.erase(track_rid)
    var track_name = _named_tracks.find_key(track_rid)
    if track_name:
        _named_tracks.erase(track_name)
    _remove_from_spatial_index(track)
    _remove_track_from_graph_members(track)
    _tracks.erase(track_rid)
    _clear_topology()
    _mark_topology_changed()
    tracks_changed.emit()

## Returns track RID registered by the unique name
func get_track_rid_by_name(name: String) -> RID:
    var rid: Variant = _named_tracks.get(name)
    if rid == null:
        return UNDEFINED_TRACK
    return rid

## Returns active track length
func get_active_track_length(track_rid: RID) -> float:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return 0.0
    return track.get_active_length()
    
## Updates the track identified by RID
func update_track(
    track_rid: RID,
    type: int,
    name: String,
    curve1: MaszynaTrackCurve,
    curve2: MaszynaTrackCurve,
    width: float,
) -> void:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return
    var previous_endpoints: Array[Vector3] = track.get_endpoints()
    var changed_connected_endpoint: bool = _has_connected_endpoint_changed(track, previous_endpoints, curve1, curve2)
    var existing_name = _named_tracks.find_key(track_rid)
    if name and existing_name and not existing_name == name and _named_tracks.get(existing_name) == track_rid:
        _named_tracks.erase(existing_name)
    if name and _named_tracks.has(name):
        if not _named_tracks[name] == track_rid:
            push_error("Named track already exists: " + name)
            return
    if name:
        _named_tracks[name] = track_rid

    track.type = type
    track.curve1 = curve1
    track.curve2 = curve2
    track.width = width
    track.switch_common_endpoint_index = _detect_switch_common_endpoint(curve1, curve2)
    track.get_endpoints()
    _update_spatial_index(track)
    if changed_connected_endpoint:
        _mark_topology_changed()
    tracks_changed.emit()


## Returns TrackSegment for provided RID (FIXME: TrackSegment should not be exposed)
func get_track(track_rid: RID) -> TrackSegment:
    return _tracks.get(track_rid)


func get_track_rids() -> Array[RID]:
    return _tracks.keys()


func get_rail_height() -> float:
    # Based on RailProfile definition, the first point is [x, y, ...].
    # RailProfile points are structured as [x, y, nx, ny, v].
    # Y is at index 1.
    return 0.180 # Based on DEFAULT_RAIL_PROFILE_POINTS in track_rendering_server.gd [0.111, -0.180, ...]


func sample_track_transform(track_rid: RID, offset: float) -> Transform3D:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return Transform3D.IDENTITY
    var curve_data: MaszynaTrackCurve = track.get_active_curve()
    if not curve_data:
        return Transform3D.IDENTITY

    var curve: Curve3D = _build_domain_curve(curve_data)
    var length: float = curve.get_baked_length()
    var safe_offset: float = clampf(offset, 0.0, length)
    var origin: Vector3 = curve.sample_baked(safe_offset, true)
    var tangent: Vector3 = _sample_curve_tangent(curve, safe_offset)
    var roll: float = _sample_curve_roll(curve_data, safe_offset, length)
    return _build_track_transform(origin, tangent, roll)


func advance_on_track(track_rid: RID, offset: float, direction: Direction, distance: float) -> TrackMotionResult:
    var result: TrackMotionResult = TrackMotionResult.new()
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return result

    var current_track_rid: RID = track_rid
    var current_direction: Direction = direction
    var current_length: float = get_active_track_length(current_track_rid)
    var current_offset: float = clampf(offset, 0.0, current_length)
    var remaining: float = absf(distance)
    var request_sign: float = -1.0 if distance < 0.0 else 1.0
    var movement_sign: float = _get_direction_sign(current_direction) * request_sign
    var hop_count: int = 0

    while remaining > 0.0001 and hop_count < 1024:
        hop_count += 1
        track = _tracks.get(current_track_rid)
        if not track:
            break
        current_length = get_active_track_length(current_track_rid)
        if current_length <= 0.0:
            break

        var distance_to_endpoint: float = current_length - current_offset if movement_sign > 0.0 else current_offset
        if remaining <= distance_to_endpoint:
            current_offset += movement_sign * remaining
            remaining = 0.0
            break

        current_offset = current_length if movement_sign > 0.0 else 0.0
        remaining -= distance_to_endpoint

        var endpoint_index: int = _get_active_endpoint_index(track, movement_sign > 0.0)
        var connection: Dictionary = _get_motion_neighbor_connection(current_track_rid, endpoint_index)
        if not connection:
            break

        var next_track_rid: RID = connection["track_rid"]
        var next_track: TrackSegment = _tracks.get(next_track_rid)
        if not next_track:
            break
        var next_endpoint_index: int = connection["endpoint_index"]
        var next_movement_sign: float = _get_movement_sign_from_entry_endpoint(next_track, next_endpoint_index)
        if is_zero_approx(next_movement_sign):
            break

        current_track_rid = next_track_rid
        current_length = get_active_track_length(current_track_rid)
        current_offset = 0.0 if next_movement_sign > 0.0 else current_length
        current_direction = _get_direction_from_sign(next_movement_sign * request_sign)
        movement_sign = next_movement_sign

    result.track_rid = current_track_rid
    result.offset = current_offset
    result.direction = current_direction
    return result


func get_tracks_in_aabb(aabb: Rect2) -> Array[RID]:
    var result: Array[RID] = []
    var seen: Dictionary = {}

    var min_cell = (aabb.position / _GRID_CELL_SIZE).floor()
    var max_cell = ((aabb.position + aabb.size) / _GRID_CELL_SIZE).floor()

    for x in range(int(min_cell.x), int(max_cell.x) + 1):
        for y in range(int(min_cell.y), int(max_cell.y) + 1):
            var cell = Vector2i(x, y)
            var cell_tracks = _spatial_grid.get(cell)
            if cell_tracks:
                for rid in cell_tracks:
                    if not seen.has(rid):
                        var track = _tracks.get(rid)
                        if track and aabb.intersects(track.aabb):
                            result.append(rid)
                            seen[rid] = true
    return result


func _update_spatial_index(track: TrackSegment) -> void:
    _remove_from_spatial_index(track)

    # Calculate AABB
    var points: Array[Vector3] = []
    if track.curve1:
        points.append(track.curve1.p1)
        points.append(track.curve1.p2)
    if track.curve2:
        points.append(track.curve2.p1)
        points.append(track.curve2.p2)

    if not points:
        return

    var rect = Rect2(Vector2(points[0].x, points[0].z), Vector2.ZERO)
    for p in points:
        rect = rect.expand(Vector2(p.x, p.z))

    track.aabb = rect.grow(5.0) # Small margin

    var min_cell = (track.aabb.position / _GRID_CELL_SIZE).floor()
    var max_cell = ((track.aabb.position + track.aabb.size) / _GRID_CELL_SIZE).floor()

    for x in range(int(min_cell.x), int(max_cell.x) + 1):
        for y in range(int(min_cell.y), int(max_cell.y) + 1):
            var cell = Vector2i(x, y)
            var cell_tracks = _spatial_grid.get(cell)
            if cell_tracks == null:
                cell_tracks = []
                _spatial_grid[cell] = cell_tracks
            if not track.track_rid in cell_tracks:
                cell_tracks.append(track.track_rid)
            if not cell in track.grid_cells:
                track.grid_cells.append(cell)


func _remove_from_spatial_index(track: TrackSegment) -> void:
    for cell in track.grid_cells:
        var cell_tracks = _spatial_grid.get(cell)
        if not cell_tracks == null:
            cell_tracks.erase(track.track_rid)
            if not cell_tracks:
                _spatial_grid.erase(cell)
    track.grid_cells.clear()


func get_connected_track_rid(track_rid: RID, endpoint_index: int) -> RID:

    var connection: Dictionary = _get_unique_neighbor_connection(track_rid, endpoint_index)
    if not connection:
        return UNDEFINED_TRACK
    return connection["track_rid"]


func get_connected_track_rids(track_rid: RID, endpoint_index: int) -> Array[RID]:

    var result: Array[RID] = []
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:

        return result
    if endpoint_index < 0 or endpoint_index >= track.node_ids.size():
        return result
    var node_id: int = track.node_ids[endpoint_index]
    if node_id == INVALID_NODE_ID:

        return result
    var node: TrackNode = _get_node(node_id)
    if not node:
        return result
    for ref: EndpointRef in node.endpoint_refs:
        if ref.track_rid == track_rid:
            continue
        if not ref.track_rid in result:
            result.append(ref.track_rid)

    return result


func get_track_branch_neighbors(track_rid: RID, branch_index: int) -> BranchNeighbors:
    var neighbors: BranchNeighbors = BranchNeighbors.new()
    var track: TrackSegment = _tracks.get(track_rid)
    if not track or track.switch_common_endpoint_index < 0:
        return neighbors
    if branch_index == 1 and not track.curve2:
        return neighbors

    var previous_endpoint_index: int = _get_branch_endpoint_index(branch_index, false)
    var next_endpoint_index: int = _get_branch_endpoint_index(branch_index, true)
    if previous_endpoint_index == -1 or next_endpoint_index == -1:
        return neighbors

    var previous_connection: Dictionary = _get_unique_neighbor_connection(track_rid, previous_endpoint_index)
    if previous_connection:
        neighbors.previous_track_rid = previous_connection["track_rid"]
        neighbors.previous_endpoint_index = previous_connection["endpoint_index"]

    var next_connection: Dictionary = _get_unique_neighbor_connection(track_rid, next_endpoint_index)
    if next_connection:
        neighbors.next_track_rid = next_connection["track_rid"]
        neighbors.next_endpoint_index = next_connection["endpoint_index"]

    return neighbors


func get_switch_track(track_rid: RID) -> TrackSegment:

    var track: TrackSegment = _tracks.get(track_rid)
    if not track:

        return null
    if track.switch_common_endpoint_index < 0:

        return null
    var node_id: int = track.node_ids[track.switch_common_endpoint_index]
    if node_id == INVALID_NODE_ID:

        return null
    var node: TrackNode = _get_node(node_id)
    if not node:
        return null
    var target_track = null
    for ref: EndpointRef in node.endpoint_refs:
        if ref.track_rid == track_rid:
            continue
        var linked_track: TrackSegment = _tracks.get(ref.track_rid)
        if not linked_track:
            continue
        if linked_track.curve2:
            continue
        target_track = linked_track
        break

    return target_track

func get_switch_state(track_rid: RID) -> SwitchState:
    var track: TrackSegment = _tracks.get(track_rid)
    if track:
        return track.switch_state
    else:
        push_error("Track RID is not valid: ", track_rid)
        return SwitchState.COMMON

func get_track_name(track_rid: RID) -> String:
    var track: TrackSegment = _tracks.get(track_rid)
    if track:
        var name = _named_tracks.find_key(track_rid)
        return name if name else ""
    else:
        push_error("Track RID is not valid: ", track_rid)
        return ""

func get_track_curve1(track_rid: RID) -> MaszynaTrackCurve:
    var track: TrackSegment = _tracks.get(track_rid)
    if track:
        return track.curve1
    else:
        push_error("Track RID is not valid: ", track_rid)
        return null

func get_track_curve2(track_rid: RID) -> MaszynaTrackCurve:
    var track: TrackSegment = _tracks.get(track_rid)
    if track:
        return track.curve2
    else:
        push_error("Track RID is not valid: ", track_rid)
        return null

func set_switch_state(track_rid: RID, state: SwitchState) -> void:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return
    if track.switch_state == state:
        return
    var from_state: int = track.switch_state
    track.switch_state = state
    # MaSzyna Track.cpp:1743-1755 sets desired switch offset.
    track.switch_desired_offset = SWITCH_MAX_OFFSET + track.switch_f_offset_delay if state == SwitchState.DIVERGING else -track.switch_f_offset_delay
    switch_state_changed.emit(track_rid, track.switch_state)
    switching_started.emit(track_rid, from_state, state)

    var existing_tween: Tween = _switch_tweens.get(track_rid)
    if existing_tween:
        existing_tween.kill()
        _switch_tweens.erase(track_rid)

    var duration: float = _get_switch_offset_duration(track, track.switch_desired_offset)
    if duration <= 0.0:
        track.switch_f_offset = track.switch_desired_offset
        switching_finished.emit(track_rid, state)
        return

    # MaSzyna Track.cpp:1933-1955 animates domain offset.
    var tween: Tween = create_tween()
    _switch_tweens[track_rid] = tween
    tween.tween_property(track, "switch_f_offset", track.switch_desired_offset, duration)
    tween.finished.connect(func():
        _switch_tweens.erase(track_rid)
        track.switch_f_offset = track.switch_desired_offset
        switching_finished.emit(track_rid, state)
    )


func _get_switch_offset_duration(track: TrackSegment, target_offset: float) -> float:
    var remaining_distance: float = absf(target_offset - track.switch_f_offset)
    if is_zero_approx(remaining_distance):
        return 0.0
    var full_distance: float = SWITCH_MAX_OFFSET + track.switch_f_offset_delay * 2.0
    var full_duration: float = 2.0
    return full_duration * remaining_distance / full_distance


func get_topology_summary() -> Dictionary:
    var stats = { "graphs": [], "orphaned_tracks_count": 0 }

    # Calculate graphs stats
    for graph_id in _graph_members:
        var track_rids = _graph_members[graph_id]
        var num_tracks = track_rids.size()
        var num_switches = 0
        var total_length = 0.0

        for track_rid in track_rids:
            var track = _tracks.get(track_rid)
            if not track: continue
            if track.type == TrackType.SWITCH:
                num_switches += 1
            total_length += get_active_track_length(track_rid)

        stats.graphs.append({
            "id": graph_id,
            "num_tracks": num_tracks,
            "num_switches": num_switches,
            "total_length": total_length
        })

    # Calculate orphaned tracks
    for track_rid in _tracks:
        var track = _tracks[track_rid]
        var is_orphaned = true
        for node_id in track.node_ids:
            if node_id == INVALID_NODE_ID:
                continue
            var node = _get_node(node_id)
            if node and node.endpoint_refs.size() > 1:
                is_orphaned = false
                break
        if is_orphaned:
            stats.orphaned_tracks_count += 1
    return stats


func rebuild_topology() -> void:
    _clear_topology()
    _connect_all_tracks()

    _rebuild_graph_ids()
    is_topology_changed = false
    tracks_changed.emit()


func _rebuild_graph_ids() -> void:
    _graph_members.clear()
    var visited: Dictionary = {}
    for track_rid: RID in _tracks.keys():
        if visited.has(track_rid):
            continue
        var graph_id: int = _allocate_graph_id()
        var queue: Array[RID] = [track_rid]
        while queue:
            var current_rid: RID = queue.pop_front()
            if visited.has(current_rid):
                continue
            visited[current_rid] = true
            var current_track: TrackSegment = _tracks.get(current_rid)
            if not current_track:
                continue
            current_track.graph_id = graph_id
            if not _graph_members.has(graph_id):
                _graph_members[graph_id] = []
            _graph_members[graph_id].append(current_rid)

            for node_id: int in current_track.node_ids:
                if node_id == INVALID_NODE_ID:
                    continue
                var node: TrackNode = _get_node(node_id)
                if not node:
                    continue
                for ref: EndpointRef in node.endpoint_refs:
                    if ref.track_rid == current_rid:
                        continue
                    if not visited.has(ref.track_rid):
                        queue.append(ref.track_rid)


func _allocate_graph_id() -> int:
    var graph_id: int = _next_graph_id
    _next_graph_id += 1
    return graph_id


func _get_or_create_node(world_position: Vector3, track_rid: RID, endpoint_index: int) -> int:
    var node_id: int = _find_own_node(world_position, track_rid)
    if node_id == INVALID_NODE_ID:
        node_id = _create_node(world_position)
    var node: TrackNode = _nodes[node_id]
    node.endpoint_refs.append(EndpointRef.new(track_rid, endpoint_index))
    return node_id


func _get_node(node_id: int) -> TrackNode:
    if node_id < 0 or node_id >= _nodes.size():
        return null
    return _nodes[node_id]


func _get_unique_neighbor_connection(track_rid: RID, endpoint_index: int) -> Dictionary:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return {}
    if endpoint_index < 0 or endpoint_index >= track.node_ids.size():
        return {}
    var node_id: int = track.node_ids[endpoint_index]
    var node: TrackNode = _get_node(node_id)
    if not node:
        return {}

    var unique_track_rid: RID = UNDEFINED_TRACK
    var unique_endpoint_index: int = -1
    for ref: EndpointRef in node.endpoint_refs:
        if ref.track_rid == track_rid:
            continue
        if not unique_track_rid.is_valid():
            unique_track_rid = ref.track_rid
            unique_endpoint_index = ref.endpoint_index
            continue
        if not unique_track_rid == ref.track_rid or not unique_endpoint_index == ref.endpoint_index:
            return {}
    if not unique_track_rid.is_valid():
        return {}
    return {
        "track_rid": unique_track_rid,
        "endpoint_index": unique_endpoint_index,
    }


func _get_motion_neighbor_connection(track_rid: RID, endpoint_index: int) -> Dictionary:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return {}
    if endpoint_index < 0 or endpoint_index >= track.node_ids.size():
        return {}
    var node: TrackNode = _get_node(track.node_ids[endpoint_index])
    if not node:
        return {}

    var unique_track_rid: RID = UNDEFINED_TRACK
    var unique_endpoint_index: int = -1
    for ref: EndpointRef in node.endpoint_refs:
        if ref.track_rid == track_rid:
            continue
        var candidate_track: TrackSegment = _tracks.get(ref.track_rid)
        if not candidate_track:
            continue
        if not _is_active_endpoint(candidate_track, ref.endpoint_index):
            continue
        if not unique_track_rid.is_valid():
            unique_track_rid = ref.track_rid
            unique_endpoint_index = ref.endpoint_index
            continue
        if not unique_track_rid == ref.track_rid or not unique_endpoint_index == ref.endpoint_index:
            return {}
    if not unique_track_rid.is_valid():
        return {}
    return {
        "track_rid": unique_track_rid,
        "endpoint_index": unique_endpoint_index,
    }


func _get_branch_endpoint_index(branch_index: int, is_end: bool) -> int:
    if branch_index == 0:
        return 1 if is_end else 0
    if branch_index == 1:
        return 3 if is_end else 2
    return -1


func _get_active_endpoint_index(track: TrackSegment, is_end: bool) -> int:
    var branch_index: int = 1 if track.get_active_curve() == track.curve2 else 0
    return _get_branch_endpoint_index(branch_index, is_end)


func _get_movement_sign_from_entry_endpoint(track: TrackSegment, endpoint_index: int) -> float:
    if endpoint_index == _get_active_endpoint_index(track, false):
        return 1.0
    if endpoint_index == _get_active_endpoint_index(track, true):
        return -1.0
    return 0.0


func _is_active_endpoint(track: TrackSegment, endpoint_index: int) -> bool:
    return endpoint_index == _get_active_endpoint_index(track, false) or endpoint_index == _get_active_endpoint_index(track, true)


func _add_track_topology(track: TrackSegment) -> void:
    track.node_ids = [-1, -1, -1, -1]
    if track.curve1:
        track.node_ids[0] = _get_or_create_node(track.curve1.p1, track.track_rid, 0)
        track.node_ids[1] = _get_or_create_node(track.curve1.p2, track.track_rid, 1)
    if track.curve2:
        track.node_ids[2] = _get_or_create_node(track.curve2.p1, track.track_rid, 2)
        track.node_ids[3] = _get_or_create_node(track.curve2.p2, track.track_rid, 3)


func _remove_track_from_graph_members(track: TrackSegment) -> void:
    if track.graph_id == -1:
        return
    var members: Array = _graph_members.get(track.graph_id, [])
    members.erase(track.track_rid)
    if members.is_empty():
        _graph_members.erase(track.graph_id)
    else:
        _graph_members[track.graph_id] = members
    track.graph_id = -1


func _clear_topology() -> void:
    _nodes.clear()
    _graph_members.clear()
    for track: TrackSegment in _tracks.values():
        track.graph_id = -1
        track.node_ids = [-1, -1, -1, -1]


func _connect_all_tracks() -> void:
    # 1. Create initial nodes for all track endpoints
    for track: TrackSegment in _tracks.values():
        _add_track_topology(track)

    # 2. Collect all endpoints
    var all_endpoints = []
    for track: TrackSegment in _tracks.values():
        for endpoint_index in range(4):
            if track.node_ids[endpoint_index] != INVALID_NODE_ID:
                all_endpoints.append({
                    "track_rid": track.track_rid,
                    "endpoint_index": endpoint_index,
                    "position": _get_endpoint_position(track, endpoint_index),
                    "node_id": track.node_ids[endpoint_index]
                })

    # 3. Merge endpoints that are close to each other
    for i in range(all_endpoints.size()):
        for j in range(i + 1, all_endpoints.size()):
            var ep1 = all_endpoints[i]
            var ep2 = all_endpoints[j]

            if ep1.node_id == ep2.node_id:
                continue

            if ep1.position.distance_to(ep2.position) <= _ENDPOINT_EPSILON:
                _merge_endpoint_nodes(ep1.track_rid, ep1.endpoint_index, ep2.track_rid, ep2.endpoint_index)
                # Update node_id for subsequent checks
                var new_node_id = _tracks.get(ep1.track_rid).node_ids[ep1.endpoint_index]
                ep2.node_id = new_node_id


func _create_node(world_position: Vector3) -> int:
    var node_id: int = _nodes.size()
    var node: TrackNode = TrackNode.new()
    node.id = node_id
    node.world_position = world_position
    _nodes.append(node)
    return node_id


func _find_own_node(world_position: Vector3, track_rid: RID) -> int:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return INVALID_NODE_ID
    for node_id: int in track.node_ids:
        var node: TrackNode = _get_node(node_id)
        if not node:
            continue
        if node.world_position.distance_to(world_position) <= _ENDPOINT_EPSILON:
            return node.id
    return INVALID_NODE_ID


func _merge_endpoint_nodes(
    first_track_rid: RID,
    first_endpoint_index: int,
    second_track_rid: RID,
    second_endpoint_index: int,
) -> void:
    print("Executing merge_endpoint_nodes")
    var first_track: TrackSegment = _tracks.get(first_track_rid)
    var second_track: TrackSegment = _tracks.get(second_track_rid)
    if not first_track or not second_track:
        return
    var first_node_id: int = first_track.node_ids[first_endpoint_index]
    var second_node_id: int = second_track.node_ids[second_endpoint_index]
    if first_node_id == INVALID_NODE_ID or second_node_id == INVALID_NODE_ID:
        return
    if first_node_id == second_node_id:
        return
    var first_node: TrackNode = _get_node(first_node_id)
    var second_node: TrackNode = _get_node(second_node_id)
    if not first_node or not second_node:
        return
    for ref: EndpointRef in second_node.endpoint_refs:
        first_node.endpoint_refs.append(ref)
        var ref_track: TrackSegment = _tracks.get(ref.track_rid)
        if ref_track:
            ref_track.node_ids[ref.endpoint_index] = first_node_id
    _nodes[second_node_id] = null


func _has_external_connection(track_rid: RID, endpoint_index: int) -> bool:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return false
    if endpoint_index < 0 or endpoint_index >= track.node_ids.size():
        return false
    var node: TrackNode = _get_node(track.node_ids[endpoint_index])
    if not node:
        return false
    for ref: EndpointRef in node.endpoint_refs:
        if not ref.track_rid == track_rid:
            return true
    return false


func _get_endpoint_position(track: TrackSegment, endpoint_index: int) -> Vector3:
    match endpoint_index:
        0:
            if track.curve1:
                return track.curve1.p1
        1:
            if track.curve1:
                return track.curve1.p2
        2:
            if track.curve2:
                return track.curve2.p1
        3:
            if track.curve2:
                return track.curve2.p2
    return Vector3.ZERO


func _has_connected_endpoint_changed(
    track: TrackSegment,
    previous_endpoints: Array[Vector3],
    curve1: MaszynaTrackCurve,
    curve2: MaszynaTrackCurve,
) -> bool:
    var next_endpoints: Array[Vector3] = _get_curve_endpoints(curve1, curve2)
    for endpoint_index: int in range(previous_endpoints.size()):
        if not _has_external_connection(track.track_rid, endpoint_index):
            continue
        if endpoint_index >= next_endpoints.size():
            return true
        if previous_endpoints[endpoint_index].distance_to(next_endpoints[endpoint_index]) > _ENDPOINT_EPSILON:
            return true
    return false


func _get_curve_endpoints(curve1: MaszynaTrackCurve, curve2: MaszynaTrackCurve) -> Array[Vector3]:
    var result: Array[Vector3] = []
    if curve1:
        result.append(curve1.p1)
        result.append(curve1.p2)
    if curve2:
        result.append(curve2.p1)
        result.append(curve2.p2)
    return result


func _mark_topology_changed() -> void:
    is_topology_changed = true
    topology_changed.emit()


func _track_can_initiate_connections(track: TrackSegment) -> bool:
    match track.type:
        TrackType.NORMAL, TrackType.ROAD, TrackType.RIVER, TrackType.TURN, TrackType.TABLE:
            return true
    return false


func _get_track_group(track: TrackSegment) -> int:
    match track.type:
        TrackType.NORMAL, TrackType.SWITCH, TrackType.TURN, TrackType.TABLE:
            return _TRACK_GROUP_RAIL
        TrackType.ROAD, TrackType.CROSS:
            return _TRACK_GROUP_ROAD
        TrackType.RIVER, TrackType.TRIBUTARY:
            return _TRACK_GROUP_WATER
    return _TRACK_GROUP_NONE


func _detect_switch_common_endpoint(curve1: MaszynaTrackCurve, curve2: MaszynaTrackCurve) -> int:
    return _get_curve_common_endpoint_index(curve1, curve2)


func _get_curve_common_endpoint_index(curve: MaszynaTrackCurve, other_curve: MaszynaTrackCurve) -> int:
    if not curve or not other_curve:
        return -1
    if curve.p1.distance_to(other_curve.p1) <= _ENDPOINT_EPSILON:
        return 0
    if curve.p1.distance_to(other_curve.p2) <= _ENDPOINT_EPSILON:
        return 0
    if curve.p2.distance_to(other_curve.p1) <= _ENDPOINT_EPSILON:
        return 1
    if curve.p2.distance_to(other_curve.p2) <= _ENDPOINT_EPSILON:
        return 1
    return -1



func _build_domain_curve(curve_data: MaszynaTrackCurve) -> Curve3D:
    var curve: Curve3D = Curve3D.new()
    curve.closed = false
    curve.bake_interval = float(ProjectSettings.get_setting("maszyna/track_curve_bake_interval", 10.0))
    curve.add_point(
        curve_data.p1 + Vector3(0.0, _get_roll_fix_height(curve_data.roll1), 0.0),
        Vector3.ZERO,
        curve_data.c1
    )
    curve.add_point(
        curve_data.p2 + Vector3(0.0, _get_roll_fix_height(curve_data.roll2), 0.0),
        curve_data.c2,
        Vector3.ZERO
    )
    return curve


func _sample_curve_tangent(curve: Curve3D, offset: float) -> Vector3:
    var length: float = curve.get_baked_length()
    if length <= 0.0:
        return Vector3.FORWARD
    var sample_distance: float = minf(0.1, length)
    var previous_offset: float = clampf(offset - sample_distance, 0.0, length)
    var next_offset: float = clampf(offset + sample_distance, 0.0, length)
    if is_equal_approx(previous_offset, next_offset):
        previous_offset = 0.0
        next_offset = length
    var tangent: Vector3 = curve.sample_baked(next_offset, true) - curve.sample_baked(previous_offset, true)
    if tangent.length_squared() <= 0.000001:
        return Vector3.FORWARD
    return tangent.normalized()


func _sample_curve_roll(curve_data: MaszynaTrackCurve, offset: float, length: float) -> float:
    if length <= 0.0:
        return curve_data.roll1
    return lerpf(curve_data.roll1, curve_data.roll2, clampf(offset / length, 0.0, 1.0))


func _build_track_transform(origin: Vector3, tangent: Vector3, roll_degrees: float) -> Transform3D:
    var forward: Vector3 = tangent.normalized()
    if forward.length_squared() <= 0.000001:
        forward = Vector3.FORWARD

    var reference_up: Vector3 = Vector3.UP
    if absf(forward.dot(reference_up)) > 0.999:
        reference_up = Vector3.RIGHT

    var z_axis: Vector3 = -forward
    var x_axis: Vector3 = reference_up.cross(z_axis).normalized()
    var y_axis: Vector3 = z_axis.cross(x_axis).normalized()
    var basis: Basis = Basis(x_axis, y_axis, z_axis).orthonormalized()
    basis = basis.rotated(forward, deg_to_rad(roll_degrees)).orthonormalized()
    return Transform3D(basis, origin)


func _get_roll_fix_height(roll_degrees: float) -> float:
    return absf(sin(deg_to_rad(roll_degrees)) * 0.75)


func _get_direction_sign(direction: Direction) -> float:
    return -1.0 if direction == Direction.ALIGNED_WITH_CONSIST_FRONT else 1.0


func _get_direction_from_sign(direction_sign: float) -> Direction:
    return Direction.ALIGNED_WITH_CONSIST_FRONT if direction_sign < 0.0 else Direction.OPPOSITE_TO_CONSIST_FRONT
