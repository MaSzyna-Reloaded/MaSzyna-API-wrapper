@tool
extends Node3D
class_name RailVehicle3D

# FIXME: Head Display implementation is experimental and only for demo purposes

@export_node_path("TrainController") var controller_path:NodePath = NodePath(""):
    set(x):
        if not x == controller_path:
            controller_path = x
            _controller = null
            _dirty = true

@export_node_path("Path3D") var track_path:NodePath = NodePath(""):
    set(x):
        if not x == track_path:
            # OPTIMIZATION: If we already have a track node, don't reset if it's the same
            if is_inside_tree() and _track:
                var new_node = get_node_or_null(x)
                if new_node == _track:
                    track_path = x
                    return
            track_path = x
            _track = null
            _dirty = true

@export var vertical_offset: float = 0.0:
    set(x):
        vertical_offset = x
        _dirty = true

@export var distance_on_track:float = 0.0:
    set(x):
        distance_on_track = x
        if Engine.is_editor_hint():
            _dirty = true
@export var track_reverse:bool = false:
    set(x):
        track_reverse = x
        _dirty = true

@export var bogie_paths: Array[NodePath] = []:
    set(x):
        bogie_paths = x
        _bogie_nodes.clear()
        _dirty = true

@export var car_paths: Array[NodePath] = []:
    set(x):
        car_paths = x
        _car_nodes.clear()
        _dirty = true

@export var wheel_paths: Array[NodePath] = []:
    set(x):
        wheel_paths = x
        _wheel_nodes.clear()
        _dirty = true

@export var cabin_scene:PackedScene
@export var cabin_rotate_180deg:bool = false
@export_node_path("E3DModelInstance") var low_poly_cabin_path:NodePath = NodePath("")

@export_node_path("E3DModelInstance") var head_display_e3d_path:NodePath = NodePath(""):
    set(x):
        if not x == head_display_e3d_path:
            head_display_e3d_path = x
            _head_display_e3d = null
            _dirty = true

@export var head_display_material:Material:
    set(x):
        if not x == head_display_material:
            head_display_material = x
            _needs_head_display_update = true

@export_node_path("MeshInstance3D") var head_display_node_path = NodePath(""):
    set(x):
        if not x == head_display_node_path:
            head_display_node_path = x
            _needs_head_display_update = true

var _dirty:bool = true
var _needs_head_display_update: bool = false
var _head_display_e3d:E3DModelInstance
var _cabin:Cabin3D
var _cabin_unit:Node3D
var _camera:FreeCamera3D
var _controller:TrainController
var _track:Path3D
var _t:float = 0.0
var _car_nodes: Array[Node3D] = []
var _bogie_nodes: Array[Node3D] = []
var _wheel_nodes: Array[Node3D] = []
var _wheel_angles: Array[float] = [] # Current rotation angle for each wheel
var _rest_transforms: Dictionary = {} # Stores Node -> Transform3D relative to RailVehicle3D root
var _local_rest_bases: Dictionary = {} # Stores Node -> Basis (local)
var _instance_pivots_cache: Dictionary = {} # E3DModelInstance -> Array (pivots)
var _instance_wheels_cache: Dictionary = {} # E3DModelInstance -> Array (wheels)
var _physics_body: RID
var _physics_shape: RID
@export var physics_box_size: Vector3 = Vector3(3.0, 4.0, 15.0):
    set(x):
        physics_box_size = x
        if _is_physics_registered and _physics_shape.is_valid():
            PhysicsServer3D.shape_set_data(_physics_shape, physics_box_size * 0.5)

var _is_physics_registered: bool = false
var _is_snapping: bool = false


func _notification(what):
    if what == NOTIFICATION_TRANSFORM_CHANGED:
        if _is_snapping: return
        if Engine.is_editor_hint() and is_inside_tree() and _track and _track.curve:
            var local_pos = _track.to_local(global_position)
            var offset = _track.curve.get_closest_offset(local_pos)
            
            _is_snapping = true
            # Silent update to distance_on_track to avoid recursive calls
            distance_on_track = offset
            global_transform = _get_track_transform_at_z_y_only(0.0)
            _is_snapping = false


# Calculate Transform3D relative to RailVehicle3D root using ONLY local transforms
# to ensure we capture the "design-time" state regardless of current global position.
func _get_relative_transform(node: Node3D) -> Transform3D:
    var t = node.transform
    var p = node.get_parent()
    while p != null and p != self:
        if p is Node3D:
            t = p.transform * t
        p = p.get_parent()
    return t


func enter_cabin(player:MaszynaPlayer):
    _camera = player.get_camera()
    _cabin = cabin_scene.instantiate() as Cabin3D
    if not _cabin:
        push_error("Root node of cabin scene must be a Cabin3D")
        return

    if controller_path:
        var controller = get_node(controller_path)
        if controller:
            _cabin.controller_path = controller.get_path()

    _cabin.visible = false

    var _jump_into_cabin = func():
        if low_poly_cabin_path:
            var low_poly_cabin = get_node_or_null(low_poly_cabin_path)
            if low_poly_cabin:
                low_poly_cabin.visible = false

        var cabin_enter_camera_transform = _cabin.get_camera_transform()
        player.remove_child(_camera)
        _cabin.add_child(_camera)
        _camera.bound_enabled = _cabin.camera_bound_enabled
        _camera.bound_min = _cabin.camera_bound_min
        _camera.bound_max = _cabin.camera_bound_max
        _camera.bound_min.y += 0.5
        _camera.bound_max.y += 1.8

        if cabin_enter_camera_transform:
            _camera.global_transform = cabin_enter_camera_transform
        else:
            _camera.position = Vector3(0, 3, 0)

        _camera.velocity_multiplier = 0.2

    _cabin.cabin_ready.connect(_jump_into_cabin)
    
    # Position cabin on the correct model unit
    _cabin_unit = null
    if low_poly_cabin_path:
        _cabin_unit = get_node_or_null(low_poly_cabin_path)
    
    if not _cabin_unit:
        for child in get_children(true):
            if child is E3DModelInstance and child != _cabin:
                _cabin_unit = child
                break
                
    add_child(_cabin)
    if _cabin_unit:
        _cabin.global_transform = _cabin_unit.global_transform
    else:
        _cabin.global_transform = self.global_transform

    await get_tree().process_frame
    await get_tree().process_frame
    if _cabin:
        _cabin.visible = true


func leave_cabin(player:Node):
    if low_poly_cabin_path:
        var low_poly_cabin = get_node(low_poly_cabin_path)
        if low_poly_cabin:
            low_poly_cabin.visible = true
    var cam_trn = _camera.global_transform
    _cabin.remove_child(_camera)
    player.add_child(_camera)
    _camera.bound_enabled = false
    _camera.global_transform = cam_trn
    _camera.global_transform.origin = self.global_transform.origin + Vector3(5, 0, 0)
    _camera.global_transform.origin.y += 1.75
    _camera.look_at(self.global_position+Vector3(0, 1.75, -5))
    _camera.velocity_multiplier = 1.0
    _cabin.get_parent().remove_child(_cabin)
    _cabin.queue_free()
    _cabin = null
    _cabin_unit = null


func _on_e3d_loaded():
    _rest_transforms.clear()
    _local_rest_bases.clear()
    _wheel_nodes.clear()
    _wheel_angles.clear()
    _instance_pivots_cache.clear()
    _instance_wheels_cache.clear()
    _dirty = true
    _needs_head_display_update = true


func get_controller() -> TrainController:
    if controller_path:
        return get_node(controller_path)
    else:
        return null


func _get_next_connected_track(track: Node3D, forward: bool) -> Node3D:
    if not track: return null
    var prop = "next_track" if forward else "previous_track"
    var path = track.get(prop) if track.has_method("get") else null
    if path == null or path.is_empty(): return null
    var next = track.get_node_or_null(path)
    if not next: return null
    if next is Path3D: # MaszynaTrack3D is a Path3D
        return next
    if next.has_method("resolve_track"): # SwitchNode
        return next.resolve_track(track)
    return null


func _update_head_display():
    if is_inside_tree():
        if head_display_node_path:
            var node:MeshInstance3D = get_node_or_null(head_display_node_path)
            if node:
                node.material_override = head_display_material
                _needs_head_display_update = false
            else:
                _needs_head_display_update = false
        else:
            _needs_head_display_update = false


func _process(delta):
    if _dirty:
        _dirty = false
        if head_display_e3d_path:
            _head_display_e3d = get_node_or_null(head_display_e3d_path)
            if _head_display_e3d:
                if not _head_display_e3d.e3d_loaded.is_connected(_on_e3d_loaded):
                    _head_display_e3d.e3d_loaded.connect(_on_e3d_loaded)

        if controller_path and is_inside_tree():
            _controller = get_node_or_null(controller_path)
        
        if track_path and is_inside_tree():
            _track = get_node_or_null(track_path)
            if _track and _track.curve:
                # 1. Update list of nodes to be controlled
                _car_nodes.clear()
                for path in car_paths:
                    var node = get_node_or_null(path)
                    if node is Node3D: _car_nodes.append(node)
                
                _bogie_nodes.clear()
                for path in bogie_paths:
                    var node = get_node_or_null(path)
                    if node is Node3D: _bogie_nodes.append(node)
                
                _wheel_nodes.clear()
                _wheel_angles.clear()
                for path in wheel_paths:
                    var node = get_node_or_null(path)
                    if node is Node3D:
                        _wheel_nodes.append(node)
                        _wheel_angles.append(0.0)
                
                # Auto-detect bogies from ALL children (including internal E3D ones)
                for child in get_children(true):
                    if child.has_method("get_bogie_nodes"):
                        if not child.e3d_loaded.is_connected(_on_e3d_loaded):
                            child.e3d_loaded.connect(_on_e3d_loaded)
                        var b_nodes = child.get("bogie_nodes")
                        for bn in b_nodes:
                            if bn is Node3D and not _bogie_nodes.has(bn):
                                _bogie_nodes.append(bn)
                
                # Robust recursive wheel detection over the whole vehicle subtree
                _detect_wheels_recursive(self)
                
                # 2. Capture rest transforms if missing (ensure we have design-time state)
                # MOVED TO _process logic to ensure we always have them when needed
                
                # 3. Snap parent to track
                if Engine.is_editor_hint():
                    _notification(NOTIFICATION_TRANSFORM_CHANGED)

    _t += delta
    if _t > 0.25 and _needs_head_display_update:
        _t = 0.0
        _update_head_display()

    if not Engine.is_editor_hint():
        if _controller:
            var velocity = _controller.state.get("velocity", 0.0)
            if _track and _track.curve:
                if track_reverse:
                    distance_on_track -= velocity * delta
                else:
                    distance_on_track += velocity * delta
                
                # SEAMLESS TRANSITION
                var max_transitions = 5
                var transitioned = false
                while max_transitions > 0:
                    var track_length = _track.curve.get_baked_length()
                    if distance_on_track > track_length:
                        var next = _get_next_connected_track(_track, true)
                        if next:
                            distance_on_track -= track_length
                            _track = next
                            transitioned = true
                            max_transitions -= 1
                            continue
                    elif distance_on_track < 0:
                        var next = _get_next_connected_track(_track, false)
                        if next:
                            var next_len = next.curve.get_baked_length()
                            distance_on_track += next_len
                            _track = next
                            transitioned = true
                            max_transitions -= 1
                            continue
                    break
                
                if transitioned:
                    track_path = get_path_to(_track)
                
                global_transform = _get_track_transform_at_z_y_only(0.0)

        # Capture rest transforms if missing (ensure we have design-time state)
        for node in _car_nodes + _bogie_nodes + _wheel_nodes:
            if not _rest_transforms.has(node):
                _rest_transforms[node] = _get_relative_transform(node)
        
        for node in _wheel_nodes:
            if not _local_rest_bases.has(node):
                _local_rest_bases[node] = node.basis
        
        # Find all E3DModelInstance children to apply per-instance rotation logic
        var e3d_instances: Array[E3DModelInstance] = []
        var low_poly_cabin_node = get_node_or_null(low_poly_cabin_path) if low_poly_cabin_path else null
        for child in get_children(true):
            if child is E3DModelInstance:
                e3d_instances.append(child)
                if not _rest_transforms.has(child):
                    _rest_transforms[child] = _get_relative_transform(child)

        # Update each E3DModelInstance independently
        for e3d in e3d_instances:
            if _cabin and e3d == _cabin:
                continue
            var pivots = _instance_pivots_cache.get(e3d)
            var inst_wheels = _instance_wheels_cache.get(e3d)
            var b_nodes = e3d.get_bogie_nodes()
            
            if pivots == null:
                if b_nodes.size() == 2:
                    pivots = [b_nodes[0], b_nodes[1]]
                else:
                    # Fallback to furthest wheels if no exactly 2 bogies
                    if inst_wheels == null:
                        inst_wheels = _get_wheels_for_node(e3d)
                        _instance_wheels_cache[e3d] = inst_wheels
                    
                    if inst_wheels.size() >= 2:
                        pivots = _get_furthest_pivots(inst_wheels)
                    else:
                        pivots = []
                _instance_pivots_cache[e3d] = pivots

            if inst_wheels == null:
                inst_wheels = _get_wheels_for_node(e3d)
                _instance_wheels_cache[e3d] = inst_wheels
            
            var is_low_poly_cabin = (e3d == low_poly_cabin_node)

            if pivots.size() == 2:
                var p1_node = pivots[0]
                var p2_node = pivots[1]
                
                # 1. Calculate theoretical global transforms of pivots FIRST
                var t1 = _get_node_track_transform(p1_node)
                var t2 = _get_node_track_transform(p2_node)
                
                # 2. Position car based on theoretical pivots' tracks
                var p1 = t1.origin
                var p2 = t2.origin
                
                # Design-time local positions of pivots RELATIVE TO THE INSTANCE
                # (to find the car's origin relative to them)
                var rest_t = _rest_transforms[e3d]
                var lp1 = _rest_transforms[p1_node].origin - rest_t.origin
                var lp2 = _rest_transforms[p2_node].origin - rest_t.origin
                
                # Design-time distance between pivots (Z only for lerp ratio)
                var design_dist_z = abs(lp1.z - lp2.z)
                
                # Car rotation: Z points between pivots, horizontal only
                var dir = p2 - p1
                dir.y = 0
                var car_z = dir.normalized()
                if lp1.z > lp2.z: # If p1 is further forward than p2
                    car_z = -car_z
                
                if car_z.is_zero_approx():
                    # Fallback to horizontal track orientation
                    var t1_y = _get_track_transform_at_z_y_only(rest_t.origin.z + lp1.z)
                    car_z = t1_y.basis.z
                    
                var car_y = Vector3.UP
                
                if is_low_poly_cabin:
                    # Low poly cabin might need to follow track tilt if it's not strictly Y-axis?
                    # But the user said "ONLY calculate rotation on Y axis" for car/locomotive.
                    # Let's stick to Y-axis for now as requested.
                    pass
                
                var car_x = car_y.cross(car_z).normalized()
                var car_basis = Basis(car_x, car_y, car_z)
                
                if is_low_poly_cabin and cabin_rotate_180deg:
                    car_basis = car_basis.rotated(Vector3.UP, PI)

                # The car body should be positioned such that its local origin maps to the track points.
                # We use the lerped position of the pivots to find the car origin.
                var car_pos = p1
                if design_dist_z > 0.001:
                    # Find where the car's local origin (z=0) is between pivots
                    var t_lerp = -lp1.z / (lp2.z - lp1.z)
                    car_pos = p1.lerp(p2, t_lerp)
                    
                    # Also account for the average Y and X offsets of the pivots
                    var lp_lerp = lp1.lerp(lp2, t_lerp)
                    car_pos -= car_basis * lp_lerp
                else:
                    car_pos -= car_basis * lp1
                
                # Apply the global_transform to the specific E3D instance
                e3d.global_transform = Transform3D(car_basis, car_pos)
                
                # Now explicitly update bogies and wheels so they rotate to follow the track curve
                _update_attached_nodes(b_nodes)
                _update_attached_nodes(inst_wheels)
            else:
                # If not exactly 2 pivots, update this instance as a single point, Y-only
                var rest_t = _rest_transforms[e3d]
                var dz = rest_t.origin.z
                var rest_t_no_z = rest_t
                rest_t_no_z.origin.z = 0
                e3d.global_transform = _get_track_transform_at_z_y_only(dz) * rest_t_no_z
                
                if is_low_poly_cabin and cabin_rotate_180deg:
                    e3d.rotate_y(PI)

                # Still update its bogies and wheels with full rotation
                _update_attached_nodes(b_nodes)
                _update_attached_nodes(inst_wheels)

        # Update high-poly cabin to follow its assigned unit
        if _cabin and is_instance_valid(_cabin):
            var target = _cabin_unit if is_instance_valid(_cabin_unit) else self
            _cabin.global_transform = target.global_transform
            if cabin_rotate_180deg and target != low_poly_cabin_node:
                _cabin.rotate_y(PI)
        
        _animate_wheels(delta)
        
        if _is_physics_registered and _physics_body.is_valid():
            PhysicsServer3D.body_set_state(_physics_body, PhysicsServer3D.BODY_STATE_TRANSFORM, self.global_transform)
    else:
        _animate_wheels(delta)


func _get_wheels_for_node(node: Node) -> Array[Node3D]:
    var wheels: Array[Node3D] = []
    _get_wheels_recursive(node, wheels)
    return wheels

func _get_wheels_recursive(node: Node, wheels: Array[Node3D]):
    for child in node.get_children(true):
        var name_low = child.name.to_lower()
        if name_low.begins_with("wheel") or name_low.begins_with("kolo") or name_low.begins_with("koło") or name_low.begins_with("axle"):
            if child is Node3D:
                wheels.append(child)
        _get_wheels_recursive(child, wheels)

func _get_furthest_pivots(nodes: Array) -> Array:
    if nodes.size() < 2: return nodes
    var p1 = nodes[0]
    var p2 = nodes[1]
    var max_dist = 0.0
    for i in range(nodes.size()):
        for j in range(i + 1, nodes.size()):
            var r1 = _rest_transforms.get(nodes[i])
            var r2 = _rest_transforms.get(nodes[j])
            if not r1: r1 = _get_relative_transform(nodes[i])
            if not r2: r2 = _get_relative_transform(nodes[j])
            var dist = abs(r1.origin.z - r2.origin.z)
            if dist > max_dist:
                max_dist = dist
                p1 = nodes[i]
                p2 = nodes[j]
    return [p1, p2]


func _detect_wheels_recursive(node: Node):
    for child in node.get_children(true):
        var name_low = child.name.to_lower()
        # Search for wheels, axle nodes, and the Polish names used in MaSzyna
        # Prioritize nodes starting with wheel/kolo
        if name_low.begins_with("wheel") or name_low.begins_with("kolo") or name_low.begins_with("koło") or name_low.begins_with("axle"):
            if child is Node3D and not _wheel_nodes.has(child) and not _bogie_nodes.has(child) and not _car_nodes.has(child):
                _wheel_nodes.append(child)
                if not _wheel_angles.size() >= _wheel_nodes.size():
                    _wheel_angles.append(0.0)
                if not _local_rest_bases.has(child):
                    _local_rest_bases[child] = child.basis
        _detect_wheels_recursive(child)


func _animate_wheels(delta: float):
    if not _controller: return
    
    # nrot is in revolutions per second (obr/s)
    var nrot = _controller.state.get("wheel_rotation", 0.0)
    # Get direction from controller (-1, 0, 1). 
    # Use it to flip rotation sign as requested.
    var direction = _controller.state.get("direction", 1)
    if direction == 0: direction = 1 # Neutral roll fallback
    
    var rotation_speed = nrot * 2.0 * PI * direction
    
    for i in range(_wheel_nodes.size()):
        var wheel = _wheel_nodes[i]
        if not is_instance_valid(wheel): continue
        
        if i >= _wheel_angles.size(): _wheel_angles.append(0.0)
        
        _wheel_angles[i] += rotation_speed * delta
        # Limit to 2PI to avoid precision issues over long runs
        _wheel_angles[i] = fmod(_wheel_angles[i], 2.0 * PI)
        
        var rest_basis = _local_rest_bases.get(wheel, wheel.basis)
        # Apply rotation relative to its local rest basis around the local X axis
        wheel.transform.basis = rest_basis * Basis(Vector3.RIGHT, _wheel_angles[i])


func _get_track_transform_at_z_y_only(dz: float) -> Transform3D:
    var t = _get_track_transform_at_z(dz)
    var dir = t.basis.z
    dir.y = 0
    if dir.is_zero_approx():
        dir = Vector3.FORWARD
    dir = dir.normalized()
    var b = Basis(Vector3.UP.cross(dir).normalized(), Vector3.UP, dir)
    return Transform3D(b, t.origin)


func _get_track_transform_at_z(dz: float) -> Transform3D:
    var sample_dist = distance_on_track + dz
    var ct = _sample_track_smooth(_track, sample_dist)
    var target_basis = ct.basis
    #if not track_reverse:
    #    target_basis = target_basis.rotated(target_basis.y.normalized(), PI)
    return Transform3D(target_basis, ct.origin)


func _get_node_track_transform(node: Node3D) -> Transform3D:
    var rest_t = _rest_transforms.get(node)
    if not rest_t:
        rest_t = _get_relative_transform(node)
        _rest_transforms[node] = rest_t
        
    var dz = rest_t.origin.z
    var ct = _get_track_transform_at_z(dz)
    
    var rest_t_no_z = rest_t
    rest_t_no_z.origin.z = 0
    return ct * rest_t_no_z

func _update_attached_nodes(nodes: Array):
    for node in nodes:
        var rest_t = _rest_transforms.get(node)
        if not rest_t:
            rest_t = _get_relative_transform(node)
            _rest_transforms[node] = rest_t
            
        var dz = rest_t.origin.z
        
        # Create a transform for the node that preserves its relative X/Y and rotation
        # but aligns its Z-frame to the curve point.
        var rest_t_no_z = rest_t
        rest_t_no_z.origin.z = 0
        node.global_transform = _get_track_transform_at_z(dz) * rest_t_no_z


func _ready() -> void:
    if Engine.is_editor_hint():
        set_notify_transform(true)
    _needs_head_display_update = true
    _dirty = true
    E3DModelInstanceManager.instances_reloaded.connect(func(): _needs_head_display_update = true)
    
    for child in get_children(true):
        if child.has_signal("e3d_loaded"):
            if not child.e3d_loaded.is_connected(_on_e3d_loaded):
                child.e3d_loaded.connect(_on_e3d_loaded)
    
    if not Engine.is_editor_hint():
        _physics_body = PhysicsServer3D.body_create()
        PhysicsServer3D.body_set_mode(_physics_body, PhysicsServer3D.BODY_MODE_KINEMATIC)
        _physics_shape = PhysicsServer3D.box_shape_create()
        PhysicsServer3D.shape_set_data(_physics_shape, physics_box_size * 0.5) # Half-extents
        PhysicsServer3D.body_add_shape(_physics_body, _physics_shape)
        PhysicsServer3D.body_set_space(_physics_body, get_world_3d().space)
        _is_physics_registered = true

func _exit_tree() -> void:
    if _is_physics_registered:
        if _physics_body.is_valid():
            PhysicsServer3D.free_rid(_physics_body)
            _physics_body = RID()
        if _physics_shape.is_valid():
            PhysicsServer3D.free_rid(_physics_shape)
            _physics_shape = RID()
        _is_physics_registered = false


func _get_track_at_offset(start_track: Node3D, offset: float) -> Array:
    var current_track = start_track
    var target_dist = offset
    var max_traversals = 10
    while max_traversals > 0:
        if not current_track or not current_track.curve: break
        var length = current_track.curve.get_baked_length()
        if target_dist < 0:
            var prev = _get_next_connected_track(current_track, false)
            if prev:
                target_dist += prev.curve.get_baked_length()
                current_track = prev
                max_traversals -= 1
                continue
        elif target_dist > length:
            var next = _get_next_connected_track(current_track, true)
            if next:
                target_dist -= length
                current_track = next
                max_traversals -= 1
                continue
        break
    return [current_track, target_dist]


func _sample_track_smooth(start_track: Node3D, offset: float) -> Transform3D:
    var res = _get_track_at_offset(start_track, offset)
    var track = res[0]
    var dist = res[1]
    
    if not track or not track.curve:
        return Transform3D()
        
    var origin = track.global_transform * track.curve.sample_baked(dist)
    
    # Smooth rotation by sampling two points (0.2m ahead and behind)
    var res_prev = _get_track_at_offset(track, dist - 0.2)
    var res_next = _get_track_at_offset(track, dist + 0.2)
    
    var p_prev = origin
    if res_prev[0] and res_prev[0].curve:
        p_prev = res_prev[0].global_transform * res_prev[0].curve.sample_baked(res_prev[1])
        
    var p_next = origin
    if res_next[0] and res_next[0].curve:
        p_next = res_next[0].global_transform * res_next[0].curve.sample_baked(res_next[1])
    
    var forward = (p_next - p_prev).normalized()
    if forward.is_zero_approx():
        return track.global_transform * track.curve.sample_baked_with_rotation(dist)
        
    var ct_rot = track.global_transform * track.curve.sample_baked_with_rotation(dist)
    var up = ct_rot.basis.y.normalized()
    
    var basis = Basis()
    basis.z = forward
    basis.x = up.cross(basis.z).normalized()
    basis.y = basis.z.cross(basis.x).normalized()
    
    var t = Transform3D(basis, origin)
    
    if track.has_method("get_rail_top_vertical_offset"):
        t.origin += up * (track.get_rail_top_vertical_offset() + vertical_offset)
        
    return t
