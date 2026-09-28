extends Node3D
class_name MaszynaPlayer

signal controlled_vehicle_changed
## Switched between the cabin view (in a cab) and the exterior view (on foot, external cameras)
signal cabin_view_changed(in_cabin:bool)
## The external camera started watching the vehicle, or stopped (an invalid RID)
signal external_view_changed(vehicle:RID)

@export var start_train_id:String = "":
    set(x):
        if not start_train_id == x:
            start_train_id = x
            if x:
                _auto_start_pending = false
            _dirty = true

## Without a start_train_id the player takes the first vehicle that has a controller. Off where the
## scene names the train itself once its scenery is loaded - a vehicle taken while the scenery still
## loads is not coupled yet, and the cab it activates reaches no other car of its unit.
@export var auto_start:bool = true

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
var _auto_start_pending:bool = false
## The vehicle the player chose in the world (or re-enters); a vehicle is held, never looked up by
## its scenery name, which two vehicles may share and one may lack
var _requested_vehicle:RailVehicle3D
var _released_vehicle:RID
var _cabin_view:bool = false
## Where the free camera is put once the player is out of the cab, and the velocity it glides on
## at (show_vehicle(), leave_external_view())
var _camera_placement_pending:bool = false
var _camera_placement:Transform3D
var _camera_glide:Vector3
## The end vehicle_get_coupled() starts listing a trainset from
const FRONT_END:int = 0
## show_vehicle(): the camera stands this far to the side per metre of the vehicle's length, never
## nearer than the minimum [m], at eye height above the vehicle's origin [m]
const SHOW_DISTANCE_PER_LENGTH:float = 1.2
const SHOW_MIN_DISTANCE:float = 12.0
const SHOW_EYE_HEIGHT:float = 1.75

func _ready() -> void:
    _auto_start_pending = auto_start and not start_train_id
    sfx_player.bank = sfx_bank
    headlamp.shadow_reverse_cull_face = ProjectSettings.get_setting("maszyna/lights/reverse_cull_face", false)
    # the glow follows the spot: both are children of the camera, so their transform is view space
    var glow_material:ShaderMaterial = (headlamp_glow.mesh as QuadMesh).material as ShaderMaterial
    glow_material.set_shader_parameter(&"light_position", headlamp.position)
    glow_material.set_shader_parameter(&"light_direction", -headlamp.transform.basis.z)
    glow_material.set_shader_parameter(&"light_color", headlamp.light_color)
    # scenery content is registered, not built - it is streamed around this camera
    SceneryStreamingServer.set_camera(get_camera())
    CabinHUDMouseSystem.set_camera(get_camera().get_instance_id())
    SceneryHUDMouseServer.set_camera(get_camera().get_instance_id())


func _exit_tree() -> void:
    SceneryStreamingServer.set_camera(null)

func _process(_delta:float) -> void:
    if _dirty:
        _dirty = false
        var _changed:bool = false
        var target_vehicle:RailVehicle3D = _find_start_vehicle()

        # the external view ends when the player takes a cab, when the free camera is put somewhere,
        # and with the cab it looked at (Shift+F4); a vehicle followed from outside is kept while
        # the player only steps out of the cab
        if external_camera.current and (target_vehicle or _camera_placement_pending
                or external_camera.vehicle == controlled_vehicle):
            _set_external_view(false)

        if controlled_vehicle:
            controlled_vehicle.leave_cabin(self)
            controlled_vehicle = null
            _changed = true

        if target_vehicle and not target_vehicle == last_controlled_vehicle:
            # a train left on foot (F4) stays the player's, and coming back to it (F4) changes
            # nothing (drivermode.cpp:1244); its driver takes the trainset back only when the player
            # takes a vehicle of another trainset (simulation.cpp:257-270)
            if is_instance_valid(last_controlled_vehicle):
                var trainset:Array[RID] = RailVehicleServer.vehicle_get_coupled(
                        last_controlled_vehicle.get_rid(), FRONT_END, RailVehicleController.COUPLING_ELEMENT_COUPLER)
                if not trainset.has(target_vehicle.get_rid()):
                    for vehicle:RID in trainset:
                        DriverSystem.vehicle_set_control_active(vehicle, true)
            # the player takes the vehicle over: its driver, if it has one, only takes orders
            # (drivermode.cpp:266)
            DriverSystem.vehicle_set_control_active(target_vehicle.get_rid(), false)
        if target_vehicle:
            controlled_vehicle = target_vehicle
            # a glide left by the external view does not carry into the cab
            get_camera().glide(Vector3.ZERO)
            controlled_vehicle.enter_cabin(self)
            last_controlled_vehicle = controlled_vehicle
            _dirty = false
            _changed = true
        elif start_train_id or _requested_vehicle or _auto_start_pending:
            _dirty = true

        if _changed:
            controlled_vehicle_changed.emit()
            _update_cabin_view()
        if _camera_placement_pending and not controlled_vehicle:
            _camera_placement_pending = false
            get_camera().global_transform = _camera_placement
            get_camera().glide(_camera_glide)

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
    # a vehicle followed from outside goes with the scenery as well
    if external_camera.current:
        _set_external_view(false)
    start_train_id = ""
    _request_vehicle(null)
    if controlled_vehicle:
        await controlled_vehicle_changed

func _input(event):
    if CabinHUDMouseSystem.input(event) or SceneryHUDMouseServer.input(event):
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

    # Train.cpp:1088-1118 - Shift+Q hands the player's train to its driver, Q takes it back; on foot
    # as well, the train left there is still the player's
    if is_instance_valid(last_controlled_vehicle):
        if event.is_action_pressed("ai_driver_enable", false, true):
            # switched off first, as the original does, so that a driver already driving starts over
            DriverSystem.vehicle_set_control_active(last_controlled_vehicle.get_rid(), false)
            DriverSystem.vehicle_set_control_active(last_controlled_vehicle.get_rid(), true)
        elif event.is_action_pressed("ai_driver_disable", false, true):
            DriverSystem.vehicle_set_control_active(last_controlled_vehicle.get_rid(), false)

    # drivermode.cpp:803-804 - Shift+F4 cycles the external views, F4 returns from them to the cab
    if event.is_action_pressed("external_view_cycle", false, true):
        if external_camera.current:
            external_camera.next_view()
        elif controlled_vehicle:
            follow_vehicle(controlled_vehicle.get_rid())

    if event.is_action_pressed("cabin_mode_toggle", false, true):
        # the cab left behind takes the player back; on foot, the free camera goes on from where
        # the external camera is
        if external_camera.current and controlled_vehicle:
            _set_external_view(false)
        elif external_camera.current:
            leave_external_view()
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

## Leaves the cab, as F4 does, and puts the free camera beside the vehicle, looking at it
func show_vehicle(vehicle:RID) -> void:
    var body:Transform3D = RailVehicleServer.vehicle_get_transform(vehicle)
    var length:float = RailVehicleServer.vehicle_dump_config(vehicle).get("length", 0.0)
    var camera_position:Vector3 = body.origin + body.basis.x.normalized() * maxf(
            SHOW_MIN_DISTANCE, length * SHOW_DISTANCE_PER_LENGTH) + Vector3.UP * SHOW_EYE_HEIGHT
    _leave_cabin_to_free_camera(Transform3D(Basis(), camera_position).looking_at(
            body.origin + Vector3.UP * SHOW_EYE_HEIGHT), Vector3.ZERO)

## The external view ends in the free camera, left where the external camera is and gliding on
## until it stops; the player leaves the cab, as F4 does
func leave_external_view() -> void:
    _leave_cabin_to_free_camera(external_camera.global_transform, external_camera.velocity)

## Into the vehicle's cab, as when the player picks it in the world: the player takes it over, and
## the trainset the player drove before goes back to its driver
func enter_vehicle(vehicle:RID) -> void:
    var node:RailVehicle3D = instance_from_id(RailVehicleServer.vehicle_get_rail_vehicle(vehicle)) as RailVehicle3D
    if not node:
        return
    start_train_id = ""
    _auto_start_pending = false
    _request_vehicle(node)

## Back to the vehicle the player drives, as F4 does: out of the external view into its cab, or
## into its cab from on foot
func return_to_vehicle() -> void:
    if controlled_vehicle:
        _set_external_view(false)
    elif is_instance_valid(last_controlled_vehicle):
        _request_vehicle(last_controlled_vehicle)

## The player lets the trainset go: its driver drives it again (simulation.cpp:257-270). A player
## watching it from outside stays on foot where the camera is; one following another vehicle only
## steps out of the cab and keeps following
func hand_over_vehicle() -> void:
    if not is_instance_valid(last_controlled_vehicle):
        return
    for vehicle:RID in RailVehicleServer.vehicle_get_coupled(
            last_controlled_vehicle.get_rid(), FRONT_END, RailVehicleController.COUPLING_ELEMENT_COUPLER):
        DriverSystem.vehicle_set_control_active(vehicle, true)
    if controlled_vehicle and external_camera.vehicle == controlled_vehicle:
        leave_external_view()
    elif controlled_vehicle:
        _leave_cabin()
    last_controlled_vehicle = null
    controlled_vehicle_changed.emit()

## True while the player sits in a cab and looks from it, not from an external camera
func is_in_cabin_view() -> bool:
    return _cabin_view

## The vehicle the external camera watches; an invalid RID while it is off
func get_external_view_vehicle() -> RID:
    return external_camera.vehicle.get_rid() if external_camera.current and is_instance_valid(
            external_camera.vehicle) else RID()

func _leave_cabin_to_free_camera(placement:Transform3D, glide:Vector3) -> void:
    _camera_placement_pending = true
    _camera_placement = placement
    _camera_glide = glide
    _leave_cabin()

## The player steps out of the cab in _process, as F4 does, and is not put into another one
func _leave_cabin() -> void:
    start_train_id = ""
    _auto_start_pending = false
    _request_vehicle(null)

## The external views of the vehicle (Shift+F4, the vehicle card's Follow) without taking it over -
## the player keeps the cab, and F4 returns to it
func follow_vehicle(vehicle:RID) -> void:
    var node:RailVehicle3D = instance_from_id(RailVehicleServer.vehicle_get_rail_vehicle(vehicle)) as RailVehicle3D
    if not node:
        return
    external_camera.activate(node, (external_camera if external_camera.current else get_camera()).global_transform)
    _set_external_view(true)

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
    if not p_enabled:
        camera.make_current()
    _update_cabin_view()
    external_view_changed.emit(get_external_view_vehicle())

func _update_cabin_view() -> void:
    var in_cabin:bool = controlled_vehicle and not external_camera.current
    if in_cabin == _cabin_view:
        return
    _cabin_view = in_cabin
    get_tree().set_group(MaszynaEnvironmentNode.GROUP, &"cabin_view", in_cabin)
    # scenery models are clicked only while walking (drivermouseinput.cpp:336)
    SceneryHUDMouseServer.set_active(not in_cabin)
    cabin_view_changed.emit(in_cabin)

func get_camera() -> FreeCamera3D:
    if not _camera:
        _camera = get_node("Camera3D") as FreeCamera3D
    return _camera
