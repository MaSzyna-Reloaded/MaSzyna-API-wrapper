extends MaszynaGutTest

## Regression coverage for a real in-game bug: radio_channel's valid range is 1..10 (0 isn't a
## valid channel), but the knob's physical rest position is still switch_position=1 (its first
## notch), not switch_position=0 - without value_offset, switch_position=1 rendered as ONE STEP
## PAST rest, and decrease could never visually return to rest at all.

func test_value_offset_shifts_rotation_but_not_switch_position_or_clamping():
    var widget := CabinSwitch.new()
    widget.switch_min_position = 1
    widget.switch_max_position = 10
    widget.value_offset = 1
    widget.mesh_rotation = Vector3(0, 36.0, 0) # per-unit shape, as computed for value=1
    add_child_autofree(widget)

    widget.switch_position = 1
    widget._update_state()

    assert_eq(widget.switch_position, 1, "switch_position itself must stay the real, unshifted state value")
    assert_eq(widget._target_mesh_rotation, Vector3.ZERO, "channel 1 is the knob's rest position - zero rotation")

    widget.switch_position = 2
    widget._update_state()
    assert_eq(widget._target_mesh_rotation, Vector3(0, 36.0, 0), "channel 2 is exactly one step past rest")


func test_value_offset_defaults_to_zero_matching_prior_behavior():
    var widget := CabinSwitch.new()
    widget.mesh_rotation = Vector3(0, 36.0, 0)
    add_child_autofree(widget)

    widget.switch_position = 1
    widget._update_state()

    assert_eq(widget._target_mesh_rotation, Vector3(0, 36.0, 0), "with no offset, position 1 still rotates one full step (mainctrl's own existing behavior)")


## _process_tool()'s own periodic refresh (every ~0.05s, independent of _update_state()) had its
## own separate, un-offset copy of this same computation - a real regression that slipped past
## the tests above because they only ever called _update_state() directly, never letting
## _process_tool() actually run a frame. This drives the widget through real _process() frames
## instead, the same path the running game takes, so it would have caught it.
func test_value_offset_also_applies_after_a_real_process_frame():
    var widget := CabinSwitch.new()
    widget.switch_min_position = 1
    widget.switch_max_position = 10
    widget.value_offset = 1
    widget.mesh_rotation = Vector3(0, 36.0, 0)
    widget.switch_position = 1
    add_child_autofree(widget)

    await wait_idle_frames(10)

    assert_eq(widget._target_mesh_rotation, Vector3.ZERO, "channel 1 must still be rest position after _process_tool()'s own periodic refresh")


func _key_event(action:String, echo:bool) -> InputEventKey:
    var source:InputEventKey = InputMap.action_get_events(action)[0] as InputEventKey
    var event := InputEventKey.new()
    event.keycode = source.keycode
    event.physical_keycode = source.physical_keycode
    event.pressed = true
    event.echo = echo
    return event


## OnCommand_mastercontrollerincrease acts on key repeat too (Train.cpp:1096) - holding + keeps
## stepping the controller; controls without repeat_on_hold (e.g. the reverser) ignore the echo.
func test_repeat_on_hold_steps_on_key_echo():
    var widget := CabinSwitch.new()
    widget.switch_max_position = 10
    widget.action_increase = "main_controller_increase"
    widget.repeat_on_hold = true
    add_child_autofree(widget)

    widget._input(_key_event("main_controller_increase", false))
    widget._input(_key_event("main_controller_increase", true))
    widget._input(_key_event("main_controller_increase", true))
    assert_eq(widget.switch_position, 3, "press + two key repeats step three times")

    widget.repeat_on_hold = false
    widget._input(_key_event("main_controller_increase", true))
    assert_eq(widget.switch_position, 3, "without repeat_on_hold the key repeat is ignored")
