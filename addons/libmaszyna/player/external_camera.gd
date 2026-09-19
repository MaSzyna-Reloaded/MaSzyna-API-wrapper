extends Camera3D
class_name ExternalCamera3D

## External views of the driven train, cycled with Shift+F4 - driver_mode::ExternalView()
## (drivermode.cpp:916-1037). The camera flies to the selected view like a drone and always
## looks at the controlled vehicle. Dragging with the right mouse button orbits the view around
## the vehicle, the arrow keys and PageUp/PageDown move the view offset. It is active while it
## is the current camera.

enum View {CONSIST_FRONT, CONSIST_REAR, BOGIE, DRIVEBY}

## How fast the drone follows its target (1/s)
@export var response:float = 2.0
## Mouse orbit sensitivity (degrees per pixel)
@export var sensitivity:float = 0.25
## Speed of moving the view offset with the arrow keys and PageUp/PageDown (m/s)
@export var move_speed:float = 5.0
## The same speed with Shift held (m/s)
@export var move_speed_fast:float = 20.0

var vehicle:RailVehicle3D
var view:View = View.CONSIST_FRONT

var _dirty:bool = false
# the vehicle the view is attached to (null for the drive-by point) and its local offset
var _view_vehicle:RailVehicle3D
var _view_offset:Vector3 = Vector3.ZERO
# the bogie view looks at the bogie of _view_vehicle instead of the vehicle center
var _bogie_look_offset:Vector3 = Vector3.ZERO
var _look_target:Vector3 = Vector3.ZERO
var _orbit:Vector2 = Vector2.ZERO
# driver_mode::m_externalviewconfigs (drivermode.cpp:590-596) - per view offset and orbit of the
# current vehicle, restored when the view is selected again; cleared when the vehicle changes
var _view_configs:Dictionary = {}


## Starts the flight from p_from (the cab camera). The selected view is kept between activations,
## like m_externalviewmode of the original.
func activate(p_vehicle:RailVehicle3D, p_from:Transform3D) -> void:
    if not vehicle == p_vehicle:
        _view_configs.clear()
    vehicle = p_vehicle
    global_transform = p_from
    _look_target = p_from.origin - p_from.basis.z * 10.0
    _dirty = true
    make_current()


func next_view() -> void:
    view = ((view + 1) % View.size()) as View
    _dirty = true


func _input(event:InputEvent) -> void:
    if not current:
        return
    if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_RIGHT:
        Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED if event.pressed else Input.MOUSE_MODE_VISIBLE)
    elif event is InputEventMouseMotion and Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
        _orbit.x -= deg_to_rad(event.relative.x * sensitivity)
        _orbit.y = clampf(_orbit.y - deg_to_rad(event.relative.y * sensitivity), -1.4, 1.4)


func _process(delta:float) -> void:
    if not current or not vehicle:
        return
    if _dirty:
        _dirty = false
        _process_dirty()
    _move_view_offset(delta)
    if _view_vehicle:
        _view_configs[view] = {"offset": _view_offset, "orbit": _orbit}

    var look_target:Vector3 = _view_vehicle.global_transform * _bogie_look_offset if view == View.BOGIE else _get_vehicle_center(vehicle)
    var view_position:Vector3 = _view_vehicle.global_transform * _view_offset if _view_vehicle else _view_offset
    var arm:Vector3 = (view_position - look_target).rotated(Vector3.UP, _orbit.x)
    var pitch_axis:Vector3 = Vector3.UP.cross(arm)
    if not pitch_axis.is_zero_approx():
        arm = arm.rotated(pitch_axis.normalized(), _orbit.y)

    var weight:float = 1.0 - exp(-response * delta)
    global_position = global_position.lerp(look_target + arm, weight)
    _look_target = _look_target.lerp(look_target, weight)
    if not global_position.is_equal_approx(_look_target):
        look_at(_look_target, Vector3.UP)


## Sets the view up once per selection, like the default view setup of ExternalView(), or restores
## the one remembered for this view.
func _process_dirty() -> void:
    _orbit = Vector2.ZERO
    var controller:TrainController = vehicle.get_controller()
    var state:Dictionary = TrainSystem.get_train_state(controller.train_id)
    var cabin_occupied:int = state.get("cabin_occupied", 0)
    var direction:int = state.get("direction", 0)
    var cab:int = 1 if cabin_occupied == 0 else cabin_occupied
    # Godot vehicles face -Z; MaSzyna's vehicle frame is (left, up, front)
    var front:Vector3 = -vehicle.global_basis.z.normalized()
    var left:Vector3 = -vehicle.global_basis.x.normalized()

    if view == View.DRIVEBY:
        # driver_mode::DistantView(false) (drivermode.cpp:904-907) - a fixed point 50 m ahead of the cab
        _view_vehicle = null
        _view_offset = (
            vehicle.global_position
            + front * cabin_occupied * 50.0
            + Vector3(-10.0 * left.x * cab, 1.6, -10.0 * left.z * cab))
        return

    # drivermode.cpp:951-1019 - offsetflip from the occupied cab and the active direction
    var flip:float = cab * (1 if direction == 0 else direction)
    if view == View.CONSIST_REAR:
        flip = -flip

    # Mechanik->Vehicle(end::front / end::rear) - the last vehicle of the consist on that side
    _view_vehicle = _find_vehicle(_get_consist_end(controller, 0 if flip > 0.0 else 1))
    var owner_controller:TrainController = _view_vehicle.get_controller()
    var width:float = owner_controller.dimensions_width
    var height:float = owner_controller.dimensions_height
    var length:float = owner_controller.dimensions_length

    var offset:Vector3
    match view:
        View.CONSIST_FRONT:
            offset = Vector3(1.5 * width * flip, maxf(5.0, 1.25 * height), -0.4 * length * flip)
        View.CONSIST_REAR:
            offset = Vector3(1.5 * width * flip, maxf(5.0, 1.25 * height), 0.2 * length * flip)
        View.BOGIE:
            offset = Vector3(-0.65 * width * flip, 0.9, 0.15 * length * flip)
            # the original looks ahead along the vehicle side at bogie height (drivermode.cpp:1021),
            # so aim at the front bogie area (Godot local, -Z ahead)
            _bogie_look_offset = Vector3(0.0, 0.9, -0.35 * length * flip)
    # the MaSzyna (left, up, front) frame is yawed by 180 degrees in Godot
    _view_offset = Vector3(-offset.x, offset.y, -offset.z)

    # restore view config (drivermode.cpp:942-946)
    var config:Dictionary = _view_configs.get(view, {})
    if config:
        _view_offset = config["offset"]
        _orbit = config["orbit"]


## TCamera::Update() (Camera.cpp:191-213) - in the external view the keys move the owner offset
## relative to the camera heading (the original moves it at 2 m/s).
func _move_view_offset(delta:float) -> void:
    var move:Vector3 = Vector3(
        float(Input.is_physical_key_pressed(KEY_RIGHT)) - float(Input.is_physical_key_pressed(KEY_LEFT)),
        float(Input.is_physical_key_pressed(KEY_PAGEUP)) - float(Input.is_physical_key_pressed(KEY_PAGEDOWN)),
        float(Input.is_physical_key_pressed(KEY_DOWN)) - float(Input.is_physical_key_pressed(KEY_UP)))
    if move.is_zero_approx():
        return
    var speed:float = move_speed_fast if Input.is_key_pressed(KEY_SHIFT) else move_speed
    var world_move:Vector3 = Basis(Vector3.UP, global_rotation.y) * move.normalized() * speed * delta
    _view_offset += _view_vehicle.global_basis.inverse() * world_move if _view_vehicle else world_move


func _get_vehicle_center(p_vehicle:RailVehicle3D) -> Vector3:
    return p_vehicle.global_position + p_vehicle.global_basis.y.normalized() * 0.5 * p_vehicle.get_controller().dimensions_height


func _get_consist_end(controller:TrainController, end:int) -> TrainController:
    var last:TrainController = controller
    var next:TrainController = last.get_coupled_controller(end)
    while next:
        end = 1 - last.get_coupled_end(end)
        last = next
        next = last.get_coupled_controller(end)
    return last


func _find_vehicle(controller:TrainController) -> RailVehicle3D:
    for node:Node in get_tree().get_root().find_children("", "RailVehicle3D", true, false):
        var candidate:RailVehicle3D = node as RailVehicle3D
        if candidate and candidate.get_controller() == controller:
            return candidate
    return vehicle
