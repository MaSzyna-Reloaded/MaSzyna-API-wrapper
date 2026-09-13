extends MaszynaGutTest

## sound_on/sound_off's play-on-transition must fire on every FLASH (matches SM42's own
## hand-authored reference exactly - cabin_blinker.gd's `blink` signal plays a click on every
## blink cycle while the alerter stays active, not once per alert session), not just on the
## overall enabled=false->true/true->false transition - confirmed real regression: binding the
## sound trigger to the static "enabled" flag instead of the per-flash "active_now" value meant
## the click only fired once when the alert started, never again while it kept blinking.
func test_sound_plays_on_every_blink_transition_not_just_session_start():
    var widget := CabinSpotLight3D.new()
    widget.blink_time = 999.0 # long enough that the timer itself never fires during this test -
    # blink transitions below are all driven manually via _on_blink_timeout()
    add_child_autofree(widget)

    # AudioStreamGenerator (not MaszynaAudioStream) - it always instantiates a valid playback with
    # no file dependency, so this test can trigger the real .play() call path without erroring.
    var stream_on := AudioStreamGenerator.new()
    var stream_off := AudioStreamGenerator.new()
    widget.sound_on = stream_on
    widget.sound_off = stream_off
    widget.enabled = true

    widget._update_state() # session start: active_now flips false -> true
    assert_eq(widget._sound.stream, stream_on)

    widget._on_blink_timeout() # next flash: active_now flips true -> false
    assert_eq(widget._sound.stream, stream_off, "should click again on the very next flash, not stay silent for the rest of the session")

    widget._on_blink_timeout() # flips back false -> true
    assert_eq(widget._sound.stream, stream_on)


func test_sound_does_not_replay_when_state_is_unchanged():
    var widget := CabinSpotLight3D.new()
    add_child_autofree(widget)

    var stream_on := AudioStreamGenerator.new()
    widget.sound_on = stream_on
    widget.enabled = true

    widget._update_state()
    widget._sound.stream = null # clear so a spurious replay would be observable
    widget._update_state() # no state change since the previous call

    assert_null(widget._sound.stream)


## light_enabled=false is for indicator labels with no real per-vehicle lamp data (e.g. i-radio,
## i-security_cabsignal) - the light must stay dark even while active, but the on/off submodel
## pair and the click sound (both driven from the same active_now, not the light) must still work.
func test_light_enabled_false_suppresses_light_but_not_submodel_or_sound():
    var widget := CabinSpotLight3D.new()
    widget.light_enabled = false
    widget.light_energy_on = 1.0
    add_child_autofree(widget)

    var on_target := Node3D.new()
    var off_target := Node3D.new()
    add_child_autofree(on_target)
    add_child_autofree(off_target)
    widget.on_target_path = widget.get_path_to(on_target)
    widget.off_target_path = widget.get_path_to(off_target)
    widget._on_target = on_target
    widget._off_target = off_target

    var stream_on := AudioStreamGenerator.new()
    widget.sound_on = stream_on
    widget.enabled = true

    widget._update_state()

    assert_eq(widget._target_light_energy, 0.0, "light itself must stay dark with no real lamp data")
    assert_true(on_target.visible, "submodel toggling is independent of the light itself")
    assert_false(off_target.visible)
    assert_eq(widget._sound.stream, stream_on, "click sound is independent of the light itself")
