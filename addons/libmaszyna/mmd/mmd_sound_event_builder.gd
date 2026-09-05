extends RefCounted
class_name MmdSoundEventBuilder

## Converts one MmdSoundSourceDefinition into an in-memory SfxEvent (plain .new() + property
## assignment - gnd-sfx resources need no .tres serialization to work at runtime, confirmed by
## reading every gnd-sfx resource class). Which of three shapes gets built depends only on which
## fields MmdSoundSourceParser populated, never on the MMD source syntax that produced them:
## - non-empty chunks -> 1 SfxAutomation (matches engine.tres's crossfading idle clips).
## - sound_begin and/or sound_end (+ optional sound_main) -> up to 3 clips, end clip
##   trigger_mode = TRIGGER_SUSTAIN (matches horn1.tres/horn2.tres's begin/trwa/stop shape).
## - sound_main only -> 1 looping clip (matches oil_pump.tres's shape).

const _DEFAULT_CHUNK_RANGE:float = 100.0 # matches sound_source::deserialize()'s own Chunkrange default


static func build(definition:MmdSoundSourceDefinition, event_name:StringName, sound_parameter:StringName = &"") -> SfxEvent:
    var event := SfxEvent.new()
    event.name = event_name

    if not definition.chunks.is_empty():
        event.automations = [_build_automation(definition, sound_parameter)]
        return event

    if not definition.sound_begin.is_empty() or not definition.sound_end.is_empty():
        event.clips = _build_begin_main_end_clips(definition)
        return event

    if not definition.sound_main.is_empty():
        var clip := SfxClip.new()
        clip.stream = _build_stream(definition.sound_main, true)
        event.clips = [clip]

    return event


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


## Ports audio/sound.cpp's own chunk fadein/fadeout math - the ACTIVE code path (a commented-out
## alternate crossfade-interpolation formula exists in the same function but is dead code, never
## executed by the running engine): chunk i stays fully audible up to the NEXT chunk's threshold,
## and fades in starting crossfade_percent% of the way through the PREVIOUS chunk's own gap -
## producing the overlapping regions SfxAutomation crossfades between. Not a guessed layering
## shape - the same formula the original engine itself uses to place these chunks.
static func _build_automation(definition:MmdSoundSourceDefinition, sound_parameter:StringName) -> SfxAutomation:
    var chunks:Array[Dictionary] = definition.chunks.duplicate()
    chunks.sort_custom(func(a:Dictionary, b:Dictionary) -> bool: return int(a["threshold"]) < int(b["threshold"]))

    var automation := SfxAutomation.new()
    automation.parameter_name = sound_parameter
    var clips:Array[SfxClip] = []
    var max_domain:float = 0.0

    for idx in range(chunks.size()):
        var threshold:float = float(chunks[idx]["threshold"])
        var fadein:float
        if idx == 0:
            fadein = maxf(0.0, threshold)
        else:
            var previous_threshold:float = float(chunks[idx - 1]["threshold"])
            fadein = threshold - 0.01 * definition.crossfade_percent * (threshold - previous_threshold)
        var fadeout:float
        if idx == chunks.size() - 1:
            fadeout = maxf(_DEFAULT_CHUNK_RANGE, threshold)
        else:
            fadeout = float(chunks[idx + 1]["threshold"])

        var clip := SfxClip.new()
        clip.stream = _build_stream(chunks[idx]["filename"], true)
        clip.offset = fadein
        clip.length = maxf(0.0, fadeout - fadein)
        clips.append(clip)
        max_domain = maxf(max_domain, fadeout)

    automation.clips = clips
    automation.max_domain = max_domain
    return automation


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
