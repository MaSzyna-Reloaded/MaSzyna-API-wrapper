@tool
extends Node

class ControllerState:
    var object_id: int = 0


class VehicleState:
    var track_rid: RID = TrackManager.UNDEFINED_TRACK
    var track_offset: float = 0.0
    var track_direction: TrackManager.Direction = TrackManager.Direction.DIRECTION_NORMAL
    var switch_track: TrackManager.SwitchTrack = TrackManager.SwitchTrack.TRACK_COMMON
    var controller_rid: RID = RID()


const _ENDPOINT_EPSILON: float = 0.25
const _SWITCH_FORCE_SEGMENT_COUNT: int = 6
const _SWITCH_FORCE_BLADE_RATIO: float = 0.65

var _controllers: Dictionary[RID, ControllerState] = {}
var _controller_vehicles: Dictionary[RID, RID] = {}
var _vehicles: Dictionary[RID, VehicleState] = {}
var _next_controller_id: int = 0
var _next_vehicle_id: int = 0


func _enter_tree() -> void:
    Engine.register_singleton("RailVehiclePhysicsServer", self)


func _exit_tree() -> void:
    Engine.unregister_singleton("RailVehiclePhysicsServer")


func controller_create(controller: Object = null) -> RID:
    _next_controller_id += 1
    var controller_rid: RID = rid_from_int64(_next_controller_id)
    var state: ControllerState = ControllerState.new()
    if controller:
        state.object_id = controller.get_instance_id()
    _controllers[controller_rid] = state
    return controller_rid


func controller_free(controller_rid: RID) -> void:
    if not _controllers.has(controller_rid):
        return
    _controllers.erase(controller_rid)
    _controller_vehicles.erase(controller_rid)
    for state: VehicleState in _vehicles.values():
        if state.controller_rid == controller_rid:
            state.controller_rid = RID()


func vehicle_create() -> RID:
    _next_vehicle_id += 1
    var vehicle_rid: RID = rid_from_int64(_next_vehicle_id)
    _vehicles[vehicle_rid] = VehicleState.new()
    return vehicle_rid


func vehicle_free(vehicle_rid: RID) -> void:
    var state: VehicleState = _vehicles.get(vehicle_rid)
    if not state:
        return
    if state.controller_rid.is_valid() and _controller_vehicles.get(state.controller_rid) == vehicle_rid:
        _controller_vehicles.erase(state.controller_rid)
    _vehicles.erase(vehicle_rid)


func vehicle_bind_controller(vehicle_rid: RID, controller_rid: RID) -> void:
    var state: VehicleState = _vehicles.get(vehicle_rid)
    if not state:
        return
    if controller_rid.is_valid() and not _controllers.has(controller_rid):
        return
    if state.controller_rid.is_valid() and _controller_vehicles.get(state.controller_rid) == vehicle_rid:
        _controller_vehicles.erase(state.controller_rid)
    state.controller_rid = controller_rid
    if controller_rid.is_valid():
        _controller_vehicles[controller_rid] = vehicle_rid


func vehicle_set_track(
    vehicle_rid: RID,
    track_rid: RID,
    track_offset: float,
    track_direction: TrackManager.Direction
) -> void:
    var state: VehicleState = _vehicles.get(vehicle_rid)
    if not state:
        return
    state.track_rid = track_rid
    state.track_direction = track_direction
    state.switch_track = TrackManager.switch_get_active_track(track_rid) if TrackManager.track_is_switch(track_rid) else TrackManager.SwitchTrack.TRACK_COMMON
    state.track_offset = clampf(track_offset, 0.0, _get_track_length(track_rid, state.switch_track))


func process_movement(vehicle_rid: RID, delta: float) -> void:
    var state: VehicleState = _vehicles.get(vehicle_rid)
    if not state:
        return
    if state.track_rid.is_valid() and not TrackManager.track_exists(state.track_rid):
        return
    if not state.controller_rid.is_valid():
        return
    var controller_state: ControllerState = _controllers.get(state.controller_rid)
    if not controller_state or controller_state.object_id == 0:
        return
    var controller: TrainController = instance_from_id(controller_state.object_id) as TrainController
    if not controller:
        return
    vehicle_move(vehicle_rid, controller.process_movement(delta))


func vehicle_move(vehicle_rid: RID, distance: float) -> void:
    var state: VehicleState = _vehicles.get(vehicle_rid)
    if not state or not state.track_rid.is_valid():
        return
    if not TrackManager.track_exists(state.track_rid):
        return
    if is_zero_approx(distance):
        return

    var current_track_rid: RID = state.track_rid
    var current_track_offset: float = clampf(state.track_offset, 0.0, _get_track_length(state.track_rid, state.switch_track))
    var current_track_direction: TrackManager.Direction = state.track_direction
    var current_switch_track: TrackManager.SwitchTrack = state.switch_track
    var remaining: float = absf(distance)
    var request_sign: float = -1.0 if distance < 0.0 else 1.0
    var movement_sign: float = (
        -1.0 if current_track_direction == TrackManager.Direction.DIRECTION_NORMAL else 1.0
    ) * request_sign
    var hop_count: int = 0

    while remaining > 0.0001 and hop_count < 1024:
        hop_count += 1
        if not current_track_rid.is_valid():
            break
        if not TrackManager.track_exists(current_track_rid):
            break

        var current_length: float = _get_track_length(current_track_rid, current_switch_track)
        if current_length <= 0.0:
            break

        var distance_to_endpoint: float = current_length - current_track_offset if movement_sign > 0.0 else current_track_offset
        var endpoint_index: int = _get_switch_track_endpoint_index(current_switch_track, movement_sign > 0.0)
        var requested_distance: float = minf(remaining, distance_to_endpoint)
        var next_offset_on_track: float = current_track_offset + movement_sign * requested_distance

        if TrackManager.track_is_switch(current_track_rid):
            var active_track: TrackManager.SwitchTrack = TrackManager.switch_get_active_track(current_track_rid)
            if not active_track == current_switch_track:
                var blade_boundary_offset: float = current_length
                var curve1: Curve3D = _build_domain_curve(TrackManager.track_get_curve1(current_track_rid))
                var curve2: Curve3D = _build_domain_curve(TrackManager.track_get_curve2(current_track_rid))
                if curve1 and curve2:
                    var branch_curve: Curve3D = curve2 if current_switch_track == TrackManager.SwitchTrack.TRACK_DIVERGING else curve1
                    var frog_dist: float = minf(curve1.get_baked_length(), curve2.get_baked_length())
                    var frog_step: float = 0.5
                    var frog_sample_distance: float = 0.0
                    while frog_sample_distance < frog_dist:
                        var point1: Vector3 = curve1.sample_baked(frog_sample_distance)
                        var point2: Vector3 = curve2.sample_baked(frog_sample_distance)
                        if point1.distance_to(point2) >= TrackManager.track_get_width(current_track_rid):
                            frog_dist = frog_sample_distance
                            break
                        frog_sample_distance += frog_step
                    var sampled_points: PackedVector3Array = PackedVector3Array()
                    if frog_dist > 0.0:
                        for sample_index: int in range(_SWITCH_FORCE_SEGMENT_COUNT + 1):
                            var sample_distance: float = frog_dist * float(sample_index) / float(_SWITCH_FORCE_SEGMENT_COUNT)
                            sampled_points.push_back(branch_curve.sample_baked(sample_distance, true))
                    if sampled_points.size() >= 2:
                        blade_boundary_offset = 0.0
                        var blade_sample_count: int = mini(
                            int(ceil(float(_SWITCH_FORCE_SEGMENT_COUNT) * _SWITCH_FORCE_BLADE_RATIO)),
                            sampled_points.size() - 1
                        )
                        for sample_index: int in range(blade_sample_count):
                            blade_boundary_offset += sampled_points[sample_index].distance_to(sampled_points[sample_index + 1])
                if current_track_offset > blade_boundary_offset and next_offset_on_track <= blade_boundary_offset:
                    TrackManager.switch_set_active_track(current_track_rid, current_switch_track)

        if remaining <= distance_to_endpoint:
            current_track_offset += movement_sign * remaining
            remaining = 0.0
            break

        current_track_offset = current_length if movement_sign > 0.0 else 0.0
        remaining -= distance_to_endpoint

        var connection: TrackManager.EndpointRef = _get_motion_connection(
            current_track_rid,
            endpoint_index
        )
        if not connection:
            break

        var next_track_rid: RID = connection.track_rid
        if not next_track_rid.is_valid():
            break

        var next_endpoint_index: int = connection.endpoint_index
        var next_switch_track: TrackManager.SwitchTrack = _get_switch_track_from_endpoint(next_endpoint_index)
        var next_movement_sign: float = _get_movement_sign_from_entry_endpoint(next_endpoint_index, next_switch_track)
        if is_zero_approx(next_movement_sign):
            break

        current_track_rid = next_track_rid
        current_switch_track = next_switch_track
        current_track_offset = 0.0 if next_movement_sign > 0.0 else _get_track_length(current_track_rid, current_switch_track)
        current_track_direction = (
            TrackManager.Direction.DIRECTION_NORMAL
            if next_movement_sign * request_sign < 0.0
            else TrackManager.Direction.DIRECTION_REVERSED
        )
        movement_sign = next_movement_sign

    state.track_rid = current_track_rid
    state.track_offset = current_track_offset
    state.track_direction = current_track_direction
    state.switch_track = current_switch_track


func vehicle_get_transform(vehicle_rid: RID) -> Transform3D:
    var state: VehicleState = _vehicles.get(vehicle_rid)
    if not state or not state.track_rid.is_valid():
        return Transform3D.IDENTITY
    if not TrackManager.track_exists(state.track_rid):
        return Transform3D.IDENTITY

    var curve_data: MaszynaTrackCurve = _get_curve_data(state.track_rid, state.switch_track)
    var curve: Curve3D = _build_domain_curve(curve_data)
    if not curve_data or not curve:
        return Transform3D.IDENTITY

    var length: float = curve.get_baked_length()
    var safe_offset: float = clampf(state.track_offset, 0.0, length)
    var origin: Vector3 = curve.sample_baked(safe_offset, true)
    var tangent: Vector3 = _sample_curve_tangent(curve, safe_offset)
    var roll: float = _sample_curve_roll(curve_data, safe_offset, length)
    var track_transform: Transform3D = _build_track_transform(origin, tangent, roll)
    if state.track_direction == TrackManager.Direction.DIRECTION_REVERSED:
        track_transform.basis = track_transform.basis.rotated(track_transform.basis.y.normalized(), PI).orthonormalized()
    track_transform.origin.y += TrackManager.RAIL_HEIGHT
    return track_transform


func controller_get_transform(controller_rid: RID) -> Transform3D:
    var vehicle_rid: RID = _controller_vehicles.get(controller_rid, RID())
    if not vehicle_rid.is_valid():
        return Transform3D.IDENTITY
    return vehicle_get_transform(vehicle_rid)


func _get_track_length(track_rid: RID, switch_track: TrackManager.SwitchTrack) -> float:
    if not track_rid.is_valid() or not TrackManager.track_exists(track_rid):
        return 0.0
    if TrackManager.track_is_switch(track_rid):
        return TrackManager.switch_track_get_length(track_rid, switch_track)
    return TrackManager.track_get_length(track_rid)


func _get_curve_data(track_rid: RID, switch_track: TrackManager.SwitchTrack) -> MaszynaTrackCurve:
    if not TrackManager.track_exists(track_rid):
        return null
    if TrackManager.track_is_switch(track_rid) and switch_track == TrackManager.SwitchTrack.TRACK_DIVERGING:
        return TrackManager.track_get_curve2(track_rid)
    return TrackManager.track_get_curve1(track_rid)


func _get_motion_connection(track_rid: RID, endpoint_index: int) -> TrackManager.EndpointRef:
    if not TrackManager.track_exists(track_rid):
        return null
    var connections: Array[TrackManager.EndpointRef] = TrackManager.track_get_endpoint_connections(track_rid, endpoint_index)
    var unique_connection: TrackManager.EndpointRef = null
    var unique_forced_switch_track: TrackManager.SwitchTrack = TrackManager.SwitchTrack.TRACK_COMMON
    var has_unique_forced_switch_track: bool = false

    for raw_connection: TrackManager.EndpointRef in connections:
        var candidate_connection: TrackManager.EndpointRef = TrackManager.EndpointRef.new(
            raw_connection.track_rid,
            raw_connection.endpoint_index
        )
        var candidate_forced_switch_track: TrackManager.SwitchTrack = TrackManager.SwitchTrack.TRACK_COMMON
        var has_candidate_forced_switch_track: bool = false
        if TrackManager.track_is_switch(raw_connection.track_rid):
            if _is_common_switch_endpoint(raw_connection.track_rid, raw_connection.endpoint_index):
                candidate_connection.endpoint_index = _get_switch_track_endpoint_index(
                    TrackManager.switch_get_active_track(raw_connection.track_rid),
                    false
                )
            else:
                candidate_forced_switch_track = _get_switch_track_from_endpoint(raw_connection.endpoint_index)
                has_candidate_forced_switch_track = true
        if not _is_motion_accessible_endpoint(candidate_connection.track_rid, candidate_connection.endpoint_index) \
        and not has_candidate_forced_switch_track:
            continue
        if not unique_connection:
            unique_connection = candidate_connection
            unique_forced_switch_track = candidate_forced_switch_track
            has_unique_forced_switch_track = has_candidate_forced_switch_track
            continue
        if not unique_connection.track_rid == candidate_connection.track_rid \
        or not unique_connection.endpoint_index == candidate_connection.endpoint_index \
        or not has_unique_forced_switch_track == has_candidate_forced_switch_track \
        or (has_unique_forced_switch_track and not unique_forced_switch_track == candidate_forced_switch_track):
            return null

    if not unique_connection:
        return null
    if has_unique_forced_switch_track:
        if not TrackManager.switch_get_active_track(unique_connection.track_rid) == unique_forced_switch_track:
            TrackManager.switch_set_active_track(unique_connection.track_rid, unique_forced_switch_track)
    return unique_connection


func _get_switch_track_endpoint_index(
    switch_track: TrackManager.SwitchTrack,
    is_end: bool
) -> TrackManager.EndpointIndex:
    if switch_track == TrackManager.SwitchTrack.TRACK_DIVERGING:
        return TrackManager.EndpointIndex.CURVE2_P2 if is_end else TrackManager.EndpointIndex.CURVE2_P1
    return TrackManager.EndpointIndex.CURVE1_P2 if is_end else TrackManager.EndpointIndex.CURVE1_P1


func _get_switch_track_from_endpoint(endpoint_index: int) -> TrackManager.SwitchTrack:
    if endpoint_index >= TrackManager.EndpointIndex.CURVE2_P1:
        return TrackManager.SwitchTrack.TRACK_DIVERGING
    return TrackManager.SwitchTrack.TRACK_COMMON


func _get_movement_sign_from_entry_endpoint(
    endpoint_index: int,
    switch_track: TrackManager.SwitchTrack
) -> float:
    if endpoint_index == _get_switch_track_endpoint_index(switch_track, false):
        return 1.0
    if endpoint_index == _get_switch_track_endpoint_index(switch_track, true):
        return -1.0
    return 0.0


func _is_motion_accessible_endpoint(track_rid: RID, endpoint_index: int) -> bool:
    if not TrackManager.track_exists(track_rid):
        return false
    if TrackManager.track_is_switch(track_rid):
        var active_track: TrackManager.SwitchTrack = TrackManager.switch_get_active_track(track_rid)
        return endpoint_index == _get_switch_track_endpoint_index(active_track, false) \
            or endpoint_index == _get_switch_track_endpoint_index(active_track, true)
    return endpoint_index == TrackManager.EndpointIndex.CURVE1_P1 \
        or endpoint_index == TrackManager.EndpointIndex.CURVE1_P2


func _is_common_switch_endpoint(track_rid: RID, endpoint_index: int) -> bool:
    if not TrackManager.track_exists(track_rid):
        return false
    var common_endpoint_index: int = TrackManager.track_get_common_endpoint_index(track_rid)
    if common_endpoint_index == TrackManager.SwitchCommonPoint.POINT_NONE:
        return false
    if endpoint_index == common_endpoint_index:
        return true
    return _get_endpoint_position(track_rid, endpoint_index).distance_to(
        _get_endpoint_position(track_rid, common_endpoint_index)
    ) <= _ENDPOINT_EPSILON


func _get_endpoint_position(track_rid: RID, endpoint_index: int) -> Vector3:
    if not TrackManager.track_exists(track_rid):
        return Vector3.ZERO
    if endpoint_index == TrackManager.EndpointIndex.CURVE1_P1:
        return TrackManager.track_get_curve1(track_rid).p1
    if endpoint_index == TrackManager.EndpointIndex.CURVE1_P2:
        return TrackManager.track_get_curve1(track_rid).p2
    if endpoint_index == TrackManager.EndpointIndex.CURVE2_P1:
        return TrackManager.track_get_curve2(track_rid).p1
    if endpoint_index == TrackManager.EndpointIndex.CURVE2_P2:
        return TrackManager.track_get_curve2(track_rid).p2
    return Vector3.ZERO


func _build_domain_curve(curve_data: MaszynaTrackCurve) -> Curve3D:
    if not curve_data:
        return null
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
