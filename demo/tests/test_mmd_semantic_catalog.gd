extends MaszynaGutTest

## Catalog entries added after the "unhandled MMD label" survey - each pins down the exact
## command/state_property pair verified against the C++ wrapper source (see
## mmd_semantic_catalog.gd's own inline comments for the exact source lines), so a future
## refactor that accidentally breaks one of these strings fails a test instead of silently
## regressing in-game.

func test_battery_sw_uses_battery_command_and_state():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("battery_sw")
    assert_eq(entry["widget_class"], CabinButton)
    assert_eq(entry["fixed_fields"]["command"], "battery")
    assert_eq(entry["fixed_fields"]["state_property"], "battery_enabled")
    assert_eq(entry["fixed_fields"]["action"], "battery_toggle")


func test_converter_sw_uses_converter_command_and_state():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("converter_sw")
    assert_eq(entry["widget_class"], CabinButton)
    assert_eq(entry["fixed_fields"]["command"], "converter")
    assert_eq(entry["fixed_fields"]["state_property"], "converter_enabled")


func test_compressor_sw_uses_compressor_command_and_state():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("compressor_sw")
    assert_eq(entry["widget_class"], CabinButton)
    assert_eq(entry["fixed_fields"]["command"], "compressor")
    assert_eq(entry["fixed_fields"]["state_property"], "compressor_enabled")
    assert_eq(entry["fixed_fields"]["action"], "compressor_toggle")


func test_radiochannel_sw_is_an_interactive_multi_position_switch():
    # some vehicles (Radmor-style radios) have a real turnable selector knob under this label -
    # CabinGauge is display-only (no input), so it must be CabinSwitch like mainctrl.
    var entry:Dictionary = MmdSemanticCatalog.get_entry("radiochannel_sw")
    assert_eq(entry["widget_class"], CabinSwitch)
    assert_eq(entry["fixed_fields"]["command_increase"], "radio_channel_increase")
    assert_eq(entry["fixed_fields"]["command_decrease"], "radio_channel_decrease")
    assert_eq(entry["fixed_fields"]["state_property"], "radio_channel")
    assert_eq(entry["fixed_fields"]["switch_min_position"], 1)
    assert_eq(entry["fixed_fields"]["switch_max_position"], 10)
    # channel 1 is the knob's physical rest position, not switch_position=0 (0 isn't a valid
    # channel at all) - without this the knob renders one full step past rest for every channel.
    assert_eq(entry["fixed_fields"]["value_offset"], 1)


func test_radiochannelnext_sw_fires_increase_once_per_press():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("radiochannelnext_sw")
    assert_eq(entry["widget_class"], CabinButton)
    assert_eq(entry["fixed_fields"]["command"], "radio_channel_increase")
    assert_eq(entry["fixed_fields"]["controller_mode"], CabinButton.ControllerMode.On)
    assert_eq(entry["fixed_fields"]["action"], "radio_channel_increase")


func test_radiochannelprev_sw_fires_decrease_once_per_press():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("radiochannelprev_sw")
    assert_eq(entry["widget_class"], CabinButton)
    assert_eq(entry["fixed_fields"]["command"], "radio_channel_decrease")
    assert_eq(entry["fixed_fields"]["controller_mode"], CabinButton.ControllerMode.On)
    assert_eq(entry["fixed_fields"]["action"], "radio_channel_decrease")


func test_pantfront_sw_sends_front_selector_as_command_param():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("pantfront_sw")
    assert_eq(entry["widget_class"], CabinButton)
    assert_eq(entry["fixed_fields"]["command"], "pantograph")
    assert_eq(entry["fixed_fields"]["command_param"], TrainElectricEngine.PANTOGRAPH_FIRST)
    assert_eq(entry["fixed_fields"]["state_property"], "current_collector/pantograph_first_active")


func test_distcounter_binds_total_distance():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("distcounter")
    assert_eq(entry["widget_class"], CabinGauge)
    assert_eq(entry["fixed_fields"]["state_property"], "total_distance")


func test_hvcurrent1_binds_current1():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("hvcurrent1")
    assert_eq(entry["widget_class"], CabinGauge)
    assert_eq(entry["fixed_fields"]["state_property"], "current1")


func test_i_radio_indicator_and_powered_omnilight_are_separate():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("i-radio")
    assert_eq(entry["widget_class"], CabinSpotLight3D)
    assert_eq(entry["fixed_fields"]["state_property"], "radio_enabled")
    assert_true(entry["position_at_submodel"])
    assert_eq(entry["light_widget_class"], CabinOmniLight3D)
    assert_eq(entry["light_fixed_fields"]["state_property"], "radio_powered")
    assert_eq(entry["light_fixed_fields"]["light_color"], Color(0.0, 0.738281, 0.121986, 1.0))
    assert_eq(entry["light_fixed_fields"]["light_energy_on"], 0.05)
    assert_eq(entry["light_fixed_fields"]["omni_range"], 0.1)


func test_cab_light_indicator_and_spotlight_are_separate():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("i-cablight")
    assert_eq(entry["widget_class"], CabinIndicator3D)
    assert_eq(entry["fixed_fields"]["state_property"], "roof_light_enabled")
    assert_eq(entry["light_widget_class"], CabinSpotLight3D)
    assert_true(entry["flip_upward_spotlight"])
    assert_eq(entry["light_fixed_fields"]["state_property"], "roof_light_enabled")
    assert_true(entry["light_fixed_fields"]["light_enabled"])


func test_instrument_light_indicator_and_omnilight_are_separate():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("i-instrumentlight")
    assert_eq(entry["widget_class"], CabinIndicator3D)
    assert_eq(entry["fixed_fields"]["state_property"], "devices_light_enabled")
    assert_eq(entry["light_widget_class"], CabinOmniLight3D)
    assert_eq(entry["light_fixed_fields"]["state_property"], "devices_light_enabled")


func test_front_and_rear_light_indicators_bind_to_the_correct_ilights_bit():
    var expected:Dictionary = {
        "i-upperlight": "lights/front_headlight_upper_enabled",
        "i-leftlight": "lights/front_headlight_left_enabled",
        "i-rightlight": "lights/front_headlight_right_enabled",
        "i-leftend": "lights/front_redmarker_left_enabled",
        "i-rightend": "lights/front_redmarker_right_enabled",
        "i-rearupperlight": "lights/rear_headlight_upper_enabled",
        "i-rearleftlight": "lights/rear_headlight_left_enabled",
        "i-rearrightlight": "lights/rear_headlight_right_enabled",
        "i-rearleftend": "lights/rear_redmarker_left_enabled",
        "i-rearrightend": "lights/rear_redmarker_right_enabled",
    }
    for label:String in expected:
        var entry:Dictionary = MmdSemanticCatalog.get_entry(label)
        assert_eq(entry["widget_class"], CabinSpotLight3D, label)
        assert_eq(entry["fixed_fields"]["state_property"], expected[label], label)
        assert_true(entry["position_at_submodel"], label)
        assert_false(entry["fixed_fields"].has("blink_time"), "%s is steady on/off, not blinking" % label)


func test_i_security_cabsignal_binds_cabsignal_blinking():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("i-security_cabsignal")
    assert_eq(entry["widget_class"], CabinSpotLight3D)
    assert_eq(entry["fixed_fields"]["state_property"], "cabsignal_blinking")
    assert_eq(entry["fixed_fields"]["blink_time"], 0.2)
    assert_false(entry["fixed_fields"].has("light_enabled"))


func test_i_security_aware_indicator_opts_into_an_integrated_light():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("i-security_aware")
    assert_true(entry["fixed_fields"]["light_enabled"])


func test_cabin_spot_light_3d_defaults_light_enabled_to_false():
    var widget := CabinSpotLight3D.new()
    add_child_autofree(widget)
    assert_false(widget.light_enabled)
