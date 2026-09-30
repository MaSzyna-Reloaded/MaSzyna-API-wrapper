extends MaszynaGutTest


func test_engine_gain_uses_rpm_and_load_without_synthetic_sound_state() -> void:
    var controller:VehicleController = build_vehicle()
    controller.power = 1000.0
    controller.apply_configuration()
    var runtime:TrainSoundSystem.BankRuntime = TrainSoundSystem.BankRuntime.new()
    runtime.vehicle_rid = controller.get_rid()
    var source:MmdSoundSourceDefinition = MmdSoundSourceDefinition.new()
    source.amplitude_offset = 0.5
    source.amplitude_factor = 1.5
    var state:Dictionary = {
        "engine_rpm_ratio": 0.8,
        "engine_power": 500.0,
    }

    # level = 0.75 * 0.8 + 0.25 * 0.5 = 0.725
    assert_almost_eq(TrainSoundSystem._engine_gain(runtime, state, source), 1.5875, 0.001)



func test_engine_gain_clamps_to_event_modulation_domain() -> void:
    var controller:VehicleController = build_vehicle()
    controller.power = 1000.0
    controller.apply_configuration()
    var runtime:TrainSoundSystem.BankRuntime = TrainSoundSystem.BankRuntime.new()
    runtime.vehicle_rid = controller.get_rid()
    var source:MmdSoundSourceDefinition = MmdSoundSourceDefinition.new()
    source.amplitude_offset = 0.5
    source.amplitude_factor = 2.0

    assert_almost_eq(TrainSoundSystem._engine_gain(
            runtime, {"engine_rpm_ratio": 2.0, "engine_power": 4000.0}, source), 2.0, 0.001)
    assert_almost_eq(TrainSoundSystem._engine_gain(
            runtime, {"engine_rpm_ratio": -1.0, "engine_power": -100.0}, source), 0.5, 0.001)



func test_internal_sound_is_silent_without_an_occupied_listener_vehicle() -> void:
    var runtime:TrainSoundSystem.BankRuntime = TrainSoundSystem.BankRuntime.new()
    var source:MmdSoundSourceDefinition = MmdSoundSourceDefinition.new()
    source.placement = &"internal"

    assert_almost_eq(TrainSoundSystem._soundproofing(runtime, source), 0.0, 0.001)


const SM42:VehicleModel = preload("res://tests/fixtures/sm42_vehicle.tres")
const PLAYER_SCENE:PackedScene = preload("res://addons/libmaszyna/player/player.tscn")
## Where the camera stands to hear the vehicle at the origin, and where it cannot
const NEAR:Vector3 = Vector3(0.0, 0.0, 10.0)
const FAR:Vector3 = Vector3(0.0, 0.0, 10000.0)
const BATTERY_VOLTAGE:float = 110.0
const BOOKEND_LENGTH:float = 1.0
const MIX_RATE:int = 8000
const MAX_WAIT:float = 2.0
## Long enough for the sweep to have looked at the listener, and for a few trigger ticks
const SETTLE:float = 4.0 * TrainSoundSystem.SWEEP_INTERVAL
const EVENT_TIME_TOLERANCE:float = 0.25

var _vehicle_rid:RID
var _physics_node:VehiclePhysicsNode
var _vehicle:RailVehicle3D
var _camera:Camera3D
var _sound:SfxPlayer3D


## The vehicle leaves before its physics node is freed - it disconnects from its controller then
func after_each() -> void:
    if is_instance_valid(_vehicle):
        remove_child(_vehicle)
        _vehicle.free()
    _vehicle = null


## A vehicle at the origin, not given its controller yet, and the player whose camera is the listener
func _build_vehicle() -> void:
    _physics_node = build_vehicle_node("SoundCullingTest", SM42)
    var controller:VehicleController = _physics_node.get_controller()
    controller.battery_voltage = BATTERY_VOLTAGE
    controller.apply_configuration()
    _vehicle_rid = controller.get_rid()
    _vehicle = RailVehicle3D.new()
    add_child(_vehicle)

    var player:MaszynaPlayer = PLAYER_SCENE.instantiate()
    player.auto_start = false
    add_child_autoqfree(player)
    # on foot the player looks through the free camera
    _camera = player.free_camera


func _attach_controller() -> void:
    _vehicle.controller_path = _vehicle.get_path_to(_physics_node)


## One bank on the vehicle: an "engine" event shaped like the one MmdSoundEventBuilder builds
## (opening bookend at 0, the loop after it) on the battery
func _register_bank(trigger_mode:int) -> void:
    var begin_clip:SfxClip = SfxClip.new()
    begin_clip.stream = _silent_stream()
    var main_clip:SfxClip = SfxClip.new()
    main_clip.stream = _silent_stream()
    main_clip.offset = BOOKEND_LENGTH
    var clips:Array[SfxClip] = [begin_clip, main_clip]
    var event:SfxEvent = SfxEvent.new()
    event.name = &"engine"
    event.clips = clips
    var events:Array[SfxEvent] = [event]
    _sound = SfxPlayer3D.new()
    _sound.bank = SfxBank.new()
    _sound.bank.events = events
    _vehicle.add_child(_sound)
    var soundproofing:Array[PackedFloat32Array] = []
    TrainSoundSystem.register_bank(_sound, {
        "vehicle": _vehicle,
        "soundproofing": soundproofing,
        "triggers": [{
            "state_property": "battery_enabled",
            "trigger_mode": trigger_mode,
            "sound_event": &"engine",
        }],
    })


## The order of the game: the bank is registered while its vehicle is being built, before it has a
## controller (MmdSoundBankInstancer) - the controller comes with the vehicle's own announcement
func _build(trigger_mode:int) -> void:
    _build_vehicle()
    _register_bank(trigger_mode)
    _attach_controller()


func _silent_stream() -> AudioStreamWAV:
    var stream:AudioStreamWAV = AudioStreamWAV.new()
    stream.format = AudioStreamWAV.FORMAT_8_BITS
    stream.mix_rate = MIX_RATE
    var data:PackedByteArray = PackedByteArray()
    data.resize(int(BOOKEND_LENGTH * MIX_RATE))
    stream.data = data
    return stream


func _playing() -> bool:
    return _sound.is_playing(&"engine")


func test_running_sound_plays_again_when_the_camera_comes_back() -> void:
    _build(TrainSoundSystem.TRIGGER_MODE_TOGGLE)
    _camera.global_position = NEAR
    VehicleServer.vehicle_send_command(_vehicle_rid, "battery", true)
    await wait_until(_playing, MAX_WAIT)
    assert_true(_playing(), "heard while the camera is near")

    _camera.global_position = FAR
    await wait_until(func() -> bool: return not _playing(), MAX_WAIT)
    assert_false(_playing(), "silent while the camera is away")

    _camera.global_position = NEAR
    await wait_until(_playing, MAX_WAIT)
    assert_true(_playing(), "heard again once the camera is back")


func test_sound_switched_on_while_heard_plays_its_opening_bookend() -> void:
    _build(TrainSoundSystem.TRIGGER_MODE_TOGGLE)
    _camera.global_position = FAR
    await wait_seconds(SETTLE)
    _camera.global_position = NEAR
    await wait_seconds(SETTLE)
    VehicleServer.vehicle_send_command(_vehicle_rid, "battery", true)
    await wait_until(_playing, MAX_WAIT)

    assert_almost_eq(
            float(_sound.get_event_visualization_state(&"engine")["event_time"]), 0.0,
            EVENT_TIME_TOLERANCE, "starts at its opening bookend")


func test_one_shot_due_while_the_camera_is_away_is_dropped() -> void:
    _build(TrainSoundSystem.TRIGGER_MODE_CHANGE)
    _camera.global_position = NEAR
    await wait_seconds(SETTLE)
    _camera.global_position = FAR
    await wait_seconds(SETTLE)
    VehicleServer.vehicle_send_command(_vehicle_rid, "battery", true)
    await wait_seconds(SETTLE)
    _camera.global_position = NEAR
    await wait_seconds(SETTLE)

    assert_false(_playing(), "the change happened unheard")


func test_bank_registered_after_its_vehicle_has_a_controller_is_heard() -> void:
    _build_vehicle()
    _attach_controller()
    _register_bank(TrainSoundSystem.TRIGGER_MODE_TOGGLE)
    _camera.global_position = NEAR
    VehicleServer.vehicle_send_command(_vehicle_rid, "battery", true)
    await wait_until(_playing, MAX_WAIT)

    assert_true(_playing())
