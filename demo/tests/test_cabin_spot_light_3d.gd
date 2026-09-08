extends MaszynaGutTest


func test_indicator_sound_plays_on_every_blink_transition():
    var indicator:CabinIndicator3D = CabinIndicator3D.new()
    indicator.blink_time = 999.0
    add_child_autofree(indicator)

    var sound:CabinIndicatorSound3D = CabinIndicatorSound3D.new()
    var stream_on:AudioStreamGenerator = AudioStreamGenerator.new()
    var stream_off:AudioStreamGenerator = AudioStreamGenerator.new()
    sound.sound_on = stream_on
    sound.sound_off = stream_off
    add_child_autofree(sound)
    indicator.state_changed.connect(sound.set_active)

    indicator.enabled = true
    indicator._update_state()
    assert_eq(sound.stream, stream_on)

    indicator._on_blink_timeout()
    assert_eq(sound.stream, stream_off)

    indicator._on_blink_timeout()
    assert_eq(sound.stream, stream_on)


func test_indicator_does_not_emit_when_state_is_unchanged():
    var indicator:CabinIndicator3D = CabinIndicator3D.new()
    add_child_autofree(indicator)
    watch_signals(indicator)

    indicator.enabled = true
    indicator._update_state()
    indicator._update_state()

    assert_signal_emit_count(indicator, "state_changed", 1)


func test_indicator_targets_and_spotlight_are_independent_nodes():
    var indicator:CabinIndicator3D = CabinIndicator3D.new()
    var light:CabinSpotLight3D = CabinSpotLight3D.new()
    light.light_energy_on = 1.0
    add_child_autofree(indicator)
    add_child_autofree(light)

    var on_target:Node3D = Node3D.new()
    var off_target:Node3D = Node3D.new()
    add_child_autofree(on_target)
    add_child_autofree(off_target)
    indicator.on_target_path = indicator.get_path_to(on_target)
    indicator.off_target_path = indicator.get_path_to(off_target)
    indicator._process_dirty()
    indicator.state_changed.connect(light.set_enabled)

    indicator.enabled = true
    indicator._update_state()

    assert_true(on_target.visible)
    assert_false(off_target.visible)
    assert_eq(light._target_light_energy, 1.0)
