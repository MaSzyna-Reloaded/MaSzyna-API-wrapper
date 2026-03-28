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

@export var distance_on_track:float = 0.0
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
            if cabin_rotate_180deg:
                _camera.global_basis = global_basis
            else:
                _camera.global_basis = global_basis.rotated(Vector3.UP, deg_to_rad(180))
        else:
            _camera.position = Vector3(0, 3, 0)

        _camera.velocity_multiplier = 0.2

    _cabin.cabin_ready.connect(_jump_into_cabin)
    add_child(_cabin)
    _cabin.global_transform = self.global_transform
    if cabin_rotate_180deg:
        _cabin.rotate_y(deg_to_rad(180))

    await get_tree().process_frame
    await get_tree().process_frame
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


func _on_e3d_loaded():
    _rest_transforms.clear()
    _local_rest_bases.clear()
    _wheel_nodes.clear()
    _wheel_angles.clear()
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
                var t = _track.global_transform * _track.curve.sample_baked_with_rotation(distance_on_track)
                if not track_reverse:
                    t.basis = t.basis.rotated(t.basis.y.normalized(), PI)
                
                if _track.has_method("get_rail_top_vertical_offset"):
                    t.origin += t.basis.y.normalized() * _track.get_rail_top_vertical_offset()
                
                global_transform = t

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
                
                var t = _track.global_transform * _track.curve.sample_baked_with_rotation(distance_on_track)
                if not track_reverse:
                    t.basis = t.basis.rotated(t.basis.y.normalized(), PI)
                
                if _track.has_method("get_rail_top_vertical_offset"):
                    t.origin += t.basis.y.normalized() * _track.get_rail_top_vertical_offset()
                
                global_transform = t

        # Capture rest transforms if missing (ensure we have design-time state)
        for node in _car_nodes + _bogie_nodes:
            if not _rest_transforms.has(node):
                _rest_transforms[node] = _get_relative_transform(node)
        
        for node in _wheel_nodes:
            if not _local_rest_bases.has(node):
                _local_rest_bases[node] = node.basis
        
        # If we have 2 bogies, we use them as pivot points for the car body
        if _bogie_nodes.size() == 2:
            var b1 = _bogie_nodes[0]
            var b2 = _bogie_nodes[1]
            
            # 1. Update bogies FIRST
            _update_attached_nodes([b1, b2])
            
            # 2. Position car based on bogies
            var p1 = b1.global_position
            var p2 = b2.global_position
            
            # Design-time local positions of bogies (to find the car's center relative to them)
            var rb1_z = _rest_transforms[b1].origin.z
            var rb2_z = _rest_transforms[b2].origin.z
            
            # Current distance between bogies in global space
            var current_dist = p1.distance_to(p2)
            # Design-time distance between bogies
            var design_dist = abs(rb1_z - rb2_z)
            
            # The car body should be centered between bogies based on their design-time Z-offsets
            # Car position is the weighted average of bogie positions
            var ratio = 0.0
            if design_dist > 0.001:
                # If rb1_z=10 and rb2_z=-10, design_dist=20. If car_center=0, we need p1 + (p2-p1) * 10/20 = p1 + (p2-p1) * 0.5
                ratio = abs(rb1_z) / design_dist
                
            var car_pos = p1.lerp(p2, ratio)
            
            # Car rotation: Z points from b1 to b2 (or vice versa depending on orientation)
            # We want the car's Z axis to align with the vector between bogies.
            var car_z = (p2 - p1).normalized()
            if rb1_z > rb2_z: # If b1 is further forward than b2
                car_z = -car_z
                
            # For the Up vector, we can average the bogies' Up vectors or use the track's Up at car_pos
            var car_up = (b1.global_transform.basis.y + b2.global_transform.basis.y).normalized()
            
            # Construct the car's global transform
            # Godot uses Y-up, -Z forward (MaSzyna uses -Z forward too in E3D, but here we align to track)
            var car_basis = Basis()
            car_basis.z = car_z
            car_basis.x = car_up.cross(car_z).normalized()
            car_basis.y = car_z.cross(car_basis.x).normalized()
            
            var car_gt = Transform3D(car_basis, car_pos)
            
            for node in _car_nodes:
                var rest_t = _rest_transforms.get(node)
                if not rest_t:
                    rest_t = _get_relative_transform(node)
                    _rest_transforms[node] = rest_t
                # Remove Z from rest transform because we already accounted for bogie-to-car-center distance
                var rest_t_no_z = rest_t
                rest_t_no_z.origin.z = 0
                node.global_transform = car_gt * rest_t_no_z
                
            _animate_wheels(delta)
        else:
            # Fallback to legacy single-point snapping if not exactly 2 bogies
            _update_attached_nodes(_car_nodes)
            _update_attached_nodes(_bogie_nodes)
            _animate_wheels(delta)
    else:
        _animate_wheels(delta)


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


func _update_attached_nodes(nodes: Array):
    for node in nodes:
        var rest_t = _rest_transforms.get(node)
        if not rest_t:
            rest_t = _get_relative_transform(node)
            _rest_transforms[node] = rest_t
            
        var dz = rest_t.origin.z
        var target_dist = distance_on_track + dz
        var current_track = _track
        
        # Traverse tracks if needed for this specific node offset
        var max_traversals = 5
        while max_traversals > 0:
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
                
        var ct = current_track.global_transform * current_track.curve.sample_baked_with_rotation(target_dist)
        var target_basis = ct.basis
        if not track_reverse:
            target_basis = target_basis.rotated(target_basis.y.normalized(), PI)
            
        if current_track.has_method("get_rail_top_vertical_offset"):
            ct.origin += target_basis.y.normalized() * current_track.get_rail_top_vertical_offset()

        # Create a transform for the node that preserves its relative X/Y and rotation
        # but aligns its Z-frame to the curve point.
        var rest_t_no_z = rest_t
        rest_t_no_z.origin.z = 0
        node.global_transform = Transform3D(target_basis, ct.origin) * rest_t_no_z


func _ready() -> void:
    _needs_head_display_update = true
    _dirty = true
    E3DModelInstanceManager.instances_reloaded.connect(func(): _needs_head_display_update = true)
    
    for child in get_children(true):
        if child.has_signal("e3d_loaded"):
            if not child.e3d_loaded.is_connected(_on_e3d_loaded):
                child.e3d_loaded.connect(_on_e3d_loaded)
