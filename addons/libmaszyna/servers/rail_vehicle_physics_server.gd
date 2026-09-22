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
    ## moved since the mover location was last updated
    var moved: bool = true
    ## per end: the neighbour was already reported as none, so reporting it again says nothing
    var neighbour_cleared: Array[bool] = [false, false]
    ## whether track_rid is a switch - part of describing the occupied track, and a track never
    ## turns into one, so it is asked when the vehicle changes track rather than on every step
    var track_is_switch: bool = false


## Original engine: primary physics update rate and the iteration limit per frame (drivermode.cpp:186-206)
const PHYSICS_STEP: float = 0.01
const MAX_PHYSICS_ITERATIONS: int = 20
## Reports physics inconsistencies with push_error (see _check_movement, _check_velocity_jumps)
const DIAGNOSTICS_SETTING: StringName = &"maszyna/debug/physics_diagnostics"
## A vehicle moved along the track by more or less than requested (m)
const DIAGNOSTICS_MOVE_TOLERANCE: float = 0.001
## Velocity change of a vehicle within one physics frame reported as a kick (m/s^2)
const DIAGNOSTICS_MAX_ACCELERATION: float = 3.0
## Distance the neighbour scan steps past a track endpoint to enter the connected track
const _SCAN_ENDPOINT_EPSILON: float = 0.001

var _controllers: Dictionary[RID, ControllerState] = {}
var _controller_vehicles: Dictionary[RID, RID] = {}
var _vehicles: Dictionary[RID, VehicleState] = {}
var _next_controller_id: int = 0
var _next_vehicle_id: int = 0
var _diagnostics: bool = false
var _diagnostics_velocity: Dictionary[TrainController, float] = {}


func _enter_tree() -> void:
    Engine.register_singleton("RailVehiclePhysicsServer", self)
    # the step and RailVehicle3D's visual transform now both run on the frame, so the step has to
    # come first - otherwise the vehicles render the position of the previous frame
    process_priority = -100


func _exit_tree() -> void:
    Engine.unregister_singleton("RailVehiclePhysicsServer")


## Global step of every registered controller, the same phases as the original vehicle_table::update()
## (DynObj.cpp:8181): locations and neighbours once per frame, then forces for all vehicles before
## movement of all vehicles in each iteration, so coupled vehicles see each other's state consistently.
##
## Runs on the rendered frame, not on Godot's fixed physics tick, exactly like the original: the
## engine calls vehicle_table::update(Deltatime, Iterationcount) once per frame and lets the
## iteration count absorb a long frame. On the fixed tick the same step ran several times per frame
## to catch up (multiplying the whole simulation by up to max_physics_steps_per_frame), and the
## vehicle transform - which RailVehicle3D updates per rendered frame - repeated a stale position
## whenever the frame rate and the tick rate disagreed, which is what made the vehicles judder.
func _process(delta: float) -> void:
    if Engine.is_editor_hint():
        return
    var controllers: Array[TrainController] = []
    for controller_rid: RID in _controllers:
        var controller: TrainController = _get_controller(controller_rid)
        if controller:
            controllers.append(controller)
    if not controllers:
        return
    _diagnostics = bool(ProjectSettings.get_setting(DIAGNOSTICS_SETTING, false))

    var track_vehicles: Dictionary[RID, Array] = {}
    for controller: TrainController in controllers:
        var vehicle_rid: RID = _controller_vehicles.get(controller.get_rid(), RID())
        var state: VehicleState = _vehicles.get(vehicle_rid)
        # a vehicle that has not moved keeps its location - sampling the track is not needed
        if not state or state.moved:
            controller.update_location()
            controller._emit_position_changed_if_needed()
        if state:
            state.moved = false
            if not track_vehicles.has(state.track_rid):
                track_vehicles[state.track_rid] = []
            track_vehicles[state.track_rid].append(vehicle_rid)
    # once per update(), like the original (DynObj.cpp:8186-8193)
    for controller: TrainController in controllers:
        _update_neighbours(controller, track_vehicles)

    var iterations: int = clampi(ceili(delta / PHYSICS_STEP), 1, MAX_PHYSICS_ITERATIONS)
    var step: float = delta / iterations
    for iteration: int in iterations:
        # Forces of all, then movement of all, the original's phase order (DynObj.cpp:8199-8205),
        # and the cheap FastUpdate in every sub-iteration but the last (DynObj.cpp:4086). The whole
        # per-controller loop runs inside TrainSystem.step_vehicles(): done from here it was four
        # calls across the binding per controller per iteration, thousands of Variant marshallings
        # per frame for arithmetic the original does in a plain C++ loop.
        var distances: PackedFloat64Array = TrainSystem.step_vehicles(
                controllers, step, iteration == iterations - 1)
        for index: int in controllers.size():
            if is_zero_approx(distances[index]):
                continue
            var vehicle_rid: RID = _controller_vehicles.get(controllers[index].get_rid(), RID())
            if vehicle_rid.is_valid():
                _apply_movement(vehicle_rid, distances[index])
    for controller: TrainController in controllers:
        if controller.is_physics_active():
            controller.update_state()
    if _diagnostics:
        _check_velocity_jumps(controllers, delta)


## Diagnostics: a velocity jump within one frame is a kick - with a consistent track movement it
## comes from the forces, typically a coupler reacting to an inconsistent vehicle position.
func _check_velocity_jumps(controllers: Array[TrainController], delta: float) -> void:
    for controller: TrainController in controllers:
        var velocity: float = controller.get_velocity()
        var acceleration: float = (velocity - _diagnostics_velocity.get(controller, velocity)) / delta
        _diagnostics_velocity[controller] = velocity
        if absf(acceleration) > DIAGNOSTICS_MAX_ACCELERATION:
            push_error("RailVehiclePhysicsServer: %s kicked, dV/dt=%.2f m/s^2 at V=%.2f m/s, Ft=%.0f N" % [
                controller.train_id, acceleration, velocity, float(controller.get_state().get("Ft", 0.0))])


## Original engine: TDynamicObject::update_neighbours() (DynObj.cpp:7135) - a coupled end keeps its
## coupled vehicle (resolved by the controller), a free end looks for the nearest vehicle on the route.
func _clear_neighbour(controller: TrainController, state: VehicleState, end: int) -> void:
    if state and state.neighbour_cleared[end]:
        return
    if state:
        state.neighbour_cleared[end] = true
    controller.update_neighbour(end, null, -1, 0.0)


func _update_neighbours(controller: TrainController, track_vehicles: Dictionary[RID, Array]) -> void:
    var vehicle_rid: RID = _controller_vehicles.get(controller.get_rid(), RID())
    var state: VehicleState = _vehicles.get(vehicle_rid)
    # straight from the mover: asking for the whole state here rebuilt it for every
    # controller of every frame, and this is usually the frame's first read
    var velocity: float = controller.get_velocity()
    # 10m ~= 140 km/h at 4 fps + safety margin (DynObj.cpp:7160)
    var scan_range: float = maxf(10.0, absf(velocity)) + 40.0
    # the track does not change between the two ends, so it is asked about once
    var on_track: bool = state and TrackManager.track_exists(state.track_rid)
    for end: int in 2:
        # a coupled end is not cleared by this call: the original recomputes its coupler distance
        # on every update (DynObj.cpp:7144-7154, called from vehicle_table::update(),
        # DynObj.cpp:8193) and CouplerForce() starts from it on every step (Mover.cpp:4781) -
        # skip it and the couplers stretch without any force building up
        if controller.is_coupled(end):
            if state:
                state.neighbour_cleared[end] = false
            controller.update_neighbour(end, null, -1, 0.0)
            continue
        if not on_track:
            _clear_neighbour(controller, state, end)
            continue
        var found: Array = _find_vehicle(vehicle_rid, state, end, scan_range, track_vehicles)
        if not found:
            _clear_neighbour(controller, state, end)
            continue
        state.neighbour_cleared[end] = false
        var other_state: VehicleState = _vehicles[found[0]]
        controller.update_neighbour(end, _get_controller(other_state.controller_rid), found[1], found[2])


## Original engine: TDynamicObject::find_vehicle() (DynObj.cpp:7183) - scans the route from the vehicle
## center towards the given end (0 front, 1 rear). Returns [vehicle_rid, facing end of the found
## vehicle, center to center distance along the route] or an empty array.
func _find_vehicle(
    vehicle_rid: RID,
    state: VehicleState,
    end: int,
    scan_range: float,
    track_vehicles: Dictionary[RID, Array]
) -> Array:
    # server distances are rear-relative, see process_movement()
    var request_sign: float = -1.0 if end == 0 else 1.0
    var cursor: VehicleState = VehicleState.new()
    cursor.track_rid = state.track_rid
    cursor.track_is_switch = state.track_is_switch
    cursor.track_offset = state.track_offset
    cursor.track_direction = state.track_direction
    cursor.switch_track = state.switch_track
    var scanned: float = 0.0
    var min_along: float = 0.0

    while scanned < scan_range:
        # same conversion to the curve offset direction as in _move_vehicle_state()
        var movement_sign: float = (
            -1.0 if cursor.track_direction == TrackManager.Direction.DIRECTION_NORMAL else 1.0
        ) * request_sign
        var found_rid: RID = RID()
        var found_along: float = INF
        for other_rid: RID in track_vehicles.get(cursor.track_rid, []):
            var other: VehicleState = _vehicles[other_rid]
            if other_rid == vehicle_rid or not other.switch_track == cursor.switch_track:
                continue
            var along: float = (other.track_offset - cursor.track_offset) * movement_sign
            if along > min_along and along < found_along:
                found_rid = other_rid
                found_along = along
        if found_rid.is_valid():
            var found_state: VehicleState = _vehicles[found_rid]
            var found_front_sign: float = 1.0 if found_state.track_direction == TrackManager.Direction.DIRECTION_NORMAL else -1.0
            var found_end: int = 0 if is_equal_approx(found_front_sign, -movement_sign) else 1
            return [found_rid, found_end, scanned + found_along]

        var length: float = TrackManager.track_get_length(cursor.track_rid, cursor.switch_track)
        var distance_to_endpoint: float = length - cursor.track_offset if movement_sign > 0.0 else cursor.track_offset
        var previous_track_rid: RID = cursor.track_rid
        _move_vehicle_state(cursor, request_sign * (distance_to_endpoint + _SCAN_ENDPOINT_EPSILON), false)
        if cursor.track_rid == previous_track_rid:
            return []
        scanned += distance_to_endpoint + _SCAN_ENDPOINT_EPSILON
        # a vehicle standing right at the entry point is still ahead
        min_along = -_SCAN_ENDPOINT_EPSILON
    return []


func _get_controller(controller_rid: RID) -> TrainController:
    var controller_state: ControllerState = _controllers.get(controller_rid)
    if not controller_state or controller_state.object_id == 0:
        return null
    return instance_from_id(controller_state.object_id) as TrainController


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
        var controller_state: ControllerState = _controllers.get(controller_rid)
        var controller: TrainController = instance_from_id(controller_state.object_id) as TrainController
        if controller:
            controller._emit_position_changed_if_needed()


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
    state.moved = true
    state.track_is_switch = TrackManager.track_is_switch(track_rid)
    state.switch_track = TrackManager.switch_get_active_track(track_rid) if state.track_is_switch else TrackManager.SwitchTrack.TRACK_COMMON
    state.track_offset = clampf(track_offset, 0.0, TrackManager.track_get_length(track_rid, state.switch_track))
    var remaining_offset:float = track_offset - state.track_offset
    var direction_sign:float = -1.0 if track_direction == TrackManager.Direction.DIRECTION_NORMAL else 1.0
    _move_vehicle_state(state, remaining_offset * direction_sign, false)


## [param controller] is what the step loop already holds - resolving it back from the vehicle RID
## costs two dictionary lookups and an instance_from_id() on every vehicle of every iteration, and
## an EZT/DMU is never deactivated by the physics (Mover.cpp:4489), so a scenery full of EMUs pays
## it hundreds of times per frame while standing still.
func process_movement(vehicle_rid: RID, delta: float, controller: TrainController = null) -> void:
    var state: VehicleState = _vehicles.get(vehicle_rid)
    if not state:
        return
    if not state.track_rid == TrackManager.UNDEFINED_TRACK and not TrackManager.track_exists(state.track_rid):
        return
    if not controller:
        if not state.controller_rid.is_valid():
            return
        var controller_state: ControllerState = _controllers.get(state.controller_rid)
        if not controller_state or controller_state.object_id == 0:
            return
        controller = instance_from_id(controller_state.object_id) as TrainController
        if not controller:
            return
    # TrainController.process_movement() is front-relative (mirrors mover->V);
    # this server's track-offset math is rear-relative - negate at the boundary.
    var distance: float = -controller.process_movement(delta)
    # the track was checked above and Mover physics cannot remove one; a track that disappears
    # anyway is caught by _move_vehicle_state(), whose track_get_length() returns 0 for it
    if is_zero_approx(distance):
        return
    # the position_changed signal follows once per physics step, with the location update
    _move_vehicle_state(state, distance, true)


## Walks a vehicle the distance its mover asked for, the part of process_movement() the step loop
## needs once it has the distances from TrainSystem.step_vehicles()
func _apply_movement(vehicle_rid: RID, distance: float) -> void:
    var state: VehicleState = _vehicles.get(vehicle_rid)
    if not state:
        return
    if not state.track_rid == TrackManager.UNDEFINED_TRACK and not TrackManager.track_exists(state.track_rid):
        return
    _move_vehicle_state(state, distance, true)


func vehicle_move(vehicle_rid: RID, distance: float) -> void:
    var state: VehicleState = _vehicles.get(vehicle_rid)
    if not state or not TrackManager.track_exists(state.track_rid):
        return
    _move_vehicle_state(state, distance, true)
    if state.controller_rid.is_valid():
        var controller_state: ControllerState = _controllers.get(state.controller_rid)
        var controller: TrainController = instance_from_id(controller_state.object_id) as TrainController
        if controller:
            controller._emit_position_changed_if_needed()


func _move_vehicle_state(state: VehicleState, distance: float, force_switch_state: bool) -> void:
    if is_zero_approx(distance):
        return
    state.moved = true

    var current_track_rid: RID = state.track_rid
    var current_switch_track: TrackManager.SwitchTrack = state.switch_track
    # Length of the occupied branch, kept across the loop: every query here crosses the autoload
    # boundary and this one used to be made twice for the same track on every step.
    var current_is_switch: bool = state.track_is_switch
    var current_length: float = TrackManager.track_get_length(current_track_rid, current_switch_track)
    var current_track_offset: float = clampf(state.track_offset, 0.0, current_length)
    var current_track_direction: TrackManager.Direction = state.track_direction
    var remaining: float = absf(distance)
    var request_sign: float = -1.0 if distance < 0.0 else 1.0
    var start_point: Vector3 = (
        TrackManager.track_get_domain_curve(current_track_rid, current_switch_track).sample_baked(current_track_offset, false)
        if _diagnostics and force_switch_state else Vector3.ZERO
    )
    # Convert movement relative to the vehicle front into curve offset movement.
    # Positive sign moves toward the branch end, negative toward the branch start.
    var movement_sign: float = (
        -1.0 if current_track_direction == TrackManager.Direction.DIRECTION_NORMAL else 1.0
    ) * request_sign

    while remaining > 0.0001:
        # track_get_length() returns 0 for a track that is gone, so this covers track_exists() too
        if current_length <= 0.0:
            break

        # Find the endpoint this movement heads toward on the occupied branch.
        var distance_to_endpoint: float = current_length - current_track_offset if movement_sign > 0.0 else current_track_offset
        var endpoint_index: int
        if current_is_switch:
            endpoint_index = TrackManager.switch_get_branch_end_endpoint(current_track_rid, current_switch_track) \
                if movement_sign > 0.0 else TrackManager.switch_get_branch_start_endpoint(current_track_rid, current_switch_track)
            # If the vehicle is reversing through a switch blade on a non-active
            # branch, keep the branch it already occupies and force the switch back.
            var active_track: TrackManager.SwitchTrack = TrackManager.switch_get_active_track(current_track_rid)
            if not active_track == current_switch_track:
                var requested_distance: float = minf(remaining, distance_to_endpoint)
                var next_offset_on_track: float = current_track_offset + movement_sign * requested_distance
                var blade_boundary_offset: float = TrackManager.switch_get_blade_boundary_offset(current_track_rid, current_switch_track)
                if (
                    force_switch_state
                    and current_track_offset > blade_boundary_offset
                    and next_offset_on_track <= blade_boundary_offset
                ):
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
        var connection: TrackManager.EndpointRef = _get_motion_connection(
            current_track_rid,
            endpoint_index,
            force_switch_state,
        )
        if not connection:
            break

        current_track_rid = connection.track_rid
        current_is_switch = TrackManager.track_is_switch(current_track_rid)
        # The connection endpoint is where we enter the next track. For switches,
        # that endpoint also tells which branch the vehicle now occupies.
        if current_is_switch:
            current_switch_track = TrackManager.switch_get_endpoint_branch(current_track_rid, connection.endpoint_index)
            var entered_at_end: bool = connection.endpoint_index == TrackManager.switch_get_branch_end_endpoint(current_track_rid, current_switch_track)
            movement_sign = -1.0 if entered_at_end else 1.0
        else:
            current_switch_track = TrackManager.SwitchTrack.TRACK_COMMON
            movement_sign = -1.0 if connection.endpoint_index == TrackManager.EndpointIndex.CURVE1_P2 else 1.0
        # Entering at branch start means offset grows; entering at branch end
        # means offset decreases from the branch length.
        current_length = TrackManager.track_get_length(current_track_rid, current_switch_track)
        current_track_offset = 0.0 if movement_sign > 0.0 else current_length
        current_track_direction = (
            TrackManager.Direction.DIRECTION_NORMAL
            if movement_sign * request_sign < 0.0
            else TrackManager.Direction.DIRECTION_REVERSED
        )

    state.track_rid = current_track_rid
    state.track_is_switch = current_is_switch
    state.track_offset = current_track_offset
    state.track_direction = current_track_direction
    state.switch_track = current_switch_track
    if _diagnostics and force_switch_state:
        _check_movement(state, start_point, absf(distance) - remaining)


## Diagnostics: the vehicle must move in the world by the distance it moved along the track (a step
## is far shorter than any curve radius, so the chord equals the arc) - a mismatch shifts the mover
## location and kicks the coupled vehicles.
func _check_movement(state: VehicleState, start_point: Vector3, moved: float) -> void:
    var end_point: Vector3 = TrackManager.track_get_domain_curve(state.track_rid, state.switch_track).sample_baked(state.track_offset, false)
    var world_moved: float = start_point.distance_to(end_point)
    if absf(world_moved - moved) > DIAGNOSTICS_MOVE_TOLERANCE:
        push_error("RailVehiclePhysicsServer: moved %.4f m in the world instead of %.4f m on track %s (offset %.3f)" % [
            world_moved, moved, TrackManager.track_get_name(state.track_rid), state.track_offset])


func vehicle_get_transform(vehicle_rid: RID) -> Transform3D:
    var state: VehicleState = _vehicles.get(vehicle_rid)
    if not state or not TrackManager.track_exists(state.track_rid):
        return Transform3D.IDENTITY
    return _get_vehicle_transform(state)


func vehicle_get_transform_at_distance(vehicle_rid: RID, distance: float) -> Transform3D:
    var state: VehicleState = _vehicles.get(vehicle_rid)
    if not state or not TrackManager.track_exists(state.track_rid):
        return Transform3D.IDENTITY
    return _get_vehicle_transform(_sample_state(state, distance))


func _get_vehicle_transform(state: VehicleState) -> Transform3D:
    var curve_data: MaszynaTrackCurve = TrackManager.track_get_curve(state.track_rid, state.switch_track)
    var curve: Curve3D = TrackManager.track_get_domain_curve(state.track_rid, state.switch_track)
    if not curve_data or not curve:
        return Transform3D.IDENTITY

    var length: float = curve.get_baked_length()
    var safe_offset: float = clampf(state.track_offset, 0.0, length)
    # linear on purpose: the cubic interpolation has no neighbour point at the curve ends, so the position
    # advanced there only about 60% of the offset - every vehicle lost centimetres at each track joint,
    # which kicked the consist through its couplers
    var origin: Vector3 = curve.sample_baked(safe_offset, false)
    var sample_distance: float = minf(0.1, length)
    var previous_offset: float = clampf(safe_offset - sample_distance, 0.0, length)
    var next_offset: float = clampf(safe_offset + sample_distance, 0.0, length)
    if is_equal_approx(previous_offset, next_offset):
        previous_offset = 0.0
        next_offset = length
    var forward: Vector3 = curve.sample_baked(next_offset, false) - curve.sample_baked(previous_offset, false)
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


## Track under the vehicle and its centre along that track, measured towards the vehicle front.
func controller_get_track_position(controller_rid: RID) -> Dictionary:
    var state: VehicleState = _vehicles.get(_controller_vehicles.get(controller_rid, RID()))
    if not state or not TrackManager.track_exists(state.track_rid):
        return {"track_rid": TrackManager.UNDEFINED_TRACK, "along": 0.0}
    return {
        "track_rid": state.track_rid,
        # moving forward decreases the offset on a track run in its normal direction
        "along": -state.track_offset if state.track_direction == TrackManager.Direction.DIRECTION_NORMAL else state.track_offset,
    }


## Running shape of the bogies (DynObj.cpp:2950-2970): the curve radius from the yaw difference of
## the bogie pivots (0.0 on a straight or above 15 km) and the mean cant of both bogies in radians.
## Samples the track twice - call it only when the radius is needed.
func controller_get_curve(controller_rid: RID, bogie_pivot_spacing: float) -> Dictionary:
    var state: VehicleState = _vehicles.get(_controller_vehicles.get(controller_rid, RID()))
    if not state or not TrackManager.track_exists(state.track_rid):
        return {"radius": 0.0, "cant": 0.0}
    var front: VehicleState = _sample_state(state, 0.5 * bogie_pivot_spacing)
    var rear: VehicleState = _sample_state(state, -0.5 * bogie_pivot_spacing)
    var front_forward: Vector3 = -_get_vehicle_transform(front).basis.z
    var rear_forward: Vector3 = -_get_vehicle_transform(rear).basis.z
    var yaw_difference: float = atan2(front_forward.x, front_forward.z) - atan2(rear_forward.x, rear_forward.z)
    yaw_difference = wrapf(yaw_difference, -PI, PI)
    var radius: float = 0.0
    if not is_zero_approx(sin(yaw_difference * 0.5)):
        radius = -0.5 * bogie_pivot_spacing / sin(yaw_difference * 0.5)
    if absf(radius) > 15000.0:
        radius = 0.0
    return {"radius": radius, "cant": deg_to_rad(0.5 * (_get_roll(front) + _get_roll(rear)))}


func _sample_state(state: VehicleState, distance: float) -> VehicleState:
    var sampled_state: VehicleState = VehicleState.new()
    sampled_state.track_rid = state.track_rid
    sampled_state.track_is_switch = state.track_is_switch
    sampled_state.track_offset = state.track_offset
    sampled_state.track_direction = state.track_direction
    sampled_state.switch_track = state.switch_track
    _move_vehicle_state(sampled_state, distance, false)
    return sampled_state


func _get_roll(state: VehicleState) -> float:
    var curve_data: MaszynaTrackCurve = TrackManager.track_get_curve(state.track_rid, state.switch_track)
    var length: float = TrackManager.track_get_length(state.track_rid, state.switch_track)
    if not curve_data or length <= 0.0:
        return 0.0
    return lerpf(curve_data.roll1, curve_data.roll2, clampf(state.track_offset / length, 0.0, 1.0))


func _get_motion_connection(
    track_rid: RID,
    endpoint_index: int,
    force_switch_state: bool = true,
) -> TrackManager.EndpointRef:
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
    if has_unique_forced_switch_track and force_switch_state:
        # Force branch-side switch entry only after ambiguity checks, so an
        # ambiguous topology node cannot change switch state as a side effect.
        if not TrackManager.switch_get_active_track(unique_connection.track_rid) == unique_forced_switch_track:
            TrackManager.switch_set_active_track(unique_connection.track_rid, unique_forced_switch_track)
    return unique_connection
