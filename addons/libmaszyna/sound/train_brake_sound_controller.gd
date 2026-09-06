extends Node
class_name TrainBrakeSoundController

@export_node_path("TrainController") var controller_path:NodePath = NodePath("")

var sources:Dictionary = {}
var configured_soundproofing:Array[PackedFloat32Array] = []
var _controller:TrainController
var _player:SfxPlayer3D
var _vehicle:RailVehicle3D
var _bind_timer:Timer
var _events_built:bool = false
var _last_config_fingerprint:Array = []
var _anchored_cabin_instance_id:int = 0

const VOLUME_FACTOR_SETTING:StringName = &"maszyna/sound/brake_volume_factor"
const EXTERIOR_VOLUME_FACTOR_SETTING:StringName = &"maszyna/sound/brake_exterior_volume_factor"
const CABIN_UNIT_SIZE_FACTOR_SETTING:StringName = &"maszyna/sound/brake_cabin_unit_size_factor"
const EXTERIOR_UNIT_SIZE_FACTOR_SETTING:StringName = &"maszyna/sound/brake_exterior_unit_size_factor"

static var DEFAULT_PROOFING:Array[PackedFloat32Array] = [
    PackedFloat32Array([1.0, sqrt(0.2), 1.0, sqrt(0.65), sqrt(0.2), sqrt(0.2)]),
    PackedFloat32Array([sqrt(0.2), 1.0, sqrt(0.2), sqrt(0.65), sqrt(0.65), sqrt(0.65)]),
    PackedFloat32Array([sqrt(0.2), sqrt(0.2), sqrt(0.2), sqrt(0.65), sqrt(0.65), 1.0]),
    PackedFloat32Array([sqrt(0.01), sqrt(0.01), sqrt(0.01), sqrt(0.2), sqrt(0.2), 1.0]),
    PackedFloat32Array([1.0, 1.0, 1.0, 1.0, 1.0, 1.0]),
]


func _ready() -> void:
    _player = get_parent() as SfxPlayer3D
    _vehicle = _player.get_parent() as RailVehicle3D
    _bind_timer = Timer.new()
    _bind_timer.wait_time = 0.05
    _bind_timer.timeout.connect(_bind_controller)
    add_child(_bind_timer)
    _bind_timer.start()


func _bind_controller() -> void:
    if not controller_path:
        return
    _controller = get_node_or_null(controller_path) as TrainController
    if not _controller:
        return
    _bind_timer.stop()
    var sound_context:TrainSoundContext = _controller.get_node("SoundContext") as TrainSoundContext
    sound_context.soundproofing = configured_soundproofing
    _ensure_events_built()
    _controller.config_changed.connect(_ensure_events_built)


func _process(_delta:float) -> void:
    if not _controller or not _events_built:
        return
    _update_spatial_anchors()
    _update_brake_sounds()


func _ensure_events_built() -> void:
    var fingerprint:Array = _config_fingerprint(_controller.config)
    if _events_built and fingerprint == _last_config_fingerprint:
        return
    var built:Array[SfxEvent] = BrakeSfxEventFactory.build_events(sources, _controller.config)
    if built.is_empty():
        return
    var regular_events:Array[SfxEvent] = []
    for event:SfxEvent in _player.bank.events:
        if not BrakeSfxEventFactory.EVENT_PARAMETERS.has(event.name):
            regular_events.append(event)
    regular_events.append_array(built)
    _player.bank.events = regular_events
    _events_built = true
    _last_config_fingerprint = fingerprint


func _config_fingerprint(config:Dictionary) -> Array:
    return [
        config.get("brake_handle_type", TrainBrake.BRAKE_HANDLE_TYPE_NO_HANDLE),
        config.get("brake_local_handle_type", TrainBrake.BRAKE_HANDLE_TYPE_NO_HANDLE),
        config.get("brake_valve_type", TrainBrake.BRAKE_VALVE_NO_VALVE),
        config.get("brake_ep_enabled", false),
        config.get("brake_local_handle_available", false),
    ]


func _update_brake_sounds() -> void:
    var state:Dictionary = _controller.state
    var batch:Dictionary = {}
    for event_name:StringName in BrakeSfxEventFactory.EVENT_PARAMETERS:
        if not _player.bank.get_event(event_name):
            continue
        var event_parameters:Dictionary = {}
        var has_active_parameter:bool = false
        for parameter_name:StringName in BrakeSfxEventFactory.EVENT_PARAMETERS[event_name]:
            var state_key:String = BrakeSfxEventFactory.EVENT_PARAMETERS[event_name][parameter_name]
            var value:float = _parameter_value(state.get(state_key, 0.0))
            event_parameters[parameter_name] = value
            has_active_parameter = has_active_parameter or not is_zero_approx(value)
        event_parameters[&"soundproofing"] = _soundproofing(_primary_label(event_name))
        event_parameters[&"unit_size"] = _unit_size_factor()
        event_parameters[&"gain"] = _volume_factor()
        if not _player.is_playing(event_name) and has_active_parameter:
            _player.play(event_name, event_parameters)
        if _player.is_playing(event_name):
            batch[event_name] = event_parameters
    _player.set_parameters(batch)


func _parameter_value(raw:Variant) -> float:
    if typeof(raw) == TYPE_BOOL:
        return 1.0 if raw else 0.0
    return float(raw) if raw else 0.0


func _primary_label(event_name:StringName) -> String:
    for label:String in BrakeSfxEventFactory.EVENT_LABEL_GROUPS.get(event_name, []):
        if sources.has(label):
            return label
    return ""


func _source(label:String) -> MmdSoundSourceDefinition:
    return sources.get(label) as MmdSoundSourceDefinition


func _inside_source_vehicle() -> bool:
    return not int(_controller.state.get("sound/listener_context", 5)) == 5


func _soundproofing(label:String) -> float:
    var source:MmdSoundSourceDefinition = _source(label)
    if not source:
        return 1.0
    var placement:int = _placement_index(source)
    if placement == 0 and _inside_source_vehicle():
        var cabin_context:int = int(_controller.state.get("sound/listener_context", 0))
        var cabin_proofing:float = _profile_value(placement, cabin_context)
        if source.soundproofing.size() == 6 and not is_equal_approx(source.soundproofing[cabin_context], -1.0):
            cabin_proofing = sqrt(clampf(source.soundproofing[cabin_context], 0.0, 1.0))
        return cabin_proofing
    var listener_vehicle:RailVehicle3D = _listener_vehicle()
    if not listener_vehicle and placement == 0:
        return 0.0
    var listener_controller:TrainController = listener_vehicle.get_controller() if listener_vehicle else null
    var listener_context:int = int(listener_controller.state.get("sound/listener_context", 5)) if listener_controller else 5
    var source_context:int = listener_context if listener_vehicle == _vehicle else 5
    var proofing:float = _profile_value(placement, source_context)
    if source.soundproofing.size() == 6 and not is_equal_approx(source.soundproofing[source_context], -1.0):
        proofing = sqrt(clampf(source.soundproofing[source_context], 0.0, 1.0))
    if listener_vehicle and not listener_vehicle == _vehicle:
        proofing *= _controller_profile_value(listener_controller, 2, listener_context)
    return proofing


func _placement_index(source:MmdSoundSourceDefinition) -> int:
    match source.placement:
        &"internal": return 0
        &"engine": return 1
        &"external": return 2
        &"custom": return 4
    return 3


func _unit_size_factor() -> float:
    if _inside_source_vehicle():
        return float(ProjectSettings.get_setting(CABIN_UNIT_SIZE_FACTOR_SETTING, 2.0))
    return float(ProjectSettings.get_setting(EXTERIOR_UNIT_SIZE_FACTOR_SETTING, 1.0))


func _volume_factor() -> float:
    var factor:float = float(ProjectSettings.get_setting(VOLUME_FACTOR_SETTING, 2.0))
    if not _inside_source_vehicle():
        factor *= float(ProjectSettings.get_setting(EXTERIOR_VOLUME_FACTOR_SETTING, 1.0))
    return factor


func _update_spatial_anchors() -> void:
    var cabin:Cabin3D = _listener_cabin()
    if not cabin or not cabin.get_parent() == _vehicle or cabin.get_instance_id() == _anchored_cabin_instance_id:
        return
    _anchored_cabin_instance_id = cabin.get_instance_id()
    var brake_anchor:Vector3 = _cabin_anchor(cabin, "brakectrl_")
    var local_anchor:Vector3 = _cabin_anchor(cabin, "localbrake_")
    if local_anchor == Vector3.ZERO:
        local_anchor = brake_anchor
    var fallback:Vector3 = (cabin.camera_bound_min + cabin.camera_bound_max) * 0.5 + Vector3.UP
    if brake_anchor == Vector3.ZERO:
        brake_anchor = fallback
    if local_anchor == Vector3.ZERO:
        local_anchor = fallback
    _apply_anchor(&"pipe_hiss", brake_anchor)
    _apply_anchor(&"local_brake_hiss", local_anchor)


func _apply_anchor(event_name:StringName, position:Vector3) -> void:
    var source:MmdSoundSourceDefinition = _source(_primary_label(event_name))
    if not source or not source.offset == Vector3.ZERO:
        return
    var event:SfxEvent = _player.bank.get_event(event_name)
    if event and event.spatial_config:
        event.spatial_config.position = position


func _cabin_anchor(cabin:Cabin3D, prefix:String) -> Vector3:
    var widget:Node = cabin.find_child("%s*" % prefix, true, false)
    if not widget:
        return Vector3.ZERO
    var mesh_path:NodePath = widget.get("mesh_path")
    var mesh:Node3D = widget.get_node_or_null(mesh_path) as Node3D if mesh_path else null
    return _vehicle.to_local(mesh.global_position) if mesh else Vector3.ZERO


func _listener_cabin() -> Cabin3D:
    var camera:Camera3D = get_viewport().get_camera_3d()
    var node:Node = camera
    while node:
        if node is Cabin3D:
            return node as Cabin3D
        node = node.get_parent()
    return null


func _profile_value(placement:int, context:int) -> float:
    return _controller_profile_value(_controller, placement, context)


func _controller_profile_value(controller:TrainController, placement:int, context:int) -> float:
    if controller:
        var profile:Array = controller.config.get("sound/soundproofing", [])
        if profile.size() == 5:
            var value:float = profile[placement][context]
            if not is_equal_approx(value, -1.0):
                return sqrt(clampf(value, 0.0, 1.0))
    return DEFAULT_PROOFING[placement][context]


func _listener_vehicle() -> RailVehicle3D:
    var camera:Camera3D = get_viewport().get_camera_3d()
    var node:Node = camera
    while node:
        if node is RailVehicle3D:
            return node as RailVehicle3D
        node = node.get_parent()
    return null
