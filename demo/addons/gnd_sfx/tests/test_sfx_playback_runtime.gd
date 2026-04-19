extends MaszynaGutTest

var runtime: SfxPlaybackRuntime
var players: Array = []


func before_each() -> void:
    runtime = SfxPlaybackRuntime.new()
    players = []
    for _i in range(4):
        var player := AudioStreamPlayer.new()
        players.append(player)
        add_child_autoqfree(player)
    runtime.set_players(players)


func after_each() -> void:
    runtime.clear()
    players.clear()


func test_time_offset_track_starts_only_after_threshold() -> void:
    var event := SfxEvent.new()
    event.name = &"timed"
    event.tracks = [_make_track(0.5)]

    runtime.play(event)
    assert_eq(runtime._active_voices.size(), 0, "Track should not start before crossing time offset")

    runtime.update(0.6)
    assert_eq(runtime._active_voices.size(), 1, "Track should start after time threshold is crossed")


func test_seek_rebuilds_event_tracks_for_new_offset() -> void:
    var event := SfxEvent.new()
    event.name = &"seekable"
    event.tracks = [_make_track(1.0)]

    runtime.play(event)
    assert_eq(runtime._active_voices.size(), 0)

    runtime.seek(event.name, 1.1)
    assert_eq(runtime._active_voices.size(), 1, "Seek should activate tracks whose offset is already behind playback time")


func test_stream_offset_is_separate_from_timeline_offset() -> void:
    var event := SfxEvent.new()
    event.name = &"stream_offset"
    var track := _make_track(1.0)
    track.stream_offset = 0.25
    event.tracks = [track]

    runtime.play(event, 1.5)
    assert_eq(runtime._active_voices.size(), 1)

    var start_position := runtime._resolve_voice_start_position(runtime._get_latest_instance(event.name), track)
    assert_eq(start_position, 0.75, "Time tracks should add elapsed event time on top of stream_offset")


func test_time_track_length_limits_playback_segment() -> void:
    var event := SfxEvent.new()
    event.name = &"time_length"
    var track := _make_track(0.0, 25.0)
    track.stream_offset = 14.0
    track.length = 4.0
    track.fade_out_curve = _make_linear_curve(0.0, 4.0, 1.0, 0.0)
    event.tracks = [track]

    runtime.play(event)
    var voice: SfxPlaybackRuntime.ActiveVoice = runtime._active_voices[0]

    assert_eq(voice.stream_end_position, 18.0, "Track length should cap playback at stream_offset + length")
    assert_eq(runtime._resolve_remaining_track_time(voice, 14.0), 4.0, "Remaining time should be measured against the shortened segment")
    assert_eq(runtime._sample_time_fade_out_curve(track.fade_out_curve, 4.0), 1.0, "Fade-out should start at the beginning of the shortened tail")
    assert_eq(runtime._sample_time_fade_out_curve(track.fade_out_curve, 0.0), 0.0, "Fade-out should end at silence at the shortened segment end")


func test_time_track_curves_use_local_time_and_remaining_time() -> void:
    var event := SfxEvent.new()
    event.name = &"curve_domains"
    var track := _make_track(0.0, 25.0)
    track.stream_offset = 14.0
    track.fade_in_curve = _make_linear_curve(0.0, 4.0, 0.0, 1.0)
    track.fade_out_curve = _make_linear_curve(0.0, 4.0, 1.0, 0.0)
    event.tracks = [track]

    runtime.play(event)
    var voice: SfxPlaybackRuntime.ActiveVoice = runtime._active_voices[0]

    assert_eq(runtime._resolve_local_track_time(voice, voice.stream_start_position), 0.0, "Fade-in domain should start at local track time")
    assert_eq(runtime._resolve_remaining_track_time(voice, voice.stream.get_length() - 4.0), 4.0, "Fade-out domain should count remaining playback time")
    assert_eq(runtime._sample_time_fade_out_curve(track.fade_out_curve, 4.0), 1.0, "Fade-out should stay at full gain when four seconds remain")
    assert_eq(runtime._sample_time_fade_out_curve(track.fade_out_curve, 0.0), 0.0, "Fade-out should reach silence at the end of playback")


func test_time_track_fade_out_curve_does_not_require_reversed_shape() -> void:
    var track := _make_track(0.0, 25.0)
    track.stream_offset = 14.0
    track.fade_out_curve = _make_linear_curve(0.0, 4.0, 1.0, 0.0)

    assert_eq(track.stream.get_length(), 25.0)
    assert_eq(runtime._sample_time_fade_out_curve(track.fade_out_curve, 11.0), 1.0, "Playback should start audible before the last four seconds")
    assert_eq(runtime._sample_time_fade_out_curve(track.fade_out_curve, 4.0), 1.0, "Fade-out should start when four seconds remain")
    assert_eq(runtime._sample_time_fade_out_curve(track.fade_out_curve, 2.0), 0.5, "Fade-out should interpolate naturally toward the end")
    assert_eq(runtime._sample_time_fade_out_curve(track.fade_out_curve, 0.0), 0.0, "Fade-out should end at silence")


func test_automation_tracks_start_after_parameter_threshold_crossing() -> void:
    var event := SfxEvent.new()
    event.name = &"engine"
    event.automations = [_make_automation(&"rpm", 0.0, 1000.0, 500.0)]

    runtime.play(event, 0.0, {"rpm": 250.0})
    assert_eq(runtime._active_voices.size(), 0, "Automation track should not start below offset threshold")

    runtime.modulate(event.name, {"rpm": 750.0})
    assert_eq(runtime._active_voices.size(), 1, "Automation track should start after threshold crossing")


func test_automation_tracks_use_stream_offset_without_parameter_delta() -> void:
    var event := SfxEvent.new()
    event.name = &"automation_stream_offset"
    var automation := _make_automation(&"rpm", 0.0, 1000.0, 500.0)
    var track: SfxTrack = automation.tracks[0]
    track.stream_offset = 0.4
    event.automations = [automation]

    runtime.play(event, 0.0, {"rpm": 800.0})
    assert_eq(runtime._active_voices.size(), 1)

    var start_position := runtime._resolve_voice_start_position(runtime._get_latest_instance(event.name), track, automation)
    assert_eq(start_position, 0.4, "Automation tracks should start from stream_offset only")


func test_phase_locked_automation_tracks_use_event_clock_for_start_position() -> void:
    var event := SfxEvent.new()
    event.name = &"phase_locked"
    var automation := _make_automation(&"speed", 0.0, 200.0, 10.0)
    automation.phase_locked = true
    automation.phase_period = 2.0
    var track: SfxTrack = automation.tracks[0]
    track.stream = _make_test_wav(4.0)
    track.stream_offset = 0.25
    event.automations = [automation]

    runtime.play(event, 1.25, {"speed": 20.0})
    assert_eq(runtime._active_voices.size(), 1)

    var expected_start := 1.5
    var voice: SfxPlaybackRuntime.ActiveVoice = runtime._active_voices[0]
    assert_almost_eq(voice.stream_start_position, expected_start, 0.0001, "Phase-locked automation tracks should derive start position from event time")


func test_phase_locked_automation_tracks_apply_phase_offset() -> void:
    var event := SfxEvent.new()
    event.name = &"phase_offset"
    var automation := _make_automation(&"speed", 0.0, 200.0, 10.0)
    automation.phase_locked = true
    automation.phase_period = 2.0
    var track: SfxTrack = automation.tracks[0]
    track.stream = _make_test_wav(4.0)
    track.phase_offset = 0.5
    event.automations = [automation]

    runtime.play(event, 1.25, {"speed": 20.0})
    assert_eq(runtime._active_voices.size(), 1)

    var voice: SfxPlaybackRuntime.ActiveVoice = runtime._active_voices[0]
    assert_almost_eq(voice.stream_start_position, 1.75, 0.0001, "Track phase_offset should shift the shared automation phase")


func test_automation_curves_are_shifted_by_track_offset() -> void:
    var track := _make_track(460.0)
    track.fade_in_curve = _make_linear_curve(0.0, 100.0, 0.0, 1.0)

    assert_eq(runtime._sample_automation_curve(track.fade_in_curve, track, 460.0, 1.0), 0.0, "Fade-in should start at the track offset")
    assert_eq(runtime._sample_automation_curve(track.fade_in_curve, track, 510.0, 1.0), 0.5, "Curve midpoint should land offset units later in automation space")
    assert_eq(runtime._sample_automation_curve(track.fade_in_curve, track, 560.0, 1.0), 1.0, "Curve max domain should also be shifted by the track offset")


func test_automation_pitch_curve_uses_full_automation_domain() -> void:
    var automation := _make_automation(&"rpm", 0.0, 1000.0, 460.0)
    automation.pitch_curve = _make_linear_curve(0.0, 1000.0, 1.0, 2.0)
    var track: SfxTrack = automation.tracks[0]
    track.pitch_curve = _make_linear_curve(0.0, 100.0, 1.0, 1.5)

    assert_eq(runtime._sample_automation_domain_curve(automation.pitch_curve, 0.0, 1.0), 1.0, "Automation pitch curve should start at the automation domain start")
    assert_eq(runtime._sample_automation_domain_curve(automation.pitch_curve, 500.0, 1.0), 1.5, "Automation pitch curve should sample directly from automation_value")
    assert_eq(runtime._sample_automation_domain_curve(automation.pitch_curve, 1000.0, 1.0), 2.0, "Automation pitch curve should reach the end of its domain")
    assert_eq(runtime._sample_automation_curve(track.pitch_curve, track, 460.0, 1.0), 1.0, "Track pitch curve should still be local to track offset")
    assert_eq(runtime._sample_automation_curve(track.pitch_curve, track, 560.0, 1.0), 1.5, "Track pitch curve should still use local automation value")


func test_automation_fade_out_curve_is_reversed_in_local_track_domain() -> void:
    var track := _make_track(310.0)
    track.fade_in_curve = _make_linear_curve(0.0, 90.0, 0.0, 1.0)
    track.fade_out_curve = _make_linear_curve(0.0, 90.0, 1.0, 0.0)

    assert_eq(runtime._sample_automation_curve(track.fade_in_curve, track, 310.0, 1.0), 0.0, "Fade-in should start silent at the threshold")
    assert_eq(runtime._sample_automation_curve(track.fade_in_curve, track, 355.0, 1.0), 0.5, "Fade-in should use local automation value")
    assert_eq(runtime._sample_automation_curve(track.fade_in_curve, track, 400.0, 1.0), 1.0, "Fade-in should reach full gain at the end of the local domain")
    assert_eq(runtime._sample_automation_fade_out_curve(track.fade_out_curve, track, 310.0, 1.0), 0.0, "Fade-out should read the end of the curve at the threshold")
    assert_eq(runtime._sample_automation_fade_out_curve(track.fade_out_curve, track, 355.0, 1.0), 0.5, "Fade-out midpoint should mirror local automation space")
    assert_eq(runtime._sample_automation_fade_out_curve(track.fade_out_curve, track, 400.0, 1.0), 1.0, "Fade-out should not zero the track at the end of the local domain")


func test_automation_length_shifts_fade_out_curve_to_track_end() -> void:
    var track := _make_track(405.0)
    track.length = 285.0
    track.fade_in_curve = _make_linear_curve(0.0, 90.0, 0.0, 1.0)
    track.fade_out_curve = _make_linear_curve(0.0, 90.0, 1.0, 0.0)

    assert_eq(runtime._sample_automation_curve(track.fade_in_curve, track, 600.0, 1.0), 1.0, "Fade-in should already be at full gain by the time fade-out starts")
    assert_eq(runtime._sample_automation_fade_out_curve(track.fade_out_curve, track, 600.0, 1.0), 1.0, "Fade-out should start one curve duration before offset + length")
    assert_eq(runtime._sample_automation_fade_out_curve(track.fade_out_curve, track, 645.0, 1.0), 0.5, "Fade-out should interpolate across the last curve duration before track end")
    assert_eq(runtime._sample_automation_fade_out_curve(track.fade_out_curve, track, 690.0, 1.0), 0.0, "Fade-out should end exactly at offset + length")


func test_automation_length_uses_last_curve_window_before_track_end() -> void:
    var track := _make_track(760.0)
    track.length = 180.0
    track.fade_in_curve = _make_linear_curve(0.0, 90.0, 0.0, 1.0)
    track.fade_out_curve = _make_linear_curve(0.0, 90.0, 1.0, 0.0)

    assert_eq(runtime._sample_automation_curve(track.fade_in_curve, track, 856.0, 1.0), 1.0, "Fade-in should already be fully open at 856")
    assert_gt(runtime._sample_automation_fade_out_curve(track.fade_out_curve, track, 856.0, 1.0), 0.9, "Fade-out should only be slightly attenuated at 856")


func test_stop_with_release_keeps_instance_alive_until_adsr_finishes() -> void:
    var event := SfxEvent.new()
    event.name = &"released"
    event.adsr_enabled = true
    event.release = 0.2
    event.tracks = [_make_track(0.0)]

    runtime.play(event)
    assert_true(runtime._instances.has(event.name))
    assert_eq(runtime._active_voices.size(), 1)

    runtime.stop(event.name, false)
    assert_true(runtime._instances.has(event.name), "Event instance should stay alive during release")

    runtime.update(0.1)
    assert_true(runtime._instances.has(event.name), "Release should still be in progress")

    runtime.update(0.2)
    assert_false(runtime._instances.has(event.name), "Event instance should be removed after release")
    assert_eq(runtime._active_voices.size(), 0, "Voices should be stopped after release")


func test_polyphony_enabled_allows_multiple_instances_of_same_event() -> void:
    var event := SfxEvent.new()
    event.name = &"poly"
    event.polyphony_enabled = true
    event.tracks = [_make_track(0.0)]

    runtime.play(event)
    runtime.play(event)

    assert_eq(runtime._get_instances_for_event(event.name).size(), 2, "Polyphonic event should keep multiple instances alive")
    assert_eq(runtime._active_voices.size(), 2, "Each polyphonic instance should allocate its own voice")


func test_polyphony_disabled_replaces_existing_instance() -> void:
    var event := SfxEvent.new()
    event.name = &"mono"
    event.polyphony_enabled = false
    event.tracks = [_make_track(0.0)]

    runtime.play(event)
    runtime.play(event)

    assert_eq(runtime._get_instances_for_event(event.name).size(), 1, "Monophonic event should replace the previous instance")
    assert_eq(runtime._active_voices.size(), 1, "Monophonic event should keep only one active voice")


func test_stop_targets_latest_instance_for_polyphonic_event() -> void:
    var event := SfxEvent.new()
    event.name = &"poly_stop"
    event.polyphony_enabled = true
    event.tracks = [_make_track(0.0)]

    runtime.play(event)
    runtime.play(event)
    assert_eq(runtime._get_instances_for_event(event.name).size(), 2)

    runtime.stop(event.name, true)

    assert_eq(runtime._get_instances_for_event(event.name).size(), 1, "Named stop should only remove the newest polyphonic instance")
    assert_eq(runtime._active_voices.size(), 1, "One older polyphonic voice should remain active")


func test_is_playing_tracks_instance_lifecycle() -> void:
    var event := SfxEvent.new()
    event.name = &"playing_state"
    event.tracks = [_make_track(0.0)]

    assert_false(runtime.is_playing(event.name))

    runtime.play(event)
    assert_true(runtime.is_playing(event.name))

    runtime.stop(event.name, true)
    assert_false(runtime.is_playing(event.name))


func test_sustain_track_does_not_start_during_initial_playback() -> void:
    var event := SfxEvent.new()
    event.name = &"sustain_wait"
    var sustain_track := _make_track(0.0)
    sustain_track.trigger_mode = SfxTrack.TriggerMode.TRIGGER_SUSTAIN
    event.tracks = [sustain_track]

    runtime.play(event)

    assert_eq(runtime._active_voices.size(), 0, "Sustain tracks should not start on play")
    assert_true(runtime._instances.has(event.name), "Event instance should remain active until stopped")


func test_stop_with_release_triggers_sustain_track_once() -> void:
    var event := SfxEvent.new()
    event.name = &"sustain_once"
    event.adsr_enabled = true
    event.release = 0.5
    var sustain_track := _make_track(0.0)
    sustain_track.trigger_mode = SfxTrack.TriggerMode.TRIGGER_SUSTAIN
    event.tracks = [sustain_track]

    runtime.play(event)
    runtime.stop(event.name, false)

    assert_eq(runtime._active_voices.size(), 1, "Sustain track should start on non-immediate stop")

    runtime.stop(event.name, false)
    assert_eq(runtime._active_voices.size(), 1, "Repeated non-immediate stop should not retrigger sustain track")


func test_sustain_track_offset_uses_release_clock_and_stream_offset() -> void:
    var event := SfxEvent.new()
    event.name = &"sustain_offset"
    var sustain_track := _make_track(0.15, 1.0)
    sustain_track.trigger_mode = SfxTrack.TriggerMode.TRIGGER_SUSTAIN
    sustain_track.stream_offset = 0.3
    event.tracks = [sustain_track]

    runtime.play(event)
    runtime.stop(event.name, false)
    assert_eq(runtime._active_voices.size(), 0, "Delayed sustain track should not be queued after stop")
    assert_false(runtime._instances.has(event.name), "Instance should be collected when only delayed sustain remains")


func test_sustain_track_start_position_does_not_inherit_event_playback_time() -> void:
    var event := SfxEvent.new()
    event.name = &"sustain_stream_start"
    var sustain_track := _make_track(0.0, 1.0)
    sustain_track.trigger_mode = SfxTrack.TriggerMode.TRIGGER_SUSTAIN
    sustain_track.stream_offset = 0.1
    event.tracks = [sustain_track]

    runtime.play(event)
    runtime.update(0.25)
    runtime.stop(event.name, false)

    var voice: SfxPlaybackRuntime.ActiveVoice = runtime._active_voices[0]
    assert_eq(voice.stream_start_position, 0.1, "Sustain track should start from stream_offset, not from elapsed event playback time")


func test_immediate_stop_does_not_trigger_sustain_track() -> void:
    var event := SfxEvent.new()
    event.name = &"sustain_immediate"
    var sustain_track := _make_track(0.0)
    sustain_track.trigger_mode = SfxTrack.TriggerMode.TRIGGER_SUSTAIN
    event.tracks = [sustain_track]

    runtime.play(event)
    runtime.stop(event.name, true)

    assert_false(runtime._instances.has(event.name), "Immediate stop should remove the event instance")
    assert_eq(runtime._active_voices.size(), 0, "Immediate stop should not start sustain tracks")


func test_stop_without_adsr_keeps_instance_alive_until_sustain_track_finishes() -> void:
    var event := SfxEvent.new()
    event.name = &"sustain_no_adsr"
    var sustain_track := _make_track(0.0, 0.2)
    sustain_track.trigger_mode = SfxTrack.TriggerMode.TRIGGER_SUSTAIN
    event.tracks = [sustain_track]

    runtime.play(event)
    runtime.stop(event.name, false)

    assert_true(runtime._instances.has(event.name), "Non-immediate stop without ADSR should keep the instance alive for sustain playback")
    assert_eq(runtime._active_voices.size(), 1, "Sustain track should still start without ADSR")

    runtime._players[0].stop()
    runtime.handle_player_finished(runtime._players[0])
    runtime.update(0.0)

    assert_false(runtime._instances.has(event.name), "Instance should be removed after sustain playback ends")
    assert_eq(runtime._active_voices.size(), 0, "No voices should remain after sustain playback ends")


func test_stop_without_adsr_stops_existing_timeline_loop_before_starting_sustain_track() -> void:
    var event := SfxEvent.new()
    event.name = &"sustain_cuts_loop"
    var timeline_track := _make_track(0.0, 1.0)
    timeline_track.stream = _make_test_wav(1.0, true)
    var sustain_track := _make_track(0.0, 0.2)
    sustain_track.trigger_mode = SfxTrack.TriggerMode.TRIGGER_SUSTAIN
    event.tracks = [timeline_track, sustain_track]

    runtime.play(event)
    assert_eq(runtime._active_voices.size(), 1, "Timeline track should start on play")
    assert_eq(runtime._active_voices[0].track, timeline_track, "Initial voice should belong to the looping timeline track")

    runtime.stop(event.name, false)

    assert_eq(runtime._active_voices.size(), 1, "Only the sustain track should remain after non-ADSR stop")
    assert_eq(runtime._active_voices[0].track, sustain_track, "Non-ADSR stop should cut the old loop and start the sustain track")


func test_stop_without_adsr_uses_track_fade_out_curve_for_existing_voice() -> void:
    var event := SfxEvent.new()
    event.name = &"sustain_fades_loop"
    var timeline_track := _make_track(0.0, 1.0)
    timeline_track.stream = _make_test_wav(1.0, true)
    timeline_track.fade_out_curve = _make_linear_curve(0.0, 0.2, 1.0, 0.0)
    var sustain_track := _make_track(0.0, 0.2)
    sustain_track.trigger_mode = SfxTrack.TriggerMode.TRIGGER_SUSTAIN
    event.tracks = [timeline_track, sustain_track]

    runtime.play(event)
    runtime.stop(event.name, false)

    assert_eq(runtime._active_voices.size(), 2, "Looping timeline voice should fade out while sustain track starts")
    assert_true(runtime._active_voices[0].track == timeline_track or runtime._active_voices[1].track == timeline_track)

    runtime.update(0.1)
    var fading_voice := runtime._find_voice(runtime._get_latest_instance(event.name), timeline_track)
    assert_not_null(fading_voice, "Timeline voice should still exist halfway through its stop fade")
    assert_almost_eq(db_to_linear(fading_voice.player.volume_db), 0.5, 0.1, "Timeline voice should follow the fade_out_curve during non-ADSR stop")

    runtime.update(0.11)
    assert_null(runtime._find_voice(runtime._get_latest_instance(event.name), timeline_track), "Timeline voice should be removed after its fade_out_curve duration")


func test_stop_without_event_adsr_uses_track_adsr_release_for_looping_voice() -> void:
    var event := SfxEvent.new()
    event.name = &"track_adsr_release"
    var timeline_track := _make_track(0.0, 1.0)
    timeline_track.stream = _make_test_wav(1.0, true)
    timeline_track.adsr_enabled = true
    timeline_track.release = 0.2
    var sustain_track := _make_track(0.0, 0.2)
    sustain_track.trigger_mode = SfxTrack.TriggerMode.TRIGGER_SUSTAIN
    event.tracks = [timeline_track, sustain_track]

    runtime.play(event)
    runtime.stop(event.name, false)

    assert_eq(runtime._active_voices.size(), 2, "Looping voice should release while sustain track starts")

    runtime.update(0.1)
    var fading_voice := runtime._find_voice(runtime._get_latest_instance(event.name), timeline_track)
    assert_not_null(fading_voice, "Timeline voice should still exist halfway through track ADSR release")
    assert_almost_eq(db_to_linear(fading_voice.player.volume_db), 0.5, 0.1, "Track ADSR release should attenuate the looping voice immediately after stop")

    runtime.update(0.11)
    assert_null(runtime._find_voice(runtime._get_latest_instance(event.name), timeline_track), "Timeline voice should end after its track ADSR release")


func test_track_adsr_sustain_applies_while_voice_is_held() -> void:
    var event := SfxEvent.new()
    event.name = &"track_adsr_hold"
    var track := _make_track(0.0, 1.0)
    track.adsr_enabled = true
    track.attack = 0.1
    track.decay = 0.1
    track.sustain = 0.25
    event.tracks = [track]

    runtime.play(event)
    runtime.update(0.1)
    runtime.update(0.1)
    runtime.update(0.05)

    assert_eq(runtime._active_voices.size(), 1)
    assert_almost_eq(_player_linear_gain(0), 0.25, 0.1, "Track ADSR sustain should hold the voice below unity even before stop")


func test_horn_like_overlap_stop_starts_sustain_track() -> void:
    var local_runtime := SfxPlaybackRuntime.new()
    var local_players: Array = []
    for _i in range(2):
        var player := AudioStreamPlayer.new()
        local_players.append(player)
        add_child_autoqfree(player)
    local_runtime.set_players(local_players)

    var event := SfxEvent.new()
    event.name = &"horn_like"

    var start_track := _make_track(0.0, 0.36)
    var loop_track := _make_track(0.2, 1.0)
    loop_track.stream = _make_test_wav(1.0, true)
    var sustain_track := _make_track(0.0, 0.3)
    sustain_track.trigger_mode = SfxTrack.TriggerMode.TRIGGER_SUSTAIN

    event.tracks = [start_track, loop_track, sustain_track]

    local_runtime.play(event)
    local_runtime.update(0.25)
    assert_eq(local_runtime._active_voices.size(), 2, "Horn overlap should have start and loop voices active before stop")

    local_runtime.stop(event.name, false)
    assert_eq(local_runtime._active_voices.size(), 1, "Sustain track should replace overlapping horn voices on stop")

    var sustain_voice := local_runtime._find_voice(local_runtime._get_latest_instance(event.name), sustain_track)
    assert_not_null(sustain_voice, "Horn-like sustain track should start during overlap stop")
    assert_true(sustain_voice.player.playing, "Sustain voice should still be playing after overlapping voices are stopped")


func test_new_voice_steals_oldest_releasing_voice_before_delaying_start() -> void:
    var local_runtime := SfxPlaybackRuntime.new()
    var local_players: Array = []
    for _i in range(2):
        var player := AudioStreamPlayer.new()
        local_players.append(player)
        add_child_autoqfree(player)
    local_runtime.set_players(local_players)

    var releasing_event := SfxEvent.new()
    releasing_event.name = &"releasing"
    var releasing_track := _make_track(0.0, 1.0)
    releasing_track.stream = _make_test_wav(1.0, true)
    releasing_track.fade_out_curve = _make_linear_curve(0.0, 0.5, 1.0, 0.0)
    var releasing_sustain := _make_track(0.0, 0.25)
    releasing_sustain.trigger_mode = SfxTrack.TriggerMode.TRIGGER_SUSTAIN
    releasing_event.tracks = [releasing_track, releasing_sustain]

    var held_event := SfxEvent.new()
    held_event.name = &"held"
    var held_track := _make_track(0.0, 1.0)
    held_track.stream = _make_test_wav(1.0, true)
    held_event.tracks = [held_track]

    var newcomer_event := SfxEvent.new()
    newcomer_event.name = &"newcomer"
    newcomer_event.tracks = [_make_track(0.0, 0.2)]

    local_runtime.play(releasing_event)
    local_runtime.play(held_event)
    local_runtime.stop(releasing_event.name, false)

    assert_eq(local_runtime._active_voices.size(), 2, "Releasing sustain and held voice should occupy the full pool before the steal")
    assert_null(local_runtime._find_voice(local_runtime._get_latest_instance(releasing_event.name), releasing_track), "Stop should already evict the older releasing loop when sustain starts at the limit")
    assert_not_null(local_runtime._find_voice(local_runtime._get_latest_instance(releasing_event.name), releasing_sustain), "Sustain should be the remaining releasing voice before the next play")

    local_runtime.play(newcomer_event)

    assert_eq(local_runtime._active_voices.size(), 2, "New voice should start immediately by stealing a slot")
    assert_not_null(local_runtime._find_voice(local_runtime._get_latest_instance(newcomer_event.name), newcomer_event.tracks[0]), "Newcomer voice should exist right after play")
    assert_null(local_runtime._find_voice(local_runtime._get_latest_instance(releasing_event.name), releasing_sustain), "Remaining releasing voice should be stolen before touching non-releasing voices")
    assert_not_null(local_runtime._find_voice(local_runtime._get_latest_instance(held_event.name), held_track), "Non-releasing voice should not be stolen while a releasing one exists")


func test_new_voice_steals_oldest_global_voice_when_no_releasing_voice_exists() -> void:
    var local_runtime := SfxPlaybackRuntime.new()
    var local_players: Array = []
    for _i in range(2):
        var player := AudioStreamPlayer.new()
        local_players.append(player)
        add_child_autoqfree(player)
    local_runtime.set_players(local_players)

    var first_event := SfxEvent.new()
    first_event.name = &"first"
    var first_track := _make_track(0.0, 1.0)
    first_track.stream = _make_test_wav(1.0, true)
    first_event.tracks = [first_track]

    var second_event := SfxEvent.new()
    second_event.name = &"second"
    var second_track := _make_track(0.0, 1.0)
    second_track.stream = _make_test_wav(1.0, true)
    second_event.tracks = [second_track]

    var newcomer_event := SfxEvent.new()
    newcomer_event.name = &"newest"
    newcomer_event.tracks = [_make_track(0.0, 0.2)]

    local_runtime.play(first_event)
    local_runtime.play(second_event)
    local_runtime.play(newcomer_event)

    assert_eq(local_runtime._active_voices.size(), 2, "New voice should steal instead of waiting for a free player")
    assert_null(local_runtime._find_voice(local_runtime._get_latest_instance(first_event.name), first_track), "Oldest global voice should be stolen first")
    assert_not_null(local_runtime._find_voice(local_runtime._get_latest_instance(second_event.name), second_track), "More recent global voice should remain active")
    assert_not_null(local_runtime._find_voice(local_runtime._get_latest_instance(newcomer_event.name), newcomer_event.tracks[0]), "New voice should start immediately")


func test_rapid_replay_with_non_immediate_stop_does_not_delay_new_starts_at_track_limit() -> void:
    var local_runtime := SfxPlaybackRuntime.new()
    var local_players: Array = []
    for _i in range(2):
        var player := AudioStreamPlayer.new()
        local_players.append(player)
        add_child_autoqfree(player)
    local_runtime.set_players(local_players)

    var event := SfxEvent.new()
    event.name = &"rapid"
    event.polyphony_enabled = true
    var loop_track := _make_track(0.0, 1.0)
    loop_track.stream = _make_test_wav(1.0, true)
    loop_track.fade_out_curve = _make_linear_curve(0.0, 0.5, 1.0, 0.0)
    var sustain_track := _make_track(0.0, 0.2)
    sustain_track.trigger_mode = SfxTrack.TriggerMode.TRIGGER_SUSTAIN
    event.tracks = [loop_track, sustain_track]

    local_runtime.play(event)
    local_runtime.stop(event.name, false)
    local_runtime.play(event)

    var instances := local_runtime._get_instances_for_event(event.name)
    assert_eq(instances.size(), 2, "Polyphonic replay should keep the releasing instance while starting a new one")
    assert_eq(local_runtime._active_voices.size(), 2, "Replay should reuse a slot immediately instead of delaying the new start")
    assert_not_null(local_runtime._find_voice(instances[1], loop_track), "Newest instance should start its loop immediately")
    assert_not_null(local_runtime._find_voice(instances[0], sustain_track), "Existing sustain should remain audible if it is not the stolen voice")


func test_automation_equal_power_crossfade_applies_sqrt() -> void:
    var event := SfxEvent.new()
    event.name = &"equal_power"
    var automation := _make_automation(&"param", 0.0, 1.0, 0.0)
    automation.crossfade_mode = SfxAutomation.CrossfadeMode.EQUAL_POWER
    var track := automation.tracks[0]
    track.fade_in_curve = _make_linear_curve(0.0, 1.0, 0.0, 1.0)
    event.automations = [automation]
    
    runtime.play(event, 0.0, {&"param": 1.0})
    
    var player = runtime._players[0]
    # gain=1.0, sqrt(1.0)=1.0
    assert_almost_eq(db_to_linear(player.volume_db), 1.0, 0.01, "EQUAL_POWER mode should apply sqrt to track gain")


func test_linear_crossfade_normalizes_overlapping_automation_tracks() -> void:
    var event := SfxEvent.new()
    event.name = &"linear_overlap"
    var automation := SfxAutomation.new()
    automation.parameter_name = &"param"
    automation.min_domain = 0.0
    automation.max_domain = 1.0
    automation.crossfade_mode = SfxAutomation.CrossfadeMode.LINEAR
    var first_track := _make_track(0.0)
    first_track.fade_in_curve = _make_constant_curve(1.0)
    first_track.fade_out_curve = _make_linear_curve(0.0, 1.0, 1.0, 0.0)
    var second_track := _make_track(0.0)
    second_track.fade_in_curve = _make_linear_curve(0.0, 1.0, 0.0, 1.0)
    second_track.fade_out_curve = _make_constant_curve(1.0)
    automation.tracks = [first_track, second_track]
    event.automations = [automation]

    runtime.play(event, 0.0, {&"param": 0.5})

    assert_eq(runtime._active_voices.size(), 2, "Both overlapping automation tracks should be active")
    assert_almost_eq(_player_linear_gain(0), 0.5, 0.01, "The first track should be normalized to half gain at midpoint")
    assert_almost_eq(_player_linear_gain(1), 0.5, 0.01, "The second track should be normalized to half gain at midpoint")
    assert_almost_eq(_sum_player_linear_gain([0, 1]), 1.0, 0.01, "Linear overlap should preserve total amplitude")


func test_equal_power_crossfade_normalizes_group_power() -> void:
    var event := SfxEvent.new()
    event.name = &"equal_power_overlap"
    var automation := SfxAutomation.new()
    automation.parameter_name = &"param"
    automation.min_domain = 0.0
    automation.max_domain = 1.0
    automation.crossfade_mode = SfxAutomation.CrossfadeMode.EQUAL_POWER
    var first_track := _make_track(0.0)
    first_track.fade_in_curve = _make_constant_curve(1.0)
    first_track.fade_out_curve = _make_linear_curve(0.0, 1.0, 1.0, 0.0)
    var second_track := _make_track(0.0)
    second_track.fade_in_curve = _make_linear_curve(0.0, 1.0, 0.0, 1.0)
    second_track.fade_out_curve = _make_constant_curve(1.0)
    automation.tracks = [first_track, second_track]
    event.automations = [automation]

    runtime.play(event, 0.0, {&"param": 0.5})

    assert_eq(runtime._active_voices.size(), 2, "Both overlapping automation tracks should be active")
    assert_almost_eq(_player_linear_gain(0), sqrt(0.5), 0.01, "Equal-power midpoint should keep each voice at sqrt(0.5)")
    assert_almost_eq(_player_linear_gain(1), sqrt(0.5), 0.01, "Equal-power midpoint should mirror the matching voice gain")
    assert_almost_eq(_sum_player_power([0, 1]), 1.0, 0.02, "Equal-power overlap should preserve total power")
    assert_gt(_player_linear_gain(0), 0.5, "Equal-power should differ from linear normalization at midpoint")


func test_automation_crossfade_normalization_is_scoped_per_event_instance_and_automation() -> void:
    var event_a := SfxEvent.new()
    event_a.name = &"scope_a"
    var automation_a := SfxAutomation.new()
    automation_a.parameter_name = &"param"
    automation_a.min_domain = 0.0
    automation_a.max_domain = 1.0
    automation_a.crossfade_mode = SfxAutomation.CrossfadeMode.LINEAR
    var a_first := _make_track(0.0)
    a_first.fade_in_curve = _make_constant_curve(1.0)
    a_first.fade_out_curve = _make_linear_curve(0.0, 1.0, 1.0, 0.0)
    var a_second := _make_track(0.0)
    a_second.fade_in_curve = _make_linear_curve(0.0, 1.0, 0.0, 1.0)
    a_second.fade_out_curve = _make_constant_curve(1.0)
    automation_a.tracks = [a_first, a_second]
    event_a.automations = [automation_a]

    var event_b := SfxEvent.new()
    event_b.name = &"scope_b"
    var automation_b := SfxAutomation.new()
    automation_b.parameter_name = &"param"
    automation_b.min_domain = 0.0
    automation_b.max_domain = 1.0
    automation_b.crossfade_mode = SfxAutomation.CrossfadeMode.LINEAR
    var b_track := _make_track(0.0)
    b_track.fade_in_curve = _make_constant_curve(1.0)
    b_track.fade_out_curve = _make_constant_curve(1.0)
    automation_b.tracks = [b_track]
    event_b.automations = [automation_b]

    runtime.play(event_a, 0.0, {&"param": 0.5})
    runtime.play(event_b, 0.0, {&"param": 0.5})

    assert_eq(runtime._active_voices.size(), 3, "The control event should add one independent automation voice")
    assert_almost_eq(_sum_player_linear_gain([0, 1]), 1.0, 0.01, "Normalization should only apply within the overlapping event instance")
    assert_almost_eq(_player_linear_gain(2), 1.0, 0.01, "An independent event instance should keep its own full gain")


func _make_track(offset: float, duration_seconds := 0.5) -> SfxTrack:
    var track := SfxTrack.new()
    track.stream = _make_test_wav(duration_seconds)
    track.offset = offset
    return track


func _make_automation(name: StringName, min_domain: float, max_domain: float, offset: float) -> SfxAutomation:
    var automation := SfxAutomation.new()
    automation.parameter_name = name
    automation.min_domain = min_domain
    automation.max_domain = max_domain
    automation.tracks = [_make_track(offset)]
    return automation


func _make_test_wav(duration_seconds := 0.5, loop := false) -> AudioStreamWAV:
    var stream := AudioStreamWAV.new()
    stream.format = AudioStreamWAV.FORMAT_8_BITS
    stream.mix_rate = 44100
    stream.stereo = false
    stream.loop_mode = AudioStreamWAV.LOOP_FORWARD if loop else AudioStreamWAV.LOOP_DISABLED
    var sample_count := int(round(44100.0 * duration_seconds))
    var audio_data := PackedByteArray()
    audio_data.resize(sample_count)
    for index in range(sample_count):
        audio_data[index] = 128
    stream.data = audio_data
    return stream


func _make_linear_curve(x0: float, x1: float, y0: float, y1: float) -> Curve:
    var curve := Curve.new()
    curve.min_domain = minf(x0, x1)
    curve.max_domain = maxf(x0, x1)
    curve.min_value = minf(y0, y1)
    curve.max_value = maxf(y0, y1)
    curve.add_point(Vector2(x0, y0))
    curve.add_point(Vector2(x1, y1))
    return curve


func _make_constant_curve(value: float) -> Curve:
    var curve := Curve.new()
    curve.min_domain = 0.0
    curve.max_domain = 1.0
    curve.min_value = value
    curve.max_value = value
    curve.add_point(Vector2(0.0, value))
    curve.add_point(Vector2(1.0, value))
    return curve


func _player_linear_gain(index: int) -> float:
    return db_to_linear(runtime._players[index].volume_db)


func _sum_player_linear_gain(indices: Array[int]) -> float:
    var total := 0.0
    for index in indices:
        total += _player_linear_gain(index)
    return total


func _sum_player_power(indices: Array[int]) -> float:
    var total := 0.0
    for index in indices:
        var gain := _player_linear_gain(index)
        total += gain * gain
    return total
