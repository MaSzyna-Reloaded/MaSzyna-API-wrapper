extends MaszynaGutTest

const FIXTURE_PATH := "res://tests/fixtures/test_cabin.mmd"


func test_parse_body_model_reads_top_level_models_key():
    assert_eq(MmdCabinInstancer.parse_body_model(FIXTURE_PATH), "test_body")


func test_cab1_bounds_and_driver_position():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    assert_eq(definition.cab_number, 1)
    assert_eq(definition.bounds_min, Vector3(-1.0, 1.0, -2.0))
    assert_eq(definition.bounds_max, Vector3(1.0, 2.0, -1.0))
    assert_eq(definition.driver_pos, Vector3(0.5, 3.0, -3.0))
    assert_eq(definition.model_relpath, "test_kabina")


func test_cab2_does_not_leak_cab1_driver_position():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 2, {})
    assert_eq(definition.cab_number, 2)
    assert_eq(definition.driver_pos, Vector3(-0.5, 3.0, -3.0))


func test_instruments_are_shared_between_cabs():
    var cab1:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    var cab2:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 2, {})
    assert_eq(cab1.instruments.size(), cab2.instruments.size(), "instrument list is shared, not per-cab")


func test_plain_form_instrument_is_parsed():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    var mainctrl:MmdInstrumentDescriptor = _find(definition, "mainctrl")
    assert_not_null(mainctrl)
    assert_eq(mainctrl.submodel_name, "nastawnik")
    assert_eq(mainctrl.animation_type, "rot")
    assert_almost_eq(mainctrl.scale, -0.056, 0.0001)
    assert_almost_eq(mainctrl.friction, 0.1, 0.0001)


func test_colon_glued_label_and_value_are_split():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    var dirkey:MmdInstrumentDescriptor = _find(definition, "dirkey")
    assert_not_null(dirkey, "dirkey:kier (no space after colon) should still parse")
    assert_eq(dirkey.submodel_name, "kier")


func test_block_form_instrument_captures_button_type():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    var brakectrl:MmdInstrumentDescriptor = _find(definition, "brakectrl")
    assert_not_null(brakectrl)
    assert_eq(brakectrl.submodel_name, "zasadniczy")
    assert_eq(brakectrl.button_type, "return")


func test_line_and_block_comments_are_stripped():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    # If comments leaked into the token stream, "security_reset_bt" (the label right after the
    # block comment) would either be missing or have a garbled submodel_name.
    var security:MmdInstrumentDescriptor = _find(definition, "security_reset_bt")
    assert_not_null(security)
    assert_eq(security.submodel_name, "czuwak")


func test_duplicate_labels_are_preserved_in_order():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    var tachometers:Array[MmdInstrumentDescriptor] = []
    for descriptor:MmdInstrumentDescriptor in definition.instruments:
        if descriptor.label == "tachometer":
            tachometers.append(descriptor)
    assert_eq(tachometers.size(), 2)
    assert_eq(tachometers[0].submodel_name, "predkosciomierz")
    assert_eq(tachometers[1].submodel_name, "predkosciomierz2")


func test_unrecognized_label_does_not_desync_following_labels():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    var unknown:MmdInstrumentDescriptor = _find(definition, "unknownlabel")
    assert_not_null(unknown)
    # If "unknownlabel"'s 5 tokens were misconsumed, the include right after it would desync too.
    var radio:MmdInstrumentDescriptor = _find(definition, "radio_sw")
    assert_not_null(radio, "include right after an unrecognized label should still parse")


func test_include_substitutes_positional_parameter():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    var radio:MmdInstrumentDescriptor = _find(definition, "radio_sw")
    assert_not_null(radio)
    assert_eq(radio.submodel_name, "radio_antenna")


func test_random_include_choice_is_persisted_across_reparse():
    var random_choices:Dictionary = {}
    var first:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, random_choices)
    var second:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, random_choices)
    var whistle_first:MmdInstrumentDescriptor = _find(first, "whistle_bt")
    var whistle_second:MmdInstrumentDescriptor = _find(second, "whistle_bt")
    assert_not_null(whistle_first)
    assert_eq(whistle_first.submodel_name, whistle_second.submodel_name)


func test_cab0_definition_ends_the_instrument_section():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    for descriptor:MmdInstrumentDescriptor in definition.instruments:
        assert_ne(descriptor.label, "driver0pos", "cab0's own fields must not leak in as an instrument")


func _find(definition:MmdCabinDefinition, label:String) -> MmdInstrumentDescriptor:
    for descriptor:MmdInstrumentDescriptor in definition.instruments:
        if descriptor.label == label:
            return descriptor
    return null
