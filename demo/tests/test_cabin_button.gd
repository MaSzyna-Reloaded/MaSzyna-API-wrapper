extends MaszynaGutTest

## Regression coverage for radiochannelnext_sw/radiochannelprev_sw not reacting to their bound
## InputMap action while otherwise-identical monostable=false buttons (compressor_sw etc.) work
## fine - the only difference is monostable=true + controller_mode=ControllerMode.On, so this
## isolates that exact combination with a simulated real key event (not a direct method call),
## the same path a physical keypress takes.

const TEST_ACTION := "test_cabin_button_action"
const SM42:VehicleModel = preload("res://tests/fixtures/sm42_vehicle.tres")
const SHOWN_CONTROL:StringName = &"test_shown_control"

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

## E186's vigilance pedal (pedal_sifa rot 0.008 -0.008) is modelled pushed: released it rests at
## the MMD offset, pushed it returns to the model's own pose.
func test_rotation_offset_is_the_released_pose():
    const ROTATION_DEGREES:float = 0.008 * 360.0
    var cab:Node3D = Node3D.new()
    var pedal:MeshInstance3D = MeshInstance3D.new()
    pedal.name = "Pedal"
    cab.add_child(pedal)
    var widget:CabinButton = CabinButton.new()
    widget.mesh_rotation = Vector3(0.0, ROTATION_DEGREES, 0.0)
    widget.mesh_rotation_offset = Vector3(0.0, -ROTATION_DEGREES, 0.0)
    cab.add_child(widget)
    widget.mesh_path = NodePath("../Pedal")
    add_child_autofree(cab)
    await wait_idle_frames(3)

    var released:Basis = Basis(Vector3.UP, deg_to_rad(-ROTATION_DEGREES))
    assert_true(pedal.transform.basis.is_equal_approx(released), "released pedal should rest at the offset")

    widget.pushed = true
    await wait_seconds(1.5)

    assert_true(pedal.transform.basis.is_equal_approx(Basis.IDENTITY), "pushed pedal should reach the modelled pose")

## FINDINGS.md 2026-09-29: a cab built on a running vehicle showed each button's state by setting
## `pushed`, which acted on the vehicle - it lowered 3E/1-42's pantograph and opened its line
## breaker. Showing the vehicle's state acts on nothing; only the hand does.
func test_showing_the_vehicle_state_does_not_act():
    var physics_node:VehiclePhysicsNode = build_vehicle_node("CabinButtonTest", SM42)
    # freed before the node it is driven by, as test_driver_route_table.gd does
    var vehicle_node:RailVehicle3D = RailVehicle3D.new()
    add_child(vehicle_node)
    vehicle_node.controller_path = vehicle_node.get_path_to(physics_node)
    await wait_idle_frames(2)
    var vehicle:RID = vehicle_node.get_rid()
    var acted:Array[StringName] = []
    var handler:Callable = func(_state:CabinState, action:StringName, _value:Variant) -> Variant:
        acted.append(action)
        return null
    var cab:int = CabinSystem.occupied_cab(vehicle)
    CabinSystem.register_control(vehicle, cab, SHOWN_CONTROL, handler)
    var widget:CabinButton = CabinButton.new()
    widget.control_id = SHOWN_CONTROL
    widget.state_property = "main_switch_enabled"
    widget.pushed = true
    add_child_autofree(widget)
    await wait_idle_frames(1)

    widget.set_vehicle_rid(vehicle)
    await wait_idle_frames(2)

    assert_false(widget.pushed, "the button shows the open line breaker")
    assert_eq(acted.size(), 0, "and acts on nothing to show it")
    widget.press()
    assert_eq(acted.size(), 1, "the hand acts")
    CabinSystem.unregister_control(vehicle, cab, SHOWN_CONTROL, handler)
    remove_child(vehicle_node)
    vehicle_node.queue_free()

## A cab rebuilt on a vehicle (the player back in it, the other cab occupied) builds its buttons
## anew. One with no vehicle state behind it - E186's universal1, the screen's pantograph page -
## showed itself off while the cab still held it on, and the first press did nothing.
func test_rebuilt_button_shows_what_the_cab_holds():
    var physics_node:VehiclePhysicsNode = build_vehicle_node("CabinButtonRebuilt", SM42)
    var vehicle_node:RailVehicle3D = RailVehicle3D.new()
    add_child(vehicle_node)
    vehicle_node.controller_path = vehicle_node.get_path_to(physics_node)
    await wait_idle_frames(2)
    var vehicle:RID = vehicle_node.get_rid()
    var cab:int = CabinSystem.occupied_cab(vehicle)
    # what LegacyCabinForwardCommands does for a cab-only control
    var handler:Callable = func(state:CabinState, _action:StringName, value:Variant) -> Variant:
        state.set_value(SHOWN_CONTROL, value)
        return null
    CabinSystem.register_control(vehicle, cab, SHOWN_CONTROL, handler)
    var first:CabinButton = CabinButton.new()
    first.control_id = SHOWN_CONTROL
    add_child_autofree(first)
    first.set_vehicle_rid(vehicle)
    await wait_idle_frames(2)
    first.press()
    assert_true(CabinSystem.get_control(vehicle, cab, SHOWN_CONTROL), "the press is held by the cab")

    var rebuilt:CabinButton = CabinButton.new()
    rebuilt.control_id = SHOWN_CONTROL
    add_child_autofree(rebuilt)
    rebuilt.set_vehicle_rid(vehicle)
    await wait_idle_frames(2)

    assert_true(rebuilt.pushed, "the rebuilt button shows the control on")
    rebuilt.press()
    assert_false(CabinSystem.get_control(vehicle, cab, SHOWN_CONTROL), "and the first press turns it off")
    CabinSystem.unregister_control(vehicle, cab, SHOWN_CONTROL, handler)
    remove_child(vehicle_node)
    vehicle_node.queue_free()
