extends MaszynaGutTest

## The light selector (lights_sw) steps through the FIZ LightsList: presets and lights the vehicle
## with the preset it lands on (TTrain::OnCommand_lightspresetactivatenext, Train.cpp:5193;
## TDynamicObject::SetLights, DynObj.cpp:7322).

var train: VehicleController
var lighting: VehicleLighting


func before_each():
    train = build_vehicle("TestLightPresets")
    lighting = MoverVehicleLighting.new()
    var upper := LightListItem.new()
    upper.cabin_a_head_light = true
    upper.cabin_b_left_red_signal = true
    upper.cabin_b_right_red_signal = true
    var lower_pair := LightListItem.new()
    lower_pair.cabin_a_left_white_signal = true
    lower_pair.cabin_a_right_white_signal = true
    var presets:Array[LightListItem] = [upper, lower_pair]
    lighting.lights_list = presets
    lighting.lights_default_selector_position = 1
    train.add_component(lighting)
    train.apply_configuration()
    await wait_idle_frames(2)


func test_the_selector_starts_at_the_default_preset():
    assert_eq(train.state["light_position"], 1)
    assert_eq(train.state["light_selector_position"], 0)
    assert_eq(train.config["light_position_max"], 1)


func test_stepping_the_selector_lights_the_next_preset():
    train.send_command("increase_light_selector_position")
    assert_eq(train.state["light_position"], 2)
    assert_true(train.state["lights/front_headlight_left_enabled"])
    assert_true(train.state["lights/front_headlight_right_enabled"])
    assert_false(train.state["lights/front_headlight_upper_enabled"])

    train.send_command("decrease_light_selector_position")
    assert_eq(train.state["light_position"], 1)
    assert_true(train.state["lights/front_headlight_upper_enabled"])
    assert_true(train.state["lights/rear_redmarker_left_enabled"])
    assert_true(train.state["lights/rear_redmarker_right_enabled"])


# Train.cpp:5205 - without Wrap= the selector stops at its last preset
func test_the_selector_does_not_wrap_unless_told_to():
    train.send_command("increase_light_selector_position")
    train.send_command("increase_light_selector_position")
    assert_eq(train.state["light_position"], 2)
