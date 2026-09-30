extends Node3D
class_name MaszynaPlayer

## The player: what it drives is PlayerServer's, where it looks from PlayerCameraServer's - the
## player follows both with its nodes: its cab camera into the cab of the vehicle driven, the camera
## of the view made current. The cab stays the player's while it looks from outside and its controls
## keep working, as the original's simulation::Train does in its free fly mode (command.cpp:884-889).

@export var start_vehicle_id:String = "":
    set(x):
        if not start_vehicle_id == x:
            start_vehicle_id = x
            if x:
                _auto_start_pending = false
            _dirty = true

## Without a start_vehicle_id the player takes the first vehicle whose simulation is ready. Off where the
## scene names the train itself once its scenery is loaded - a vehicle taken while the scenery still
## loads is not coupled yet, and the cab it activates reaches no other car of its unit.
@export var auto_start:bool = true

## Player's own sounds (the "flashlight" event with a "toggle" automation), provided by the game
@export var sfx_bank:SfxBank

## The cab camera - it sits in the cab interior of the vehicle driven (_show_cabin())
@onready var _cabin_camera:FreeCamera3D = $Camera3D
@onready var train_sound_listener:TrainSoundListener3D = $TrainSoundListener3D
@onready var free_camera:FreeCamera3D = $FreeCamera3D
@onready var external_camera:ExternalCamera3D = $ExternalCamera3D
## Player's head torch - goes with the player's head: the cab camera, or the free camera outside
@onready var headlamp:SpotLight3D = $Camera3D/Headlamp
## Screen-space near-field glow inside the headlamp cone (headlamp_glow.gdshader)
@onready var headlamp_glow:MeshInstance3D = $Camera3D/HeadlampGlow
## Non-positional: the player's own sounds are at the listener, where a 3D player gains nothing
@onready var sfx_player:SfxPlayer = $PlayerSfx
## start_vehicle_id changed - its vehicle is looked for until the scenery has it
var _dirty: bool = true
var _auto_start_pending:bool = false
var _released_vehicle:RID
## The vehicle whose cab interior is shown with the cab camera in it - the one the player drives;
## the cab's widgets take the keys (CabinButton, CabinSwitch), so it stands while the player looks
## from outside too
var _cabin_vehicle:RailVehicle3D
var _follow_jump_distance:float = ProjectSettings.get_setting(FOLLOW_JUMP_DISTANCE_SETTING, FOLLOW_JUMP_DISTANCE_DEFAULT)
## Stepping out of the cab, driver_mode::DistantView(true) (drivermode.cpp:1060): beside the vehicle
## on the side of the occupied cab, this far beyond its width and this high above it [m]
const DISTANT_VIEW_SIDE_MARGIN:float = 1.25
const DISTANT_VIEW_HEIGHT:float = 1.6
## The cab camera moves within the cab's bounds raised this much from the floor and to the ceiling
## the MMD gives [m] (mmd_cabin_definition.gd - the raw bounds)
const CABIN_BOUND_FLOOR_RAISE:float = 0.5
const CABIN_BOUND_CEILING_RAISE:float = 1.8
## What the driver reads when a Radio-Stop brakes the vehicle driven, and for how long [s]
## (TDynamicObject::RadioStop(), DynObj.cpp:7242 - for a vehicle a human drives)
const RADIO_STOP_TRANSCRIPT:String = "!! RADIO-STOP !!"
const RADIO_STOP_TRANSCRIPT_SECONDS:float = 10.0
## A vehicle to follow farther than this from the view is not flown to: the view jumps beside it, where
## the crosshair puts it, and follows from there [m]
const FOLLOW_JUMP_DISTANCE_SETTING:StringName = &"maszyna/camera/follow_jump_distance"
const FOLLOW_JUMP_DISTANCE_DEFAULT:float = 10000.0

func _ready() -> void:
    _auto_start_pending = auto_start and not start_vehicle_id
    sfx_player.bank = sfx_bank
    headlamp.shadow_reverse_cull_face = ProjectSettings.get_setting("maszyna/lights/reverse_cull_face", false)
    # the glow follows the spot: both are children of the camera, so their transform is view space
    var glow_material:ShaderMaterial = (headlamp_glow.mesh as QuadMesh).material as ShaderMaterial
    glow_material.set_shader_parameter(&"light_position", headlamp.position)
    glow_material.set_shader_parameter(&"light_direction", -headlamp.transform.basis.z)
    glow_material.set_shader_parameter(&"light_color", headlamp.light_color)
    CabinHUDMouseSystem.mouse_set_camera(_cabin_camera.get_instance_id())
    PlayerServer.player_vehicle_changed.connect(_on_player_vehicle_changed)
    PlayerCameraServer.camera_changed.connect(_on_camera_changed)
    PlayerCameraServer.camera_placed.connect(_on_camera_placed)
    RailVehicleServer.vehicle_emergency_signal_received.connect(_on_vehicle_emergency_signal_received)
    _show_camera(_mode_camera())


func _exit_tree() -> void:
    PlayerServer.player_vehicle_changed.disconnect(_on_player_vehicle_changed)
    PlayerCameraServer.camera_changed.disconnect(_on_camera_changed)
    PlayerCameraServer.camera_placed.disconnect(_on_camera_placed)
    RailVehicleServer.vehicle_emergency_signal_received.disconnect(_on_vehicle_emergency_signal_received)
    SceneryStreamingServer.streaming_set_camera(null)

func _process(_delta:float) -> void:
    if _dirty:
        _dirty = false
        var vehicle:RailVehicle3D = _find_start_vehicle()
        if vehicle:
            PlayerServer.player_enter_vehicle(vehicle.get_rid())
        elif start_vehicle_id or _auto_start_pending:
            _dirty = true

    var cabin:Cabin3D = _cabin_camera.get_parent() as Cabin3D
    if cabin:
        _cabin_camera.h_offset = cabin.get_camera_shake_offset().x * 1.5
        _cabin_camera.rotation.z = cabin.get_camera_shake_roll()
    else:
        _cabin_camera.h_offset = 0.0
        _cabin_camera.rotation.z = 0.0

## Before the scenery holding the vehicles is freed: the player leaves the cab (the cab camera lives
## in it) and the view stops following, as the followed vehicle goes with the scenery as well
func clear_start_train() -> void:
    start_vehicle_id = ""
    PlayerServer.player_leave_vehicle()
    PlayerCameraServer.camera_set_mode(PlayerCameraServer.CAMERA_MODE_FREE)

func _input(event):
    if CabinHUDMouseSystem.mouse_input(event) or SceneryHUDMouseServer.mouse_input(event):
        get_viewport().set_input_as_handled()
        return
    if event.is_action_pressed("flashlight_toggle", false, true):
        var enabled:bool = headlamp.visible
        headlamp.visible = not enabled
        headlamp_glow.visible = not enabled
        # "toggle" automation: 0 - switching on click, 1 - switching off click
        sfx_player.play_automation(&"flashlight", &"toggle", float(enabled))

    var walking:bool = PlayerCameraServer.camera_get_mode() == PlayerCameraServer.CAMERA_MODE_FREE
    var driven:RID = PlayerServer.player_get_vehicle()
    # on foot, F4 or change_vehicle takes the vehicle in front of the player; F4 only while the
    # player has no cab to go back to
    var picked:RailVehicle3D = _picked_vehicle() if walking and (event.is_action_pressed("change_vehicle")
            or (event.is_action_pressed("cabin_mode_toggle", false, true) and not driven.is_valid())) else null
    if picked:
        PlayerServer.player_enter_vehicle(picked.get_rid())
    elif event.is_action_pressed("cabin_mode_toggle", false, true):
        PlayerCameraServer.camera_toggle_cabin()

    # Train.cpp:6644-6720 - Home (cabchangeforward) / End (cabchangebackward).
    if driven.is_valid() and event.is_action_pressed("cabin_previous"):
        VehicleServer.vehicle_send_command(driven, "cab_change", 1)
    if driven.is_valid() and event.is_action_pressed("cabin_next"):
        VehicleServer.vehicle_send_command(driven, "cab_change", -1)

    if walking:
        _walk_mode_input(event)

    # Train.cpp:1088-1118 - Shift+Q hands the player's train to its driver, Q takes it back
    if driven.is_valid():
        if event.is_action_pressed("ai_driver_enable", false, true):
            # switched off first, as the original does, so that a driver already driving starts over
            DriverSystem.vehicle_set_control_active(driven, false)
            DriverSystem.vehicle_set_control_active(driven, true)
        elif event.is_action_pressed("ai_driver_disable", false, true):
            DriverSystem.vehicle_set_control_active(driven, false)

    # drivermode.cpp:803-804 - Shift+F4 follows the player's train and cycles its views
    if event.is_action_pressed("external_view_cycle", false, true):
        PlayerCameraServer.camera_cycle_follow_view()

## Walk mode: the brake releaser, the manual brake and coupling act on the vehicle nearest to the
## player - TTrain::OnCommand_independentbrakebailoff (Train.cpp:1580-1595),
## OnCommand_manualbrakeincrease/decrease (Train.cpp:1809-1837) via find_nearest_consist_vehicle(),
## OnCommand_nearestcarcouplingincrease/disconnect (Train.cpp:6207-6249) at its nearest coupler.
## Taken here, so the occupied cab does not act on the same key as well.
func _walk_mode_input(event:InputEvent) -> void:
    if event.is_action_pressed("brake_release", false, true):
        _released_vehicle = _find_nearest_vehicle()
        if _released_vehicle.is_valid():
            VehicleServer.vehicle_send_command(_released_vehicle, "brake_releaser", true)
    elif event.is_action_released("brake_release", true) and _released_vehicle.is_valid():
        VehicleServer.vehicle_send_command(_released_vehicle, "brake_releaser", false)
        _released_vehicle = RID()
    elif event.is_action_pressed("manual_brake_increase", true, true):
        _send_to_nearest_train("manual_brake_increase")
    elif event.is_action_pressed("manual_brake_decrease", true, true):
        _send_to_nearest_train("manual_brake_decrease")
    elif event.is_action_pressed("coupler_connect", false, true):
        _send_to_nearest_train("coupler_connect", free_camera.global_position)
    elif event.is_action_pressed("coupler_disconnect", false, true):
        _send_to_nearest_train("coupler_disconnect", free_camera.global_position)
    else:
        return
    get_viewport().set_input_as_handled()


func _send_to_nearest_train(command:String, p1:Variant = null) -> void:
    var vehicle:RID = _find_nearest_vehicle()
    if vehicle.is_valid():
        VehicleServer.vehicle_send_command(vehicle, command, p1)


## TTrain::find_nearest_consist_vehicle() (Train.cpp:1023) scans up to 1500 m for the vehicle
## nearest to the camera.
func _find_nearest_vehicle() -> RID:
    var position:Vector3 = free_camera.global_position
    var nearest:RID = RID()
    var nearest_distance:float = 1500.0
    for vehicle:RID in VehicleServer.vehicle_get_rids():
        var distance:float = position.distance_to(RailVehicleServer.vehicle_get_transform(vehicle).origin)
        if distance < nearest_distance:
            nearest_distance = distance
            nearest = vehicle
    return nearest


## The vehicle with a cab in front of the free camera, or null
func _picked_vehicle() -> RailVehicle3D:
    var detector:ShapeCast3D = free_camera.get_node("RailVehicleDetector")
    if not detector.is_colliding():
        return null
    var node:Node = detector.get_collider(0)
    while node and not node is RailVehicle3D:
        node = node.get_parent()
    var vehicle:RailVehicle3D = node as RailVehicle3D
    return vehicle if vehicle and vehicle.cabin_scene else null


func _find_start_vehicle() -> RailVehicle3D:
    var vehicles:Array[Node] = get_tree().get_root().find_children("", "RailVehicle3D", true, false)
    if start_vehicle_id:
        for node:Node in vehicles:
            var vehicle:RailVehicle3D = node as RailVehicle3D
            if vehicle and _get_vehicle_id(vehicle) == start_vehicle_id:
                return vehicle
        return null

    if _auto_start_pending:
        for node:Node in vehicles:
            var vehicle:RailVehicle3D = node as RailVehicle3D
            if vehicle and VehicleServer.vehicle_is_simulation_ready(vehicle.get_rid()):
                _auto_start_pending = false
                return vehicle
    return null

func _get_vehicle_id(vehicle:RailVehicle3D) -> String:
    return VehicleServer.vehicle_get_name(vehicle.get_rid())

## The keys of the cab's controls act on the vehicle driven, whether the player looks from its cab
## or from outside - the cab logic is the vehicle's (CabinSystem), not the 3D cab's
func _unhandled_input(event:InputEvent) -> void:
    var logic:CabinLogic = CabinSystem.vehicle_get_cab_logic(PlayerServer.player_get_vehicle())
    if logic:
        logic.input(event)


func _on_vehicle_emergency_signal_received(vehicle:RID) -> void:
    if vehicle == PlayerServer.player_get_vehicle():
        TranscriptSystem.add_line(RADIO_STOP_TRANSCRIPT, 0.0, RADIO_STOP_TRANSCRIPT_SECONDS)


## The player sits in the cab of the vehicle taken over and leaves the one let go. Let go while
## looked from its cab, the view steps out first, while the cab camera is still in the cab.
func _on_player_vehicle_changed(vehicle:RID, _previous:RID) -> void:
    if not vehicle.is_valid() and PlayerCameraServer.camera_get_mode() == PlayerCameraServer.CAMERA_MODE_CABIN:
        PlayerCameraServer.camera_set_mode(PlayerCameraServer.CAMERA_MODE_FREE)
    if _cabin_vehicle:
        _hide_cabin()
    if vehicle.is_valid():
        _show_cabin(vehicle)


## The vehicle's cab interior shown, the cab camera in it, where the cab puts the driver
func _show_cabin(vehicle:RID) -> void:
    var node:RailVehicle3D = instance_from_id(RailVehicleServer.vehicle_get_rail_vehicle(vehicle)) as RailVehicle3D
    node.show_cabin()
    var cabin:Cabin3D = node.get_cabin()
    if not cabin:
        return
    _cabin_vehicle = node
    _cabin_camera.reparent(cabin, false)
    cabin.camera_configuration_changed.connect(_on_cabin_camera_configuration_changed)
    _on_cabin_camera_configuration_changed()


## The cab camera taken out of the cab, and its interior freed
func _hide_cabin() -> void:
    var cabin:Cabin3D = _cabin_vehicle.get_cabin()
    cabin.camera_configuration_changed.disconnect(_on_cabin_camera_configuration_changed)
    _cabin_camera.reparent(self)
    _cabin_vehicle.hide_cabin()
    _cabin_vehicle = null


## The cab camera at the driver's place of the cab, within its bounds, looking the way the occupied
## cab faces (drivermode.cpp:1071 VectorFront * CabOccupied - the cab carries the vehicle's turn
## already, so cab 1 turns back from it)
func _on_cabin_camera_configuration_changed() -> void:
    var cabin:Cabin3D = _cabin_camera.get_parent() as Cabin3D
    _cabin_camera.bound_enabled = cabin.get_camera_bound_enabled()
    _cabin_camera.bound_min = cabin.get_camera_bound_min() + Vector3.UP * CABIN_BOUND_FLOOR_RAISE
    _cabin_camera.bound_max = cabin.get_camera_bound_max() + Vector3.UP * CABIN_BOUND_CEILING_RAISE
    _cabin_camera.global_transform = cabin.get_camera_transform()
    var basis:Basis = cabin.global_basis
    _cabin_camera.global_basis = basis if cabin.get_cab_number() < 0 else basis.rotated(Vector3.UP, PI)


## The camera of the view: from the cab camera the free camera steps out beside the vehicle
## (driver_mode::DistantView(true), drivermode.cpp:1054-1069), from the following camera it goes on
## from where that one was and as fast (drivermode.cpp:1093); the following camera flies to the view of
## the vehicle it follows
func _on_camera_changed() -> void:
    var previous:Camera3D = get_viewport().get_camera_3d()
    var camera:Camera3D = _mode_camera()
    if camera == free_camera and previous == _cabin_camera and _cabin_vehicle:
        var body:Transform3D = _cabin_vehicle.global_transform
        var cabin_occupied:int = VehicleServer.vehicle_dump_state(_cabin_vehicle.get_rid()).get("cabin_occupied", 0)
        # MaSzyna's vehicle frame is (left, up, front), Godot vehicles face -Z
        var side:Vector3 = -body.basis.x.normalized() * (1 if cabin_occupied == 0 else cabin_occupied)
        var width:float = VehicleServer.vehicle_get_dimensions(_cabin_vehicle.get_rid()).x
        var head:Vector3 = _cabin_camera.global_position
        var position:Vector3 = Vector3(head.x, body.origin.y, head.z) \
                + side * (width + DISTANT_VIEW_SIDE_MARGIN) + Vector3.UP * DISTANT_VIEW_HEIGHT
        free_camera.global_transform = Transform3D(Basis(), position).looking_at(body.origin)
        free_camera.glide(Vector3.ZERO)
    elif camera == free_camera and previous == external_camera:
        free_camera.global_transform = external_camera.global_transform
        # the free camera zooms by its field of view, the following one by its distance: the view
        # goes on as it was
        free_camera.fov = external_camera.fov
        free_camera.glide(external_camera.velocity)
    elif camera == external_camera:
        var target_rid:RID = PlayerCameraServer.camera_get_target()
        var target:RailVehicle3D = instance_from_id(RailVehicleServer.vehicle_get_rail_vehicle(target_rid)) as RailVehicle3D
        external_camera.view = PlayerCameraServer.camera_get_follow_view() as ExternalCamera3D.View
        # another vehicle, or following anew: its view applied in full, flown to from where the view
        # is - or from beside the vehicle, when it is far; the same one keeps its view and offsets
        if not (previous == external_camera and external_camera.vehicle == target):
            var from:Transform3D = previous.global_transform
            if from.origin.distance_to(target.global_position) > _follow_jump_distance:
                from = PlayerCameraServer.camera_get_show_transform(target_rid)
            external_camera.activate(target, from)
    if not camera == previous:
        _show_camera(camera)


## The free camera where PlayerCameraServer placed it (the vehicle card's crosshair)
func _on_camera_placed(transform:Transform3D) -> void:
    free_camera.global_transform = transform
    free_camera.glide(Vector3.ZERO)
    _show_camera(free_camera)


func _mode_camera() -> Camera3D:
    match PlayerCameraServer.camera_get_mode():
        PlayerCameraServer.CAMERA_MODE_CABIN:
            return _cabin_camera
        PlayerCameraServer.CAMERA_MODE_FOLLOW:
            return external_camera
    return free_camera


## The camera looked through is current and the only one moved by the keys, the scenery streams and
## is picked around it (not from the cab, drivermouseinput.cpp:336), and the player's head torch
## goes with the player's head
func _show_camera(camera:Camera3D) -> void:
    for head:FreeCamera3D in [_cabin_camera, free_camera]:
        head.process_mode = Node.PROCESS_MODE_INHERIT if head == camera else Node.PROCESS_MODE_DISABLED
    camera.make_current()
    if camera is FreeCamera3D and not headlamp.get_parent() == camera:
        headlamp.reparent(camera, false)
        headlamp_glow.reparent(camera, false)
    SceneryStreamingServer.streaming_set_camera(camera)
    var cabin_view:bool = camera == _cabin_camera
    get_tree().set_group(MaszynaEnvironmentNode.GROUP, &"cabin_view", cabin_view)
    SceneryHUDMouseServer.mouse_set_camera(camera.get_instance_id())
    SceneryHUDMouseServer.mouse_set_active(not cabin_view)
