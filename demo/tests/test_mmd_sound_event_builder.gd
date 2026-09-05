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
    assert_eq((automation.clips[0].stream as MaszynaAudioStream).file_path, "silence1")
    assert_almost_eq(automation.clips[0].offset, 400.0, 0.01)
    assert_almost_eq(automation.clips[0].length, 200.0, 0.01) # fadeout(600) - fadein(400)

    assert_eq((automation.clips[1].stream as MaszynaAudioStream).file_path, "idle_low")
    assert_almost_eq(automation.clips[1].offset, 420.0, 0.01) # 600 - 0.9*(600-400)
    assert_almost_eq(automation.clips[1].length, 580.0, 0.01) # fadeout(1000) - fadein(420)

    assert_eq((automation.clips[2].stream as MaszynaAudioStream).file_path, "idle_high")
    assert_almost_eq(automation.clips[2].offset, 640.0, 0.01) # 1000 - 0.9*(1000-600)
    assert_almost_eq(automation.clips[2].length, 360.0, 0.01) # max(100, 1000) - 640

    assert_almost_eq(automation.max_domain, 1000.0, 0.01)
