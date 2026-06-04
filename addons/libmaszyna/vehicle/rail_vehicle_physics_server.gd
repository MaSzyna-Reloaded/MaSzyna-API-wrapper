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
    state.track_offset = clampf(track_offset, 0.0, TrackManager.track_get_length(track_rid, state.switch_track))


func process_movement(vehicle_rid: RID, delta: float) -> void:
    var state: VehicleState = _vehicles.get(vehicle_rid)
    if not state:
        return
    if not state.track_rid == TrackManager.UNDEFINED_TRACK and not TrackManager.track_exists(state.track_rid):
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
    if not state or not TrackManager.track_exists(state.track_rid):
        return
    if is_zero_approx(distance):
        return

    var current_track_rid: RID = state.track_rid
    var current_switch_track: TrackManager.SwitchTrack = state.switch_track
    var current_track_offset: float = clampf(state.track_offset, 0.0, TrackManager.track_get_length(current_track_rid, current_switch_track))
    var current_track_direction: TrackManager.Direction = state.track_direction
    var remaining: float = absf(distance)
    var request_sign: float = -1.0 if distance < 0.0 else 1.0
    # Convert movement relative to the vehicle front into curve offset movement.
    # Positive sign moves toward the branch end, negative toward the branch start.
    var movement_sign: float = (
        -1.0 if current_track_direction == TrackManager.Direction.DIRECTION_NORMAL else 1.0
    ) * request_sign

    while remaining > 0.0001:
        if not TrackManager.track_exists(current_track_rid):
            break
        var current_length: float = TrackManager.track_get_length(current_track_rid, current_switch_track)
        if current_length <= 0.0:
            break

        # Find the endpoint this movement heads toward on the occupied branch.
        var distance_to_endpoint: float = current_length - current_track_offset if movement_sign > 0.0 else current_track_offset
        var endpoint_index: int
        if TrackManager.track_is_switch(current_track_rid):
            endpoint_index = TrackManager.switch_get_branch_end_endpoint(current_track_rid, current_switch_track) \
                if movement_sign > 0.0 else TrackManager.switch_get_branch_start_endpoint(current_track_rid, current_switch_track)
            # If the vehicle is reversing through a switch blade on a non-active
            # branch, keep the branch it already occupies and force the switch back.
            var active_track: TrackManager.SwitchTrack = TrackManager.switch_get_active_track(current_track_rid)
            if not active_track == current_switch_track:
                var requested_distance: float = minf(remaining, distance_to_endpoint)
                var next_offset_on_track: float = current_track_offset + movement_sign * requested_distance
                var blade_boundary_offset: float = TrackManager.switch_get_blade_boundary_offset(current_track_rid, current_switch_track)
                if current_track_offset > blade_boundary_offset and next_offset_on_track <= blade_boundary_offset:
                    TrackManager.switch_set_active_track(current_track_rid, current_switch_track)
        else:
            endpoint_index = TrackManager.EndpointIndex.CURVE1_P2 if movement_sign > 0.0 else TrackManager.EndpointIndex.CURVE1_P1

        # Hot path: normal simulation steps stay within the current branch and
        # return before asking topology for the next track.
        if remaining <= distance_to_endpoint:
            current_track_offset += movement_sign * remaining
            remaining = 0.0
            break

        current_track_offset = current_length if movement_sign > 0.0 else 0.0
        remaining -= distance_to_endpoint

        # Large init/debug jumps cross endpoints by following the single
        # unambiguous connection. Ambiguous nodes stop at the endpoint.
        var connection: TrackManager.EndpointRef = _get_motion_connection(current_track_rid, endpoint_index)
        if not connection:
            break

        current_track_rid = connection.track_rid
        # The connection endpoint is where we enter the next track. For switches,
        # that endpoint also tells which branch the vehicle now occupies.
        if TrackManager.track_is_switch(current_track_rid):
            current_switch_track = TrackManager.switch_get_endpoint_branch(current_track_rid, connection.endpoint_index)
            var entered_at_end: bool = connection.endpoint_index == TrackManager.switch_get_branch_end_endpoint(current_track_rid, current_switch_track)
            movement_sign = -1.0 if entered_at_end else 1.0
        else:
            current_switch_track = TrackManager.SwitchTrack.TRACK_COMMON
            movement_sign = -1.0 if connection.endpoint_index == TrackManager.EndpointIndex.CURVE1_P2 else 1.0
        # Entering at branch start means offset grows; entering at branch end
        # means offset decreases from the branch length.
        current_track_offset = 0.0 if movement_sign > 0.0 else TrackManager.track_get_length(current_track_rid, current_switch_track)
        current_track_direction = (
            TrackManager.Direction.DIRECTION_NORMAL
            if movement_sign * request_sign < 0.0
            else TrackManager.Direction.DIRECTION_REVERSED
        )

    state.track_rid = current_track_rid
    state.track_offset = current_track_offset
    state.track_direction = current_track_direction
    state.switch_track = current_switch_track


func vehicle_get_transform(vehicle_rid: RID) -> Transform3D:
    var state: VehicleState = _vehicles.get(vehicle_rid)
    if not state or not TrackManager.track_exists(state.track_rid):
        return Transform3D.IDENTITY

    var curve_data: MaszynaTrackCurve = TrackManager.track_get_curve(state.track_rid, state.switch_track)
    var curve: Curve3D = TrackManager.track_get_domain_curve(state.track_rid, state.switch_track)
    if not curve_data or not curve:
        return Transform3D.IDENTITY

    var length: float = curve.get_baked_length()
    var safe_offset: float = clampf(state.track_offset, 0.0, length)
    var origin: Vector3 = curve.sample_baked(safe_offset, true)
    var sample_distance: float = minf(0.1, length)
    var previous_offset: float = clampf(safe_offset - sample_distance, 0.0, length)
    var next_offset: float = clampf(safe_offset + sample_distance, 0.0, length)
    if is_equal_approx(previous_offset, next_offset):
        previous_offset = 0.0
        next_offset = length
    var forward: Vector3 = curve.sample_baked(next_offset, true) - curve.sample_baked(previous_offset, true)
    if forward.length_squared() <= 0.000001:
        forward = Vector3.FORWARD
    else:
        forward = forward.normalized()

    var reference_up: Vector3 = Vector3.UP
    if absf(forward.dot(reference_up)) > 0.999:
        reference_up = Vector3.RIGHT

    var z_axis: Vector3 = -forward
    var x_axis: Vector3 = reference_up.cross(z_axis).normalized()
    var y_axis: Vector3 = z_axis.cross(x_axis).normalized()
    var roll: float = curve_data.roll1 if length <= 0.0 else lerpf(curve_data.roll1, curve_data.roll2, clampf(safe_offset / length, 0.0, 1.0))
    var track_transform: Transform3D = Transform3D(
        Basis(x_axis, y_axis, z_axis).orthonormalized().rotated(forward, deg_to_rad(roll)).orthonormalized(),
        origin
    )
    if state.track_direction == TrackManager.Direction.DIRECTION_REVERSED:
        track_transform.basis = track_transform.basis.rotated(track_transform.basis.y.normalized(), PI).orthonormalized()
    track_transform.origin.y += TrackManager.RAIL_HEIGHT
    return track_transform


func controller_get_transform(controller_rid: RID) -> Transform3D:
    var vehicle_rid: RID = _controller_vehicles.get(controller_rid, RID())
    if not vehicle_rid.is_valid():
        return Transform3D.IDENTITY
    return vehicle_get_transform(vehicle_rid)


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
            var common_endpoints: Array[TrackManager.EndpointIndex] = TrackManager.switch_get_common_endpoints(raw_connection.track_rid)
            if common_endpoints.has(raw_connection.endpoint_index):
                # Entering from the common point follows whichever branch is active;
                # remap the shared endpoint to the active branch endpoint that
                # represents the same physical point.
                var active_track: TrackManager.SwitchTrack = TrackManager.switch_get_active_track(raw_connection.track_rid)
                var branch_start_endpoint: int = TrackManager.switch_get_branch_start_endpoint(raw_connection.track_rid, active_track)
                candidate_connection.endpoint_index = branch_start_endpoint if common_endpoints.has(branch_start_endpoint) else TrackManager.switch_get_branch_end_endpoint(raw_connection.track_rid, active_track)
            else:
                # Entering from a branch side physically selects that branch, even
                # when the switch is currently set to another route. The actual
                # switch state is forced only after the connection is proven unique.
                candidate_forced_switch_track = TrackManager.switch_get_endpoint_branch(
                    raw_connection.track_rid,
                    raw_connection.endpoint_index
                )
                has_candidate_forced_switch_track = true

        # Discard endpoints that cannot be used for motion on the selected route.
        # Branch-side switch entry is allowed because it will force that branch.
        var is_motion_accessible: bool = false
        if TrackManager.track_is_switch(candidate_connection.track_rid):
            var active_track: TrackManager.SwitchTrack = TrackManager.switch_get_active_track(candidate_connection.track_rid)
            is_motion_accessible = candidate_connection.endpoint_index == TrackManager.switch_get_branch_start_endpoint(candidate_connection.track_rid, active_track) \
                or candidate_connection.endpoint_index == TrackManager.switch_get_branch_end_endpoint(candidate_connection.track_rid, active_track)
        else:
            is_motion_accessible = candidate_connection.endpoint_index == TrackManager.EndpointIndex.CURVE1_P1 \
                or candidate_connection.endpoint_index == TrackManager.EndpointIndex.CURVE1_P2
        if not is_motion_accessible and not has_candidate_forced_switch_track:
            continue

        if not unique_connection:
            unique_connection = candidate_connection
            unique_forced_switch_track = candidate_forced_switch_track
            has_unique_forced_switch_track = has_candidate_forced_switch_track
            continue

        # More than one different usable target means the node is ambiguous.
        # Identical candidates can happen at switch common points and still count
        # as one route.
        var is_same_candidate: bool = unique_connection.track_rid == candidate_connection.track_rid \
            and unique_connection.endpoint_index == candidate_connection.endpoint_index \
            and has_unique_forced_switch_track == has_candidate_forced_switch_track \
            and (
                not has_unique_forced_switch_track
                or unique_forced_switch_track == candidate_forced_switch_track
            )
        if not is_same_candidate:
            return null

    if not unique_connection:
        return null
    if has_unique_forced_switch_track:
        # Force branch-side switch entry only after ambiguity checks, so an
        # ambiguous topology node cannot change switch state as a side effect.
        if not TrackManager.switch_get_active_track(unique_connection.track_rid) == unique_forced_switch_track:
            TrackManager.switch_set_active_track(unique_connection.track_rid, unique_forced_switch_track)
    return unique_connection
