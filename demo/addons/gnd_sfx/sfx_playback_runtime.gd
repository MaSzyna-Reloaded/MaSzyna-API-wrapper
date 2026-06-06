extends RefCounted
class_name SfxPlaybackRuntime


class EventInstance:
    var event: SfxEvent
    var event_name: StringName = &""
    var playback_time := 0.0
    var release_time := 0.0
    var parameters: Dictionary = {}
    var status: StringName = &"stopped"
    var adsr_stage: StringName = &"idle"
    var adsr_elapsed := 0.0
    var release_start_gain := 0.0
    var triggered_event_tracks: Array[SfxTrack] = []
    var triggered_sustain_tracks: Array[SfxTrack] = []
    var triggered_automation_tracks := {}


class ActiveVoice:
    var player
    var event_instance: EventInstance
    var track: SfxTrack
    var stream: AudioStream
    var stream_start_position := 0.0
    var stream_end_position := 0.0
    var automation: SfxAutomation
    var automation_name: StringName = &""
    var generator_playback: SfxGeneratorPlayback
    var generator_state
    var generator_stream_playback: AudioStreamGeneratorPlayback
    var stopping := false
    var stop_elapsed := 0.0
    var stop_fade_duration := 0.0
    var player_token := 0
    var creation_order := 0
    var track_adsr_stage: StringName = &"idle"
    var track_adsr_elapsed := 0.0
    var track_release_start_gain := 0.0


signal finished
signal process_requirement_changed(required: bool)

var events: Array[SfxEvent] = []
var _players: Array = []
var _active_voices: Array[ActiveVoice] = []
var _instances: Dictionary = {}
var _player_tokens := {}
var _voice_creation_counter := 0


func set_events(value: Array[SfxEvent]) -> void:
    events = value


func set_players(players: Array) -> void:
    _players = players
    _notify_process_requirement_changed()


func clear() -> void:
    var had_activity := not _active_voices.is_empty() or not _instances.is_empty()
    for voice in _active_voices:
        _cleanup_voice(voice)
    for player in _players:
        _reset_player(player, true)
    _active_voices.clear()
    _instances.clear()
    _notify_process_requirement_changed()
    if had_activity:
        finished.emit()


func handle_player_finished(player) -> void:
    if not is_instance_valid(player):
        return
    if player.playing:
        return
    var token := int(_player_tokens.get(player, 0))
    var voice := _find_voice_by_player_token(player, token)
    if voice == null:
        return
    _release_voice(_find_active_voice_index(player))


func update(delta: float) -> void:
    var active_instances: Array[EventInstance] = []
    for raw_instances in _instances.values():
        for raw_instance in raw_instances:
            var instance := raw_instance as EventInstance
            if instance != null:
                active_instances.append(instance)

    for instance in active_instances:
        _update_instance(instance, delta)

    for index in range(_active_voices.size() - 1, -1, -1):
        _update_voice(index, delta)

    _collect_finished_instances()
    _notify_process_requirement_changed()


func play(event: SfxEvent, offset := 0.0, parameters: Dictionary = {}) -> void:
    if event == null:
        return

    var event_name := event.name
    if not event.polyphony_enabled:
        for instance in _get_instances_for_event(event_name):
            _stop_event_instance(instance, true)

    var instance := EventInstance.new()
    instance.event = event
    instance.event_name = event_name
    instance.playback_time = maxf(offset, 0.0)
    instance.parameters = parameters.duplicate(true)
    instance.status = &"playing"
    _enter_attack(instance)
    _register_instance(instance)

    _refresh_event_tracks(instance, -1.0, instance.playback_time)
    _refresh_automation_tracks(instance, true, {})
    _notify_process_requirement_changed()


func seek(event_name: StringName, offset: float) -> void:
    var instance := _get_latest_instance(event_name)
    if instance == null:
        return

    var rebuilt_parameters: Dictionary = instance.parameters.duplicate(true)
    var adsr_stage: StringName = instance.adsr_stage
    var adsr_elapsed: float = instance.adsr_elapsed
    var release_time: float = instance.release_time
    var release_start_gain: float = instance.release_start_gain
    var was_releasing: bool = instance.status == &"releasing"

    _stop_event_instance(instance, true)

    var rebuilt_instance := EventInstance.new()
    rebuilt_instance.event = instance.event
    rebuilt_instance.event_name = event_name
    rebuilt_instance.playback_time = maxf(offset, 0.0)
    rebuilt_instance.release_time = release_time
    rebuilt_instance.parameters = rebuilt_parameters
    rebuilt_instance.status = &"releasing" if was_releasing else &"playing"
    rebuilt_instance.adsr_stage = adsr_stage
    rebuilt_instance.adsr_elapsed = adsr_elapsed
    rebuilt_instance.release_start_gain = release_start_gain
    rebuilt_instance.triggered_sustain_tracks = instance.triggered_sustain_tracks.duplicate()
    _register_instance(rebuilt_instance)

    _refresh_event_tracks(rebuilt_instance, -1.0, rebuilt_instance.playback_time)
    if was_releasing:
        _refresh_sustain_tracks(rebuilt_instance, -1.0, rebuilt_instance.release_time)
    _refresh_automation_tracks(rebuilt_instance, true, {})
    _notify_process_requirement_changed()


func modulate(event_name: StringName, parameters: Dictionary) -> void:
    var instance := _get_latest_instance(event_name)
    if instance == null:
        return

    var previous_values := {}
    for key in parameters.keys():
        previous_values[key] = instance.parameters.get(key)
        instance.parameters[key] = parameters[key]

    _refresh_automation_tracks(instance, false, previous_values)
    for voice in _active_voices:
        if voice.event_instance == instance and voice.automation != null:
            _apply_voice_state(voice)
    _notify_process_requirement_changed()


func set_parameters(parameters: Dictionary) -> void:
    for event_name in parameters.keys():
        var event_parameters = parameters[event_name]
        if event_parameters is Dictionary:
            modulate(StringName(event_name), event_parameters)


func stop(event_name_or_immediate = null, immediate: bool = false) -> void:
    var event_name := &""
    if event_name_or_immediate is bool:
        immediate = event_name_or_immediate
    elif event_name_or_immediate != null:
        event_name = StringName(event_name_or_immediate)

    if event_name:
        var instance := _get_latest_stoppable_instance(event_name)
        if instance != null:
            _stop_event_instance(instance, immediate)
    else:
        var instances_to_stop: Array[EventInstance] = []
        for raw_instances in _instances.values():
            for raw_instance in raw_instances:
                var instance := raw_instance as EventInstance
                if instance != null:
                    instances_to_stop.append(instance)
        for instance in instances_to_stop:
            _stop_event_instance(instance, immediate)

    _collect_finished_instances()
    _notify_process_requirement_changed()


func play_automation(event: SfxEvent, automation_name: StringName, value: float = 0.0, restart: bool = false) -> void:
    if event == null:
        return

    if restart or not is_playing(event.name):
        play(event, 0.0, {automation_name: value})
        return

    modulate(event.name, {automation_name: value})


func stop_automation(event: SfxEvent, _automation_name: StringName, immediate: bool = false) -> void:
    if event == null:
        return
    stop(event.name, immediate)


func is_playing(event_name: StringName) -> bool:
    return not _get_instances_for_event(event_name).is_empty()


func requires_process() -> bool:
    return not _active_voices.is_empty() or not _instances.is_empty()


func _update_instance(instance: EventInstance, delta: float) -> void:
    if instance == null:
        return

    var previous_time := instance.playback_time
    var previous_release_time := instance.release_time
    if instance.status == &"playing":
        instance.playback_time += delta
        _refresh_event_tracks(instance, previous_time, instance.playback_time)
    elif instance.status == &"releasing":
        instance.release_time += delta
        _refresh_sustain_tracks(instance, previous_release_time, instance.release_time)

    _update_adsr(instance, delta)


func _refresh_event_tracks(instance: EventInstance, previous_time: float, current_time: float) -> void:
    for track in instance.event.tracks:
        if track == null:
            continue
        if track.trigger_mode != SfxTrack.TriggerMode.TRIGGER_TIMELINE:
            continue
        if instance.triggered_event_tracks.has(track):
            continue
        if current_time < track.offset:
            continue
        if previous_time >= 0.0 and previous_time > current_time:
            continue
        if _start_voice(instance, track):
            instance.triggered_event_tracks.append(track)


func _refresh_sustain_tracks(instance: EventInstance, previous_time: float, current_time: float) -> void:
    for track in instance.event.tracks:
        if track == null:
            continue
        if track.trigger_mode != SfxTrack.TriggerMode.TRIGGER_SUSTAIN:
            continue
        if instance.triggered_sustain_tracks.has(track):
            continue
        if current_time < track.offset:
            continue
        if previous_time >= 0.0 and previous_time > current_time:
            continue
        if _start_voice(instance, track):
            instance.triggered_sustain_tracks.append(track)


func _refresh_automation_tracks(instance: EventInstance, initial: bool, previous_values: Dictionary) -> void:
    for automation in instance.event.automations:
        if automation == null:
            continue

        var automation_name := automation.parameter_name
        if String(automation_name).is_empty():
            continue

        var current_value = instance.parameters.get(automation_name, automation.min_domain)
        var previous_value = previous_values.get(automation_name, null)
        if initial or previous_value == null:
            previous_value = automation.min_domain if automation.max_domain >= automation.min_domain else automation.max_domain

        if not instance.triggered_automation_tracks.has(automation_name):
            instance.triggered_automation_tracks[automation_name] = []

        var triggered_tracks: Array = instance.triggered_automation_tracks[automation_name]
        for track in automation.tracks:
            if track == null:
                continue
            if track.trigger_mode != SfxTrack.TriggerMode.TRIGGER_TIMELINE:
                continue
            if triggered_tracks.has(track):
                continue
            if not _automation_threshold_crossed(automation, previous_value, current_value, track.offset, initial):
                continue

            if _start_voice(instance, track, automation):
                triggered_tracks.append(track)


func _automation_threshold_crossed(automation: SfxAutomation, previous_value: float, current_value: float, threshold: float, initial: bool) -> bool:
    var ascending: bool = automation.max_domain >= automation.min_domain
    if initial:
        return current_value >= threshold if ascending else current_value <= threshold
    if ascending:
        return previous_value < threshold and current_value >= threshold
    return previous_value > threshold and current_value <= threshold


func _update_adsr(instance: EventInstance, delta: float) -> void:
    if not instance.event.adsr_enabled:
        instance.adsr_stage = &"sustain"
        instance.adsr_elapsed = 0.0
        return

    instance.adsr_elapsed += delta

    match instance.adsr_stage:
        &"attack":
            if instance.event.attack <= 0.0 or instance.adsr_elapsed >= instance.event.attack:
                instance.adsr_stage = &"decay"
                instance.adsr_elapsed = 0.0
        &"decay":
            if instance.event.decay <= 0.0 or instance.adsr_elapsed >= instance.event.decay:
                instance.adsr_stage = &"sustain"
                instance.adsr_elapsed = 0.0
        &"release":
            if instance.event.release <= 0.0 or instance.adsr_elapsed >= instance.event.release:
                instance.adsr_stage = &"stopped"
                instance.adsr_elapsed = 0.0
                _stop_event_instance(instance, true)


func _enter_attack(instance: EventInstance) -> void:
    if not instance.event.adsr_enabled:
        instance.adsr_stage = &"sustain"
        instance.adsr_elapsed = 0.0
        return

    if instance.event.attack > 0.0:
        instance.adsr_stage = &"attack"
    elif instance.event.decay > 0.0:
        instance.adsr_stage = &"decay"
    else:
        instance.adsr_stage = &"sustain"
    instance.adsr_elapsed = 0.0
    instance.release_start_gain = 0.0


func _current_adsr_gain(instance: EventInstance) -> float:
    if instance == null:
        return 1.0
    if not instance.event.adsr_enabled:
        return 1.0

    match instance.adsr_stage:
        &"attack":
            if instance.event.attack <= 0.0:
                return 1.0
            return clampf(instance.adsr_elapsed / instance.event.attack, 0.0, 1.0)
        &"decay":
            if instance.event.decay <= 0.0:
                return clampf(instance.event.sustain, 0.0, 1.0)
            return lerpf(1.0, clampf(instance.event.sustain, 0.0, 1.0), clampf(instance.adsr_elapsed / instance.event.decay, 0.0, 1.0))
        &"release":
            if instance.event.release <= 0.0:
                return 0.0
            return lerpf(instance.release_start_gain, 0.0, clampf(instance.adsr_elapsed / instance.event.release, 0.0, 1.0))
        &"stopped":
            return 0.0
        _:
            return clampf(instance.event.sustain, 0.0, 1.0)


func _enter_track_attack(voice: ActiveVoice) -> void:
    if voice == null or voice.track == null or not voice.track.adsr_enabled:
        voice.track_adsr_stage = &"sustain"
        voice.track_adsr_elapsed = 0.0
        voice.track_release_start_gain = 0.0
        return

    if voice.track.attack > 0.0:
        voice.track_adsr_stage = &"attack"
    elif voice.track.decay > 0.0:
        voice.track_adsr_stage = &"decay"
    else:
        voice.track_adsr_stage = &"sustain"
    voice.track_adsr_elapsed = 0.0
    voice.track_release_start_gain = 0.0


func _update_track_adsr(voice: ActiveVoice, delta: float) -> void:
    if voice == null or voice.track == null:
        return
    if not voice.track.adsr_enabled:
        voice.track_adsr_stage = &"sustain"
        voice.track_adsr_elapsed = 0.0
        return

    voice.track_adsr_elapsed += delta

    match voice.track_adsr_stage:
        &"attack":
            if voice.track.attack <= 0.0 or voice.track_adsr_elapsed >= voice.track.attack:
                voice.track_adsr_stage = &"decay"
                voice.track_adsr_elapsed = 0.0
        &"decay":
            if voice.track.decay <= 0.0 or voice.track_adsr_elapsed >= voice.track.decay:
                voice.track_adsr_stage = &"sustain"
                voice.track_adsr_elapsed = 0.0
        &"release":
            if voice.track.release <= 0.0 or voice.track_adsr_elapsed >= voice.track.release:
                voice.track_adsr_stage = &"stopped"
                voice.track_adsr_elapsed = 0.0


func _current_track_adsr_gain(voice: ActiveVoice) -> float:
    if voice == null or voice.track == null or not voice.track.adsr_enabled:
        return 1.0

    match voice.track_adsr_stage:
        &"attack":
            if voice.track.attack <= 0.0:
                return 1.0
            return clampf(voice.track_adsr_elapsed / voice.track.attack, 0.0, 1.0)
        &"decay":
            if voice.track.decay <= 0.0:
                return clampf(voice.track.sustain, 0.0, 1.0)
            return lerpf(1.0, clampf(voice.track.sustain, 0.0, 1.0), clampf(voice.track_adsr_elapsed / voice.track.decay, 0.0, 1.0))
        &"release":
            if voice.track.release <= 0.0:
                return 0.0
            return lerpf(voice.track_release_start_gain, 0.0, clampf(voice.track_adsr_elapsed / voice.track.release, 0.0, 1.0))
        &"stopped":
            return 0.0
        _:
            return clampf(voice.track.sustain, 0.0, 1.0)


func _begin_track_release(voice: ActiveVoice) -> bool:
    if voice == null or voice.track == null or not voice.track.adsr_enabled:
        return false
    if voice.track_adsr_stage == &"release" or voice.track_adsr_stage == &"stopped":
        return true
    voice.track_release_start_gain = _current_track_adsr_gain(voice)
    voice.track_adsr_stage = &"release"
    voice.track_adsr_elapsed = 0.0
    return voice.track.release > 0.0


func _find_active_voice_index(player) -> int:
    for index in range(_active_voices.size()):
        if _active_voices[index].player == player:
            return index
    return -1


func _find_voice(instance: EventInstance, track: SfxTrack, automation_name: StringName = &"") -> ActiveVoice:
    for voice in _active_voices:
        if voice.event_instance == instance and voice.track == track and voice.automation_name == automation_name:
            return voice
    return null


func _find_voice_by_player_token(player, token: int) -> ActiveVoice:
    for voice in _active_voices:
        if voice.player == player and voice.player_token == token:
            return voice
    return null


func _get_instances_for_event(event_name: StringName) -> Array[EventInstance]:
    if not _instances.has(event_name):
        return []
    var instances: Array[EventInstance] = []
    for raw_instance in _instances[event_name]:
        var instance := raw_instance as EventInstance
        if instance != null:
            instances.append(instance)
    return instances


func _get_latest_instance(event_name: StringName) -> EventInstance:
    var instances: Array[EventInstance] = _get_instances_for_event(event_name)
    if instances.is_empty():
        return null
    return instances[instances.size() - 1]


func _get_latest_stoppable_instance(event_name: StringName) -> EventInstance:
    var instances: Array[EventInstance] = _get_instances_for_event(event_name)
    for index in range(instances.size() - 1, -1, -1):
        var instance: EventInstance = instances[index]
        if instance != null and instance.status == &"playing":
            return instance
    for index in range(instances.size() - 1, -1, -1):
        var instance: EventInstance = instances[index]
        if instance != null:
            return instance
    return null


func _register_instance(instance: EventInstance) -> void:
    if instance == null:
        return
    if not _instances.has(instance.event_name):
        _instances[instance.event_name] = []
    var instances: Array = _instances[instance.event_name]
    instances.append(instance)
    _instances[instance.event_name] = instances


func _remove_instance(instance: EventInstance) -> void:
    if instance == null:
        return
    if not _instances.has(instance.event_name):
        return
    var instances: Array = _instances[instance.event_name]
    var index := instances.find(instance)
    if index != -1:
        instances.remove_at(index)
    if instances.is_empty():
        _instances.erase(instance.event_name)
    else:
        _instances[instance.event_name] = instances


func _has_instance(instance: EventInstance) -> bool:
    if instance == null:
        return false
    return _get_instances_for_event(instance.event_name).has(instance)


func _get_available_player():
    for player in _players:
        if _find_active_voice_index(player) == -1:
            return player
    return null


func _find_voice_to_steal() -> ActiveVoice:
    var oldest_releasing: ActiveVoice = null
    var oldest_global: ActiveVoice = null
    for voice in _active_voices:
        if voice == null:
            continue
        if oldest_global == null or voice.creation_order < oldest_global.creation_order:
            oldest_global = voice
        if voice.event_instance != null and voice.event_instance.status == &"releasing":
            if oldest_releasing == null or voice.creation_order < oldest_releasing.creation_order:
                oldest_releasing = voice
    return oldest_releasing if oldest_releasing != null else oldest_global


func _acquire_player_for_new_voice():
    var player = _get_available_player()
    if player != null:
        return player

    var victim := _find_voice_to_steal()
    if victim == null or not is_instance_valid(victim.player):
        return null

    var stolen_player = victim.player
    _stop_voice(_find_active_voice_index(stolen_player))
    if _find_active_voice_index(stolen_player) != -1:
        return null
    return stolen_player


func _resolve_audio_bus(track: SfxTrack, automation: SfxAutomation = null) -> StringName:
    if track != null and not String(track.audio_bus).is_empty():
        return track.audio_bus
    if automation != null and not String(automation.audio_bus).is_empty():
        return automation.audio_bus
    return &"Master"


func _get_curve_duration(curve: Curve) -> float:
    if curve == null:
        return 0.0
    return maxf(curve.max_domain - curve.min_domain, 0.0)


func _sample_curve_gain(curve: Curve, elapsed: float) -> float:
    if curve == null:
        return 1.0

    var duration := _get_curve_duration(curve)
    if duration <= 0.0:
        return clampf(curve.sample(curve.max_domain), 0.0, 1.0)


    var sample_position := curve.min_domain + clampf(elapsed, 0.0, duration)
    
    return clampf(curve.sample(sample_position), 0.0, 1.0)


func _sample_time_fade_out_curve(curve: Curve, remaining: float) -> float:
    if curve == null:
        return 1.0

    var duration := _get_curve_duration(curve)
    if duration <= 0.0:
        return clampf(curve.sample(curve.max_domain), 0.0, 1.0)

    var clamped_remaining := clampf(remaining, 0.0, duration)
    var sample_position := curve.min_domain + (duration - clamped_remaining)
    return clampf(curve.sample(sample_position), 0.0, 1.0)


func _sample_automation_curve(curve: Curve, track: SfxTrack, value: float, default_value: float) -> float:
    if curve == null:
        return 1.0

    var sample_position := clampf(value - track.offset, curve.min_domain, curve.max_domain)
    return curve.sample(sample_position)


func _sample_automation_domain_curve(curve: Curve, value: float, default_value: float) -> float:
    if curve == null:
        return 1.0

    var sample_position := clampf(value, curve.min_domain, curve.max_domain)
    return curve.sample(sample_position)


func _sample_automation_fade_out_curve(curve: Curve, track: SfxTrack, value: float, default_value: float) -> float:
    if curve == null:
        return 1.0

    if track.length > 0.0:
        var duration := _get_curve_duration(curve)
        if duration <= 0.0:
            return clampf(curve.sample(curve.max_domain), 0.0, 1.0)

        var fade_end := track.offset + track.length
        var remaining := maxf(fade_end - value, 0.0)
        var clamped_remaining := clampf(remaining, 0.0, duration)
        var sample_position := curve.min_domain + (duration - clamped_remaining)
        return curve.sample(sample_position)

    var local_offset := track.offset

    var local_value := clampf(value - local_offset, curve.min_domain, curve.max_domain)
    var sample_position := curve.min_domain + (curve.max_domain - local_value)
    return curve.sample(sample_position)


func _is_generator_voice(voice: ActiveVoice) -> bool:
    return voice.generator_playback != null and voice.generator_stream_playback != null


func _set_player_gain(player, gain: float) -> void:
    player.volume_db = linear_to_db(maxf(gain, 0.0001))


func _build_generator_context(voice: ActiveVoice, delta: float) -> Dictionary:
    var playback_position := 0.0
    if is_instance_valid(voice.player):
        playback_position = voice.player.get_playback_position()

    var automation_value = null
    if voice.automation != null:
        automation_value = voice.event_instance.parameters.get(voice.automation_name, voice.automation.min_domain)

    return {
        "delta": delta,
        "playback_position": playback_position,
        "event_name": voice.event_instance.event_name,
        "track": voice.track,
        "player": voice.player,
        "stream_playback": voice.generator_stream_playback,
        "event_time": voice.event_instance.playback_time,
        "parameters": voice.event_instance.parameters,
        "automation_name": voice.automation_name,
        "automation_value": automation_value,
    }


func _pump_generator_voice(voice: ActiveVoice, delta: float) -> void:
    if not _is_generator_voice(voice):
        return
    voice.generator_playback.update(voice.generator_state, _build_generator_context(voice, delta))


func _build_track_stream(track: SfxTrack) -> AudioStream:
    if track == null or track.stream == null:
        return null

    if track.stream is AudioStreamGenerator:
        if track.generator_playback == null:
            push_error("AudioStreamGenerator track requires generator_playback")
            return null
        var stream_copy = track.stream.duplicate(true)
        return stream_copy as AudioStream if stream_copy != null else track.stream

    return track.stream


func _resolve_voice_start_position(instance: EventInstance, track: SfxTrack, automation: SfxAutomation = null) -> float:
    var start_position := maxf(track.stream_offset, 0.0)
    if automation == null:
        if track.trigger_mode == SfxTrack.TriggerMode.TRIGGER_SUSTAIN:
            return start_position
        start_position += maxf(instance.playback_time - track.offset, 0.0)
    return start_position


func _resolve_phase_locked_automation_start_position(instance: EventInstance, track: SfxTrack, automation: SfxAutomation, stream: AudioStream) -> float:
    var start_position := maxf(track.stream_offset, 0.0)
    if instance == null or automation == null or stream == null:
        return start_position
    if not automation.phase_locked or automation.phase_period <= 0.0:
        return start_position

    var stream_length := maxf(stream.get_length(), 0.0)
    var available_length := maxf(stream_length - start_position, 0.0)
    if available_length <= 0.0:
        return start_position

    var phase := fposmod(instance.playback_time + track.phase_offset, automation.phase_period)
    if phase > available_length:
        phase = fposmod(phase, available_length)
    return start_position + phase


func _resolve_local_track_time(voice: ActiveVoice, playback_position: float) -> float:
    return maxf(playback_position - voice.stream_start_position, 0.0)


func _resolve_remaining_track_time(voice: ActiveVoice, playback_position: float) -> float:
    if voice.stream == null:
        return 0.0
    var end_position := voice.stream_end_position
    if end_position <= 0.0:
        end_position = maxf(voice.stream.get_length(), 0.0)
    if end_position <= 0.0:
        return 0.0
    return maxf(end_position - playback_position, 0.0)


func _resolve_voice_end_position(track: SfxTrack, stream: AudioStream) -> float:
    if stream == null:
        return 0.0

    var stream_length := maxf(stream.get_length(), 0.0)
    if stream_length <= 0.0:
        return 0.0

    var end_position := stream_length
    if track.length > 0.0:
        end_position = minf(maxf(track.stream_offset, 0.0) + track.length, stream_length)
    return end_position


func _start_voice(instance: EventInstance, track: SfxTrack, automation: SfxAutomation = null) -> bool:
    var automation_name := &""
    if automation != null:
        automation_name = automation.parameter_name
        var existing_voice := _find_voice(instance, track, automation_name)
        if existing_voice != null:
            _apply_voice_state(existing_voice)
            return true
    else:
        var time_voice := _find_voice(instance, track)
        if time_voice != null:
            return true

    var player = _acquire_player_for_new_voice()
    if player == null:
        return false

    var stream := _build_track_stream(track)
    if stream == null:
        return false

    var start_position := _resolve_voice_start_position(instance, track, automation)
    if automation != null:
        start_position = _resolve_phase_locked_automation_start_position(instance, track, automation, stream)
    var stream_length := maxf(stream.get_length(), 0.0)
    if stream_length > 0.0:
        start_position = clampf(start_position, 0.0, stream_length)
    var end_position := _resolve_voice_end_position(track, stream)
    if end_position > 0.0:
        start_position = minf(start_position, end_position)

    player.stream = stream
    player.bus = _resolve_audio_bus(track, automation)
    player.pitch_scale = 1.0
    var player_token := int(_player_tokens.get(player, 0)) + 1
    _player_tokens[player] = player_token
    player.play(start_position)

    var voice := ActiveVoice.new()
    voice.player = player
    voice.event_instance = instance
    voice.track = track
    voice.stream = stream
    voice.stream_start_position = start_position
    voice.stream_end_position = end_position
    voice.automation = automation
    voice.automation_name = automation_name
    voice.player_token = player_token
    voice.creation_order = _voice_creation_counter
    _voice_creation_counter += 1
    _enter_track_attack(voice)

    if stream is AudioStreamGenerator:
        voice.generator_playback = track.generator_playback
        voice.generator_stream_playback = player.get_stream_playback() as AudioStreamGeneratorPlayback
        if voice.generator_stream_playback == null:
            push_error("Failed to get AudioStreamGeneratorPlayback for generator track")
            _reset_player(player, true)
            return false
        voice.generator_state = voice.generator_playback.create_state(voice.generator_stream_playback, track)

    _active_voices.append(voice)
    _apply_voice_state(voice)
    _pump_generator_voice(voice, 0.0)
    return true


func _resolve_automation_raw_gain(voice: ActiveVoice) -> float:
    if voice == null or voice.automation == null or voice.event_instance == null:
        return 1.0

    var automation_value = float(voice.event_instance.parameters.get(voice.automation_name, voice.automation.min_domain))
    var fade_in := clampf(_sample_automation_curve(voice.track.fade_in_curve, voice.track, automation_value, 1.0), 0.0, 1.0)
    var fade_out := clampf(_sample_automation_fade_out_curve(voice.track.fade_out_curve, voice.track, automation_value, 1.0), 0.0, 1.0)

    if voice.automation.crossfade_mode == SfxAutomation.CrossfadeMode.EQUAL_POWER:
        return sqrt(fade_in) * sqrt(fade_out)
    return fade_in * fade_out


func _resolve_automation_track_gain(voice: ActiveVoice) -> float:
    var raw_gain := _resolve_automation_raw_gain(voice)
    if voice == null or voice.automation == null or voice.event_instance == null:
        return raw_gain

    if voice.automation.crossfade_mode == SfxAutomation.CrossfadeMode.EQUAL_POWER:
        var power_sum := 0.0
        for candidate in _active_voices:
            if candidate == null:
                continue
            if candidate.automation == null:
                continue
            if candidate.event_instance != voice.event_instance:
                continue
            if candidate.automation_name != voice.automation_name:
                continue
            var candidate_gain := _resolve_automation_raw_gain(candidate)
            power_sum += candidate_gain * candidate_gain
        if power_sum > 1.0:
            return raw_gain / sqrt(power_sum)
        return raw_gain

    var sum_raw := 0.0
    for candidate in _active_voices:
        if candidate == null:
            continue
        if candidate.automation == null:
            continue
        if candidate.event_instance != voice.event_instance:
            continue
        if candidate.automation_name != voice.automation_name:
            continue
        sum_raw += _resolve_automation_raw_gain(candidate)

    if sum_raw > 1.0:
        return raw_gain / sum_raw
    return raw_gain


func _apply_voice_state(voice: ActiveVoice) -> void:
    if voice == null or not is_instance_valid(voice.player):
        return

    var track_gain := 1.0
    var pitch := 1.0
    if voice.automation:
        var automation_value = float(voice.event_instance.parameters.get(voice.automation_name, voice.automation.min_domain))
        track_gain = _resolve_automation_track_gain(voice)
        pitch = _sample_automation_curve(voice.track.pitch_curve, voice.track, automation_value, 1.0)
        pitch *= _sample_automation_domain_curve(voice.automation.pitch_curve, automation_value, 1.0)
    else:
        var playback_position := 0.0
        if voice.player.playing:
            playback_position = voice.player.get_playback_position()
        var local_track_time := _resolve_local_track_time(voice, playback_position)
        var remaining_track_time := _resolve_remaining_track_time(voice, playback_position)
        track_gain = clampf(_sample_curve_gain(voice.track.fade_in_curve, local_track_time), 0.0, 1.0)
        if voice.stopping:
            var stop_remaining := maxf(voice.stop_fade_duration - voice.stop_elapsed, 0.0)
            track_gain *= clampf(_sample_time_fade_out_curve(voice.track.fade_out_curve, stop_remaining), 0.0, 1.0)
        else:
            track_gain *= clampf(_sample_time_fade_out_curve(voice.track.fade_out_curve, remaining_track_time), 0.0, 1.0)
        pitch = _sample_curve_gain(voice.track.pitch_curve, local_track_time)

    _set_player_gain(voice.player, clampf(_current_adsr_gain(voice.event_instance), 0.0, 1.0) * clampf(_current_track_adsr_gain(voice), 0.0, 1.0) * track_gain)
    voice.player.pitch_scale = maxf(pitch, 0.01)


func _cleanup_voice(voice: ActiveVoice) -> void:
    if voice != null and voice.generator_playback != null:
        voice.generator_playback.cleanup(voice.generator_state)


func _voice_owns_player(voice: ActiveVoice) -> bool:
    if voice == null or not is_instance_valid(voice.player):
        return false
    return int(_player_tokens.get(voice.player, 0)) == voice.player_token


func _release_voice(index: int) -> void:
    if index == -1:
        return

    var voice := _active_voices[index]
    _cleanup_voice(voice)
    if _voice_owns_player(voice):
        voice.player.volume_db = 0.0
        voice.player.pitch_scale = 1.0
    _active_voices.remove_at(index)

    if _active_voices.is_empty() and _instances.is_empty():
        finished.emit()


func _stop_voice(index: int) -> void:
    if index == -1:
        return

    var voice := _active_voices[index]
    var owns_player := _voice_owns_player(voice)
    _release_voice(index)
    if owns_player:
        _reset_player(voice.player, true)


func _reset_player(player, clear_stream := false) -> void:
    if not is_instance_valid(player):
        return
    player.stop()
    player.pitch_scale = 1.0
    player.volume_db = 0.0
    if clear_stream:
        player.stream = null


func _update_voice(index: int, delta: float) -> bool:
    if index < 0 or index >= _active_voices.size():
        return false

    var voice := _active_voices[index]
    if voice.event_instance == null or not _has_instance(voice.event_instance):
        _stop_voice(index)
        return false

    if not is_instance_valid(voice.player) or not voice.player.playing:
        _release_voice(index)
        return false

    if voice.stopping:
        voice.stop_elapsed += delta
        if voice.stop_fade_duration <= 0.0 or voice.stop_elapsed >= voice.stop_fade_duration:
            _stop_voice(index)
            return false

    _update_track_adsr(voice, delta)
    if voice.track_adsr_stage == &"stopped":
        _stop_voice(index)
        return false

    if voice.automation == null and voice.stream_end_position > 0.0 and voice.player.get_playback_position() >= voice.stream_end_position:
        _stop_voice(index)
        return false

    _apply_voice_state(voice)
    _pump_generator_voice(voice, delta)
    return true


func _stop_event_instance(instance: EventInstance, immediate: bool) -> void:
    if instance == null:
        return

    if immediate:
        _stop_instance_voices(instance)
        _remove_instance(instance)
        if _active_voices.is_empty() and _instances.is_empty():
            finished.emit()
        return

    if instance.status == &"releasing":
        return

    instance.status = &"releasing"
    instance.release_time = 0.0
    instance.release_start_gain = _current_adsr_gain(instance)
    if instance.event.adsr_enabled and instance.event.release > 0.0:
        instance.adsr_stage = &"release"
    else:
        instance.adsr_stage = &"sustain"
        _begin_instance_voice_stop(instance)
    instance.adsr_elapsed = 0.0
    _refresh_sustain_tracks(instance, -1.0, instance.release_time)


func _collect_finished_instances() -> void:
    var finished_instances: Array[EventInstance] = []
    for raw_instances in _instances.values():
        for raw_instance in raw_instances:
            var instance := raw_instance as EventInstance
            if instance == null:
                continue

            if instance.status == &"releasing" and instance.adsr_stage == &"stopped":
                finished_instances.append(instance)
                continue

            if instance.status == &"playing":
                continue

            if instance.status == &"releasing" and not _instance_has_pending_release_activity(instance):
                finished_instances.append(instance)
                continue

    for instance in finished_instances:
        _remove_instance(instance)

    if _active_voices.is_empty() and _instances.is_empty():
        finished.emit()


func _notify_process_requirement_changed() -> void:
    process_requirement_changed.emit(requires_process())


func _instance_has_pending_release_activity(instance: EventInstance) -> bool:
    if instance == null:
        return false
    for voice in _active_voices:
        if voice.event_instance == instance:
            return true
    return false


func _stop_instance_voices(instance: EventInstance) -> void:
    for index in range(_active_voices.size() - 1, -1, -1):
        var voice := _active_voices[index]
        if voice.event_instance == instance:
            _stop_voice(index)


func _begin_instance_voice_stop(instance: EventInstance) -> void:
    for index in range(_active_voices.size() - 1, -1, -1):
        var voice := _active_voices[index]
        if voice.event_instance != instance:
            continue
        if not _begin_voice_stop(voice):
            _stop_voice(index)


func _begin_voice_stop(voice: ActiveVoice) -> bool:
    if voice == null or voice.automation != null:
        return false
    if _begin_track_release(voice):
        _apply_voice_state(voice)
        return true
    var fade_duration := _get_curve_duration(voice.track.fade_out_curve)
    if fade_duration <= 0.0:
        return false
    voice.stopping = true
    voice.stop_elapsed = 0.0
    voice.stop_fade_duration = fade_duration
    _apply_voice_state(voice)
    return true
