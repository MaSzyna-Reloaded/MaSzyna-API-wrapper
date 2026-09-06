extends RefCounted
class_name BrakeSfxEventFactory

## Builds gnd-sfx SfxEvents for a vehicle's brake-related MMD sound labels, once, from parsed
## MmdSoundSourceDefinitions + TrainController.config (brake hardware facts). Composes related
## labels (begin/middle/end phases, or same-physical-effect variants) into one multi-automation
## SfxEvent each, per the brake-sound redesign plan's composition map - NOT one event per label.
##
## The whole model, same as the proven engine.tres reference: an event's SfxAutomation.parameter_name
## is always a plain float signal (a physical quantity like pressure/force/speed/flow, OR - when
## that's genuinely the right model for a discrete one-shot, e.g. a click - a 0/1 pulse; never a
## pre-combined "gain" value). One or more tracks/clips with real fade_in_curve/fade_out_curve do
## ALL the continuous gain/pitch shaping, exactly like engine.tres's rpm-indexed tracks; a
## pulse-domain clip (offset ~0.5 in a [0,1] domain, cut=false) is the same mechanism used for a
## one-shot, since gnd-sfx's own automation-domain-entry tracking already triggers it exactly once
## per pulse with no runtime edge-detection needed.
##
## Runtime control (TrainBrakeSoundController) is correspondingly trivial: play() each event once,
## lazily, the first time it's fed (never stop() - an inactive automation costs nothing, and the
## curves themselves fade gain to ~0 at rest), then just copy TrainController.state values into
## modulate()/set_parameters() every frame. It never decides gain/pitch/which-sample-plays itself.

## event_name -> ordered MMD labels composing it (first one present in a vehicle's `sources` wins
## as the "primary" label for purposes like soundproofing/placement lookups that need exactly one -
## see TrainBrakeSoundController). Mirrors MmdSoundCatalog's own label -> event_name map; kept here
## too (not just derived from the catalog) since it's this factory's own composition decision.
const EVENT_LABEL_GROUPS := {
    &"brake_shoe": ["brakesound", "brake"],
    &"brake_release_hiss": ["unbrake"],
    &"emergency_brake_hiss": ["emergencybrake"],
    &"wheel_slip_squeal": ["slipperysound"],
    &"brake_releaser": ["releaser"],
    &"brake_accelerator": ["brakeacc"],
    &"brake_cylinder_click": ["brakecylinderinc", "brakecylinderdec"],
    &"ep_brake_click": ["epbrakeinc", "epbrakedec"],
    &"springbrake": ["springbrake", "springbrakeoff"],
    &"local_brake_hiss": ["localbrakesound", "localbrakesound2"],
    &"pipe_hiss": ["airsound", "airsound2", "airsound3", "airsound4", "airsound5"],
}

const EVENT_PARAMETERS := {
    &"brake_shoe": {&"speed": "speed", &"force_ratio": "brake_force_ratio"},
    &"emergency_brake_hiss": {&"brake_emergency_valve_flow": "brake_emergency_valve_flow"},
    &"wheel_slip_squeal": {&"slipping_wheels": "slipping_wheels"},
    &"brake_releaser": {
        &"brake_releaser_active": "brake_releaser_active",
        &"brake_air_pressure": "brake_air_pressure",
    },
    &"springbrake": {&"spring_brake_active": "spring_brake/active"},
    &"local_brake_hiss": {&"brake_local_valve_flow": "brake_local_valve_flow"},
    &"pipe_hiss": {
        &"brake_main_valve_flow": "brake_main_valve_flow",
        &"brake_controller_position": "brake_controller_position",
        &"brake_control_pressure": "brake_control_pressure",
    },
}
const CONTINUOUS_RELEASE_SECONDS:float = 0.6


static func build_events(sources:Dictionary, config:Dictionary) -> Array[SfxEvent]:
    var events:Array[SfxEvent] = []

    var brake_shoe:SfxEvent = _build_brake_shoe(
            sources.get("brake"), sources.get("brakesound"),
            float(config.get("max_speed", 150.0)))
    if brake_shoe:
        events.append(brake_shoe)

    var emergency:SfxEvent = _build_continuous_event(
            sources.get("emergencybrake"), &"emergency_brake_hiss", &"brake_emergency_valve_flow", 0.0, 1.0)
    if emergency:
        events.append(emergency)

    var slippery:SfxEvent = _build_continuous_event(
            sources.get("slipperysound"), &"wheel_slip_squeal", &"slipping_wheels", 0.5, 1.5)
    if slippery:
        events.append(slippery)

    var releaser:SfxEvent = _build_releaser(sources.get("releaser"))
    if releaser:
        events.append(releaser)

    var spring_brake:SfxEvent = _build_spring_brake(
            sources.get("springbrake"), sources.get("springbrakeoff"))
    if spring_brake:
        events.append(spring_brake)

    var local_hiss:SfxEvent = _build_local_brake_hiss(
            sources.get("localbrakesound"), sources.get("localbrakesound2"))
    if local_hiss:
        events.append(local_hiss)

    var pipe_hiss:SfxEvent = _build_pipe_hiss(sources, config)
    if pipe_hiss:
        events.append(pipe_hiss)

    # At most one instance of any of these ambient/effect events should ever play per vehicle -
    # also makes a later config-driven rebuild (TrainBrakeSoundController re-running this factory
    # after config_changed) safe: play()-ing the newly-built replacement event under the same name
    # stops whatever instance of the old event object was still running under that name first
    # (SfxPlaybackRuntime.play(), matched by event_name not object identity), instead of leaking it.
    for event in events:
        event.polyphony_enabled = false

    return events


## "brake" (squeal, rsPisk) + "brakesound" (friction rumble, rsBrake) - both keyed
## off the same brakeforceratio in the original engine (DynObj.cpp:4417-4420), here fed as two
## speed-driven automations with force_ratio applied as event gain - two tracks
## of one shoe-on-wheel effect, exactly like engine.tres's multiple rpm-indexed tracks in one event.
## Called independently for the exterior and cabin banks. The latter supplies only its own
## "brakesound" definition, so both banks can expose an event named "brake_shoe" with distinct
## samples and spatial configuration.
static func _build_brake_shoe(
        squeal_def:MmdSoundSourceDefinition, friction_def:MmdSoundSourceDefinition,
        max_speed:float) -> SfxEvent:
    if not squeal_def and not friction_def:
        return null

    var event := SfxEvent.new()
    event.name = &"brake_shoe"
    var automations:Array[SfxAutomation] = []

    if squeal_def and _has_sound(squeal_def):
        # Best-effort parameter choice - see the redesign plan's note on brake:/brakesound:'s
        # chunk thresholds needing empirical verification against DynObj.cpp's actual render call.
        automations.append(_automation_for(squeal_def, &"speed", 0.0, 150.0))
    if friction_def and _has_sound(friction_def):
        automations.append(_brake_friction_automation(friction_def, max_speed))

    if automations.is_empty():
        return null
    event.automations = automations

    var primary:MmdSoundSourceDefinition = friction_def if friction_def else squeal_def
    event.parameter_modulations = _generic_modulations()
    event.parameter_modulations.append(_force_ratio_modulation())
    event.spatial_config = MmdSoundEventBuilder._build_spatial_config(primary)
    return event


static func _brake_friction_automation(
        definition:MmdSoundSourceDefinition, max_speed:float) -> SfxAutomation:
    if not definition.chunks.is_empty():
        var built:Dictionary = MmdSoundEventBuilder._build_automation(definition, &"speed")
        var chunk_automation:SfxAutomation = built["automation"]
        var maximum_chunk_gain:float = maxf(
                definition.amplitude_offset + definition.amplitude_factor, 0.001)
        for clip:SfxClip in chunk_automation.clips:
            clip.track.volume_db += linear_to_db(maximum_chunk_gain)
            _apply_continuous_release(clip.track)
        return chunk_automation
    var speed_limit:float = maxf(max_speed, 1.0)
    var automation := SfxAutomation.new()
    automation.parameter_name = &"speed"
    automation.min_domain = 0.05
    automation.max_domain = speed_limit
    var clip := SfxClip.new()
    clip.stream = MmdSoundEventBuilder._build_stream(_primary_sound(definition), true)
    clip.offset = 0.05
    clip.length = 0.0
    var maximum_gain:float = maxf(definition.amplitude_offset + definition.amplitude_factor, 0.001)
    var curve := Curve.new()
    curve.min_domain = 0.0
    curve.max_domain = speed_limit - clip.offset
    curve.add_point(Vector2(
            0.0,
            clampf((definition.amplitude_offset + sqrt(0.4) * definition.amplitude_factor) / maximum_gain, 0.0, 1.0)))
    curve.add_point(Vector2(
            curve.max_domain,
            clampf((definition.amplitude_offset + sqrt(0.4 + 0.6 * speed_limit / (1.0 + speed_limit))
                    * definition.amplitude_factor) / maximum_gain, 0.0, 1.0)))
    clip.fade_in_curve = curve
    var track := SfxTrack.new()
    track.track_name = StringName(definition.label)
    track.volume_db = linear_to_db(maximum_gain)
    _apply_continuous_release(track)
    clip.track = track
    automation.clips = [clip]
    if not is_equal_approx(definition.frequency_factor, 1.0) or not is_zero_approx(definition.frequency_offset):
        automation.pitch_curve = _brake_pitch_curve(definition, speed_limit)
    return automation


static func _brake_pitch_curve(definition:MmdSoundSourceDefinition, max_speed:float) -> Curve:
    var normalized_factor:float = definition.frequency_factor / (1.0 + max_speed)
    var curve := Curve.new()
    curve.min_domain = 0.0
    curve.max_domain = max_speed
    curve.add_point(Vector2(0.0, maxf(definition.frequency_offset, 0.01)))
    curve.add_point(Vector2(
            max_speed,
            maxf(definition.frequency_offset + normalized_factor * max_speed, 0.01)))
    return curve


static func _force_ratio_modulation() -> SfxParameterModulation:
    var modulation := SfxParameterModulation.new()
    modulation.parameter_name = &"force_ratio"
    modulation.target = SfxParameterModulation.Target.GAIN
    modulation.min_domain = 0.0
    modulation.max_domain = 1.0
    modulation.default_value = 0.0
    var curve := Curve.new()
    curve.min_domain = 0.0
    curve.max_domain = 1.0
    curve.add_point(Vector2(0.0, 0.0))
    curve.add_point(Vector2(1.0, 1.0))
    curve.bake_resolution = 64
    for point:int in range(1, 8):
        var ratio:float = float(point) / 8.0
        curve.add_point(Vector2(ratio, sqrt(ratio)))
    modulation.curve = curve
    return modulation


## "unbrake"/"emergencybrake"/"slipperysound"/"releaser" - each has one genuinely unique physical
## driving parameter, no natural counterpart among the other brake labels, so each stays its own
## event with a single automation on that plain float signal. Plays continuously once fed; the
## automation's own fade curve is what makes it read as silent at rest, not an outer play()/stop().
static func _build_continuous_event(
        definition:MmdSoundSourceDefinition, event_name:StringName,
        parameter_name:StringName, min_value:float, max_value:float) -> SfxEvent:
    if not definition or not _has_sound(definition):
        return null
    var event := SfxEvent.new()
    event.name = event_name
    event.automations = [_automation_for(definition, parameter_name, min_value, max_value)]
    event.parameter_modulations = _generic_modulations()
    event.spatial_config = MmdSoundEventBuilder._build_spatial_config(definition)
    return event


static func _build_releaser(definition:MmdSoundSourceDefinition) -> SfxEvent:
    if not definition or not _has_sound(definition):
        return null
    var event := SfxEvent.new()
    event.name = &"brake_releaser"
    var automation := SfxAutomation.new()
    automation.parameter_name = &"brake_releaser_active"
    automation.min_domain = 0.0
    automation.max_domain = 1.0
    var clip := SfxClip.new()
    clip.stream = MmdSoundEventBuilder._build_stream(_primary_sound(definition), true)
    clip.offset = 0.5
    clip.length = 0.5
    clip.track = _track_for(definition, true)
    automation.clips = [clip]
    event.automations = [automation]
    event.parameter_modulations = _generic_modulations()
    event.parameter_modulations.append(_pressure_gain_modulation())
    event.spatial_config = MmdSoundEventBuilder._build_spatial_config(definition)
    return event


static func _pressure_gain_modulation() -> SfxParameterModulation:
    var modulation := SfxParameterModulation.new()
    modulation.parameter_name = &"brake_air_pressure"
    modulation.target = SfxParameterModulation.Target.GAIN
    modulation.min_domain = 0.0
    modulation.max_domain = 0.8
    modulation.default_value = 0.0
    var curve := Curve.new()
    curve.min_domain = 0.0
    curve.max_domain = 0.8
    curve.add_point(Vector2(0.0, 0.0))
    curve.add_point(Vector2(0.8, 1.0))
    modulation.curve = curve
    return modulation


## A discrete one-shot/click/edge, or a coupled pair of them (advance/recede, increase/decrease,
## activate/release) sharing one event - each half is its own SfxAutomation keyed on a 0/1 pulse
## parameter, domain [0.5, 1.5], one clip with `cut = false` so the sample finishes naturally once
## triggered. gnd-sfx's own automation-domain-entry tracking (SfxPlaybackRuntime.
## _refresh_automation_clips's triggered_clips bookkeeping) already fires the clip exactly once per
## pulse and re-arms once the parameter drops back below the domain - no runtime edge-detection
## needed for any of these (same idea as horn1/horn2's begin/sustain/end shape in
## demo/vehicles/sm42/sounds, just via a pulse automation instead of ADSR release).
## `single_definition` is used for the one-parameter case (e.g. brakeacc); `paired_definitions`
## (key -> definition) is used for two-halves-in-one-event (e.g. cylinder/EP clicks).
static func _build_pulse_event(
        single_definition:MmdSoundSourceDefinition, event_name:StringName,
        pulse_parameters:Dictionary, paired_definitions:Dictionary = {}) -> SfxEvent:
    var automations:Array[SfxAutomation] = []
    var primary:MmdSoundSourceDefinition = null

    if single_definition:
        if not _has_sound(single_definition):
            return null
        primary = single_definition
        automations.append(_pulse_automation(single_definition, StringName(pulse_parameters["pulse"])))
    else:
        for key:String in pulse_parameters:
            var definition:MmdSoundSourceDefinition = paired_definitions.get(key)
            if not definition or not _has_sound(definition):
                continue
            if not primary:
                primary = definition
            automations.append(_pulse_automation(definition, StringName(pulse_parameters[key])))

    if automations.is_empty():
        return null
    var event := SfxEvent.new()
    event.name = event_name
    event.automations = automations
    event.parameter_modulations = _generic_modulations()
    event.spatial_config = MmdSoundEventBuilder._build_spatial_config(primary)
    return event


static func _pulse_automation(definition:MmdSoundSourceDefinition, parameter_name:StringName) -> SfxAutomation:
    var automation := SfxAutomation.new()
    automation.parameter_name = parameter_name
    automation.min_domain = 0.5
    automation.max_domain = 1.5
    var clip := SfxClip.new()
    clip.stream = MmdSoundEventBuilder._build_stream(_primary_sound(definition), false)
    clip.offset = 0.5
    clip.length = 1.0
    clip.cut = false
    clip.track = _track_for(definition)
    automation.clips = [clip]
    return automation


static func _build_spring_brake(
        activate_definition:MmdSoundSourceDefinition,
        release_definition:MmdSoundSourceDefinition) -> SfxEvent:
    var automations:Array[SfxAutomation] = []
    var primary:MmdSoundSourceDefinition = activate_definition if activate_definition else release_definition
    if activate_definition and _has_sound(activate_definition):
        automations.append(_domain_clip_automation(activate_definition, true))
    if release_definition and _has_sound(release_definition):
        automations.append(_domain_clip_automation(release_definition, false))
    if automations.is_empty():
        return null
    var event := SfxEvent.new()
    event.name = &"springbrake"
    event.automations = automations
    event.parameter_modulations = _generic_modulations()
    event.spatial_config = MmdSoundEventBuilder._build_spatial_config(primary)
    return event


static func _domain_clip_automation(
        definition:MmdSoundSourceDefinition, high_domain:bool) -> SfxAutomation:
    var automation := SfxAutomation.new()
    automation.parameter_name = &"spring_brake_active"
    automation.min_domain = 0.0
    automation.max_domain = 1.0
    var clip := SfxClip.new()
    clip.stream = MmdSoundEventBuilder._build_stream(_primary_sound(definition), false)
    clip.offset = 0.5 if high_domain else 0.0
    clip.length = 0.5
    clip.cut = false
    clip.track = _track_for(definition)
    automation.clips = [clip]
    return automation


## "localbrakesound" (release, rsSBHiss) + "localbrakesound2" (engage, rsSBHissU) - both keyed off
## brake_loco_pressure's rate of change, published from TrainBrake.cpp as two already-non-negative
## magnitudes (brake_loco_pressure_fall_rate/rise_rate) - two tracks of one independent-brake-
## cylinder hiss, one continuously-playing event.
static func _build_local_brake_hiss(
        release_def:MmdSoundSourceDefinition, engage_def:MmdSoundSourceDefinition) -> SfxEvent:
    if not release_def and not engage_def:
        return null
    var event := SfxEvent.new()
    event.name = &"local_brake_hiss"
    var automations:Array[SfxAutomation] = []
    if release_def and _has_sound(release_def):
        automations.append(_signed_flow_automation(
                release_def, &"brake_local_valve_flow", -1.0, 1.0, 1.0, 0.05))
    if engage_def and _has_sound(engage_def):
        automations.append(_signed_flow_automation(
                engage_def, &"brake_local_valve_flow", 1.0, 1.0, 1.0, 0.05))
    if automations.is_empty():
        return null
    event.automations = automations
    var primary:MmdSoundSourceDefinition = release_def if release_def else engage_def
    event.parameter_modulations = _generic_modulations()
    event.spatial_config = MmdSoundEventBuilder._build_spatial_config(primary)
    return event


## Main-pipe fill and release use one stable event API. The handle type changes only the factory
## calibration; runtime always supplies the mover's signed main-valve flow.
static func _build_pipe_hiss(sources:Dictionary, config:Dictionary) -> SfxEvent:
    var definitions:Dictionary = {
        "airsound": sources.get("airsound"),
        "airsound2": sources.get("airsound2"),
        "airsound3": sources.get("airsound3"),
        "airsound4": sources.get("airsound4"),
        "airsound5": sources.get("airsound5"),
    }
    if definitions.values().all(func(d): return d == null):
        return null

    var brake_handle_type:int = int(config.get(
            "brake_handle_type", TrainBrake.BRAKE_HANDLE_TYPE_NO_HANDLE))
    var fv_sound_model:bool = (
            brake_handle_type == TrainBrake.BRAKE_HANDLE_TYPE_FV4A
            or brake_handle_type == TrainBrake.BRAKE_HANDLE_TYPE_FVEL6)
    var fv4a_model:bool = brake_handle_type == TrainBrake.BRAKE_HANDLE_TYPE_FV4A
    var fv_flow_scale:Dictionary = {
        "airsound": 100000.0,
        "airsound2": 800000.0,
    }
    var event := SfxEvent.new()
    event.name = &"pipe_hiss"
    var automations:Array[SfxAutomation] = []
    var primary:MmdSoundSourceDefinition = null
    for label:String in ["airsound", "airsound2", "airsound3", "airsound4", "airsound5"]:
        if not fv_sound_model and label in ["airsound3", "airsound4", "airsound5"]:
            continue
        if not fv4a_model and label in ["airsound4", "airsound5"]:
            continue
        var definition:MmdSoundSourceDefinition = definitions[label]
        if not definition or not _has_sound(definition):
            continue
        if not primary:
            primary = definition
        if label == "airsound3":
            automations.append(_position_automation(
                    definition, &"brake_controller_position",
                    float(config.get("brakes_controller_position_emergency", 0.0))))
            continue
        if label == "airsound4":
            automations.append(_position_automation(
                    definition, &"brake_controller_position",
                    float(config.get("brakes_controller_position_cutoff", 0.0))))
            continue
        if label == "airsound5":
            automations.append(_automation_for(
                    definition, &"brake_control_pressure", 0.001, 5.0))
            continue
        var direction:float = 1.0 if label == "airsound" else -1.0
        var input_scale:float = 1.0
        # dpMainValve is the post-pipe, vehicle-normalized flow. FV4a's original sound path
        # reads the pre-normalized handle flow, so compensate here in this factory variant.
        var output_scale:float = 2.0 if fv_sound_model else 1.0
        var threshold:float = 0.005 if fv_sound_model else 0.05
        if label == "airsound":
            input_scale = float(fv_flow_scale[label]) if fv_sound_model else 2.0
        elif fv_sound_model:
            input_scale = float(fv_flow_scale[label])
        elif label == "airsound2" and not fv_sound_model:
            threshold = 0.01
        automations.append(_signed_flow_automation(
                definition, &"brake_main_valve_flow", direction,
                input_scale, output_scale, threshold,
                0.001 if fv_sound_model else 10.0,
                10.0))

    if automations.is_empty():
        return null
    event.automations = automations
    event.parameter_modulations = _generic_modulations()
    event.spatial_config = MmdSoundEventBuilder._build_spatial_config(primary)
    return event


static func _position_automation(
        definition:MmdSoundSourceDefinition, parameter_name:StringName,
        position:float) -> SfxAutomation:
    var automation := SfxAutomation.new()
    automation.parameter_name = parameter_name
    automation.min_domain = position - 0.49
    automation.max_domain = position + 0.49
    var clip := SfxClip.new()
    clip.stream = MmdSoundEventBuilder._build_stream(_primary_sound(definition), true)
    clip.offset = automation.min_domain
    clip.length = automation.max_domain - automation.min_domain
    clip.track = _track_for(definition, true)
    automation.clips = [clip]
    return automation


static func _signed_flow_automation(
        definition:MmdSoundSourceDefinition, parameter_name:StringName,
        direction:float, input_scale:float, output_scale:float,
        threshold:float, gain_signal_max:float = 10.0,
        domain_max:float = 10.0) -> SfxAutomation:
    var factor:float = definition.amplitude_factor * input_scale
    var offset:float = definition.amplitude_offset
    var start:float = 0.000001
    if factor > 0.0:
        start = maxf((threshold / output_scale - offset) / factor, start)
    var maximum_gain:float = maxf(output_scale * (offset + factor * gain_signal_max), 0.001)

    var automation := SfxAutomation.new()
    automation.parameter_name = parameter_name
    var clip := SfxClip.new()
    clip.stream = MmdSoundEventBuilder._build_stream(_primary_sound(definition), true)
    var curve := Curve.new()
    if direction > 0.0:
        automation.min_domain = 0.0
        automation.max_domain = domain_max
        clip.offset = minf(start, domain_max)
        clip.length = 0.0
        var span:float = gain_signal_max - minf(clip.offset, gain_signal_max)
        curve.min_domain = 0.0
        curve.max_domain = maxf(span, 0.001)
        curve.add_point(Vector2(
                0.0, clampf(output_scale * (offset + factor * absf(clip.offset)) / maximum_gain, 0.0, 1.0)))
        curve.add_point(Vector2(curve.max_domain, 1.0))
    else:
        var start_signal:float = minf(start, domain_max)
        var saturation_signal:float = minf(gain_signal_max, domain_max)
        automation.min_domain = -domain_max
        automation.max_domain = 0.0
        clip.offset = -domain_max
        clip.length = domain_max - start_signal
        curve.min_domain = 0.0
        curve.max_domain = maxf(clip.length, 0.001)
        curve.add_point(Vector2(0.0, clampf(
                output_scale * (offset + factor * domain_max) / maximum_gain, 0.0, 1.0)))
        var saturation_end:float = domain_max - saturation_signal
        if saturation_end > 0.0:
            curve.add_point(Vector2(saturation_end, 1.0))
        curve.add_point(Vector2(
                curve.max_domain,
                clampf(output_scale * (offset + factor * start_signal) / maximum_gain, 0.0, 1.0)))
    clip.fade_in_curve = curve
    var track := SfxTrack.new()
    track.track_name = StringName(definition.label)
    track.volume_db = linear_to_db(maximum_gain)
    _apply_continuous_release(track)
    clip.track = track
    automation.clips = [clip]
    return automation


## Builds one SfxAutomation for `definition` keyed on `parameter_name`, spanning [min_value,
## max_value] - the plain physical range the parameter can realistically take. Reuses
## MmdSoundEventBuilder's own chunk-crossfade automation builder when the MMD source declares a
## soundN:/pitchN: chunk table (real for e.g. su45's brake:/brakesound: blocks) - that already
## produces proper per-chunk tracks/fade curves, exactly the engine.tres shape. Otherwise builds a
## single clip/track whose fade_in_curve is the gain-vs-parameter envelope (0..1) across the whole
## range - since SfxPlaybackRuntime samples fade_in_curve relative to the clip's own offset and
## clamps beyond its domain, one curve stands in for the original's
## `amplitudeoffset + amplitudefactor*shape(x)` formula; `amplitudefactor` itself (can exceed 1.0,
## real in MMD data) becomes the clip's own SfxTrack.volume_db trim, since fade_in_curve's sampled
## result is clamped to [0,1] by the runtime and can't express gain above unity on its own.
static func _automation_for(
        definition:MmdSoundSourceDefinition, parameter_name:StringName,
        min_value:float, max_value:float) -> SfxAutomation:
    if not definition.chunks.is_empty():
        var built:Dictionary = MmdSoundEventBuilder._build_automation(definition, parameter_name)
        var chunk_automation:SfxAutomation = built["automation"]
        for clip:SfxClip in chunk_automation.clips:
            _apply_continuous_release(clip.track)
        return chunk_automation

    var automation := SfxAutomation.new()
    automation.parameter_name = parameter_name
    automation.min_domain = min_value
    automation.max_domain = max_value

    var clip := SfxClip.new()
    clip.stream = MmdSoundEventBuilder._build_stream(_primary_sound(definition), true)
    clip.offset = min_value
    clip.length = 0.0 # extends to automation.max_domain
    clip.fade_in_curve = _envelope_curve(definition.amplitude_offset, max_value - min_value)
    clip.track = _track_for(definition, true)
    automation.clips = [clip]

    if not is_equal_approx(definition.frequency_factor, 1.0) or not is_zero_approx(definition.frequency_offset):
        automation.pitch_curve = _pitch_curve(definition, min_value, max_value - min_value)

    return automation


## 0..1 gain envelope over [0, span] (relative to the clip's own offset), starting at the MMD's own
## amplitude_offset (normalized into [0,1], since fade_in_curve's sampled result is clamped there)
## and rising to 1.0 - approximates `amplitudeoffset + amplitudefactor*shape(x)`'s general
## rising-with-input shape. Exact curve control points are a tuning detail (see the redesign plan's
## attenuation/gain note) - this is a reasonable starting shape, not a final one.
static func _envelope_curve(amplitude_offset:float, span:float) -> Curve:
    var curve := Curve.new()
    curve.min_domain = 0.0
    curve.max_domain = maxf(span, 0.001)
    curve.add_point(Vector2(0.0, clampf(amplitude_offset, 0.0, 1.0)))
    curve.add_point(Vector2(curve.max_domain, 1.0))
    return curve


## Absolute-parameter-domain pitch curve (SfxAutomation.pitch_curve is sampled without any offset -
## see SfxPlaybackRuntime._sample_automation_curve) approximating
## `frequency_offset + frequency_factor*x`.
static func _pitch_curve(definition:MmdSoundSourceDefinition, min_value:float, span:float) -> Curve:
    var curve := Curve.new()
    curve.min_domain = min_value
    curve.max_domain = min_value + maxf(span, 0.001)
    curve.add_point(Vector2(min_value, maxf(definition.frequency_offset + definition.frequency_factor * min_value, 0.01)))
    curve.add_point(Vector2(curve.max_domain, maxf(definition.frequency_offset + definition.frequency_factor * curve.max_domain, 0.01)))
    return curve


static func _track_for(definition:MmdSoundSourceDefinition, continuous:bool = false) -> SfxTrack:
    var track := SfxTrack.new()
    track.track_name = StringName(definition.label)
    track.volume_db = linear_to_db(maxf(definition.amplitude_factor, 0.001))
    if continuous:
        _apply_continuous_release(track)
    return track


static func _apply_continuous_release(track:SfxTrack) -> void:
    track.adsr_enabled = true
    track.release = CONTINUOUS_RELEASE_SECONDS


## soundproofing/unit_size/gain are generic, event-wide attenuation/trim factors - not physics-
## formula shaping - so they stay identity-curve SfxParameterModulations exactly as
## MmdSoundEventBuilder._build_modulation already builds for every other parameterized event.
## "gain" here is only ever fed the operator's own maszyna/sound/brake_*_volume_factor
## ProjectSettings trim (TrainBrakeSoundController._volume_factor()) - a deliberate coarse,
## listener-position-only knob on top of correctly-calibrated per-event data, not a substitute for
## it. A per-track "pitch" modulation is deliberately NOT included - per-track pitch shaping lives
## in each automation's own pitch_curve instead, since a pitch modulation would apply event-globally
## and incorrectly affect every automation sharing one multi-track brake event.
static func _generic_modulations() -> Array[SfxParameterModulation]:
    return [
        MmdSoundEventBuilder._build_modulation(&"gain", SfxParameterModulation.Target.GAIN, 0.0, 4.0, 1.0),
        MmdSoundEventBuilder._build_modulation(&"soundproofing", SfxParameterModulation.Target.GAIN, 0.0, 1.0, 1.0),
        MmdSoundEventBuilder._build_modulation(&"soundproofing", SfxParameterModulation.Target.UNIT_SIZE, 0.0, 1.0, 1.0),
        MmdSoundEventBuilder._build_modulation(&"unit_size", SfxParameterModulation.Target.UNIT_SIZE, 0.1, 8.0, 1.0),
    ]


static func _has_sound(definition:MmdSoundSourceDefinition) -> bool:
    return (not definition.sound_main.is_empty() or not definition.sound_begin.is_empty()
            or not definition.sound_end.is_empty() or not definition.chunks.is_empty())


## Picks one representative filename for definitions used as a plain pulse-triggered clip (no
## continuous parameter to crossfade chunks against). Real data has click labels (e.g. some
## vehicles' epbrakeinc:/epbrakedec:) declared entirely as a soundN: chunk table with no
## sound_main/begin/end at all - in that case this picks the table's first (lowest-threshold)
## sample rather than leaving the clip's stream empty. Not a full port of the original's
## per-intensity click sample selection - a reasonable approximation, since these labels aren't
## used by the primary reference vehicle (SU45, Pneumatic-not-ElectroPneumatic) and can be
## revisited if audibly wrong for a vehicle that does use them.
static func _primary_sound(definition:MmdSoundSourceDefinition) -> String:
    if not definition.sound_main.is_empty():
        return definition.sound_main
    if not definition.sound_begin.is_empty():
        return definition.sound_begin
    if not definition.sound_end.is_empty():
        return definition.sound_end
    if not definition.chunks.is_empty():
        var chunks:Array[Dictionary] = definition.chunks.duplicate()
        chunks.sort_custom(func(a:Dictionary, b:Dictionary) -> bool: return int(a["threshold"]) < int(b["threshold"]))
        return chunks[0]["filename"]
    return ""
