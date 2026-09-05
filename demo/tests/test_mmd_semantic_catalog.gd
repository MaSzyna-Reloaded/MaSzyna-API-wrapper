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


func test_radiochannel_sw_is_a_passive_position_gauge_not_a_switch():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("radiochannel_sw")
    assert_eq(entry["widget_class"], CabinGauge)
    assert_eq(entry["fixed_fields"]["state_property"], "radio_channel")


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


func test_i_radio_binds_radio_enabled_not_radio_powered():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("i-radio")
    assert_eq(entry["widget_class"], CabinSpotLight3D)
    assert_eq(entry["fixed_fields"]["state_property"], "radio_enabled")
    assert_true(entry["position_at_submodel"])
    # no real per-vehicle lamp data exists for this indicator - it must rely on
    # CabinSpotLight3D's own light_enabled=false default rather than opting the light back in,
    # so it doesn't visibly show the czuwak (alerter)'s own borrowed color/range instead.
    assert_false(entry["fixed_fields"].has("light_enabled"))


func test_i_security_cabsignal_binds_cabsignal_blinking():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("i-security_cabsignal")
    assert_eq(entry["widget_class"], CabinSpotLight3D)
    assert_eq(entry["fixed_fields"]["state_property"], "cabsignal_blinking")
    assert_eq(entry["fixed_fields"]["blink_time"], 0.2)
    assert_false(entry["fixed_fields"].has("light_enabled"))


func test_i_security_aware_is_the_only_entry_opting_into_a_real_light():
    var entry:Dictionary = MmdSemanticCatalog.get_entry("i-security_aware")
    assert_true(entry["fixed_fields"]["light_enabled"])


func test_cabin_spot_light_3d_defaults_light_enabled_to_false():
    var widget := CabinSpotLight3D.new()
    add_child_autofree(widget)
    assert_false(widget.light_enabled)
