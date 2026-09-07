extends Node

const TRIGGER_INTERVAL:float = 0.05
const TRIGGER_MODE_TOGGLE:int = 0
const TRIGGER_MODE_CONTINUOUS:int = 1
const EXTERIOR_CONTEXT:int = 5
const VOLUME_FACTOR_SETTING:StringName = &"maszyna/sound/brake_volume_factor"
const EXTERIOR_VOLUME_FACTOR_SETTING:StringName = &"maszyna/sound/brake_exterior_volume_factor"
const CABIN_UNIT_SIZE_FACTOR_SETTING:StringName = &"maszyna/sound/brake_cabin_unit_size_factor"
const EXTERIOR_UNIT_SIZE_FACTOR_SETTING:StringName = &"maszyna/sound/brake_exterior_unit_size_factor"

var DEFAULT_PROOFING:Array[PackedFloat32Array] = [
    PackedFloat32Array([1.0, sqrt(0.2), 1.0, sqrt(0.65), sqrt(0.2), sqrt(0.2)]),
    PackedFloat32Array([sqrt(0.2), 1.0, sqrt(0.2), sqrt(0.65), sqrt(0.65), sqrt(0.65)]),
    PackedFloat32Array([sqrt(0.2), sqrt(0.2), sqrt(0.2), sqrt(0.65), sqrt(0.65), 1.0]),
    PackedFloat32Array([sqrt(0.01), sqrt(0.01), sqrt(0.01), sqrt(0.2), sqrt(0.2), 1.0]),
    PackedFloat32Array([1.0, 1.0, 1.0, 1.0, 1.0, 1.0]),
]

class BankRuntime extends RefCounted:
    var player:SfxPlayer3D
    var vehicle:RailVehicle3D
    var controller:TrainController
    var cabin_only:bool = false
    var enabled:bool = true
    var brake_sources:Dictionary = {}
    var soundproofing:Array[PackedFloat32Array] = []
    var triggers:Array[Dictionary] = []
    var trigger_states:Dictionary = {}
    var trigger_elapsed:float = 0.0
    var events_built:bool = false
    var anchored_cabin_instance_id:int = 0


var _banks:Dictionary = {}
var _listener:TrainSoundListener3D
var _next_trigger_id:int = 1


func set_listener(listener:TrainSoundListener3D) -> void:
    if _listener:
        _listener.context_changed.disconnect(_refresh_context)
    _listener = listener
    _listener.context_changed.connect(_refresh_context)
    _refresh_context()


func clear_listener(listener:TrainSoundListener3D) -> void:
    if not _listener == listener:
        return
    _listener.context_changed.disconnect(_refresh_context)
    _listener = null
    _refresh_context()


func register_bank(player:SfxPlayer3D, registration:Dictionary) -> void:
    var bank_id:int = player.get_instance_id()
    var runtime:BankRuntime = _banks.get(bank_id) as BankRuntime
    if not runtime:
        runtime = BankRuntime.new()
        runtime.player = player
        _banks[bank_id] = runtime
        player.tree_exiting.connect(_unregister_bank.bind(bank_id))
    runtime.vehicle = registration.get("vehicle") as RailVehicle3D
    runtime.cabin_only = bool(registration.get("cabin_only", false))
    runtime.enabled = not runtime.cabin_only
    runtime.brake_sources = registration.get("brake_sources", {})
    runtime.soundproofing = registration.get("soundproofing", [])
    for descriptor:Dictionary in registration.get("triggers", []):
        _add_trigger(runtime, descriptor)
    _resolve_controller(runtime)
    _refresh_bank_context(runtime)


func register_trigger(player:SfxPlayer3D, descriptor:Dictionary) -> int:
    var bank_id:int = player.get_instance_id()
    var runtime:BankRuntime = _banks.get(bank_id) as BankRuntime
    if not runtime:
        runtime = BankRuntime.new()
        runtime.player = player
        runtime.vehicle = descriptor.get("vehicle") as RailVehicle3D
        runtime.controller = descriptor.get("controller") as TrainController
        _banks[bank_id] = runtime
        player.tree_exiting.connect(_unregister_bank.bind(bank_id))
    return _add_trigger(runtime, descriptor)


func unregister_trigger(player:SfxPlayer3D, trigger_id:int) -> void:
    var runtime:BankRuntime = _banks.get(player.get_instance_id()) as BankRuntime
    if not runtime:
        return
    for index:int in range(runtime.triggers.size() - 1, -1, -1):
        if int(runtime.triggers[index]["id"]) == trigger_id:
            runtime.triggers.remove_at(index)
            break
    runtime.trigger_states.erase(trigger_id)


func _process(delta:float) -> void:
    var states:Dictionary = {}
    for runtime:BankRuntime in _banks.values():
        if not is_instance_valid(runtime.controller):
            _resolve_controller(runtime)
        if not runtime.controller or not runtime.enabled:
            continue
        _ensure_brake_events(runtime)
        var controller_id:int = runtime.controller.get_instance_id()
        if not states.has(controller_id):
            states[controller_id] = runtime.controller.state
        var state:Dictionary = states[controller_id]
        var batch:Dictionary = {}
        _update_brake_sounds(runtime, state, batch)
        runtime.trigger_elapsed += delta
        if runtime.trigger_elapsed >= TRIGGER_INTERVAL:
            runtime.trigger_elapsed = fmod(runtime.trigger_elapsed, TRIGGER_INTERVAL)
            _update_triggers(runtime, state, batch)
        runtime.player.set_parameters(batch)


func _add_trigger(runtime:BankRuntime, descriptor:Dictionary) -> int:
    var trigger:Dictionary = descriptor.duplicate()
    var trigger_id:int = int(trigger.get("id", 0))
    if trigger_id == 0:
        trigger_id = _next_trigger_id
        _next_trigger_id += 1
    trigger["id"] = trigger_id
    runtime.triggers.append(trigger)
    runtime.trigger_states[trigger_id] = false
    return trigger_id


func _resolve_controller(runtime:BankRuntime) -> void:
    if runtime.vehicle:
        runtime.controller = runtime.vehicle.get_controller()


func _ensure_brake_events(runtime:BankRuntime) -> void:
    if runtime.events_built or runtime.brake_sources.is_empty():
        return
    var built:Array[SfxEvent] = BrakeSfxEventFactory.build_events(
            runtime.brake_sources, runtime.controller.config)
    if built.is_empty():
        return
    var events:Array[SfxEvent] = runtime.player.bank.events.duplicate()
    events.append_array(built)
    runtime.player.bank.events = events
    runtime.events_built = true
    runtime.anchored_cabin_instance_id = 0
    _update_spatial_anchors(runtime)


func _update_brake_sounds(runtime:BankRuntime, state:Dictionary, batch:Dictionary) -> void:
    for event_name:StringName in BrakeSfxEventFactory.EVENT_PARAMETERS:
        if not runtime.player.bank.get_event(event_name):
            continue
        var event_parameters:Dictionary = {}
        var has_active_parameter:bool = false
        for parameter_name:StringName in BrakeSfxEventFactory.EVENT_PARAMETERS[event_name]:
            var state_key:String = BrakeSfxEventFactory.EVENT_PARAMETERS[event_name][parameter_name]
            var value:float = _parameter_value(state.get(state_key, 0.0))
            event_parameters[parameter_name] = value
            has_active_parameter = has_active_parameter or not is_zero_approx(value)
        event_parameters[&"soundproofing"] = _soundproofing(runtime, _primary_source(runtime, event_name))
        event_parameters[&"unit_size"] = _unit_size_factor(runtime)
        event_parameters[&"gain"] = _volume_factor(runtime)
        if not runtime.player.is_playing(event_name) and has_active_parameter:
            runtime.player.play(event_name, event_parameters)
        if runtime.player.is_playing(event_name):
            batch[event_name] = event_parameters


func _update_triggers(runtime:BankRuntime, state:Dictionary, batch:Dictionary) -> void:
    for trigger:Dictionary in runtime.triggers:
        var trigger_id:int = int(trigger["id"])
        var raw_value:Variant = state.get(String(trigger.get("state_property", "")), 0.0)
        var value:float = _parameter_value(raw_value)
        var should_play:bool = (
                value <= float(trigger.get("trigger_threshold_max", 1.0))
                and value >= float(trigger.get("trigger_threshold_min", 0.0)))
        var trigger_mode:int = int(trigger.get("trigger_mode", TRIGGER_MODE_TOGGLE))
        if trigger_mode == TRIGGER_MODE_TOGGLE:
            should_play = not is_zero_approx(value) and should_play
        var activated:bool = bool(runtime.trigger_states.get(trigger_id, false))
        var event_name:StringName = StringName(trigger.get("sound_event", &""))
        var parameter_name:StringName = StringName(trigger.get("sound_parameter", &""))
        var parameters:Dictionary = {}
        if trigger_mode == TRIGGER_MODE_CONTINUOUS and parameter_name:
            parameters[parameter_name] = value
        var source:MmdSoundSourceDefinition = trigger.get("source") as MmdSoundSourceDefinition
        if source:
            if source.label == "engine":
                parameters[&"engine_gain"] = _engine_gain(runtime, state, source)
            parameters[&"soundproofing"] = _soundproofing(runtime, source)
        else:
            var placement:StringName = StringName(trigger.get("sound_placement", &"general"))
            if not placement == &"general":
                parameters[&"soundproofing"] = _placement_soundproofing(runtime, placement)
        if should_play and not activated:
            runtime.player.play(event_name, parameters)
            runtime.trigger_states[trigger_id] = true
        elif not should_play and activated:
            runtime.player.stop(event_name, false)
            runtime.trigger_states[trigger_id] = false
        if should_play and not parameters.is_empty() and runtime.player.is_playing(event_name):
            batch[event_name] = parameters


func _engine_gain(
        runtime:BankRuntime, state:Dictionary, source:MmdSoundSourceDefinition) -> float:
    var rpm_ratio:float = clampf(float(state.get("engine_rpm_ratio", 0.0)), 0.0, 1.0)
    var nominal_power:float = runtime.controller.get_power()
    var load_ratio:float = 0.0
    if nominal_power > 0.0:
        load_ratio = maxf(float(state.get("engine_power", 0.0)) / nominal_power, 0.0)
    var level:float = 0.25 * load_ratio + 0.75 * rpm_ratio
    return clampf(source.amplitude_offset + source.amplitude_factor * level, 0.0, 2.0)


func _refresh_context() -> void:
    for runtime:BankRuntime in _banks.values():
        _refresh_bank_context(runtime)


func _refresh_bank_context(runtime:BankRuntime) -> void:
    var enabled:bool = not runtime.cabin_only or _inside_vehicle(runtime.vehicle)
    if not runtime.enabled == enabled:
        runtime.enabled = enabled
        if not enabled:
            runtime.player.stop(false)
            for trigger_id:Variant in runtime.trigger_states:
                runtime.trigger_states[trigger_id] = false
    if enabled:
        _update_spatial_anchors(runtime)


func _inside_vehicle(vehicle:RailVehicle3D) -> bool:
    return not _listener == null and not _listener.listener_cabin == null \
            and _listener.listener_vehicle == vehicle


func _soundproofing(runtime:BankRuntime, source:MmdSoundSourceDefinition) -> float:
    if not source:
        return 1.0
    if source.placement == &"general":
        return 1.0
    return _placement_soundproofing(runtime, source.placement, source.soundproofing)


func _placement_soundproofing(
        runtime:BankRuntime, placement_name:StringName,
        source_profile:PackedFloat32Array = PackedFloat32Array()) -> float:
    var placement:int = _placement_index(placement_name)
    var inside_source:bool = _inside_vehicle(runtime.vehicle)
    if placement == 0 and inside_source:
        return _source_profile_value(
                source_profile, runtime.soundproofing, placement, _listener.listener_context)
    if not _listener or not _listener.listener_vehicle:
        return 0.0 if placement == 0 else _profile_value(runtime.soundproofing, placement, EXTERIOR_CONTEXT)
    var source_context:int = _listener.listener_context if inside_source else EXTERIOR_CONTEXT
    var proofing:float = _source_profile_value(
            source_profile, runtime.soundproofing, placement, source_context)
    if not _listener.listener_vehicle == runtime.vehicle:
        proofing *= _profile_value(
                _vehicle_profile(_listener.listener_vehicle), 2, _listener.listener_context)
    return proofing


func _source_profile_value(
        source_profile:PackedFloat32Array, profile:Array[PackedFloat32Array],
        placement:int, context:int) -> float:
    if source_profile.size() == 6 and not is_equal_approx(source_profile[context], -1.0):
        return sqrt(clampf(source_profile[context], 0.0, 1.0))
    return _profile_value(profile, placement, context)


func _profile_value(profile:Array[PackedFloat32Array], placement:int, context:int) -> float:
    if profile.size() == 5:
        var value:float = profile[placement][context]
        if not is_equal_approx(value, -1.0):
            return sqrt(clampf(value, 0.0, 1.0))
    return DEFAULT_PROOFING[placement][context]


func _vehicle_profile(vehicle:RailVehicle3D) -> Array[PackedFloat32Array]:
    for runtime:BankRuntime in _banks.values():
        if runtime.vehicle == vehicle:
            return runtime.soundproofing
    return []


func _placement_index(placement:StringName) -> int:
    match placement:
        &"internal": return 0
        &"engine": return 1
        &"external": return 2
        &"custom": return 4
    return 3


func _unit_size_factor(runtime:BankRuntime) -> float:
    if _inside_vehicle(runtime.vehicle):
        return float(ProjectSettings.get_setting(CABIN_UNIT_SIZE_FACTOR_SETTING, 2.0))
    return float(ProjectSettings.get_setting(EXTERIOR_UNIT_SIZE_FACTOR_SETTING, 1.0))


func _volume_factor(runtime:BankRuntime) -> float:
    var factor:float = float(ProjectSettings.get_setting(VOLUME_FACTOR_SETTING, 2.0))
    if not _inside_vehicle(runtime.vehicle):
        factor *= float(ProjectSettings.get_setting(EXTERIOR_VOLUME_FACTOR_SETTING, 1.0))
    return factor


func _primary_source(runtime:BankRuntime, event_name:StringName) -> MmdSoundSourceDefinition:
    for label:String in BrakeSfxEventFactory.EVENT_LABEL_GROUPS.get(event_name, []):
        if runtime.brake_sources.has(label):
            return runtime.brake_sources[label] as MmdSoundSourceDefinition
    return null


func _update_spatial_anchors(runtime:BankRuntime) -> void:
    var cabin:Cabin3D = _listener.listener_cabin if _listener else null
    if not cabin or not cabin.get_parent() == runtime.vehicle \
            or cabin.get_instance_id() == runtime.anchored_cabin_instance_id:
        return
    runtime.anchored_cabin_instance_id = cabin.get_instance_id()
    var brake_anchor:Vector3 = _cabin_anchor(runtime.vehicle, cabin, "brakectrl_")
    var local_anchor:Vector3 = _cabin_anchor(runtime.vehicle, cabin, "localbrake_")
    if local_anchor == Vector3.ZERO:
        local_anchor = brake_anchor
    var fallback:Vector3 = (cabin.camera_bound_min + cabin.camera_bound_max) * 0.5 + Vector3.UP
    if brake_anchor == Vector3.ZERO:
        brake_anchor = fallback
    if local_anchor == Vector3.ZERO:
        local_anchor = fallback
    _apply_anchor(runtime, &"pipe_hiss", brake_anchor)
    _apply_anchor(runtime, &"local_brake_hiss", local_anchor)


func _apply_anchor(runtime:BankRuntime, event_name:StringName, position:Vector3) -> void:
    var source:MmdSoundSourceDefinition = _primary_source(runtime, event_name)
    if not source or not source.offset == Vector3.ZERO:
        return
    var event:SfxEvent = runtime.player.bank.get_event(event_name)
    if event and event.spatial_config:
        event.spatial_config.position = position


func _cabin_anchor(vehicle:RailVehicle3D, cabin:Cabin3D, prefix:String) -> Vector3:
    var widget:Node = cabin.find_child("%s*" % prefix, true, false)
    if not widget:
        return Vector3.ZERO
    var mesh_path:NodePath = widget.get("mesh_path")
    var mesh:Node3D = widget.get_node_or_null(mesh_path) as Node3D if mesh_path else null
    return vehicle.to_local(mesh.global_position) if mesh else Vector3.ZERO


func _parameter_value(raw:Variant) -> float:
    if typeof(raw) == TYPE_BOOL:
        return 1.0 if raw else 0.0
    return float(raw) if raw else 0.0


func _unregister_bank(bank_id:int) -> void:
    _banks.erase(bank_id)
