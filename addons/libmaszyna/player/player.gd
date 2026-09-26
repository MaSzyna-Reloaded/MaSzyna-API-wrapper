extends Node3D
class_name MaszynaPlayer

signal controlled_vehicle_changed
## Switched between the cabin view (in a cab) and the exterior view (on foot, external cameras)
signal cabin_view_changed(in_cabin:bool)

@export var start_train_id:String = "":
    set(x):
        if not start_train_id == x:
            start_train_id = x
            if x:
                _auto_start_pending = false
            _dirty = true

## Player's own sounds (the "flashlight" event with a "toggle" automation), provided by the game
@export var sfx_bank:SfxBank

var last_controlled_vehicle:RailVehicle3D
var controlled_vehicle:RailVehicle3D
var _camera:FreeCamera3D
@onready var train_sound_listener:TrainSoundListener3D = $TrainSoundListener3D
@onready var external_camera:ExternalCamera3D = $ExternalCamera3D
## Player's head torch - follows the camera (the player's head) in the cab and on foot
@onready var headlamp:SpotLight3D = $Camera3D/Headlamp
## Screen-space near-field glow inside the headlamp cone (headlamp_glow.gdshader)
@onready var headlamp_glow:MeshInstance3D = $Camera3D/HeadlampGlow
## Non-positional: the player's own sounds are at the listener, where a 3D player gains nothing
@onready var sfx_player:SfxPlayer = $PlayerSfx
var _dirty: bool = true
var _auto_start_pending:bool = true
## The vehicle the player chose in the world (or re-enters); a vehicle is held, never looked up by
## its scenery name, which two vehicles may share and one may lack
var _requested_vehicle:RailVehicle3D
var _released_vehicle:RID
var _cabin_view:bool = false

func _ready() -> void:
    sfx_player.bank = sfx_bank
    headlamp.shadow_reverse_cull_face = ProjectSettings.get_setting("maszyna/lights/reverse_cull_face", true)
    # the glow follows the spot: both are children of the camera, so their transform is view space
    var glow_material:ShaderMaterial = (headlamp_glow.mesh as QuadMesh).material as ShaderMaterial
    glow_material.set_shader_parameter(&"light_position", headlamp.position)
    glow_material.set_shader_parameter(&"light_direction", -headlamp.transform.basis.z)
    glow_material.set_shader_parameter(&"light_color", headlamp.light_color)
    # scenery content is registered, not built - it is streamed around this camera
    SceneryStreamingServer.set_camera(get_camera())
    CabinHUDMouseSystem.set_camera(get_camera().get_instance_id())


func _exit_tree() -> void:
    SceneryStreamingServer.set_camera(null)

func _process(_delta:float) -> void:
    if _dirty:
        _dirty = false
        var _changed:bool = false

        if external_camera.current:
            _set_external_view(false)

        if controlled_vehicle:
            controlled_vehicle.leave_cabin(self)
            controlled_vehicle = null
            _changed = true

        var target_vehicle:RailVehicle3D = _find_start_vehicle()
        if target_vehicle:
            controlled_vehicle = target_vehicle
            controlled_vehicle.enter_cabin(self)
            last_controlled_vehicle = controlled_vehicle
            _dirty = false
            _changed = true
        elif start_train_id or _requested_vehicle or _auto_start_pending:
            _dirty = true

        if _changed:
            controlled_vehicle_changed.emit()
            _update_cabin_view()

    var camera:FreeCamera3D = get_camera()
    var cabin:Cabin3D = camera.get_parent() as Cabin3D
    if cabin:
        camera.h_offset = cabin.get_camera_shake_offset().x * 1.5
        camera.rotation.z = cabin.get_camera_shake_roll()
    else:
        camera.h_offset = 0.0
        camera.rotation.z = 0.0

## Clears start_train_id and waits until the player has left the cab (in _process), e.g. before
## the scenery holding the vehicle is freed - the camera lives in the vehicle's cabin.
func clear_start_train() -> void:
    start_train_id = ""
    _request_vehicle(null)
    if controlled_vehicle:
        await controlled_vehicle_changed

## Sets start_train_id unless the player already has one (e.g. from MaszynaSceneryNode.scenery_loaded)
func set_start_train_if_empty(train_id:String) -> void:
    if not start_train_id:
        start_train_id = train_id

func _input(event):
    if CabinHUDMouseSystem.input(event):
        get_viewport().set_input_as_handled()
        return
    if event.is_action_pressed("flashlight_toggle", false, true):
        var enabled:bool = headlamp.visible
        headlamp.visible = not enabled
        headlamp_glow.visible = not enabled
        # "toggle" automation: 0 - switching on click, 1 - switching off click
        sfx_player.play_automation(&"flashlight", &"toggle", float(enabled))
    if event.is_action_pressed("change_vehicle") or event.is_action_pressed("cabin_mode_toggle", false, true):
        var detector:ShapeCast3D = get_camera().get_node("RailVehicleDetector")
        if not controlled_vehicle and detector.is_colliding():
            var coll:Area3D = detector.get_collider(0)
            if coll:
                var _tmp = coll.get_parent()
                while _tmp and not _tmp is RailVehicle3D:
                    _tmp = _tmp.get_parent()
                if _tmp and _tmp.cabin_scene:
                    if event.is_action_pressed("change_vehicle") or not last_controlled_vehicle:
                        _request_vehicle(_tmp as RailVehicle3D)

    # Train.cpp:6644-6720 - Home (cabchangeforward) / End (cabchangebackward).
    if controlled_vehicle and event.is_action_pressed("cabin_previous"):
        RailVehicleServer.vehicle_send_command(controlled_vehicle.get_rid(), "cab_change", 1)
    if controlled_vehicle and event.is_action_pressed("cabin_next"):
        RailVehicleServer.vehicle_send_command(controlled_vehicle.get_rid(), "cab_change", -1)

    if not controlled_vehicle:
        _walk_mode_input(event)

    # drivermode.cpp:803-804 - Shift+F4 cycles the external views, F4 returns from them to the cab
    if controlled_vehicle and event.is_action_pressed("external_view_cycle", false, true):
        if external_camera.current:
            external_camera.next_view()
        else:
            _set_external_view(true)

    if event.is_action_pressed("cabin_mode_toggle", false, true):
        if external_camera.current:
            _set_external_view(false)
        elif not controlled_vehicle:
            # the vehicle last driven may have gone with its scenery
            if is_instance_valid(last_controlled_vehicle):
                _request_vehicle(last_controlled_vehicle)
        else:
            start_train_id = ""
            _auto_start_pending = false
            _request_vehicle(null)

## Walk mode: the brake releaser, the manual brake and coupling act on the vehicle nearest to the
## player - TTrain::OnCommand_independentbrakebailoff (Train.cpp:1580-1595),
## OnCommand_manualbrakeincrease/decrease (Train.cpp:1809-1837) via find_nearest_consist_vehicle(),
## OnCommand_nearestcarcouplingincrease/disconnect (Train.cpp:6207-6249) at its nearest coupler.
func _walk_mode_input(event:InputEvent) -> void:
    if event.is_action_pressed("brake_release", false, true):
        _released_vehicle = _find_nearest_vehicle()
        if _released_vehicle.is_valid():
            RailVehicleServer.vehicle_send_command(_released_vehicle, "brake_releaser", true)
    elif event.is_action_released("brake_release", true) and _released_vehicle.is_valid():
        RailVehicleServer.vehicle_send_command(_released_vehicle, "brake_releaser", false)
        _released_vehicle = RID()
    elif event.is_action_pressed("manual_brake_increase", true, true):
        _send_to_nearest_train("manual_brake_increase")
    elif event.is_action_pressed("manual_brake_decrease", true, true):
        _send_to_nearest_train("manual_brake_decrease")
    elif event.is_action_pressed("coupler_connect", false, true):
        _send_to_nearest_train("coupler_connect", get_camera().global_position)
    elif event.is_action_pressed("coupler_disconnect", false, true):
        _send_to_nearest_train("coupler_disconnect", get_camera().global_position)


func _send_to_nearest_train(command:String, p1:Variant = null) -> void:
    var vehicle:RID = _find_nearest_vehicle()
    if vehicle.is_valid():
        RailVehicleServer.vehicle_send_command(vehicle, command, p1)


## TTrain::find_nearest_consist_vehicle() (Train.cpp:1023) scans up to 1500 m for the vehicle
## nearest to the camera.
func _find_nearest_vehicle() -> RID:
    var position:Vector3 = get_camera().global_position
    var nearest:RID = RID()
    var nearest_distance:float = 1500.0
    for vehicle:RID in RailVehicleServer.get_vehicles():
        var distance:float = position.distance_to(RailVehicleServer.vehicle_get_transform(vehicle).origin)
        if distance < nearest_distance:
            nearest_distance = distance
            nearest = vehicle
    return nearest


func _find_start_vehicle() -> RailVehicle3D:
    if is_instance_valid(_requested_vehicle):
        return _requested_vehicle
    var vehicles:Array[Node] = get_tree().get_root().find_children("", "RailVehicle3D", true, false)
    if start_train_id:
        for node:Node in vehicles:
            var vehicle:RailVehicle3D = node as RailVehicle3D
            if vehicle and _get_vehicle_train_id(vehicle) == start_train_id:
                return vehicle
        return null

    if _auto_start_pending:
        for node:Node in vehicles:
            var vehicle:RailVehicle3D = node as RailVehicle3D
            if vehicle and vehicle.get_controller():
                _auto_start_pending = false
                _request_vehicle(vehicle)
                return vehicle
    return null

## The one writer of the vehicle the player asked to enter; null asks to leave the cab
func _request_vehicle(vehicle:RailVehicle3D) -> void:
    _requested_vehicle = vehicle
    _dirty = true

func _get_vehicle_train_id(vehicle:RailVehicle3D) -> String:
    var controller:VehicleController = vehicle.get_controller()
    return controller.train_id if controller else ""

## The cab camera is frozen while the external camera is current, so the arrow keys do not move it.
func _set_external_view(p_enabled:bool) -> void:
    var camera:FreeCamera3D = get_camera()
    camera.process_mode = Node.PROCESS_MODE_DISABLED if p_enabled else Node.PROCESS_MODE_INHERIT
    SceneryStreamingServer.set_camera(external_camera if p_enabled else camera)
    if p_enabled:
        external_camera.activate(controlled_vehicle, camera.global_transform)
    else:
        camera.make_current()
    _update_cabin_view()

func _update_cabin_view() -> void:
    var in_cabin:bool = controlled_vehicle and not external_camera.current
    if in_cabin == _cabin_view:
        return
    _cabin_view = in_cabin
    get_tree().set_group(MaszynaEnvironmentNode.GROUP, &"cabin_view", in_cabin)
    cabin_view_changed.emit(in_cabin)

func get_camera() -> FreeCamera3D:
    if not _camera:
        _camera = get_node("Camera3D") as FreeCamera3D
    return _camera
