extends MaszynaGutTest


func test_sound_main_only_produces_one_looping_clip():
    var definition := MmdSoundSourceDefinition.new()
    definition.sound_main = "oilpump"
    var event:SfxEvent = MmdSoundEventBuilder.build(definition, &"oil_pump")
    assert_eq(event.name, &"oil_pump")
    assert_eq(event.clips.size(), 1)
    var stream:MaszynaAudioStream = event.clips[0].stream
    assert_eq(stream.file_path, "oilpump")
    assert_true(stream.loop)


func test_begin_main_end_produces_three_clips_with_sustain_release():
    var definition := MmdSoundSourceDefinition.new()
    definition.sound_begin = "horn1_start"
    definition.sound_main = "horn1_trwa"
    definition.sound_end = "horn1_stop"
    var event:SfxEvent = MmdSoundEventBuilder.build(definition, &"horn1")
    assert_eq(event.clips.size(), 3)
    assert_eq((event.clips[0].stream as MaszynaAudioStream).file_path, "horn1_start")
    assert_false(event.clips[0].stream.loop)
    assert_eq((event.clips[1].stream as MaszynaAudioStream).file_path, "horn1_trwa")
    assert_true(event.clips[1].stream.loop)
    assert_eq((event.clips[2].stream as MaszynaAudioStream).file_path, "horn1_stop")
    assert_eq(event.clips[2].trigger_mode, SfxClip.TriggerMode.TRIGGER_SUSTAIN)


func test_begin_only_omits_main_and_end_clips():
    var definition := MmdSoundSourceDefinition.new()
    definition.sound_begin = "click"
    var event:SfxEvent = MmdSoundEventBuilder.build(definition, &"click")
    assert_eq(event.clips.size(), 1)
    assert_eq((event.clips[0].stream as MaszynaAudioStream).file_path, "click")


func test_chunks_produce_one_automation_with_clips_positioned_by_threshold():
    var definition := MmdSoundSourceDefinition.new()
    definition.crossfade_percent = 90
    definition.chunks = [
        {"threshold": 1000, "filename": "idle_high", "pitch": 1000.0},
        {"threshold": 400, "filename": "silence1", "pitch": 400.0},
        {"threshold": 600, "filename": "idle_low", "pitch": 600.0},
    ]
    var event:SfxEvent = MmdSoundEventBuilder.build(definition, &"engine", &"rpm")
    assert_eq(event.automations.size(), 1)
    var automation:SfxAutomation = event.automations[0]
    assert_eq(automation.parameter_name, &"rpm")
    assert_eq(automation.clips.size(), 3)

    # sorted by threshold regardless of input order (400, 600, 1000)
    var low:SfxClip = automation.clips[0]
    assert_eq((low.stream as MaszynaAudioStream).file_path, "silence1")
    assert_almost_eq(low.offset, 400.0, 0.01)
    assert_almost_eq(low.length, 200.0, 0.01) # fadeout(600) - fadein(400)
    assert_null(low.fade_in_curve) # first chunk: no previous chunk to blend in from
    assert_not_null(low.fade_out_curve)

    var mid:SfxClip = automation.clips[1]
    assert_eq((mid.stream as MaszynaAudioStream).file_path, "idle_low")
    assert_almost_eq(mid.offset, 420.0, 0.01) # 600 - 0.9*(600-400)
    assert_almost_eq(mid.length, 580.0, 0.01) # fadeout(1000) - fadein(420)
    assert_not_null(mid.fade_in_curve)
    assert_not_null(mid.fade_out_curve)

    var high:SfxClip = automation.clips[2]
    assert_eq((high.stream as MaszynaAudioStream).file_path, "idle_high")
    assert_almost_eq(high.offset, 640.0, 0.01) # 1000 - 0.9*(1000-600)
    # top chunk is open-ended (0 = "active up to automation.max_domain") rather than cut off just
    # past its own threshold, so it stays audible for any RPM above 1000 instead of going silent.
    assert_almost_eq(high.length, 0.0, 0.01)
    assert_not_null(high.fade_in_curve)
    assert_null(high.fade_out_curve) # last chunk: no next chunk to blend out into

    # every chunk gets its own dedicated track so overlapping voices actually mix/crossfade
    # instead of competing on one shared master track.
    assert_eq(event.tracks.size(), 3)
    assert_true(low.track in event.tracks)
    assert_true(mid.track in event.tracks)
    assert_true(high.track in event.tracks)
    assert_ne(low.track, mid.track)

    # each chunk declared its own pitch, distinct from its neighbours, so every clip should bend
    # pitch across the crossfade zone rather than playing at a flat, unmodulated pitch.
    assert_not_null(low.pitch_curve)
    assert_not_null(mid.pitch_curve)
    assert_not_null(high.pitch_curve)

    assert_almost_eq(automation.max_domain, 101000.0, 0.01)


func test_bookends_and_chunks_combine_into_one_event_with_both_shapes():
    # mirrors engine.tres's own combined shape: a start (ignition) clip, a stop (shutdown) clip,
    # AND an rpm-driven crossfade automation all on the SAME event - not split into 3 separate
    # bank events, since MmdSoundBankInstancer merges ignition:/engine:/shutdown: into one
    # definition before calling build().
    var definition := MmdSoundSourceDefinition.new()
    definition.sound_begin = "engine_start"
    definition.sound_end = "engine_stop"
    definition.chunks = [{"threshold": 500, "filename": "idle", "pitch": 0.0}]
    var event:SfxEvent = MmdSoundEventBuilder.build(definition, &"engine", &"rpm")

    assert_eq(event.clips.size(), 2)
    assert_eq((event.clips[0].stream as MaszynaAudioStream).file_path, "engine_start")
    assert_eq((event.clips[1].stream as MaszynaAudioStream).file_path, "engine_stop")
    assert_eq(event.clips[1].trigger_mode, SfxClip.TriggerMode.TRIGGER_SUSTAIN)

    assert_eq(event.automations.size(), 1)
    assert_eq(event.automations[0].clips.size(), 1)
