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


func test_indicator_light_label_is_parsed_as_bare_submodel_name():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    var indicator:MmdInstrumentDescriptor = _find(definition, "i-security_aware")
    assert_not_null(indicator)
    assert_eq(indicator.submodel_name, "czuwak_lamp")
    assert_eq(indicator.animation_type, "", "i-*: labels have no rot/mov shape at all")
    # if this single-token shape were force-fed through the 5-token instrument parser, the
    # include right after it would desync.
    var radio:MmdInstrumentDescriptor = _find(definition, "radio_sw")
    assert_not_null(radio, "include right after an i-*: indicator label should still parse")


func test_block_form_indicator_light_reads_submodel_from_inside_the_block():
    # confirmed real (dynamic/pkp/su45_v2/301d.mmd): "i-security_cabsignal: { i-shp soundinc:
    # ... sounddec: ... }" - the submodel name is the block's FIRST token, not a token before "{"
    # (unlike every other block-form instrument label).
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    var indicator:MmdInstrumentDescriptor = _find(definition, "i-security_cabsignal")
    assert_not_null(indicator)
    assert_eq(indicator.submodel_name, "i-shp")
    assert_eq(indicator.sound_increase, "shp_on")
    assert_eq(indicator.sound_decrease, "shp_off")
    var radio:MmdInstrumentDescriptor = _find(definition, "radio_sw")
    assert_not_null(radio, "include right after a block-form i-*: indicator label should still parse")


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


func test_bare_filename_sound_fields_are_parsed_and_extension_stripped():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    var brakectrl:MmdInstrumentDescriptor = _find(definition, "brakectrl")
    assert_not_null(brakectrl)
    assert_eq(brakectrl.sound_increase, "brake_inc")
    assert_eq(brakectrl.sound_decrease, "brake_dec")
    # block form's "type:" (unrelated field, appears before the sound fields) must still parse
    assert_eq(brakectrl.button_type, "return")


func test_numbered_sound_position_fields_are_parsed():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    var main_on:MmdInstrumentDescriptor = _find(definition, "main_on_bt")
    assert_not_null(main_on)
    assert_eq(main_on.sound_positions.get(1), "click_pos1")
    assert_eq(main_on.sound_positions.get(-1), "click_neg1")


func test_bracketed_random_sound_list_resolves_to_one_entry():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    var fuelpump:MmdInstrumentDescriptor = _find(definition, "fuelpump_sw")
    assert_not_null(fuelpump)
    assert_true(
            fuelpump.sound_increase in ["variant_a", "variant_b", "variant_c"],
            "should resolve to exactly one of the bracketed candidates")


func test_random_sound_choice_is_persisted_across_reparse():
    var random_choices:Dictionary = {}
    var first:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, random_choices)
    var second:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, random_choices)
    var fuelpump_first:MmdInstrumentDescriptor = _find(first, "fuelpump_sw")
    var fuelpump_second:MmdInstrumentDescriptor = _find(second, "fuelpump_sw")
    assert_eq(fuelpump_first.sound_increase, fuelpump_second.sound_increase)


func test_nested_sound_subblock_extracts_only_soundmain():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    var oilpump:MmdInstrumentDescriptor = _find(definition, "oilpump_sw")
    assert_not_null(oilpump)
    assert_eq(oilpump.sound_increase, "nested_click")


func test_sound_fields_do_not_desync_following_labels():
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(FIXTURE_PATH, 1, {})
    # oilpump_sw's nested soundinc:{...} block is the last instrument before the includes -
    # if its brace-matching miscounted, the includes right after it would fail to parse.
    var radio:MmdInstrumentDescriptor = _find(definition, "radio_sw")
    assert_not_null(radio, "include right after a nested sound sub-block should still parse")
    assert_eq(radio.submodel_name, "radio_antenna")


func test_position_at_submodel_instance_uses_visual_aabb_center_not_pivot():
    # a submodel's authored pivot (its transform origin) is frequently off to one side (e.g. its
    # mounting point) rather than at its visual center. A directional "push forward off the
    # surface" correction was tried and reverted - confirmed real that a submodel's local Z
    # orientation isn't consistent across vehicles' art (right on SU45, wrong on EP09/SM42) - so
    # plain AABB center is the deliberate, safer default.
    var box_mesh := BoxMesh.new() # local AABB centered on the node's own origin
    var mesh_node:MeshInstance3D = add_child_autofree(MeshInstance3D.new())
    mesh_node.mesh = box_mesh
    mesh_node.position = Vector3(10.0, 0.0, 0.0)

    var widget:Node3D = add_child_autofree(Node3D.new())
    MmdCabinInstancer._position_at_submodel_instance(widget, mesh_node)

    assert_eq(widget.global_position, mesh_node.global_position)


func test_build_indicator_lights_positions_at_on_submodel_and_wires_both_targets():
    # "i-*:" labels declare a bare base name that is never itself a real submodel - the original
    # engine always searches "<name>_on"/"<name>_off" instead (Button.cpp:32-33), confirmed real
    # by live diagnostics ("Submodel 'czuwak'/'ca' not found") once the bare name is looked up.
    var descriptor := MmdInstrumentDescriptor.new()
    descriptor.label = "i-security_aware"
    descriptor.submodel_name = "czuwak"
    # confirmed real (dynamic/pkp/su45_v2/301d.mmd): "i-security_aware: { i-czuwak soundinc: ...
    # sounddec: ... }" - the click sound on each on/off transition.
    descriptor.sound_increase = "light_ca_start"
    descriptor.sound_decrease = "light_ca_stop"
    var entry:Dictionary = MmdSemanticCatalog.get_entry("i-security_aware")

    var on_node:Node3D = add_child_autofree(Node3D.new())
    on_node.position = Vector3(1.0, 2.0, 3.0)
    var off_node:Node3D = add_child_autofree(Node3D.new())
    off_node.position = Vector3(9.0, 9.0, 9.0)
    var submodel_index:Dictionary = {"czuwak_on": [on_node], "czuwak_off": [off_node]}

    var generated_root:Node3D = add_child_autofree(Node3D.new())
    var controller:TrainController = add_child_autofree(TrainController.new())
    var diagnostics:Array[Dictionary] = []
    MmdCabinInstancer._build_indicator_lights(descriptor, entry, controller, submodel_index, generated_root, 1, diagnostics)

    assert_eq(generated_root.get_child_count(), 1, "should prefer the _on submodel over _off")
    var widget:CabinSpotLight3D = generated_root.get_child(0)
    assert_eq(widget.global_position, on_node.global_position, "no depth offset - plain Node3D isn't a VisualInstance3D")
    assert_eq(widget.get_node(widget.on_target_path), on_node)
    assert_eq(widget.get_node(widget.off_target_path), off_node)
    assert_eq((widget.sound_on as MaszynaAudioStream).file_path, "light_ca_start")
    assert_eq((widget.sound_off as MaszynaAudioStream).file_path, "light_ca_stop")
    assert_eq(diagnostics.size(), 0)


func test_build_indicator_lights_builds_one_widget_per_matched_instance():
    # confirmed real: sm_42_cabin.tscn's own hand-authored reference has 3 physical lamp housings
    # ("CzuwakOmni1/2/3") for its one "i-security_aware:" label - one widget per matched submodel
    # instance, not just the first, unlike every other instrument label.
    var descriptor := MmdInstrumentDescriptor.new()
    descriptor.label = "i-security_aware"
    descriptor.submodel_name = "czuwak"
    var entry:Dictionary = MmdSemanticCatalog.get_entry("i-security_aware")

    var on_a:Node3D = add_child_autofree(Node3D.new())
    var on_b:Node3D = add_child_autofree(Node3D.new())
    var submodel_index:Dictionary = {"czuwak_on": [on_a, on_b]}

    var generated_root:Node3D = add_child_autofree(Node3D.new())
    var controller:TrainController = add_child_autofree(TrainController.new())
    var diagnostics:Array[Dictionary] = []
    MmdCabinInstancer._build_indicator_lights(descriptor, entry, controller, submodel_index, generated_root, 1, diagnostics)

    assert_eq(generated_root.get_child_count(), 2)


func test_build_indicator_lights_reports_missing_on_and_off():
    var descriptor := MmdInstrumentDescriptor.new()
    descriptor.label = "i-security_aware"
    descriptor.submodel_name = "nowhere"
    var entry:Dictionary = MmdSemanticCatalog.get_entry("i-security_aware")

    var generated_root:Node3D = add_child_autofree(Node3D.new())
    var controller:TrainController = add_child_autofree(TrainController.new())
    var diagnostics:Array[Dictionary] = []
    MmdCabinInstancer._build_indicator_lights(descriptor, entry, controller, {}, generated_root, 1, diagnostics)

    assert_eq(generated_root.get_child_count(), 0)
    assert_eq(diagnostics.size(), 1)
    assert_eq(diagnostics[0]["code"], "MMD_SUBMODEL_NOT_FOUND")


func _find(definition:MmdCabinDefinition, label:String) -> MmdInstrumentDescriptor:
    for descriptor:MmdInstrumentDescriptor in definition.instruments:
        if descriptor.label == label:
            return descriptor
    return null
