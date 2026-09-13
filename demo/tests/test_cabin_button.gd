extends MaszynaGutTest

## Regression coverage for radiochannelnext_sw/radiochannelprev_sw not reacting to their bound
## InputMap action while otherwise-identical monostable=false buttons (compressor_sw etc.) work
## fine - the only difference is monostable=true + controller_mode=ControllerMode.On, so this
## isolates that exact combination with a simulated real key event (not a direct method call),
## the same path a physical keypress takes.

const TEST_ACTION := "test_cabin_button_action"

func before_all():
    InputMap.add_action(TEST_ACTION)
    InputMap.action_add_event(TEST_ACTION, _make_key_event())

func after_all():
    InputMap.erase_action(TEST_ACTION)

func _make_key_event(pressed:bool = true) -> InputEventKey:
    var event := InputEventKey.new()
    event.physical_keycode = KEY_EQUAL
    event.pressed = pressed
    return event

func test_monostable_on_mode_fires_command_once_on_press_via_real_input_event():
    var widget := CabinButton.new()
    widget.monostable = true
    widget.controller_mode = CabinButton.ControllerMode.On
    widget.action = TEST_ACTION
    add_child_autofree(widget)
    await wait_idle_frames(1)

    Input.parse_input_event(_make_key_event(true))
    await wait_idle_frames(1)

    assert_true(widget.pushed, "pressing the bound key should set pushed=true")

    Input.parse_input_event(_make_key_event(false))
    await wait_idle_frames(1)

    assert_false(widget.pushed, "releasing the bound key should set pushed=false")
