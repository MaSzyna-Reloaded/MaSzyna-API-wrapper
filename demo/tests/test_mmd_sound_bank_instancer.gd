extends MaszynaGutTest

const FIXTURE_PATH := "res://tests/fixtures/test_sound.mmd"


func test_ignition_merges_into_empty_engine_sound_begin_but_explicit_soundend_wins_over_shutdown():
    var context := MmdImportContext.new()
    context.base_dir = FIXTURE_PATH.get_base_dir()
    var definitions:Array[MmdSoundSourceDefinition] = MmdSoundSourceParser.parse(FIXTURE_PATH, context)
    var internal_data:Array[MmdSoundSourceDefinition] = MmdSoundSourceParser.parse_internal_data(FIXTURE_PATH, context)
    MmdSoundBankInstancer._merge_ignition_and_shutdown_into_engine(definitions, internal_data)

    var engine:MmdSoundSourceDefinition = null
    for definition:MmdSoundSourceDefinition in definitions:
        if definition.label == "engine":
            engine = definition
    assert_not_null(engine)

    # engine: itself has no soundbegin: in the fixture - ignition: (internaldata:) fills it in.
    assert_eq(engine.sound_begin, "engine-start")
    # engine: DOES have its own soundend: ("engine-stop") - shutdown:'s "engine-shutdown-unused"
    # must NOT override it.
    assert_eq(engine.sound_end, "engine-stop")
    # the chunk table from engine:'s own block is untouched by the merge.
    assert_eq(engine.chunks.size(), 3)


func test_buzzer_and_buzzershp_are_catalog_matched_as_standalone_events():
    # unlike ignition:/shutdown: (merged into "engine" and never catalog-matched themselves),
    # buzzer:/buzzershp: are their own independent bank events - Train.cpp:10111-10151 plays them
    # as a SEPARATE, later-triggered stage from the light's own on/off click.
    assert_true(MmdSoundCatalog.has_label("buzzer"))
    assert_true(MmdSoundCatalog.has_label("buzzershp"))
    assert_eq(MmdSoundCatalog.get_entry("buzzer")["state_property"], "beeping")
    assert_eq(MmdSoundCatalog.get_entry("buzzershp")["state_property"], "cabsignal_beeping")


func test_internal_buzzers_route_to_the_cabin_bank() -> void:
    for label:String in ["buzzer", "buzzershp"]:
        var definition:MmdSoundSourceDefinition = MmdSoundSourceDefinition.new()
        definition.label = label
        MmdSoundBankInstancer._apply_original_defaults(definition, true)

        assert_eq(definition.placement, &"internal")


func test_build_creates_independent_exterior_and_cabin_banks() -> void:
    var vehicle:RailVehicle3D = RailVehicle3D.new()
    add_child(vehicle)
    var diagnostics:Array[Dictionary] = []
    MmdSoundBankInstancer.build_into(
            vehicle, ProjectSettings.globalize_path(FIXTURE_PATH), "", {}, diagnostics)

    var exterior:SfxPlayer3D = vehicle.get_node("ExteriorSfxPlayer3D") as SfxPlayer3D
    var cabin:SfxPlayer3D = vehicle.get_node("CabinSfxPlayer3D") as SfxPlayer3D
    assert_not_null(exterior)
    assert_not_null(cabin)
    assert_ne(exterior.bank, cabin.bank)
    assert_not_null(exterior.bank.get_event(&"engine"))
    assert_null(exterior.bank.get_event(&"buzzer"))
    assert_not_null(cabin.bank.get_event(&"buzzer"))
    assert_null(cabin.bank.get_event(&"engine"))

    vehicle.queue_free()
    await wait_idle_frames(2)


func test_original_default_placements_keep_engine_and_horns_external() -> void:
    var engine:MmdSoundSourceDefinition = MmdSoundSourceDefinition.new()
    engine.label = "engine"
    MmdSoundBankInstancer._apply_original_defaults(engine, false)
    assert_eq(engine.placement, &"engine")

    var horn:MmdSoundSourceDefinition = MmdSoundSourceDefinition.new()
    horn.label = "horn1"
    MmdSoundBankInstancer._apply_original_defaults(horn, false)
    assert_eq(horn.placement, &"external")


func test_explicit_placement_wins_over_original_defaults() -> void:
    var definition:MmdSoundSourceDefinition = MmdSoundSourceDefinition.new()
    definition.label = "engine"
    definition.placement = &"custom"
    definition.placement_defined = true
    MmdSoundBankInstancer._apply_original_defaults(definition, false)

    assert_eq(definition.placement, &"custom")
