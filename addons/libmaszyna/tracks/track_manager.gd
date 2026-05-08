@tool
extends Node

## Stores track geometry, switch state and tracks topology.
##
## The manager keeps tracks indexed by RID, exposes topology queries and
## handles switch state changes.

## Identifies branch of a switch track.
enum SwitchTrack {TRACK_COMMON, TRACK_DIVERGING}
## Identifies which endpoint is shared by both switch curves.
enum SwitchCommonPoint {POINT_NONE=-1, POINT_P1=0, POINT_P2=1}
## Identifies one of the stored track curve endpoints.
enum EndpointIndex {CURVE1_P1=0, CURVE1_P2=1, CURVE2_P1=2, CURVE2_P2=3}
## Describes the logical type of a track segment.
enum TrackType {TRACK_UNKNOWN=-1, TRACK_NORMAL=0, TRACK_SWITCH, TRACK_ROAD, TRACK_CROSS, TRACK_RIVER, TRACK_TRIBUTARY, TRACK_TURN, TRACK_TABLE}
## Group of compatible track types for topology connections.
enum TrackTypeGroup {GROUP_NONE, GROUP_RAIL, GROUP_ROAD, GROUP_WATER}
## Movement direction along the current track.
enum Direction {DIRECTION_NORMAL=0, DIRECTION_REVERSED=1}

## Emitted when a switch changes its active branch.
signal switch_active_track_changed(track_rid: RID, active_track: int)
## Emitted when animated switch offset changes.
signal switch_offset_updated(track_rid: RID, offset: float)
## Emitted when switch branch animation starts.
signal switching_started(track_rid: RID, from_track: int, to_track: int)
## Emitted when switch branch animation finishes.
signal switching_finished(track_rid: RID, active_track: int)
## Emitted when track data changes.
signal tracks_changed
## Emitted when cached topology becomes outdated.
signal topology_changed

## Maximum switch blade offset.
const SWITCH_MAX_OFFSET: float = 0.1 # MaSzyna Track.cpp:35 fMaxOffset.
## Delay applied before switch blade movement starts.
const SWITCH_OFFSET_DELAY: float = 0.05
## Default rail height used by track sampling.
const RAIL_HEIGHT: float = 0.180
## Invalid node index used by topology storage.
const INVALID_NODE_ID: int = -1

const _ENDPOINT_EPSILON: float = 0.25
const _GRID_CELL_SIZE: float = 500.0
const _ENDPOINT_GRID_CELL_SIZE: float = _ENDPOINT_EPSILON
const _SWITCH_FORCE_SEGMENT_COUNT: int = 6
const _SWITCH_FORCE_BLADE_RATIO: float = 0.65
const _TRACK_TYPE_GROUPS: Dictionary = {
    TrackType.TRACK_NORMAL: TrackTypeGroup.GROUP_RAIL,
    TrackType.TRACK_SWITCH: TrackTypeGroup.GROUP_RAIL,
    TrackType.TRACK_TURN: TrackTypeGroup.GROUP_RAIL,
    TrackType.TRACK_TABLE: TrackTypeGroup.GROUP_RAIL,
    TrackType.TRACK_ROAD: TrackTypeGroup.GROUP_ROAD,
    TrackType.TRACK_CROSS: TrackTypeGroup.GROUP_ROAD,
    TrackType.TRACK_RIVER: TrackTypeGroup.GROUP_WATER,
    TrackType.TRACK_TRIBUTARY: TrackTypeGroup.GROUP_WATER,
}
const UNDEFINED_TRACK = RID()


## Stores runtime data for one registered track.
class TrackSegment:
    ## Unique RID of the track.
    var track_rid: RID = UNDEFINED_TRACK
    ## Track type from [enum TrackType].
    var type: int = 0
    ## Primary curve of the track.
    var curve1: MaszynaTrackCurve
    ## Secondary curve used by switch tracks.
    var curve2: MaszynaTrackCurve
    ## Baked primary curve used for sampling.
    var domain_curve1: Curve3D
    ## Baked secondary curve used for sampling.
    var domain_curve2: Curve3D
    ## Logical track width.
    var width: float = 1.6
    ## Total length of all stored curves.
    var length: float = 0.0
    ## Length of [member curve1].
    var length1: float = 0.0
    ## Length of [member curve2].
    var length2: float = 0.0
    ## Connected graph identifier assigned during topology rebuild.
    var graph_id: int = -1
    ## Active switch branch.
    var active_track: int = SwitchTrack.TRACK_COMMON
    ## Delay used when computing switch offset animation.
    var switch_f_offset_delay: float = TrackManager.SWITCH_OFFSET_DELAY
    ## Target offset of the animated switch blade.
    var switch_desired_offset: float = -TrackManager.SWITCH_OFFSET_DELAY
    # MaSzyna Track.cpp:57-58 initial common offset.
    ## Current animated switch blade offset.
    var switch_f_offset: float = -TrackManager.SWITCH_OFFSET_DELAY: # fOffset in MaSzyna
        set(value):
            switch_f_offset = value
            # MaSzyna Track.cpp:1933-1941 clamps blade offsets.
            switch_f_offset1 = minf(value, TrackManager.SWITCH_MAX_OFFSET)
            switch_f_offset2 = maxf(value, 0.0)
            TrackManager.switch_offset_updated.emit(track_rid, value)
    ## Clamped left blade offset.
    var switch_f_offset1: float = -0.05 # fOffset1
    ## Clamped right blade offset.
    var switch_f_offset2: float = 0.0   # fOffset2
    ## Shared endpoint index for switch curves.
    var switch_common_endpoint_index: int = SwitchCommonPoint.POINT_NONE
    ## Marked true if switch diverges in right
    var switch_is_right: bool = false
    ## Axis-aligned bounding box used by the spatial index.
    var aabb: Rect2 = Rect2()
    ## Topology node ids for each endpoint.
    var node_ids: PackedInt32Array = [
        INVALID_NODE_ID,
        INVALID_NODE_ID,
        INVALID_NODE_ID,
        INVALID_NODE_ID,
    ]
    var _cached_endpoints: Array[Vector3]

    ## Returns length of the active branch.
    func get_length() -> float:
        if curve2 and active_track == SwitchTrack.TRACK_DIVERGING:
            return length2
        return length1

    ## Returns length of the selected switch branch.
    func switch_track_get_length(switch_track: int) -> float:
        if switch_track == SwitchTrack.TRACK_DIVERGING and curve2:
            return length2
        return length1

    ## Returns curve of the active branch.
    func get_curve() -> MaszynaTrackCurve:
        if curve2 and active_track == SwitchTrack.TRACK_DIVERGING:
            return curve2
        return curve1

    ## Returns curve of the selected switch branch.
    func switch_track_get_curve(switch_track: int) -> MaszynaTrackCurve:
        if switch_track == SwitchTrack.TRACK_DIVERGING and curve2:
            return curve2
        return curve1

    ## Returns baked curve of the active branch.
    func get_domain_curve() -> Curve3D:
        if domain_curve2 and active_track == SwitchTrack.TRACK_DIVERGING:
            return domain_curve2
        return domain_curve1

    ## Returns baked curve of the selected switch branch.
    func switch_track_get_domain_curve(switch_track: int) -> Curve3D:
        if switch_track == SwitchTrack.TRACK_DIVERGING and domain_curve2:
            return domain_curve2
        return domain_curve1

    ## Replaces stored curves and rebuilds baked curves.
    func set_curves(new_curve1: MaszynaTrackCurve, new_curve2: MaszynaTrackCurve) -> void:
        curve1 = new_curve1
        curve2 = new_curve2
        domain_curve1 = TrackManager._build_domain_curve(curve1) if curve1 else null
        domain_curve2 = TrackManager._build_domain_curve(curve2) if curve2 else null
        _cached_endpoints.clear()
        switch_is_right = false
        update_length()
        switch_common_endpoint_index = _get_curve_common_endpoint_index()

        if curve1 and curve2:
            if not switch_common_endpoint_index == SwitchCommonPoint.POINT_NONE:
                var common_position: Vector3 = curve1.p1 if switch_common_endpoint_index == SwitchCommonPoint.POINT_P1 else curve1.p2
                var primary_direction: Vector3 = _get_switch_curve_direction_from_common(curve1, common_position)
                var secondary_direction: Vector3 = _get_switch_curve_direction_from_common(curve2, common_position)
                var primary_angle: float = atan2(primary_direction.x, primary_direction.z)
                var secondary_angle: float = atan2(secondary_direction.x, secondary_direction.z)
                var angle_delta: float = secondary_angle - primary_angle
                while angle_delta > PI:
                    angle_delta -= 2.0 * PI
                while angle_delta < -PI:
                    angle_delta += 2.0 * PI
                switch_is_right = angle_delta < 0.0

    ## Recomputes cached curve lengths.
    func update_length() -> void:
        length = 0.0
        length1 = 0.0
        length2 = 0.0
        if domain_curve1:
            length1 = domain_curve1.get_baked_length()
            length += length1
        if domain_curve2:
            length2 = domain_curve2.get_baked_length()
            length += length2

    ## Returns cached endpoint positions for both curves.
    func get_endpoints() -> Array[Vector3]:
        if _cached_endpoints:
            return _cached_endpoints
        _cached_endpoints.clear()
        if curve1:
            _cached_endpoints.append(curve1.p1)
            _cached_endpoints.append(curve1.p2)
        if curve2:
            _cached_endpoints.append(curve2.p1)
            _cached_endpoints.append(curve2.p2)
        return _cached_endpoints

    func _get_curve_common_endpoint_index() -> int:
        if not curve1 or not curve2:
            return SwitchCommonPoint.POINT_NONE
        if curve1.p1.distance_to(curve2.p1) <= _ENDPOINT_EPSILON:
            return SwitchCommonPoint.POINT_P1
        if curve1.p1.distance_to(curve2.p2) <= _ENDPOINT_EPSILON:
            return SwitchCommonPoint.POINT_P1
        if curve1.p2.distance_to(curve2.p1) <= _ENDPOINT_EPSILON:
            return SwitchCommonPoint.POINT_P2
        if curve1.p2.distance_to(curve2.p2) <= _ENDPOINT_EPSILON:
            return SwitchCommonPoint.POINT_P2
        return SwitchCommonPoint.POINT_NONE

    func _get_switch_curve_direction_from_common(curve: MaszynaTrackCurve, common_position: Vector3) -> Vector3:
        if curve.p1.distance_to(common_position) <= _ENDPOINT_EPSILON:
            return curve.p2 - curve.p1
        if curve.p2.distance_to(common_position) <= _ENDPOINT_EPSILON:
            return curve.p1 - curve.p2
        return Vector3.ZERO


## References one endpoint of a registered track.
class EndpointRef:
    ## RID of the referenced track.
    var track_rid: RID = UNDEFINED_TRACK
    ## Endpoint index on the referenced track.
    var endpoint_index: EndpointIndex = EndpointIndex.CURVE1_P1

    ## Creates a reference to one track endpoint.
    func _init(p_track_rid: RID, p_endpoint_index: EndpointIndex) -> void:
        track_rid = p_track_rid
        endpoint_index = p_endpoint_index


## Stores one merged topology node.
class TrackNode:
    ## Internal node identifier.
    var id: int = INVALID_NODE_ID
    ## World position shared by connected endpoints.
    var world_position: Vector3 = Vector3.ZERO
    ## Endpoint references attached to this node.
    var endpoint_refs: Array[EndpointRef] = []


## Describes previous and next neighbors of a switch branch.
class BranchNeighbors:
    ## Track connected before the branch.
    var previous_track_rid: RID = UNDEFINED_TRACK
    ## Endpoint index connected before the branch.
    var previous_endpoint_index: EndpointIndex = EndpointIndex.CURVE1_P1
    ## Track connected after the branch.
    var next_track_rid: RID = UNDEFINED_TRACK
    ## Endpoint index connected after the branch.
    var next_endpoint_index: EndpointIndex = EndpointIndex.CURVE1_P1


var _tracks: Dictionary[RID, TrackSegment] = {}
var _named_tracks: Dictionary[String, RID] = {}
var _spatial_index: SpatialIndex = SpatialIndex.new(_GRID_CELL_SIZE)
var _next_track_id: int = 0
var _next_graph_id: int = 0
var _graph_members: Dictionary[int, Array] = {}
var _nodes: Array[TrackNode] = []
var _switch_tweens: Dictionary[RID, Tween] = {}
## True when topology needs to be rebuilt.
var is_topology_changed: bool = false

func _enter_tree() -> void:
    Engine.register_singleton("TrackManager", self)

func _exit_tree() -> void:
    Engine.unregister_singleton("TrackManager")

## Creates and registers a new empty track.
func track_create() -> RID:
    _next_track_id += 1
    var track_rid: RID = rid_from_int64(_next_track_id)
    var track: TrackSegment = TrackSegment.new()
    track.track_rid = track_rid
    _tracks[track_rid] = track
    _mark_topology_changed()
    tracks_changed.emit()
    return track_rid


## Removes a registered track and its topology data.
func track_free(track_rid: RID) -> void:
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
    _spatial_index.remove(track.track_rid)
    _remove_track_from_graph_members(track)
    _tracks.erase(track_rid)
    _clear_topology()
    _mark_topology_changed()
    tracks_changed.emit()

## Returns the track RID registered under [param name].
func track_get_rid_by_name(name: String) -> RID:
    var rid: Variant = _named_tracks.get(name)
    if rid == null:
        return UNDEFINED_TRACK
    return rid

## Returns length of the active branch for [param track_rid].
func track_get_length(track_rid: RID) -> float:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return 0.0
    return track.get_length()

## Returns length of the selected switch branch.
func switch_track_get_length(track_rid: RID, switch_track: int) -> float:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return 0.0
    return track.switch_track_get_length(switch_track)

## Replaces the stored curves of a track.
func track_update_curves(track_rid: RID, curve1: MaszynaTrackCurve, curve2: MaszynaTrackCurve) -> void:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return

    var previous_endpoints: Array[Vector3] = track.get_endpoints()
    var changed_connected_endpoint: bool = _has_connected_endpoint_changed(track, previous_endpoints, curve1, curve2)

    track.set_curves(curve1, curve2)

    if changed_connected_endpoint:
        _mark_topology_changed()

## Updates type, name and width of a track.
func track_update(
    track_rid: RID,
    type: int,
    name: String,
    width: float,
) -> void:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return
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
    track.width = width
    var points: Array[Vector3] = track.get_endpoints()
    if points:
        var rect = Rect2(Vector2(points[0].x, points[0].z), Vector2.ZERO)
        for p in points:
            rect = rect.expand(Vector2(p.x, p.z))
        track.aabb = rect.grow(5.0)
        _spatial_index.remove(track.track_rid)
        _spatial_index.add(track.track_rid, track.aabb)
    else:
        _spatial_index.remove(track.track_rid)
    tracks_changed.emit()

## Returns whether the given RID is registered.
func track_exists(track_rid: RID) -> bool:
    return _tracks.has(track_rid)

## Returns all registered track RIDs.
func track_get_rids() -> Array[RID]:
    return _tracks.keys()

## Returns width of a track.
func track_get_width(track_rid: RID) -> float:
    var track: TrackSegment = _tracks.get(track_rid)
    if track:
        return track.width
    return 0.0

## Returns endpoint positions of a track.
func track_get_endpoints(track_rid: RID) -> Array[Vector3]:
    var track: TrackSegment = _tracks.get(track_rid)
    if track:
        return track.get_endpoints()
    var endpoints: Array[Vector3] = []
    return endpoints

## Returns the shared switch endpoint index.
func track_get_common_endpoint_index(track_rid: RID) -> int:
    var track: TrackSegment = _tracks.get(track_rid)
    if track:
        return track.switch_common_endpoint_index
    return SwitchCommonPoint.POINT_NONE

## Returns the first clamped switch blade offset.
func switch_get_f_offset1(track_rid: RID) -> float:
    var track: TrackSegment = _tracks.get(track_rid)
    if track:
        return track.switch_f_offset1
    return 0.0

## Returns the second clamped switch blade offset.
func switch_get_f_offset2(track_rid: RID) -> float:
    var track: TrackSegment = _tracks.get(track_rid)
    if track:
        return track.switch_f_offset2
    return 0.0

## Returns tracks intersecting the given AABB.
func tracks_find_in_aabb(aabb: Rect2) -> Array[RID]:
    return _spatial_index.query(aabb)

## Returns external connections for one track endpoint.
func track_get_endpoint_connections(track_rid: RID, endpoint_index: EndpointIndex) -> Array[EndpointRef]:
    var result: Array[EndpointRef] = []
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
        var candidate_track: TrackSegment = _tracks.get(ref.track_rid)
        if not candidate_track:
            continue
        var is_duplicate: bool = false
        for connection: EndpointRef in result:
            if connection.track_rid == ref.track_rid and connection.endpoint_index == ref.endpoint_index:
                is_duplicate = true
                break
        if is_duplicate:
            continue
        result.append(EndpointRef.new(ref.track_rid, ref.endpoint_index))

    return result

## Returns previous and next neighbors for a switch branch.
func switch_track_get_neighbors(track_rid: RID, switch_track: SwitchTrack) -> BranchNeighbors:
    var neighbors: BranchNeighbors = BranchNeighbors.new()
    var track: TrackSegment = _tracks.get(track_rid)
    if not track or track.switch_common_endpoint_index == SwitchCommonPoint.POINT_NONE:
        return neighbors
    if switch_track == SwitchTrack.TRACK_DIVERGING and not track.curve2:
        return neighbors

    var previous_endpoint_index: EndpointIndex = _get_switch_track_endpoint_index(track, switch_track, false)
    var next_endpoint_index: EndpointIndex = _get_switch_track_endpoint_index(track, switch_track, true)
    var previous_connection: EndpointRef = _get_unique_neighbor_connection(track_rid, previous_endpoint_index)
    if previous_connection:
        neighbors.previous_track_rid = previous_connection.track_rid
        neighbors.previous_endpoint_index = previous_connection.endpoint_index

    var next_connection: EndpointRef = _get_unique_neighbor_connection(track_rid, next_endpoint_index)
    if next_connection:
        neighbors.next_track_rid = next_connection.track_rid
        neighbors.next_endpoint_index = next_connection.endpoint_index

    return neighbors

## Returns whether the track is a switch.
func track_is_switch(track_rid: RID) -> bool:
    var track: TrackSegment = _tracks.get(track_rid)
    return track and track.type == TrackType.TRACK_SWITCH

## Returns whether a switch diverges to the right.
func switch_is_right(track_rid: RID) -> bool:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track or not track.type == TrackType.TRACK_SWITCH:
        return false
    return track.switch_is_right

## Returns the currently active branch of a switch.
func switch_get_active_track(track_rid: RID) -> SwitchTrack:
    var track: TrackSegment = _tracks.get(track_rid)
    if track:
        return track.active_track
    else:
        push_error("Track RID is not valid: ", track_rid)
        return SwitchTrack.TRACK_COMMON

## Returns the registered name of a track.
func track_get_name(track_rid: RID) -> String:
    var track: TrackSegment = _tracks.get(track_rid)
    if track:
        var name = _named_tracks.find_key(track_rid)
        return name if name else ""
    else:
        push_error("Track RID is not valid: ", track_rid)
        return ""

## Returns the first curve of a track.
func track_get_curve1(track_rid: RID) -> MaszynaTrackCurve:
    var track: TrackSegment = _tracks.get(track_rid)
    if track:
        return track.curve1
    else:
        push_error("Track RID is not valid: ", track_rid)
        return null

## Returns the second curve of a track.
func track_get_curve2(track_rid: RID) -> MaszynaTrackCurve:
    var track: TrackSegment = _tracks.get(track_rid)
    if track:
        return track.curve2
    else:
        push_error("Track RID is not valid: ", track_rid)
        return null

## Changes the active branch of a switch.
func switch_set_active_track(track_rid: RID, active_track: SwitchTrack) -> void:
    var track: TrackSegment = _tracks.get(track_rid)
    if not track:
        return
    if track.active_track == active_track:
        return
    var from_track: int = track.active_track
    track.active_track = active_track
    # MaSzyna Track.cpp:1743-1755 sets desired switch offset.
    track.switch_desired_offset = SWITCH_MAX_OFFSET + track.switch_f_offset_delay if active_track == SwitchTrack.TRACK_DIVERGING else -track.switch_f_offset_delay
    switch_active_track_changed.emit(track_rid, track.active_track)
    switching_started.emit(track_rid, from_track, active_track)

    var existing_tween: Tween = _switch_tweens.get(track_rid)
    if existing_tween:
        existing_tween.kill()
        _switch_tweens.erase(track_rid)

    var _remaining_distance: float = absf(track.switch_desired_offset - track.switch_f_offset)
    var _full_distance: float = SWITCH_MAX_OFFSET + track.switch_f_offset_delay * 2.0
    var _full_duration: float = 2.0
    var duration: float = _full_duration * _remaining_distance / _full_distance

    if duration <= 0.0:
        track.switch_f_offset = track.switch_desired_offset
        switching_finished.emit(track_rid, active_track)
        return

    # MaSzyna Track.cpp:1933-1955 animates domain offset.
    var tween: Tween = create_tween()
    _switch_tweens[track_rid] = tween
    tween.tween_property(track, "switch_f_offset", track.switch_desired_offset, duration)
    tween.finished.connect(func():
        _switch_tweens.erase(track_rid)
        track.switch_f_offset = track.switch_desired_offset
        switching_finished.emit(track_rid, active_track)
    )

## Returns a summary of connected graphs and orphaned tracks.
func topology_get_summary() -> Dictionary:
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
            if track.type == TrackType.TRACK_SWITCH:
                num_switches += 1
            total_length += track_get_length(track_rid)

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


## Rebuilds all cached track topology data.
func topology_rebuild() -> void:
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


func _get_unique_neighbor_connection(track_rid: RID, endpoint_index: EndpointIndex) -> EndpointRef:
    var connections: Array[EndpointRef] = track_get_endpoint_connections(track_rid, endpoint_index)
    if not connections.size() == 1:
        return null
    return EndpointRef.new(connections[0].track_rid, connections[0].endpoint_index)


func _get_switch_track_endpoint_index(track: TrackSegment, switch_track: SwitchTrack, is_end: bool) -> EndpointIndex:
    if switch_track == SwitchTrack.TRACK_DIVERGING and track.curve2:
        return EndpointIndex.CURVE2_P2 if is_end else EndpointIndex.CURVE2_P1
    return EndpointIndex.CURVE1_P2 if is_end else EndpointIndex.CURVE1_P1

func _get_active_endpoint_index(track: TrackSegment, is_end: bool) -> EndpointIndex:
    return _get_switch_track_endpoint_index(track, _get_active_switch_track(track), is_end)

func _is_active_endpoint(track: TrackSegment, endpoint_index: EndpointIndex) -> bool:
    return endpoint_index == _get_active_endpoint_index(track, false) or endpoint_index == _get_active_endpoint_index(track, true)

func _get_active_switch_track(track: TrackSegment) -> int:
    if track.type == TrackType.TRACK_SWITCH and track.curve2:
        return track.active_track
    return SwitchTrack.TRACK_COMMON

func _add_track_topology(track: TrackSegment) -> void:
    track.node_ids = [
        INVALID_NODE_ID,
        INVALID_NODE_ID,
        INVALID_NODE_ID,
        INVALID_NODE_ID,
    ]
    if track.curve1:
        track.node_ids[EndpointIndex.CURVE1_P1] = _get_or_create_node(track.curve1.p1, track.track_rid, EndpointIndex.CURVE1_P1)
        track.node_ids[EndpointIndex.CURVE1_P2] = _get_or_create_node(track.curve1.p2, track.track_rid, EndpointIndex.CURVE1_P2)
    if track.curve2:
        track.node_ids[EndpointIndex.CURVE2_P1] = _get_or_create_node(track.curve2.p1, track.track_rid, EndpointIndex.CURVE2_P1)
        track.node_ids[EndpointIndex.CURVE2_P2] = _get_or_create_node(track.curve2.p2, track.track_rid, EndpointIndex.CURVE2_P2)

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
        track.node_ids = [
            INVALID_NODE_ID,
            INVALID_NODE_ID,
            INVALID_NODE_ID,
            INVALID_NODE_ID,
        ]

func _connect_all_tracks() -> void:
    # 1. Create initial nodes for all track endpoints
    for track: TrackSegment in _tracks.values():
        _add_track_topology(track)

    # 2. Collect all endpoints
    var all_endpoints = []
    for track: TrackSegment in _tracks.values():
        for endpoint_index: int in range(EndpointIndex.CURVE2_P2 + 1):
            if not track.node_ids[endpoint_index] == INVALID_NODE_ID:
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

            var first_track: TrackSegment = _tracks.get(ep1.track_rid)
            var second_track: TrackSegment = _tracks.get(ep2.track_rid)
            if not first_track or not second_track:
                continue
            var first_track_group: int = _TRACK_TYPE_GROUPS.get(first_track.type, TrackTypeGroup.GROUP_NONE)
            var second_track_group: int = _TRACK_TYPE_GROUPS.get(second_track.type, TrackTypeGroup.GROUP_NONE)
            if first_track_group == TrackTypeGroup.GROUP_NONE or not first_track_group == second_track_group:
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
        EndpointIndex.CURVE1_P1:
            if track.curve1:
                return track.curve1.p1
        EndpointIndex.CURVE1_P2:
            if track.curve1:
                return track.curve1.p2
        EndpointIndex.CURVE2_P1:
            if track.curve2:
                return track.curve2.p1
        EndpointIndex.CURVE2_P2:
            if track.curve2:
                return track.curve2.p2
    return Vector3.ZERO

func _has_connected_endpoint_changed(
    track: TrackSegment,
    previous_endpoints: Array[Vector3],
    curve1: MaszynaTrackCurve,
    curve2: MaszynaTrackCurve,
) -> bool:
    var next_endpoints: Array[Vector3] = track.get_endpoints()
    for endpoint_index: int in range(previous_endpoints.size()):
        if not _has_external_connection(track.track_rid, endpoint_index):
            continue
        if endpoint_index >= next_endpoints.size():
            return true
        if previous_endpoints[endpoint_index].distance_to(next_endpoints[endpoint_index]) > _ENDPOINT_EPSILON:
            return true
    return false

func _mark_topology_changed() -> void:
    is_topology_changed = true
    topology_changed.emit()


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

func _get_roll_fix_height(roll_degrees: float) -> float:
    return absf(sin(deg_to_rad(roll_degrees)) * 0.75)
