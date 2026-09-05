extends MaszynaGutTest

const FIXTURE_PATH := "res://tests/fixtures/test_sound.mmd"


func test_ignition_merges_into_empty_engine_sound_begin_but_explicit_soundend_wins_over_shutdown():
    var context := MmdImportContext.new()
    context.base_dir = FIXTURE_PATH.get_base_dir()
    var definitions:Array[MmdSoundSourceDefinition] = MmdSoundSourceParser.parse(FIXTURE_PATH, context)
    MmdSoundBankInstancer._merge_ignition_and_shutdown_into_engine(definitions, FIXTURE_PATH, context)

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
