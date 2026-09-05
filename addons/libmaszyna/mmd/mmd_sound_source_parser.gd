extends RefCounted
class_name MmdSoundSourceParser

## Parses the MMD's vehicle-wide `sounds:`...`endsounds` section into one MmdSoundSourceDefinition
## per top-level label, mirroring the original engine's audio/sound.cpp `sound_source::
## deserialize_mapping()` block-form dispatch. Reuses MmdCabinInstancer's whole-file tokenizer
## (include/(pN) expansion, BOM handling, label:value splitting) and MmdImportContext - the
## `sounds:` section shares that same include/token grammar with `internaldata:`, only the label
## shapes below it differ.
##
## Bare (non-block) label lines have no self-describing arity ("oilpump: file.wav" is 1 token,
## "tractionmotor: file.wav 10.0 0.01 0.5 0.16 0.1" is 6 - confirmed against real dynamic/pkp
## data), and only Tier 1/2 labels (see MmdSoundCatalog) are actually interpreted here - so rather
## than a per-label arity table for all ~46 possible labels, a bare label's value tokens are
## consumed generically up to the next "label:"-shaped token. Every real bare value (filename,
## float, [NNNN] prefix) is confirmed to never itself contain ':', so this can't misfire on a
## following genuine label.

## label -> bare value (if any) is this label's sound_main. Only Tier 1/2 labels confirmed to
## appear this way in real data - "battery" is confirmed always block-form.
const _BARE_SINGLE_LABELS:Dictionary = {
    "oilpump": true,
    "fuelpump": true,
}

## label -> bare form is 3 values (begin, main, end) + an optional trailing range - confirmed real
## (dynamic/pkp/401da_v1/401da_dumb.mmd: "horn1: horn-sn61-start.wav horn-sn61.wav
## horn-sn61-stop.wav 220"), matching audio/sound.cpp's own sound_type::multipart +
## sound_parameters::range legacy-form dispatch (DynObj.cpp's sHorn1/sHorn2/sHorn3.deserialize()
## call sites) exactly - begin/main/end in that fixed order (audio/sound.h's sound_id enum).
const _BARE_MULTIPART_LABELS:Dictionary = {
    "horn1": true,
    "horn2": true,
    "horn3": true,
}


static func parse(abs_mmd_path:String, context:MmdImportContext) -> Array[MmdSoundSourceDefinition]:
    var tokens:Array[String] = MmdCabinInstancer._tokenize_file(abs_mmd_path, context)

    var start_index:int = MmdCabinInstancer._find_label_index(tokens, "sounds:")
    if start_index == -1:
        return []
    start_index += 1
    var end_index:int = MmdCabinInstancer._find_label_index(tokens, "endsounds", start_index)
    if end_index == -1:
        end_index = tokens.size()

    var definitions:Array[MmdSoundSourceDefinition] = []
    var i := start_index
    while i < end_index:
        var label:String = tokens[i].to_lower()
        i += 1
        if not label.ends_with(":"):
            continue # stray value token from a desync earlier - stay aligned, same as MmdCabinInstancer
        var definition := MmdSoundSourceDefinition.new()
        definition.label = label.trim_suffix(":")
        definition.source_file = abs_mmd_path
        i += _parse_one(tokens, i, definition, context, abs_mmd_path)
        definitions.append(definition)
    return definitions


static func _parse_one(
        tokens:Array[String], i:int, definition:MmdSoundSourceDefinition,
        context:MmdImportContext, source_file:String) -> int:
    var start:int = i
    if i < tokens.size() and tokens[i] == "{":
        i += 1
        while i < tokens.size() and tokens[i] != "}":
            i += _parse_block_field(tokens, i, definition, context, source_file)
        if i < tokens.size():
            i += 1 # consume "}"
        return i - start

    # bare form
    if _BARE_SINGLE_LABELS.has(definition.label):
        var result:Dictionary = _read_random_set(tokens, i, context, source_file, definition.label)
        definition.sound_main = result["value"]
        i += int(result["consumed"])
    elif _BARE_MULTIPART_LABELS.has(definition.label):
        for part:String in ["begin", "main", "end"]:
            var result:Dictionary = _read_random_set(tokens, i, context, source_file, "%s_%s" % [definition.label, part])
            i += int(result["consumed"])
            match part:
                "begin": definition.sound_begin = result["value"]
                "main": definition.sound_main = result["value"]
                "end": definition.sound_end = result["value"]

    # any remaining bare values (range, amplitude/frequency pairs, ...) aren't consumed in v1 -
    # skip up to the next "label:"-shaped token so the stream stays aligned.
    while i < tokens.size() and not tokens[i].to_lower().ends_with(":"):
        i += 1
    return i - start


## `tokens[i]` is one block-internal key (already known to not be "}"). Returns tokens consumed,
## including the key itself.
static func _parse_block_field(
        tokens:Array[String], i:int, definition:MmdSoundSourceDefinition,
        context:MmdImportContext, source_file:String) -> int:
    var key:String = tokens[i].to_lower()
    var start:int = i
    i += 1

    match key:
        "soundmain:":
            var result:Dictionary = _read_random_set(tokens, i, context, source_file, key)
            definition.sound_main = result["value"]
            i += int(result["consumed"])
        "soundbegin:":
            var result:Dictionary = _read_random_set(tokens, i, context, source_file, key)
            definition.sound_begin = result["value"]
            i += int(result["consumed"])
        "soundend:":
            var result:Dictionary = _read_random_set(tokens, i, context, source_file, key)
            definition.sound_end = result["value"]
            i += int(result["consumed"])
        "soundset:":
            var result:Dictionary = _read_random_set(tokens, i, context, source_file, key)
            var parts:PackedStringArray = String(result["value"]).split("|")
            if parts.size() > 0:
                definition.sound_begin = _normalize_sound_filename(parts[0])
            if parts.size() > 1:
                definition.sound_main = _normalize_sound_filename(parts[1])
            if parts.size() > 2:
                definition.sound_end = _normalize_sound_filename(parts[2])
            i += int(result["consumed"])
        "crossfade:":
            if i < tokens.size():
                definition.crossfade_percent = clampi(int(tokens[i]), 0, 100)
                i += 1
        "soundproofing:", "offset:":
            i += _skip_value(tokens, i) # no gnd-sfx equivalent (soundproofing) / not in v1 scope (offset)
        _:
            if key.begins_with("sound") and _threshold_from_key(key) != null:
                var result:Dictionary = _read_random_set(tokens, i, context, source_file, key)
                definition.chunks.append({"threshold": _threshold_from_key(key), "filename": result["value"], "pitch": 0.0})
                i += int(result["consumed"])
            elif key.begins_with("pitch") and _threshold_from_key(key) != null:
                var threshold:int = _threshold_from_key(key)
                if i < tokens.size():
                    var pitch:float = float(tokens[i])
                    i += 1
                    for chunk:Dictionary in definition.chunks:
                        if int(chunk["threshold"]) == threshold:
                            chunk["pitch"] = pitch
                            break
            else:
                # amplitudefactor:/amplitudeoffset:/frequencyfactor:/frequencyoffset:/range:/
                # placement:/pitchvariation:/startoffset: and anything else unrecognized - not
                # consumed by MmdSoundEventBuilder in v1, but still skipped generically (single
                # token, or a bracketed/nested value) so the token stream stays aligned.
                i += _skip_value(tokens, i)

    return i - start


## "sound1:" -> 1, "sound-3:" -> -3, "sound400:" -> 400, "pitch600:" -> 600. Returns null for
## "soundmain:"/"soundbegin:"/"soundend:"/"soundset:"/"pitchvariation:" (no valid-int remainder).
static func _threshold_from_key(key:String) -> Variant:
    var middle:String = key.substr(5, key.length() - 6) # strip "sound"/"pitch" prefix (5 chars) and trailing ":"
    if middle.is_empty() or not middle.is_valid_int():
        return null
    return int(middle)


## Skips one value at tokens[i]: a bracketed `[ ... ]` list (soundproofing:/offset:), or a single
## token otherwise. Returns tokens consumed (NOT including the key, which the caller already did).
static func _skip_value(tokens:Array[String], i:int) -> int:
    if i >= tokens.size():
        return 0
    if tokens[i] == "[":
        var j:int = i + 1
        while j < tokens.size() and tokens[j] != "]":
            j += 1
        if j < tokens.size():
            j += 1 # consume "]"
        return j - i
    return 1


## Mirrors the original engine's deserialize_random_set(): either a single bare token, or a
## bracketed `[ a, b, c ]` random-choice list resolved to one entry (frozen via
## context.random_choices, keyed by this field's own content - same mechanism already used for
## MMD `include [...] end` and cabin soundinc:/sounddec: lists). Candidates are comma-separated in
## real data (MaszynaParser's default stop chars don't include ',', so a trailing comma ends up
## glued onto the preceding candidate token and must be stripped).
static func _read_random_set(
        tokens:Array[String], i:int, context:MmdImportContext, source_file:String, field_key:String) -> Dictionary:
    if i >= tokens.size():
        return {"value": "", "consumed": 0}

    if tokens[i] != "[":
        return {"value": _normalize_sound_filename(tokens[i]), "consumed": 1}

    var candidates:Array[String] = []
    var j:int = i + 1
    while j < tokens.size() and tokens[j] != "]":
        var candidate:String = tokens[j].trim_suffix(",")
        if not candidate.is_empty():
            candidates.append(candidate)
        j += 1
    if j < tokens.size():
        j += 1 # consume "]"
    if candidates.is_empty():
        return {"value": "", "consumed": j - i}

    var choice_key:String = "%s#%s#%s" % [source_file, field_key, "|".join(candidates)]
    if not context.random_choices.has(choice_key):
        context.random_choices[choice_key] = candidates[randi() % candidates.size()]
    return {"value": _normalize_sound_filename(context.random_choices[choice_key]), "consumed": j - i}


## Strips a trailing ".wav"/".ogg" - everything else ("[NNNN]" numeric prefixes included) is kept
## verbatim, same rule as MmdCabinInstancer._normalize_sound_filename(). Does NOT split on "|" -
## soundset:'s caller does that itself after this returns the whole chosen candidate.
static func _normalize_sound_filename(token:String) -> String:
    if token.is_empty():
        return ""
    var lower:String = token.to_lower()
    if lower.ends_with(".wav") or lower.ends_with(".ogg"):
        return token.substr(0, token.length() - 4)
    return token
