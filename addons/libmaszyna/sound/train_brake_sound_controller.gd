extends Node
class_name TrainBrakeSoundController

@export_node_path("TrainController") var controller_path:NodePath = NodePath(""):
    set(value):
        if not value == controller_path:
            controller_path = value
            _controller = null
            _dirty = true

var sources:Dictionary = {}
var configured_soundproofing:Array[PackedFloat32Array] = []
var _sound_context:TrainSoundContext
var _controller:TrainController
var _player:SfxPlayer3D
var _vehicle:RailVehicle3D
var _dirty:bool = true
var _last_brake_pressure:float = -1.0
var _last_local_pressure:float = -1.0
var _brake_pressure_change:float = 0.0
var _local_pressure_change:float = 0.0
var _emergency_flow:float = 0.0
var _air_b:float = 0.0
var _air_u:float = 0.0
var _last_cylinder_step:int = -1
var _last_ep_pressure:float = -1.0
var _ep_inc_timer:float = 0.0
var _ep_dec_timer:float = 0.0
var _last_spring_brake:bool = false
var _spring_initialized:bool = false
var _gains:Dictionary = {}
var _anchored_cabin_instance_id:int = 0

const EVENTS:Dictionary = {
    "brake": &"brake_squeal",
    "brakesound": &"brake_friction",
    "brakesound_cab": &"brake_friction_cab",
    "unbrake": &"brake_release_hiss",
    "emergencybrake": &"emergency_brake_hiss",
    "slipperysound": &"wheel_slip_squeal",
    "airsound": &"pipe_hiss_fill",
    "airsound2": &"pipe_hiss_release",
    "airsound3": &"pipe_hiss_e",
    "airsound4": &"pipe_hiss_x",
    "airsound5": &"pipe_hiss_t",
    "localbrakesound": &"local_brake_hiss",
    "localbrakesound2": &"local_brake_hiss2",
    "brakecylinderinc": &"brake_cylinder_increase",
    "brakecylinderdec": &"brake_cylinder_decrease",
    "epbrakeinc": &"ep_brake_increase",
    "epbrakedec": &"ep_brake_decrease",
    "brakeacc": &"brake_accelerator",
    "releaser": &"brake_releaser",
    "springbrake": &"spring_brake_activate",
    "springbrakeoff": &"spring_brake_release",
}

const INTERNAL_LABELS:Array[String] = [
    "brakesound_cab", "airsound", "airsound2", "airsound3", "airsound4", "airsound5",
    "localbrakesound", "localbrakesound2",
]
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


func _process(delta:float) -> void:
    _process_dirty()
    if not _controller or not _player:
        return
    _update_spatial_anchors()
    _update_brake_sounds(delta)


func _process_dirty() -> void:
    if not _dirty:
        return
    _dirty = false
    if controller_path:
        _controller = get_node_or_null(controller_path) as TrainController
        if not _controller:
            _dirty = true
            return
        _sound_context = _controller.get_node("SoundContext") as TrainSoundContext
        _sound_context.soundproofing = configured_soundproofing


func _update_brake_sounds(delta:float) -> void:
    var state:Dictionary = _controller.state
    var pressure:float = float(state.get("brake_air_pressure", 0.0))
    var local_pressure:float = float(state.get("brake_loco_pressure", 0.0))
    var max_pressure:float = maxf(float(state.get("brake_max_cylinder_pressure", 1.0)), 1.0)
    var pressure_ratio:float = maxf(pressure, 0.0) / max_pressure
    var speed:float = float(state.get("speed", 0.0))
    var max_speed:float = maxf(float(state.get("max_speed", 1.0)), 1.0)
    var force:float = float(state.get("brake_unit_force", 0.0))
    var force_ratio:float = float(state.get("brake_force_ratio", 0.0))

    _update_pressure_rates(pressure, local_pressure, delta)
    _update_handle_air(state)
    _update_local_air(pressure, local_pressure, delta)
    _update_emergency(state)
    _update_release(pressure_ratio, delta)
    _update_releaser(state, pressure)
    _update_friction(speed, max_speed, force, force_ratio)
    _update_slip(state, speed, max_speed, force)
    _update_cylinder(pressure_ratio)
    _update_ep(state, delta)
    _update_edges(state)


func _update_pressure_rates(pressure:float, local_pressure:float, delta:float) -> void:
    if _last_brake_pressure >= 0.0:
        _brake_pressure_change = lerpf(_brake_pressure_change, (_last_brake_pressure - pressure) / delta, 0.05)
    if _last_local_pressure >= 0.0:
        _local_pressure_change = lerpf(_local_pressure_change, 10.0 * (local_pressure - _last_local_pressure) / delta, 0.1)
    _last_brake_pressure = pressure
    _last_local_pressure = local_pressure


func _update_handle_air(state:Dictionary) -> void:
    if bool(state.get("brake_handle_fv_sound_model", false)):
        _air_b = lerpf(_air_b, float(state.get("brake_handle_sound_b", 0.0)), 0.05)
        _air_u = lerpf(_air_u, float(state.get("brake_handle_sound_u", 0.0)), 0.25)
        _loop_input("airsound", _air_b > 0.0, _air_b * 0.25, 0.5, 0.05)
        _loop_input("airsound2", _air_u > 0.0, _air_u, 0.5, 0.05)
        _loop_input("airsound3", true, float(state.get("brake_handle_sound_e", 0.0)), 0.5, 0.05)
        _loop_input("airsound4", true, float(state.get("brake_handle_sound_x", 0.0)), 0.5, 0.05)
        _loop_input("airsound5", true, float(state.get("brake_handle_sound_t", 0.0)), 0.5, 0.05)
    else:
        var flow:float = float(state.get("brake_main_valve_flow", 0.0))
        _air_b = (4.0 * _air_b + maxf(flow, 0.0)) / 5.0
        _air_u = (4.0 * _air_u + minf(flow, 0.0)) / 5.0
        _loop_input("airsound", _air_b > 0.0, 2.0 * _air_b, 1.0, 0.05)
        _loop_input("airsound2", _air_u < 0.0, -_air_u, 1.0, 0.01)
        _stop("airsound3")
        _stop("airsound4")
        _stop("airsound5")


func _update_local_air(pressure:float, local_pressure:float, delta:float) -> void:
    var release_gain:float = _source_gain("localbrakesound", -_local_pressure_change * 0.05)
    if _local_pressure_change < -0.05 and local_pressure > pressure - 0.05:
        _loop("localbrakesound", clampf(release_gain, 0.0, 1.5))
    else:
        _fade("localbrakesound", 0.1 * delta)
    var engage_gain:float = _source_gain("localbrakesound2", _local_pressure_change * 0.05)
    if _local_pressure_change > 0.05:
        _loop("localbrakesound2", clampf(engage_gain, 0.0, 1.5))
    else:
        _fade("localbrakesound2", 0.1 * delta)


func _update_emergency(state:Dictionary) -> void:
    var flow:float = float(state.get("brake_emergency_valve_flow", 0.0))
    if flow > 0.025:
        _emergency_flow = flow if _emergency_flow == 0.0 else lerpf(_emergency_flow, flow, 0.1)
        var input:float = clampf(clampf(_emergency_flow, 0.0, 1.0) + clampf(0.1 * float(state.get("pipe_pressure", 0.0)), 0.0, 0.5), 0.0, 1.0)
        _loop("emergencybrake", _source_gain("emergencybrake", input), _source_pitch("emergencybrake", 1.0))
    elif flow < 0.015:
        _emergency_flow = 0.0
        _stop("emergencybrake")


func _update_release(pressure_ratio:float, delta:float) -> void:
    if _brake_pressure_change > 0.05 and pressure_ratio > 0.05:
        var source:MmdSoundSourceDefinition = _source("unbrake")
        if not source:
            return
        var gain:float = source.amplitude_factor * _brake_pressure_change * (0.25 + 0.75 * pressure_ratio)
        _loop("unbrake", gain)
    else:
        _fade("unbrake", 0.5 * delta)


func _update_releaser(state:Dictionary, pressure:float) -> void:
    if bool(state.get("brake_releaser_active", false)):
        _loop("releaser", clampf(pressure * 1.25, 0.0, 1.0))
    else:
        _stop("releaser")


func _update_friction(speed:float, max_speed:float, force:float, ratio:float) -> void:
    if force > 10.0 and speed > 0.05:
        var input:float = sqrt(ratio * lerpf(0.4, 1.0, speed / (1.0 + max_speed)))
        var gain:float = _source_gain("brakesound", input)
        var pitch:float = _source_pitch("brakesound", speed / (1.0 + max_speed))
        var cab_pitch:float = _source_pitch("brakesound_cab", speed / (1.0 + max_speed))
        _loop("brakesound", gain, pitch, pitch)
        _loop("brakesound_cab", _source_gain("brakesound_cab", input), cab_pitch, cab_pitch)
    else:
        _stop("brakesound")
        _stop("brakesound_cab")

    if speed > 2.5:
        var squeal_ratio:float = ratio if force > 10.0 else 0.0
        var squeal_gain:float = _source_gain("brake", lerpf(-1.0, 1.0, squeal_ratio))
        if squeal_gain > 0.075:
            _loop("brake", squeal_gain, _source_pitch("brake", 1.0), speed * 0.01)
            return
    _fade("brake", float(_gains.get("brake", 0.0)) * 2.5 * get_process_delta_time())


func _update_slip(state:Dictionary, speed:float, max_speed:float, force:float) -> void:
    if bool(state.get("slipping_wheels", false)):
        if force > 100.0 and speed > 1.0:
            _loop("slipperysound", _source_gain(
                    "slipperysound", speed / (max_speed * (1.0 + max_speed))))
    else:
        _stop("slipperysound")


func _update_cylinder(pressure_ratio:float) -> void:
    var step:int = int(15.0 * pressure_ratio)
    if _last_cylinder_step >= 0 and not step == _last_cylinder_step:
        var label:String = "brakecylinderinc" if step > _last_cylinder_step else "brakecylinderdec"
        _one_shot(label, _source_pitch(label, 1.0), step * 0.01)
    _last_cylinder_step = step


func _update_ep(state:Dictionary, delta:float) -> void:
    if not bool(state.get("brake_ep_enabled", false)) or not bool(state.get("brake_local_handle_available", false)):
        return
    var max_control:float = maxf(float(state.get("brake_max_control_pressure", 1.0)), 1.0)
    var max_cylinder:float = maxf(float(state.get("brake_max_cylinder_pressure", 1.0)), 1.0)
    var control:float = maxf(float(state.get("brake_control_pressure", 0.0)), 0.0)
    var limiter:float = maxf(float(state.get("brake_local_aeim_position", 0.0)), float(state.get("brake_edb_cylinder_pressure", 0.0)) / max_cylinder)
    var ratio:float = minf(control / max_control, limiter)
    _ep_inc_timer += delta
    _ep_dec_timer += delta
    if _last_ep_pressure >= 0.0:
        var step:int = int(50.0 * ratio)
        var previous_step:int = int(50.0 * maxf(_last_ep_pressure, 0.0) / max_control)
        var change:int = step - previous_step
        if change > 0 and _ep_inc_timer > 0.05:
            _one_shot("epbrakeinc", _source_pitch("epbrakeinc", 1.0), step * 0.01)
            _ep_inc_timer = 0.0
        elif change < 0 and _ep_dec_timer > 0.3:
            _one_shot("epbrakedec", _source_pitch("epbrakedec", 1.0), -change * 0.01)
            _ep_dec_timer = 0.0
    if _ep_inc_timer == 0.0 or _ep_dec_timer == 0.0 or _last_ep_pressure < 0.0:
        _last_ep_pressure = minf(control, float(state.get("brake_local_aeim_position", 0.0)) * max_control)


func _update_edges(state:Dictionary) -> void:
    if bool(state.get("brake_accelerator_sound_active", false)):
        _one_shot("brakeacc")
    var spring:bool = bool(state.get("spring_brake/active", false))
    if _spring_initialized and not spring == _last_spring_brake:
        _stop("springbrakeoff" if spring else "springbrake")
        _one_shot("springbrake" if spring else "springbrakeoff")
    _last_spring_brake = spring
    _spring_initialized = true


func _loop_input(label:String, active:bool, input:float, scale:float, threshold:float) -> void:
    var gain:float = _source_gain(label, input) * scale
    if active and gain > threshold:
        _loop(label, gain)
    else:
        _stop(label)


func _loop(label:String, gain:float, pitch:float = 1.0, combined:float = 1.0) -> void:
    if not _has_source(label) or INTERNAL_LABELS.has(label) and not _inside_source_vehicle():
        _stop(label)
        return
    var event:StringName = EVENTS[label]
    var output_gain:float = gain * _volume_factor()
    var parameters:Dictionary = {
        &"gain": output_gain,
        &"pitch": 1.0 if _is_combined(label) else pitch,
        &"combined": _combined_point(combined),
        &"soundproofing": _soundproofing(label),
        &"unit_size": _unit_size_factor(),
    }
    _gains[label] = gain
    if not _player.is_playing(event):
        _player.play(event, parameters)
    else:
        _player.modulate(event, parameters)


func _one_shot(label:String, pitch:float = 1.0, combined:float = 1.0) -> void:
    if not _has_source(label):
        return
    _player.play(EVENTS[label], {
        &"gain": _volume_factor(),
        &"pitch": 1.0 if _is_combined(label) else pitch,
        &"combined": _combined_point(combined),
        &"soundproofing": _soundproofing(label),
        &"unit_size": _unit_size_factor(),
    })


func _fade(label:String, amount:float) -> void:
    var gain:float = maxf(float(_gains.get(label, 0.0)) - amount, 0.0)
    if gain < 0.05:
        _stop(label)
    else:
        _loop(label, gain)


func _stop(label:String) -> void:
    if not EVENTS.has(label):
        return
    _gains[label] = 0.0
    if _player and _player.is_playing(EVENTS[label]):
        _player.stop(EVENTS[label])


func _source_gain(label:String, input:float) -> float:
    var source:MmdSoundSourceDefinition = _source(label)
    return source.amplitude_offset + source.amplitude_factor * input if source else 0.0


func _source_pitch(label:String, input:float) -> float:
    var source:MmdSoundSourceDefinition = _source(label)
    return source.frequency_offset + source.frequency_factor * input if source else 1.0


func _source(label:String) -> MmdSoundSourceDefinition:
    return sources.get(label) as MmdSoundSourceDefinition


func _has_source(label:String) -> bool:
    var source:MmdSoundSourceDefinition = _source(label)
    if not source:
        return false
    return (not source.sound_main.is_empty() or not source.sound_begin.is_empty()
            or not source.sound_end.is_empty() or not source.chunks.is_empty())


func _is_combined(label:String) -> bool:
    var source:MmdSoundSourceDefinition = _source(label)
    return source and not source.chunks.is_empty()


func _combined_point(value:float) -> float:
    return clampf(value, 0.0, 0.99) * 100.0 if value <= 1.0 else maxf(value, 0.0) * 100.0


func _inside_source_vehicle() -> bool:
    var camera:Camera3D = get_viewport().get_camera_3d()
    if not camera:
        return false
    var node:Node = camera
    while node:
        if node is Cabin3D:
            return node.get_parent() == _vehicle
        node = node.get_parent()
    return false


func _soundproofing(label:String) -> float:
    var source:MmdSoundSourceDefinition = _source(label)
    if not source:
        return 1.0
    var listener_vehicle:RailVehicle3D = _listener_vehicle()
    var listener_controller:TrainController = listener_vehicle.get_controller() if listener_vehicle else null
    var listener_context:int = int(listener_controller.state.get("sound/listener_context", 5)) if listener_controller else 5
    var placement:int = _placement_index(source, label)
    var source_context:int = listener_context if listener_vehicle == _vehicle else 5
    var proofing:float = _profile_value(placement, source_context)
    if source.soundproofing.size() == 6 and not is_equal_approx(source.soundproofing[source_context], -1.0):
        proofing = sqrt(clampf(source.soundproofing[source_context], 0.0, 1.0))
    if listener_vehicle and not listener_vehicle == _vehicle:
        proofing *= _controller_profile_value(listener_controller, 2, listener_context)
    return proofing


func _placement_index(source:MmdSoundSourceDefinition, label:String) -> int:
    match source.placement:
        &"internal": return 0
        &"engine": return 1
        &"external": return 2
        &"custom": return 4
    return 0 if INTERNAL_LABELS.has(label) else 2


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
    for label:String in ["airsound", "airsound2", "airsound3", "airsound4", "airsound5"]:
        _apply_anchor(label, brake_anchor)
    _apply_anchor("localbrakesound", local_anchor)
    _apply_anchor("localbrakesound2", local_anchor)


func _apply_anchor(label:String, position:Vector3) -> void:
    var source:MmdSoundSourceDefinition = _source(label)
    if not source or not source.offset == Vector3.ZERO:
        return
    var event:SfxEvent = _player.bank.get_event(EVENTS[label])
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
