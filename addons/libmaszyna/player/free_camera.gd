# FreeCamera3D
#
# Based on https://github.com/adamviola/simple-free-look-camera (MIT license)
# FIXME: may not be optimized

extends Camera3D
class_name FreeCamera3D

@export var sensitivity:float = 0.25
@export var key_forward = KEY_UP
@export var key_backward = KEY_DOWN
@export var key_left = KEY_LEFT
@export var key_right = KEY_RIGHT
@export var key_up = KEY_PAGEUP
@export var key_down= KEY_PAGEDOWN

@export var enabled:bool = true
@export var acceleration:float = 30:
    set(x):
        acceleration = x

@export var acceleration_medium:float = 100
@export var acceleration_fast:float = 1000
@export var deceleration = 100:
    set(x):
        deceleration = clampf(x, 0, 1000)
@export var velocity_multiplier:float = 1.0

@export var bound_min = Vector3.ZERO
@export var bound_max = Vector3.ZERO
@export var bound_enabled:bool = false
# Mouse state
var _mouse_position = Vector2(0.0, 0.0)

# Movement state
var _direction = Vector3(0.0, 0.0, 0.0)
var _velocity = Vector3(0.0, 0.0, 0.0)

# Keyboard state
var _w = 0.0
var _s = 0.0
var _a = 0.0
var _d = 0.0
var _q = 0.0
var _e = 0.0
var accel_mode = 0

func _ready():
    Console.console_toggled.connect(_on_console_toggle)

func _on_console_toggle(console_visible):
    enabled = not console_visible

func _input(event):
    if not enabled:
        return
    # Receives mouse motion
    if event is InputEventMouseMotion:
        _mouse_position = event.relative

    # Receives mouse button input
    if event is InputEventMouseButton:
        match event.button_index:
            MOUSE_BUTTON_RIGHT: # Only allows rotation if right click down
                Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED if event.pressed else Input.MOUSE_MODE_VISIBLE)
            MOUSE_BUTTON_WHEEL_UP:
                #velocity_multiplier = clamp(velocity_multiplier * 1.1, 0.2, 20)
                fov = clamp(fov - 1, 1, 100)
            MOUSE_BUTTON_WHEEL_DOWN:
                fov = clamp(fov + 1, 1, 100)

    # Receives key input
    if event is InputEventKey:
        if event.is_command_or_control_pressed():
            accel_mode = 2
        elif event.shift_pressed:
            accel_mode = 1
        else:
            accel_mode = 0

        match event.physical_keycode:
            key_forward:
                _w = float(event.pressed)
            key_backward:
                _s = float(event.pressed)
            key_left:
                _a = float(event.pressed)
            key_right:
                _d = float(event.pressed)
            key_down:
                _q = float(event.pressed)
            key_up:
                _e = float(event.pressed)

    if Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED and event is InputEventMouseMotion:
        _update_mouselook(event.relative)

# Updates mouselook and movement every frame
func _process(delta):
    _update_movement(delta)

# Updates camera movement
func _update_movement(delta):
    # Computes desired direction from key states
    _direction = Vector3(_d - _a , _e - _q, _s - _w)

    var accel = acceleration_fast if accel_mode == 2 else acceleration_medium if accel_mode == 1 else acceleration
    # Computes the change in velocity due to desired direction and "drag"
    # The "drag" is a constant acceleration on the camera to bring it's velocity to 0
    if _direction.is_zero_approx():
        _velocity = lerp(_velocity, Vector3.ZERO, 0.1 * delta * deceleration * 4)
    else:
        _velocity = _direction * Vector3.ONE * 10 * velocity_multiplier * accel * delta

    # Checks if we should bother translating the camera
    var new_position = transform.translated_local(_velocity * delta).origin
    if bound_enabled:
        new_position.x = clamp(new_position.x, bound_min.x, bound_max.x)
        new_position.y = clamp(new_position.y, bound_min.y, bound_max.y)
        new_position.z = clamp(new_position.z, bound_min.z, bound_max.z)
    position = new_position

# Updates mouse look
func _update_mouselook(mouse_delta):
    # pitch clamping fixed (no accumlation needed)

    _mouse_position *= sensitivity
    # Adjust mouse delta by sensitivity
    var yaw_delta = mouse_delta.x * sensitivity
    var pitch_delta = mouse_delta.y * sensitivity

    # Apply yaw rotation to the parent or self (horizontal rotation)
    rotate_y(deg_to_rad(-yaw_delta))

    # Get the current pitch from the camera's rotation
    var current_pitch = rotation_degrees.x - pitch_delta
    current_pitch = clamp(current_pitch, -90, 90)

    # Apply the clamped pitch directly
    rotation_degrees.x = current_pitch
