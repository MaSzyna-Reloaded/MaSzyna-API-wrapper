extends RefCounted
class_name MmdSoundBankInstancer

## Orchestrator mirroring MmdCabinInstancer.build_into()'s role: given a vehicle's MMD file, parses
## its `sounds:` section (MmdSoundSourceParser), builds one SfxEvent per MmdSoundCatalog-matched
## label (MmdSoundEventBuilder) into a single SfxBank, and attaches a generated SfxPlayer3D (with
## one TrainSoundTrigger child per matched label) under `vehicle`. A vehicle with no `sounds:` data,
## or none of it in the catalog, gets no SfxPlayer3D at all - not an error, just nothing to play.


## `vehicle` is the RailVehicle3D DynamicRailVehicle3D builds internally - the generated
## SfxPlayer3D/TrainSoundTrigger children are added under it as INTERNAL_MODE_BACK, same as its
## ExteriorModel/FIZTrainController siblings. `fiz_controller_name` is the sibling
## FIZTrainController node's name (e.g. "FIZTrainController") - its own child TrainController
## isn't built yet at this point (FIZTrainController defers by one frame, same reason
## DynamicRailVehicle3D._rebuild() sets RailVehicle3D.controller_path as a plain string NodePath
## rather than resolving it with get_path_to()), so each trigger's controller_path is likewise a
## deterministic string path, resolved lazily by TrainSoundTrigger itself once the node exists.
static func build_into(
        vehicle:Node3D, abs_mmd_path:String, fiz_controller_name:String,
        random_choices:Dictionary, diagnostics:Array[Dictionary]) -> void:
    var context := MmdImportContext.new()
    context.base_dir = abs_mmd_path.get_base_dir()
    context.random_choices = random_choices

    var definitions:Array[MmdSoundSourceDefinition] = MmdSoundSourceParser.parse(abs_mmd_path, context)
    var internal_data:Array[MmdSoundSourceDefinition] = MmdSoundSourceParser.parse_internal_data(abs_mmd_path, context)
    _merge_ignition_and_shutdown_into_engine(definitions, internal_data)
    # buzzer:/buzzershp: (see MmdSoundCatalog) are their own catalog-matched events, unlike
    # ignition:/shutdown: which only ever get merged into "engine" and never appear standalone.
    for definition:MmdSoundSourceDefinition in internal_data:
        if definition.label == "buzzer" or definition.label == "buzzershp":
            definitions.append(definition)

    var events:Array[SfxEvent] = []
    var matched:Array[Dictionary] = []
    for definition:MmdSoundSourceDefinition in definitions:
        if not MmdSoundCatalog.has_label(definition.label):
            context.warn_unsupported_label(definition.label, abs_mmd_path, 0)
            continue
        var entry:Dictionary = MmdSoundCatalog.get_entry(definition.label)
        events.append(MmdSoundEventBuilder.build(definition, entry["event_name"], entry.get("sound_parameter", &"")))
        matched.append(entry)

    diagnostics.append_array(context.diagnostics)
    if events.is_empty():
        return

    var bank := SfxBank.new()
    bank.events = events

    var player := SfxPlayer3D.new()
    player.name = "SfxPlayer3D"
    player.bank = bank
    # Matches sm_42.tscn's own hand-tuned, proven-working values - MMD's per-event `range:` has no
    # equivalent here (gnd-sfx's max_distance is a whole-player setting, not per-event), so there
    # is nothing vehicle-specific to derive these three from.
    player.attenuation_model = AudioStreamPlayer3D.ATTENUATION_INVERSE_DISTANCE
    player.unit_size = 20.0
    player.max_distance = 100
    vehicle.add_child(player, false, Node.INTERNAL_MODE_BACK)

    var controller_path := NodePath("../../%s/TrainController" % fiz_controller_name)
    for entry:Dictionary in matched:
        var trigger := TrainSoundTrigger.new()
        trigger.name = String(entry["event_name"]).capitalize()
        trigger.state_property = entry["state_property"]
        trigger.trigger_mode = entry["trigger_mode"]
        trigger.sound_event = entry["event_name"]
        trigger.sound_parameter = entry.get("sound_parameter", &"")
        trigger.trigger_threshold_min = entry.get("trigger_threshold_min", 0.0)
        trigger.trigger_threshold_max = entry.get("trigger_threshold_max", 1.0)
        trigger.controller_path = controller_path
        player.add_child(trigger, false, Node.INTERNAL_MODE_BACK)


## ignition:/shutdown: are their own MMD labels living in internaldata: (see
## MmdSoundSourceParser.parse_internal_data()'s header comment), not part of engine:'s own block -
## but they're the SAME engine turning on/off, matching the hand-authored engine.tres reference's
## own shape exactly (one event with start/stop clips AND the rpm crossfade automation together,
## not three separate bank events/triggers for what is one physical engine). Splices their
## sound_main into engine's own sound_begin/sound_end IN PLACE (mutating the `engine` entry already
## in `definitions`), only if `engine` doesn't already declare its own soundbegin:/soundend: (rare
## but real: some vehicles put a stop sample directly in the engine: block itself) - an explicit
## in-block value always wins over the merge. ignition:/shutdown: never end up in MmdSoundCatalog
## themselves (parse() never sees them - they're outside sounds:/endsounds), so there's nothing to
## remove from `definitions` afterward.
static func _merge_ignition_and_shutdown_into_engine(
        definitions:Array[MmdSoundSourceDefinition], internal_data:Array[MmdSoundSourceDefinition]) -> void:
    var engine:MmdSoundSourceDefinition = null
    for definition:MmdSoundSourceDefinition in definitions:
        if definition.label == "engine":
            engine = definition
            break
    if not engine:
        return # nothing to merge ignition:/shutdown: into

    for definition:MmdSoundSourceDefinition in internal_data:
        if definition.label == "ignition" and engine.sound_begin.is_empty():
            engine.sound_begin = definition.sound_main
        elif definition.label == "shutdown" and engine.sound_end.is_empty():
            engine.sound_end = definition.sound_main
