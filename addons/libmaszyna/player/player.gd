extends Node3D
class_name MaszynaPlayer

## The player sits in a vehicle's cab (occupied_cabin) and looks either from it or from outside
## (external_view) - a view only: the cab stays the player's and its controls keep working, as the
## original's simulation::Train does in its free fly mode (command.cpp:884-889). Outside, the view is
## the free camera or follows a vehicle (follow_target_rid) through one of ExternalCamera3D.View.

## The player took another vehicle's cab, or none
signal occupied_cabin_changed
## The view has changed to what external_view, external_view_mode, follow_target_rid and
## external_view_follow_cam say
signal external_view_changed

## The external view flies freely, or follows a vehicle
enum ExternalViewMode {FREE, FOLLOW}

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

## The vehicle whose cab the player sits in - the player's train; null while none
var occupied_cabin:RailVehicle3D
## The player looks from outside the cab, else from occupied_cabin
var external_view:bool = true:
    set(x):
        if not external_view == x:
            external_view = x
            _view_dirty = true
## Outside: the free camera, or the external camera following follow_target_rid. The free camera
## taking over from the external camera goes on from where it is (drivermode.cpp:1093); put it
## somewhere else first to have it there (show_vehicle()).
var external_view_mode:ExternalViewMode = ExternalViewMode.FREE:
    set(x):
        if not external_view_mode == x:
            external_view_mode = x
            _view_dirty = true
## The vehicle the FOLLOW mode follows
var follow_target_rid:RID = RID():
    set(x):
        if not follow_target_rid == x:
            follow_target_rid = x
            _view_dirty = true
## Which of the views of the FOLLOW mode (Shift+F4)
var external_view_follow_cam:ExternalCamera3D.View = ExternalCamera3D.View.CONSIST_FRONT:
    set(x):
        if not external_view_follow_cam == x:
            external_view_follow_cam = x
            _view_dirty = true

var _camera:FreeCamera3D
@onready var train_sound_listener:TrainSoundListener3D = $TrainSoundListener3D
@onready var free_camera:FreeCamera3D = $FreeCamera3D
@onready var external_camera:ExternalCamera3D = $ExternalCamera3D
## Player's head torch - goes with the player's head: the cab camera, or the free camera outside
@onready var headlamp:SpotLight3D = $Camera3D/Headlamp
## Screen-space near-field glow inside the headlamp cone (headlamp_glow.gdshader)
@onready var headlamp_glow:MeshInstance3D = $Camera3D/HeadlampGlow
## Non-positional: the player's own sounds are at the listener, where a 3D player gains nothing
@onready var sfx_player:SfxPlayer = $PlayerSfx
## start_train_id changed - its vehicle is looked for until the scenery has it
var _dirty: bool = true
var _auto_start_pending:bool = false
var _view_dirty:bool = true
var _released_vehicle:RID
## The end vehicle_get_coupled() starts listing a trainset from
const FRONT_END:int = 0
## show_vehicle(): the camera stands this far to the side per metre of the vehicle's length, never
## nearer than the minimum [m], at eye height above the vehicle's origin [m]
const SHOW_DISTANCE_PER_LENGTH:float = 1.2
const SHOW_MIN_DISTANCE:float = 12.0
const SHOW_EYE_HEIGHT:float = 1.75
## Stepping out of the cab, driver_mode::DistantView(true) (drivermode.cpp:1060): beside the vehicle
## on the side of the occupied cab, this far beyond its width and this high above it [m]
const DISTANT_VIEW_SIDE_MARGIN:float = 1.25
const DISTANT_VIEW_HEIGHT:float = 1.6

func _ready() -> void:
    _auto_start_pending = auto_start and not start_train_id
    sfx_player.bank = sfx_bank
    headlamp.shadow_reverse_cull_face = ProjectSettings.get_setting("maszyna/lights/reverse_cull_face", false)
    # the glow follows the spot: both are children of the camera, so their transform is view space
    var glow_material:ShaderMaterial = (headlamp_glow.mesh as QuadMesh).material as ShaderMaterial
    glow_material.set_shader_parameter(&"light_position", headlamp.position)
    glow_material.set_shader_parameter(&"light_direction", -headlamp.transform.basis.z)
    glow_material.set_shader_parameter(&"light_color", headlamp.light_color)
    CabinHUDMouseSystem.set_camera(get_camera().get_instance_id())
    SceneryHUDMouseServer.set_camera(free_camera.get_instance_id())


func _exit_tree() -> void:
    SceneryStreamingServer.set_camera(null)

func _process(_delta:float) -> void:
    if _dirty:
        _dirty = false
        var vehicle:RailVehicle3D = _find_start_vehicle()
        if vehicle:
            enter_cabin(vehicle.get_rid())
        elif start_train_id or _auto_start_pending:
            _dirty = true
    if _view_dirty:
        _view_dirty = false
        _process_view_dirty()

    var camera:FreeCamera3D = get_camera()
    var cabin:Cabin3D = camera.get_parent() as Cabin3D
    if cabin:
        camera.h_offset = cabin.get_camera_shake_offset().x * 1.5
        camera.rotation.z = cabin.get_camera_shake_roll()
    else:
        camera.h_offset = 0.0
        camera.rotation.z = 0.0

## Before the scenery holding the vehicles is freed: the player leaves the cab (the cab camera lives
## in it) and the view stops following, as the followed vehicle goes with the scenery as well
func clear_start_train() -> void:
    start_train_id = ""
    leave_cabin()
    external_view_mode = ExternalViewMode.FREE

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

    var walking:bool = external_view and external_view_mode == ExternalViewMode.FREE
    # on foot, F4 or change_vehicle takes the vehicle in front of the player; F4 only while the
    # player has no cab to go back to
    var picked:RailVehicle3D = _picked_vehicle() if walking and (event.is_action_pressed("change_vehicle")
            or (event.is_action_pressed("cabin_mode_toggle", false, true) and not occupied_cabin)) else null
    if picked:
        enter_cabin(picked.get_rid())
    # drivermode.cpp:1244-1277 InOutKey() - F4: a followed view stops following, as the vehicle
    # card's Follow switched off does; the free camera goes back into the occupied cab; the cab
    # goes out to the free camera
    elif event.is_action_pressed("cabin_mode_toggle", false, true):
        if external_view and external_view_mode == ExternalViewMode.FOLLOW:
            external_view_mode = ExternalViewMode.FREE
        elif external_view and occupied_cabin:
            external_view = false
        elif occupied_cabin:
            free_camera.global_transform = _distant_view()
            external_view_mode = ExternalViewMode.FREE
            external_view = true

    # Train.cpp:6644-6720 - Home (cabchangeforward) / End (cabchangebackward).
    if occupied_cabin and event.is_action_pressed("cabin_previous"):
        RailVehicleServer.vehicle_send_command(occupied_cabin.get_rid(), "cab_change", 1)
    if occupied_cabin and event.is_action_pressed("cabin_next"):
        RailVehicleServer.vehicle_send_command(occupied_cabin.get_rid(), "cab_change", -1)

    if walking:
        _walk_mode_input(event)

    # Train.cpp:1088-1118 - Shift+Q hands the player's train to its driver, Q takes it back
    if occupied_cabin:
        if event.is_action_pressed("ai_driver_enable", false, true):
            # switched off first, as the original does, so that a driver already driving starts over
            DriverSystem.vehicle_set_control_active(occupied_cabin.get_rid(), false)
            DriverSystem.vehicle_set_control_active(occupied_cabin.get_rid(), true)
        elif event.is_action_pressed("ai_driver_disable", false, true):
            DriverSystem.vehicle_set_control_active(occupied_cabin.get_rid(), false)

    # drivermode.cpp:803-804 - Shift+F4 follows the player's train and cycles its views
    if event.is_action_pressed("external_view_cycle", false, true):
        if external_view and external_view_mode == ExternalViewMode.FOLLOW:
            external_view_follow_cam = ((external_view_follow_cam + 1) % ExternalCamera3D.View.size()) as ExternalCamera3D.View
        elif occupied_cabin:
            follow_target_rid = occupied_cabin.get_rid()
            external_view_mode = ExternalViewMode.FOLLOW
            external_view = true

## Walk mode: the brake releaser, the manual brake and coupling act on the vehicle nearest to the
## player - TTrain::OnCommand_independentbrakebailoff (Train.cpp:1580-1595),
## OnCommand_manualbrakeincrease/decrease (Train.cpp:1809-1837) via find_nearest_consist_vehicle(),
## OnCommand_nearestcarcouplingincrease/disconnect (Train.cpp:6207-6249) at its nearest coupler.
## Taken here, so the occupied cab does not act on the same key as well.
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
        _send_to_nearest_train("coupler_connect", free_camera.global_position)
    elif event.is_action_pressed("coupler_disconnect", false, true):
        _send_to_nearest_train("coupler_disconnect", free_camera.global_position)
    else:
        return
    get_viewport().set_input_as_handled()


func _send_to_nearest_train(command:String, p1:Variant = null) -> void:
    var vehicle:RID = _find_nearest_vehicle()
    if vehicle.is_valid():
        RailVehicleServer.vehicle_send_command(vehicle, command, p1)


## TTrain::find_nearest_consist_vehicle() (Train.cpp:1023) scans up to 1500 m for the vehicle
## nearest to the camera.
func _find_nearest_vehicle() -> RID:
    var position:Vector3 = free_camera.global_position
    var nearest:RID = RID()
    var nearest_distance:float = 1500.0
    for vehicle:RID in RailVehicleServer.get_vehicles():
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
                return vehicle
    return null

## The player takes the vehicle over and sits in its cab, leaving the one occupied before; its
## driver, if it has one, only takes orders (drivermode.cpp:266), and the trainset driven before goes
## back to its driver when the vehicle is of another one (simulation.cpp:257-270). The cab occupied
## already is only looked from again.
func enter_cabin(vehicle:RID) -> void:
    var node:RailVehicle3D = instance_from_id(RailVehicleServer.vehicle_get_rail_vehicle(vehicle)) as RailVehicle3D
    if not node:
        return
    if not node == occupied_cabin:
        if occupied_cabin:
            var trainset:Array[RID] = RailVehicleServer.vehicle_get_coupled(
                    occupied_cabin.get_rid(), FRONT_END, RailVehicleController.COUPLING_ELEMENT_COUPLER)
            if not trainset.has(vehicle):
                for trainset_vehicle:RID in trainset:
                    DriverSystem.vehicle_set_control_active(trainset_vehicle, true)
            occupied_cabin.leave_cabin(self)
        DriverSystem.vehicle_set_control_active(vehicle, false)
        occupied_cabin = node
        node.enter_cabin(self)
        occupied_cabin_changed.emit()
    external_view = false

## The player lets the trainset go: its driver drives it again (simulation.cpp:257-270). A player
## looking from the cab steps out beside it (drivermode.cpp:1260); one outside keeps the view
func leave_cabin() -> void:
    if not occupied_cabin:
        return
    for vehicle:RID in RailVehicleServer.vehicle_get_coupled(
            occupied_cabin.get_rid(), FRONT_END, RailVehicleController.COUPLING_ELEMENT_COUPLER):
        DriverSystem.vehicle_set_control_active(vehicle, true)
    if not external_view:
        free_camera.global_transform = _distant_view()
        external_view_mode = ExternalViewMode.FREE
        external_view = true
    var vehicle:RailVehicle3D = occupied_cabin
    occupied_cabin = null
    vehicle.leave_cabin(self)
    occupied_cabin_changed.emit()

## The free camera beside the vehicle, looking at it
func show_vehicle(vehicle:RID) -> void:
    var body:Transform3D = RailVehicleServer.vehicle_get_transform(vehicle)
    var length:float = RailVehicleServer.vehicle_dump_config(vehicle).get("length", 0.0)
    var camera_position:Vector3 = body.origin + body.basis.x.normalized() * maxf(
            SHOW_MIN_DISTANCE, length * SHOW_DISTANCE_PER_LENGTH) + Vector3.UP * SHOW_EYE_HEIGHT
    free_camera.global_transform = Transform3D(Basis(), camera_position).looking_at(
            body.origin + Vector3.UP * SHOW_EYE_HEIGHT)
    free_camera.glide(Vector3.ZERO)
    external_view_mode = ExternalViewMode.FREE
    external_view = true

## The camera the player looks through: the cab camera, the free camera or the external camera
func get_view_camera() -> Camera3D:
    if not external_view:
        return get_camera()
    return external_camera if external_view_mode == ExternalViewMode.FOLLOW else free_camera

## driver_mode::DistantView(true) (drivermode.cpp:1054-1069): beside the occupied vehicle, on the
## side of its occupied cab, where the cab camera is along it, looking at the vehicle
func _distant_view() -> Transform3D:
    var body:Transform3D = occupied_cabin.global_transform
    var cabin_occupied:int = RailVehicleServer.vehicle_dump_state(occupied_cabin.get_rid()).get("cabin_occupied", 0)
    # MaSzyna's vehicle frame is (left, up, front), Godot vehicles face -Z
    var left:Vector3 = -body.basis.x.normalized() * (1 if cabin_occupied == 0 else cabin_occupied)
    var width:float = occupied_cabin.get_controller().dimensions_width
    var head:Vector3 = get_camera().global_position
    var position:Vector3 = Vector3(head.x, body.origin.y, head.z) + left * (width + DISTANT_VIEW_SIDE_MARGIN) \
            + Vector3.UP * DISTANT_VIEW_HEIGHT
    return Transform3D(Basis(), position).looking_at(body.origin)

func _get_vehicle_train_id(vehicle:RailVehicle3D) -> String:
    var controller:VehicleController = vehicle.get_controller()
    return controller.train_id if controller else ""

## The view as the properties say: the camera looked through is current and the only one moved by
## the keys, the scenery streams around it, and the player's head torch goes with the player's head
func _process_view_dirty() -> void:
    var previous:Camera3D = get_viewport().get_camera_3d()
    var camera:Camera3D = get_view_camera()
    if camera == external_camera:
        external_camera.view = external_view_follow_cam
        var target:RailVehicle3D = instance_from_id(
                RailVehicleServer.vehicle_get_rail_vehicle(follow_target_rid)) as RailVehicle3D
        # from the free camera the view follows from where it is, until Shift+F4 selects a view
        if previous == free_camera:
            external_camera.attach(target, previous.global_transform)
        elif not previous == external_camera or not external_camera.vehicle == target:
            external_camera.activate(target, previous.global_transform)
    elif camera == free_camera and previous == external_camera:
        free_camera.global_transform = external_camera.global_transform
        free_camera.glide(external_camera.velocity)
    for head:FreeCamera3D in [get_camera(), free_camera]:
        head.process_mode = Node.PROCESS_MODE_INHERIT if head == camera else Node.PROCESS_MODE_DISABLED
    camera.make_current()
    if camera is FreeCamera3D and not headlamp.get_parent() == camera:
        headlamp.reparent(camera, false)
        headlamp_glow.reparent(camera, false)
    SceneryStreamingServer.set_camera(camera)
    get_tree().set_group(MaszynaEnvironmentNode.GROUP, &"cabin_view", not external_view)
    # scenery models are clicked only while walking (drivermouseinput.cpp:336)
    SceneryHUDMouseServer.set_active(external_view)
    external_view_changed.emit()

## The cab camera - it is put into the occupied cab (RailVehicle3D.enter_cabin())
func get_camera() -> FreeCamera3D:
    if not _camera:
        _camera = get_node("Camera3D") as FreeCamera3D
    return _camera
