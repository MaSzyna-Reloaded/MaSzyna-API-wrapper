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

    var stream_on := MaszynaAudioStream.new()
    var stream_off := MaszynaAudioStream.new()
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

    var stream_on := MaszynaAudioStream.new()
    widget.sound_on = stream_on
    widget.enabled = true

    widget._update_state()
    widget._sound.stream = null # clear so a spurious replay would be observable
    widget._update_state() # no state change since the previous call

    assert_null(widget._sound.stream)
