extends RefCounted
class_name MmdSoundEventBuilder

## Converts one MmdSoundSourceDefinition into an in-memory SfxEvent (plain .new() + property
## assignment - gnd-sfx resources need no .tres serialization to work at runtime, confirmed by
## reading every gnd-sfx resource class). A definition can combine BOTH shapes at once (e.g.
## "engine": begin/end from the vehicle's separate `ignition:`/`shutdown:` MMD labels, chunks from
## `engine:`'s own soundN:/pitchN: table - MmdSoundBankInstancer merges those three MMD-level
## sound sources into one definition before calling build()):
## - non-empty chunks -> 1 SfxAutomation, one clip per chunk, each on its OWN SfxTrack with a
##   fade_in_curve/fade_out_curve/pitch_curve so adjacent chunks actually crossfade instead of
##   cutting - ported from audio/sound.cpp's own sound_source::update_crossfade()/deserialize()
##   math (the gain/pitch interpolation the original engine performs at playback time), not
##   guessed. Matches engine.tres's own per-clip-track shape.
## - sound_begin and/or sound_end (+ optional sound_main) -> up to 3 plain clips, end clip
##   trigger_mode = TRIGGER_SUSTAIN (matches horn1.tres/horn2.tres's begin/trwa/stop shape). When
##   chunks are ALSO present, these become the automation's start/stop bookends on the same event
##   (matches engine.tres's own combined clips+automations shape) instead of a separate event.
## - sound_main only, no begin/end/chunks -> 1 looping clip (matches oil_pump.tres's shape).

## Generous ceiling so the TOP chunk (length=0, i.e. "extends to max_domain" - see
## _automation_clip_contains_value()) stays audible for any real RPM/parameter value instead of
## silently going quiet past its own threshold, which is what a literal port of the original
## engine's Chunkrange=100 default would do for any real chunk table (thresholds are always > 100).
const _AUTOMATION_HEADROOM:float = 100000.0


static func build(
        definition:MmdSoundSourceDefinition, event_name:StringName,
        sound_parameter:StringName = &"", parameterized:bool = false,
        soundproofed:bool = false) -> SfxEvent:
    var event := SfxEvent.new()
    event.name = event_name

    var has_bookends:bool = not definition.sound_begin.is_empty() or not definition.sound_end.is_empty()
    var has_chunks:bool = not definition.chunks.is_empty()

    if has_chunks:
        var built:Dictionary = _build_automation(definition, sound_parameter)
        event.automations = [built["automation"]]
        event.tracks = built["tracks"]

    if has_bookends:
        event.clips = _build_begin_main_end_clips(definition)
    elif not has_chunks and not definition.sound_main.is_empty():
        var clip := SfxClip.new()
        clip.stream = _build_stream(definition.sound_main, true)
        event.clips = [clip]

    if parameterized:
        event.parameter_modulations = [
            _build_modulation(&"gain", SfxParameterModulation.Target.GAIN, 0.0, 100.0, 1.0),
            _build_modulation(&"pitch", SfxParameterModulation.Target.PITCH, 0.1, 10.0, 1.0),
            _build_modulation(&"unit_size", SfxParameterModulation.Target.UNIT_SIZE, 0.1, 8.0, 1.0),
        ]

    if parameterized or soundproofed:
        event.parameter_modulations.append_array([
            _build_modulation(&"soundproofing", SfxParameterModulation.Target.GAIN, 0.0, 1.0, 1.0),
            _build_modulation(&"soundproofing", SfxParameterModulation.Target.UNIT_SIZE, 0.0, 1.0, 1.0),
        ])
        event.spatial_config = _build_spatial_config(definition)

    return event


static func _build_modulation(
        parameter_name:StringName, target:int,
        minimum:float, maximum:float, default_value:float) -> SfxParameterModulation:
    var modulation := SfxParameterModulation.new()
    modulation.parameter_name = parameter_name
    modulation.target = target
    modulation.min_domain = minimum
    modulation.max_domain = maximum
    modulation.default_value = default_value
    var curve := Curve.new()
    curve.min_domain = minimum
    curve.max_domain = maximum
    curve.min_value = minimum
    curve.max_value = maximum
    curve.add_point(Vector2(minimum, minimum))
    curve.add_point(Vector2(maximum, maximum))
    modulation.curve = curve
    return modulation


static func _build_spatial_config(definition:MmdSoundSourceDefinition) -> SfxSpatialConfig:
    var config := SfxSpatialConfig.new()
    config.position = definition.offset
    if definition.range < 0.0:
        config.attenuation_model = AudioStreamPlayer3D.ATTENUATION_DISABLED
        config.max_distance = 0.0
        config.panning_strength = 0.0
    else:
        config.unit_size = maxf(definition.range / 16.0, 0.01)
        config.max_distance = minf(definition.range * 7.5, 2750.0)
    return config


static func _build_begin_main_end_clips(definition:MmdSoundSourceDefinition) -> Array[SfxClip]:
    var clips:Array[SfxClip] = []
    # gnd-sfx's SfxEvent is a fixed timeline, unlike the original engine's real-time "play begin
    # once, then switch to looping main" state machine - the main clip's start is approximated
    # from the begin clip's own real duration (queried from the actual audio asset, not guessed)
    # so it starts right as the begin clip finishes.
    var main_offset:float = _stream_length(definition.sound_begin)

    if not definition.sound_begin.is_empty():
        var begin_clip := SfxClip.new()
        begin_clip.stream = _build_stream(definition.sound_begin, false)
        clips.append(begin_clip)

    if not definition.sound_main.is_empty():
        var main_clip := SfxClip.new()
        main_clip.stream = _build_stream(definition.sound_main, true)
        main_clip.offset = main_offset
        clips.append(main_clip)

    if not definition.sound_end.is_empty():
        var end_clip := SfxClip.new()
        end_clip.stream = _build_stream(definition.sound_end, false)
        end_clip.trigger_mode = SfxClip.TriggerMode.TRIGGER_SUSTAIN
        clips.append(end_clip)

    return clips


## Ports audio/sound.cpp's sound_source::deserialize()'s chunk fadein/fadeout placement (the
## ACTIVE code path - a commented-out alternate crossfade-interpolation formula exists in the same
## function but is dead code, never executed) AND update_crossfade()'s per-chunk gain/pitch
## interpolation (real-time playback math, not the deserialize-time one) as gnd-sfx fade_in_curve/
## fade_out_curve/pitch_curve pairs, so gnd-sfx's own crossfade blending (SfxPlaybackRuntime
## multiplies clip.pitch_curve * automation.pitch_curve, and blends simultaneously-active clips'
## fade curve outputs) reproduces the same overlap/pitch-bend behavior instead of hard-cutting
## between chunks.
static func _build_automation(definition:MmdSoundSourceDefinition, sound_parameter:StringName) -> Dictionary:
    var chunks:Array[Dictionary] = definition.chunks.duplicate()
    chunks.sort_custom(func(a:Dictionary, b:Dictionary) -> bool: return int(a["threshold"]) < int(b["threshold"]))

    # fadeins[i]/fadeouts[i] mirror sound_source::deserialize()'s own cached chunk range points
    # (audio/sound.cpp:60-85) - fadeouts has no entry for the LAST chunk since that boundary is
    # superseded by length=0 below (open-ended, see the class doc comment), not consumed anywhere.
    var fadeins:Array[float] = []
    var fadeouts:Array[float] = []
    for idx in range(chunks.size()):
        var threshold:float = float(chunks[idx]["threshold"])
        if idx == 0:
            fadeins.append(maxf(0.0, threshold))
        else:
            var previous_threshold:float = float(chunks[idx - 1]["threshold"])
            fadeins.append(threshold - 0.01 * definition.crossfade_percent * (threshold - previous_threshold))
        if idx < chunks.size() - 1:
            fadeouts.append(float(chunks[idx + 1]["threshold"]))

    var automation := SfxAutomation.new()
    automation.parameter_name = sound_parameter
    var clips:Array[SfxClip] = []
    var tracks:Array[SfxTrack] = []
    var max_threshold:float = 0.0

    for idx in range(chunks.size()):
        var threshold:float = float(chunks[idx]["threshold"])
        var is_last:bool = idx == chunks.size() - 1
        var fadein:float = fadeins[idx]

        var clip := SfxClip.new()
        clip.stream = _build_stream(chunks[idx]["filename"], true)
        clip.offset = fadein
        # top chunk: 0 means "active up to automation.max_domain" (_automation_clip_contains_value)
        # - stays audible at any RPM above its own threshold instead of cutting out past Chunkrange.
        clip.length = 0.0 if is_last else maxf(0.0, fadeouts[idx] - fadein)

        var fade_in_width:float = threshold - fadein
        clip.fade_in_curve = _build_ramp_curve(true, fade_in_width)
        if not is_last:
            var fade_out_width:float = maxf(0.0, fadeouts[idx] - fadeins[idx + 1])
            clip.fade_out_curve = _build_ramp_curve(false, fade_out_width)

        var own_pitch:float = float(chunks[idx].get("pitch", 0.0))
        if own_pitch > 0.0:
            var pitch_span:float = maxf(maxf(clip.length, fade_in_width), 0.001)
            clip.pitch_curve = _build_pitch_curve(chunks, idx, own_pitch, fadein, pitch_span)

        var track := SfxTrack.new()
        track.track_name = "chunk_%d" % int(threshold)
        clip.track = track
        tracks.append(track)

        clips.append(clip)
        max_threshold = maxf(max_threshold, threshold)

    automation.clips = clips
    automation.max_domain = max_threshold + _AUTOMATION_HEADROOM
    return {"automation": automation, "tracks": tracks}


## 0-width (or null-curve) case is deliberately left unset - SfxPlaybackRuntime treats a null
## fade_in_curve/fade_out_curve as flat gain 1.0 (see _sample_automation_curve()), correct for the
## first chunk's fade-in and the last chunk's fade-out, which have no neighbour to blend with.
static func _build_ramp_curve(ascending:bool, width:float) -> Curve:
    if width <= 0.0:
        return null
    var curve := Curve.new()
    curve.min_domain = 0.0
    curve.max_domain = width
    if ascending:
        curve.add_point(Vector2(0.0, 0.0))
        curve.add_point(Vector2(width, 1.0))
    else:
        curve.add_point(Vector2(0.0, 1.0))
        curve.add_point(Vector2(width, 0.0))
    return curve


## Approximates update_crossfade()'s per-chunk pitch ratio (each sample is tuned to sound correct
## at its OWN declared pitch value; when a neighbouring chunk's territory is entered during a
## crossfade, its pitch bends toward that neighbour's ratio so the transition sounds smoother) -
## not a byte-identical port (the original interpolates across the full inter-threshold gap using
## real-time chunk-index bookkeeping gnd-sfx's clip-local pitch_curve has no equivalent for), but
## the same underlying idea: 1.0 (natural pitch) exactly at this chunk's own threshold, bending
## toward prev_pitch/own_pitch at the start of this clip's active range and toward
## next_pitch/own_pitch at its end. Returns null (no pitch bend) if neither neighbour declared a
## pitch value - most chunks in real data do (see MmdSoundSourceParser's pitchN: parsing).
static func _build_pitch_curve(
        chunks:Array[Dictionary], idx:int, own_pitch:float, fadein:float, span:float) -> Curve:
    var ratio_from_prev:float = 1.0
    if idx > 0:
        var previous_pitch:float = float(chunks[idx - 1].get("pitch", 0.0))
        if previous_pitch > 0.0:
            ratio_from_prev = previous_pitch / own_pitch
    var ratio_to_next:float = 1.0
    if idx < chunks.size() - 1:
        var next_pitch:float = float(chunks[idx + 1].get("pitch", 0.0))
        if next_pitch > 0.0:
            ratio_to_next = next_pitch / own_pitch

    if is_equal_approx(ratio_from_prev, 1.0) and is_equal_approx(ratio_to_next, 1.0):
        return null

    var curve := Curve.new()
    curve.min_domain = 0.0
    curve.max_domain = span
    curve.add_point(Vector2(0.0, ratio_from_prev))
    var home_x:float = clampf(float(chunks[idx]["threshold"]) - fadein, 0.0, span)
    curve.add_point(Vector2(home_x, 1.0))
    if home_x < span:
        curve.add_point(Vector2(span, ratio_to_next))
    return curve


static func _build_stream(filename:String, loop:bool) -> MaszynaAudioStream:
    var stream := MaszynaAudioStream.new()
    stream.file_path = filename
    stream.loop = loop
    return stream


## Real asset duration, queried once at build time (not a guessed constant) - 0.0 if the file
## can't be resolved (e.g. in a test environment with no game data configured), which degrades to
## the main clip simply starting at the same time as begin instead of after it. Checks existence
## first (same path formula as AudioStreamManager.get_stream()) rather than calling it directly,
## since that function push_errors on a miss - appropriate when actually resolving a clip to play,
## not for this best-effort lookup where "unknown length" is an expected, silent outcome.
static func _stream_length(filename:String) -> float:
    if filename.is_empty():
        return 0.0
    var path:String = "%s/sounds/%s.ogg" % [UserSettings.get_maszyna_game_dir(), filename.to_lower()]
    if not ResourceLoader.exists(path):
        return 0.0
    var stream:AudioStream = AudioStreamManager.get_stream(filename)
    return stream.get_length() if stream else 0.0
