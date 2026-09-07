extends RefCounted
class_name MmdSoundBankInstancer

const _INTERNAL_BRAKE_LABELS:Array[String] = [
    "brakesound", "airsound", "airsound2", "airsound3", "airsound4", "airsound5",
    "localbrakesound", "localbrakesound2",
]
const _PLAYER_VOICE_COUNT:int = 24


static func build_into(
        vehicle:Node3D, abs_mmd_path:String, _fiz_controller_name:String,
        random_choices:Dictionary, diagnostics:Array[Dictionary]) -> void:
    var context := MmdImportContext.new()
    context.base_dir = abs_mmd_path.get_base_dir()
    context.random_choices = random_choices

    var exterior_definitions:Array[MmdSoundSourceDefinition] = MmdSoundSourceParser.parse(abs_mmd_path, context)
    var internal_data:Array[MmdSoundSourceDefinition] = MmdSoundSourceParser.parse_internal_data(abs_mmd_path, context)
    var soundproofing:Array[PackedFloat32Array] = MmdSoundSourceParser.parse_vehicle_soundproofing(abs_mmd_path, context)
    _merge_ignition_and_shutdown_into_engine(exterior_definitions, internal_data)

    var cabin_definitions:Array[MmdSoundSourceDefinition] = []
    for definition:MmdSoundSourceDefinition in internal_data:
        if definition.label in ["ignition", "shutdown"]:
            continue
        if not definition.label in [
                "buzzer", "buzzershp", "brakesound", "slipperysound", "airsound", "airsound2",
                "airsound3", "airsound4", "airsound5", "localbrakesound", "localbrakesound2"]:
            continue
        _apply_original_defaults(definition, true)
        if definition.placement == &"internal":
            cabin_definitions.append(definition)
        else:
            exterior_definitions.append(definition)

    var routed_exterior:Array[MmdSoundSourceDefinition] = []
    for definition:MmdSoundSourceDefinition in exterior_definitions:
        _apply_original_defaults(definition, false)
        if definition.placement == &"internal":
            cabin_definitions.append(definition)
        else:
            routed_exterior.append(definition)

    _build_player(vehicle, "ExteriorSfxPlayer3D", routed_exterior, soundproofing, context, abs_mmd_path, false)
    _build_player(vehicle, "CabinSfxPlayer3D", cabin_definitions, soundproofing, context, abs_mmd_path, true)
    diagnostics.append_array(context.diagnostics)


static func _build_player(
        vehicle:Node3D, player_name:String, definitions:Array[MmdSoundSourceDefinition],
        soundproofing:Array[PackedFloat32Array], context:MmdImportContext,
        abs_mmd_path:String, cabin_only:bool) -> void:
    var events:Array[SfxEvent] = []
    var regular_definitions:Array[MmdSoundSourceDefinition] = []
    var brake_sources:Dictionary = {}
    for definition:MmdSoundSourceDefinition in definitions:
        if not MmdSoundCatalog.has_label(definition.label):
            context.warn_unsupported_label(definition.label, abs_mmd_path, 0)
            continue
        var entry:Dictionary = MmdSoundCatalog.get_entry(definition.label)
        if entry.get("controller", &"") == &"brake":
            _apply_brake_source_defaults(definition)
            brake_sources[definition.label] = definition
            continue
        var event:SfxEvent = MmdSoundEventBuilder.build(
                definition, entry["event_name"], entry.get("sound_parameter", &""))
        event.spatial_config = MmdSoundEventBuilder._build_spatial_config(definition)
        events.append(event)
        regular_definitions.append(definition)

    var bank := SfxBank.new()
    bank.events = events
    var player := SfxPlayer3D.new()
    player.name = player_name
    player.bank = bank
    player.max_tracks = _PLAYER_VOICE_COUNT
    player.attenuation_model = AudioStreamPlayer3D.ATTENUATION_INVERSE_DISTANCE
    player.unit_size = 20.0
    player.max_distance = 100.0
    vehicle.add_child(player, false, Node.INTERNAL_MODE_BACK)

    var triggers:Array[Dictionary] = []
    for definition:MmdSoundSourceDefinition in regular_definitions:
        var entry:Dictionary = MmdSoundCatalog.get_entry(definition.label)
        triggers.append({
            "state_property": entry["state_property"],
            "trigger_mode": entry["trigger_mode"],
            "sound_event": entry["event_name"],
            "sound_parameter": entry.get("sound_parameter", &""),
            "trigger_threshold_min": entry.get("trigger_threshold_min", 0.0),
            "trigger_threshold_max": entry.get("trigger_threshold_max", 1.0),
        })

    TrainSoundSystem.register_bank(player, {
        "vehicle": vehicle,
        "cabin_only": cabin_only,
        "triggers": triggers,
        "brake_sources": brake_sources,
        "soundproofing": soundproofing,
    })


static func _apply_original_defaults(definition:MmdSoundSourceDefinition, from_internal_data:bool) -> void:
    if definition.placement_defined:
        return
    if from_internal_data:
        definition.placement = &"external" if definition.label == "slipperysound" else &"internal"
    elif definition.label == "engine":
        definition.placement = &"engine"
    elif MmdSoundCatalog.has_label(definition.label) \
            and MmdSoundCatalog.get_entry(definition.label).get("controller", &"") == &"brake":
        definition.placement = &"external"


static func _apply_brake_source_defaults(definition:MmdSoundSourceDefinition) -> void:
    if definition.label == "brake" and definition.amplitude_factor > 10.0:
        definition.amplitude_factor = 1.0
        definition.amplitude_offset = 0.0
    if not definition.range_defined:
        if definition.label == "brakesound" and definition.placement == &"internal":
            definition.range = -1.0
        elif definition.label in _INTERNAL_BRAKE_LABELS:
            definition.range = 7.5
        elif definition.label in ["brake", "brakesound", "slipperysound"]:
            definition.range = 100.0


static func _merge_ignition_and_shutdown_into_engine(
        definitions:Array[MmdSoundSourceDefinition], internal_data:Array[MmdSoundSourceDefinition]) -> void:
    var engine:MmdSoundSourceDefinition = null
    for definition:MmdSoundSourceDefinition in definitions:
        if definition.label == "engine":
            engine = definition
            break
    if not engine:
        return
    for definition:MmdSoundSourceDefinition in internal_data:
        if definition.label == "ignition" and engine.sound_begin.is_empty():
            engine.sound_begin = definition.sound_main
        elif definition.label == "shutdown" and engine.sound_end.is_empty():
            engine.sound_end = definition.sound_main
