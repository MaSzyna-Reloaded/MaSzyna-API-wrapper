extends RefCounted
class_name MmdCabinInstancer

## MMD cabin file parser/builder, mirroring FizTrainControllerInstancer's role for FIZ files.
## Scope is Etap A+B of dynamic_train_cabin_feasibility.md: cab1/cab2 model+camera resolution,
## plus "rot"/"mov" animation for the labels in MmdSemanticCatalog only ("wip"/"dgt"/"rotvar"/
## "movvar" math, audio, pyscreen, and any label outside the catalog are parsed just enough to
## keep the token stream aligned, then discarded with a diagnostic).
##
## Unlike FizTrainControllerInstancer (one MaszynaParser per physical line, because FIZ's
## grammar is line-oriented key=value), MMD's grammar is a plain token stream (label: value
## value value...), so this reads the whole file - with includes spliced in - into one flat
## token array first, then walks it by index. The trade-off: diagnostics below carry line=0
## (MaszynaParser has no cursor/position accessor to reconstruct it from mid-stream) - source
## file is still tracked. Re-add line numbers if MaszynaParser ever grows a position getter.

const _RANDOM_INCLUDE_OPEN := "["
const _RANDOM_INCLUDE_CLOSE := "]"
const _INCLUDE_END_KEYWORD := "end"
const _VARIABLE_ANIMATION_TYPES := ["rotvar", "movvar"]


## Parses an MMD file (with includes expanded) into a neutral MmdCabinDefinition for one cab.
## `random_choices` is owned by the caller and reused verbatim across repeated parse() calls
## (e.g. a later cab1<->cab2 rebuild) so a random include set isn't re-rolled each time.
static func parse(abs_mmd_path:String, cab_number:int, random_choices:Dictionary) -> MmdCabinDefinition:
    var context := MmdImportContext.new()
    context.base_dir = abs_mmd_path.get_base_dir()
    context.cab_number = cab_number
    context.random_choices = random_choices

    var tokens:Array[String] = _tokenize_file(abs_mmd_path, context)

    # An MMD file's models:/sounds:/locations: preamble (everything before the cab/instrument
    # section) has a completely different, non-uniform grammar this parser doesn't understand -
    # scanning it with the label dispatch below would immediately misparse it. Bound the real
    # parse to [first cab1definition:/cab2definition:, first cab0definition: at or after it) so
    # the preamble is skipped outright and a later desync from some other unrecognized,
    # non-uniform label (e.g. a bare "clock: analog") can't run past the real end of the section.
    var start_index:int = _find_label_index(tokens, "cab1definition:")
    if start_index == -1:
        start_index = _find_label_index(tokens, "cab2definition:")
    if start_index == -1:
        context.add_diagnostic("error", "MMD_INVALID_CAB_DEFINITION", "No cab1definition:/cab2definition: found", abs_mmd_path)
        start_index = tokens.size()
    var end_index:int = _find_label_index(tokens, "cab0definition:", start_index)
    if end_index == -1:
        end_index = tokens.size()

    var cab_data:Dictionary = {
        1: _empty_cab_data(),
        2: _empty_cab_data(),
    }
    var instruments:Array[MmdInstrumentDescriptor] = []

    var i := start_index
    while i < end_index:
        var token:String = tokens[i]
        var label:String = token.to_lower()
        i += 1

        match label:
            "cab1definition:", "cab2definition:":
                var n:int = 1 if label == "cab1definition:" else 2
                var values:Array = _read_floats(tokens, i, 6)
                i += 6
                cab_data[n]["bounds_min"] = Vector3(values[0], values[1], values[2])
                cab_data[n]["bounds_max"] = Vector3(values[3], values[4], values[5])
            "cablight:":
                i += 9 # ambient cab light color block - not wired to anything in Etap A+B
            "driver1angle:", "driver2angle:":
                var n:int = 1 if label == "driver1angle:" else 2
                var values:Array = _read_floats(tokens, i, 2)
                i += 2
                cab_data[n]["driver_angle"] = Vector2(values[0], values[1])
            "driver1pos:", "driver2pos:":
                var n:int = 1 if label == "driver1pos:" else 2
                var values:Array = _read_floats(tokens, i, 3)
                i += 3
                cab_data[n]["driver_pos"] = Vector3(values[0], values[1], values[2])
            "driver1sitpos:", "driver2sitpos:":
                var n:int = 1 if label == "driver1sitpos:" else 2
                var values:Array = _read_floats(tokens, i, 3)
                i += 3
                cab_data[n]["driver_sitpos"] = Vector3(values[0], values[1], values[2])
            "cab1model:", "cab2model:":
                var n:int = 1 if label == "cab1model:" else 2
                var model_token:String = tokens[i] if i < tokens.size() else "none"
                i += 1
                cab_data[n]["model_relpath"] = _resolve_model_relpath(model_token)
            _:
                if not label.ends_with(":"):
                    continue # stray value token, not a label - most likely a leftover from a
                    # desync caused by some other unrecognized, non-uniform-shaped label earlier
                    # in this same section (see the comment above start_index/end_index)
                var descriptor := MmdInstrumentDescriptor.new()
                descriptor.label = label.trim_suffix(":")
                descriptor.source_file = abs_mmd_path
                # "i-*:" indicator lights (Train.cpp's TButton::Load(), Button.cpp:40-56) use a
                # completely different, single-token shape ("i-security_aware: czuwak", no
                # animation/scale/offset/friction at all) than every other instrument label -
                # confirmed real and CONFIRMED to previously desync whatever follows it when force-
                # fed through _parse_instrument()'s 5-token read (both fall inside the same
                # cab1definition:/cab0definition: bounds this parser scans).
                var consumed:int = (
                        _parse_indicator(tokens, i, descriptor, context, abs_mmd_path) if descriptor.label.begins_with("i-")
                        else _parse_instrument(tokens, i, descriptor, context, abs_mmd_path))
                i += consumed
                if not descriptor.submodel_name.is_empty():
                    instruments.append(descriptor)

    var definition := MmdCabinDefinition.new()
    definition.cab_number = cab_number
    definition.bounds_min = cab_data[cab_number]["bounds_min"]
    definition.bounds_max = cab_data[cab_number]["bounds_max"]
    definition.driver_pos = cab_data[cab_number]["driver_pos"]
    definition.driver_sitpos = cab_data[cab_number]["driver_sitpos"]
    definition.driver_angle = cab_data[cab_number]["driver_angle"]
    definition.model_relpath = cab_data[cab_number]["model_relpath"]
    definition.instruments = instruments
    definition.diagnostics = context.diagnostics
    return definition


## Real MaSzyna data on Linux frequently has a case mismatch between what an MMD declares (e.g.
## "st44_A.t3d") and the actual file on disk (e.g. "st44_a.e3d") - harmless on Windows' case-
## insensitive filesystem, fatal here (feasibility doc section 3.1: ~59/730 real models affected).
## Falls back to `relpath` unchanged if no case-insensitive match exists either - the caller's
## own MMD_MODEL_NOT_FOUND still fires in that case.
static func resolve_model_case(data_path:String, relpath:String) -> String:
    if relpath.is_empty():
        return relpath
    var base_dir:String = UserSettings.get_maszyna_game_dir().path_join(data_path)
    if FileAccess.file_exists(base_dir.path_join(relpath + ".e3d")):
        return relpath

    var dir_part:String = relpath.get_base_dir()
    var file_part:String = relpath.get_file()
    var dir_access:DirAccess = DirAccess.open(base_dir.path_join(dir_part))
    if not dir_access:
        return relpath

    var wanted:String = (file_part + ".e3d").to_lower()
    dir_access.list_dir_begin()
    var entry:String = dir_access.get_next()
    while not entry.is_empty():
        if not dir_access.current_is_dir() and entry.to_lower() == wanted:
            dir_access.list_dir_end()
            return dir_part.path_join(entry.substr(0, entry.length() - 4)) # strip ".e3d"
        entry = dir_access.get_next()
    dir_access.list_dir_end()
    return relpath


## A model can need more than one dynamic-material skin slot (index 0 -> "<skin>.mat", index 1
## -> "<skin>,1.mat", etc.) - confirmed against real data: dynamic/pkp/st44_v2 ships
## st44-700.mat + st44-700,1.mat + st44-700,2.mat + st44-700,3.mat for one cab model. Passing a
## single-entry skins array leaves every slot beyond 0 with no material at all (silently missing
## texture, not an error) - scan data_path for how many consecutive ",N.mat" files exist for
## `skin` and size the array to match, rather than assuming a fixed slot count.
static func resolve_skins(data_path:String, skin:String) -> Array:
    if skin.is_empty():
        return [skin]
    var base_dir:String = UserSettings.get_maszyna_game_dir().path_join(data_path)
    var skins:Array = [skin]
    var n:int = 1
    while FileAccess.file_exists(base_dir.path_join("%s,%d.mat" % [skin, n])):
        skins.append("%s,%d" % [skin, n])
        n += 1
    return skins


## Reads just the exterior body model filename from the MMD's own top-level `models:` section
## (e.g. "models: 6da.t3d" as the file's very first line) - this is a DIFFERENT filename than
## the vehicle's shared .fiz/.mmd base name in general (confirmed against real data:
## dynamic/pkp/st44_v2's body model is not named "st44-700"), so DynamicRailVehicle3D must read
## it from here rather than assuming it equals file_name. Returns "" if the file can't be read
## or has no `models:` section - the caller decides the fallback.
static func parse_body_model(abs_mmd_path:String) -> String:
    var context := MmdImportContext.new()
    var tokens:Array[String] = _tokenize_file(abs_mmd_path, context)
    var index:int = _find_label_index(tokens, "models:")
    if index == -1 or index + 1 >= tokens.size():
        return ""
    return _resolve_model_relpath(tokens[index + 1])


## Reads the low-poly interior model filename from the MMD's own top-level `models:` section
## (e.g. "lowpolyinterior: 6da_interior.t3d") - the lower-detail interior visible from outside
## the cabin (through windows) before the player enters, matching
## RailVehicle3D.low_poly_cabin_path. Returns "" if the MMD has no such entry.
static func parse_lowpoly_interior_model(abs_mmd_path:String) -> String:
    var context := MmdImportContext.new()
    var tokens:Array[String] = _tokenize_file(abs_mmd_path, context)
    var index:int = _find_label_index(tokens, "lowpolyinterior:")
    if index == -1 or index + 1 >= tokens.size():
        return ""
    return _resolve_model_relpath(tokens[index + 1])


## Builds real, interactive cabin widgets (CabinButton/CabinSwitch/CabinKnob/CabinGauge) plus
## the cab's own E3D model as children of `generated_root`, which must already be inside the
## scene tree. Appends any build-time diagnostics to `diagnostics` (caller-owned, merged with
## `definition.diagnostics` by DynamicTrainCabin.get_diagnostics()).
static func build_into(
        generated_root:Node3D, definition:MmdCabinDefinition, controller:TrainController,
        data_path:String, skin:String, diagnostics:Array[Dictionary]) -> void:
    if definition.model_relpath.is_empty():
        diagnostics.append(_diag("error", "MMD_MODEL_NOT_FOUND", "Cab %d has no model (model: none)" % definition.cab_number, definition.cab_number))
        return

    var model_relpath:String = resolve_model_case(data_path, definition.model_relpath)
    if model_relpath != definition.model_relpath:
        diagnostics.append(_diag("info", "MMD_MODEL_CASE_NORMALIZED", "Cab model '%s' resolved case-insensitively to '%s'" % [definition.model_relpath, model_relpath], definition.cab_number))

    var model := E3DModelInstance.new()
    model.name = "CabModel"
    model.data_path = data_path
    model.model_filename = model_relpath
    model.skins = resolve_skins(data_path, skin)
    generated_root.add_child(model)

    if not model.is_e3d_loaded():
        diagnostics.append(_diag("error", "MMD_MODEL_NOT_FOUND", "Could not load cab model '%s'" % model_relpath, definition.cab_number))
        return

    var submodel_index:Dictionary = {}
    _index_submodels(model, submodel_index)

    for descriptor:MmdInstrumentDescriptor in definition.instruments:
        if not MmdSemanticCatalog.has_label(descriptor.label):
            diagnostics.append(_diag("info", "MMD_BINDING_UNSUPPORTED", "MMD label '%s' is not in the supported catalog" % descriptor.label, definition.cab_number, descriptor.label, descriptor.submodel_name))
            continue
        var entry:Dictionary = MmdSemanticCatalog.get_entry(descriptor.label)
        if entry.get("position_at_submodel", false):
            # Some real cabins have multiple physical lamp housings sharing the SAME MMD-declared
            # submodel base name (confirmed real: sm_42_cabin.tscn's own hand-authored reference
            # has 3 "CzuwakOmni" lights for its one "i-security_aware:" label) - one widget per
            # matched submodel instance, not just the first, unlike every other instrument label
            # (which only ever has one real target mesh).
            _build_indicator_lights(descriptor, entry, controller, submodel_index, generated_root, definition.cab_number, diagnostics)
            continue
        var widget:Node = _build_widget(descriptor, controller, definition.cab_number, diagnostics)
        generated_root.add_child(widget)
        # mesh_path must be resolved AFTER the widget has a place in the tree - the widget shares
        # no common ancestor with `model`'s submodels until it's actually parented under the same
        # generated_root.
        _wire_mesh_path(widget, descriptor, submodel_index, entry["mesh_path_field"], definition.cab_number, diagnostics)
        widget.set("controller_path", widget.get_path_to(controller))


static func _empty_cab_data() -> Dictionary:
    return {
        "bounds_min": Vector3.ZERO,
        "bounds_max": Vector3.ZERO,
        "driver_pos": Vector3.ZERO,
        "driver_sitpos": Vector3.ZERO,
        "driver_angle": Vector2.ZERO,
        "model_relpath": "",
    }


static func _diag(severity:String, code:String, message:String, cab_number:int, mmd_label:String = "", submodel_name:String = "") -> Dictionary:
    return {
        "severity": severity,
        "code": code,
        "source_file": "",
        "line": 0,
        "cabin_number": cab_number,
        "mmd_label": mmd_label,
        "submodel_name": submodel_name,
        "message": message,
    }


static func _find_label_index(tokens:Array[String], needle:String, from:int = 0) -> int:
    for i in range(from, tokens.size()):
        if tokens[i].to_lower() == needle:
            return i
    return -1


static func _read_floats(tokens:Array[String], start_i:int, count:int) -> Array:
    var result:Array = []
    for k in range(count):
        var idx:int = start_i + k
        result.append(float(tokens[idx]) if idx < tokens.size() else 0.0)
    return result


## Parses one instrument line's raw shape - `submodel animation scale offset friction`, or the
## `{ submodel animation scale offset friction  type: ...  ... }` block form - starting at
## `tokens[i]`. Returns the number of tokens consumed so the caller's index stays in sync even
## for a label this parser doesn't otherwise understand.
static func _parse_instrument(
        tokens:Array[String], i:int, descriptor:MmdInstrumentDescriptor,
        context:MmdImportContext, source_file:String) -> int:
    var start:int = i
    var in_block:bool = false
    if i < tokens.size() and tokens[i] == "{":
        in_block = true
        i += 1

    if i + 5 > tokens.size():
        context.add_diagnostic(
                "error", "MMD_INVALID_CAB_DEFINITION",
                "Truncated instrument definition for '%s'" % descriptor.label, source_file, 0, descriptor.label)
        return tokens.size() - start

    descriptor.submodel_name = tokens[i]
    descriptor.animation_type = tokens[i + 1].to_lower()
    descriptor.scale = float(tokens[i + 2])
    descriptor.offset = float(tokens[i + 3])
    descriptor.friction = float(tokens[i + 4])
    i += 5

    if descriptor.animation_type in _VARIABLE_ANIMATION_TYPES:
        i += 2 # endvalue, endscale - out of Etap A+B scope, discarded (position preserved)

    if in_block:
        while i < tokens.size() and tokens[i] != "}":
            var token_lower:String = tokens[i].to_lower()
            if token_lower == "type:" and i + 1 < tokens.size():
                descriptor.button_type = tokens[i + 1].to_lower()
                i += 2
            elif token_lower == "soundinc:":
                var result:Dictionary = _read_sound_field_value(tokens, i + 1, context, source_file, "soundinc")
                descriptor.sound_increase = result["value"]
                i += 1 + int(result["consumed"])
            elif token_lower == "sounddec:":
                var result:Dictionary = _read_sound_field_value(tokens, i + 1, context, source_file, "sounddec")
                descriptor.sound_decrease = result["value"]
                i += 1 + int(result["consumed"])
            else:
                var position:Variant = _parse_sound_position_label(token_lower)
                if position != null:
                    var result:Dictionary = _read_sound_field_value(tokens, i + 1, context, source_file, token_lower)
                    descriptor.sound_positions[position] = result["value"]
                    i += 1 + int(result["consumed"])
                else:
                    i += 1
        if i < tokens.size():
            i += 1 # consume "}"

    return i - start


## Parses an "i-*:" indicator light's shape (Train.cpp's TButton::Load(), Button.cpp:40-56): a
## bare submodel base name, OR a `{ submodel soundinc: ... sounddec: ... }` block whose FIRST
## token (not the label itself) is the submodel name - the original engine peeks for "{" BEFORE
## reading anything (Button.cpp:44-48: `if (Parser.peek() != "{") { Parser >> submodelname; } else
## { submodelname = Parser.getToken(...); ... }`), so the submodel name always comes from INSIDE
## the block in block form, never before it. Confirmed real and required: dynamic/pkp/su45_v2/
## 301d.mmd uses exactly this block shape ("i-security_aware: { i-czuwak soundinc: ... sounddec:
## ... }"), which a "submodel-name-then-optional-block" read (every other instrument label's
## order) misparses as submodel_name="{" and never enters the block at all - not the "submodel
## animation scale offset friction" shape every other instrument label uses (the original engine
## shows/hides a matching "<name>_on"/"<name>_off" submodel pair rather than animating one).
## MmdSemanticCatalog widgets for these labels ignore animation_type/scale/offset/friction
## entirely (left at their MmdInstrumentDescriptor defaults).
static func _parse_indicator(
        tokens:Array[String], i:int, descriptor:MmdInstrumentDescriptor,
        context:MmdImportContext, source_file:String) -> int:
    var start:int = i
    if i >= tokens.size():
        context.add_diagnostic(
                "error", "MMD_INVALID_CAB_DEFINITION",
                "Truncated indicator definition for '%s'" % descriptor.label, source_file, 0, descriptor.label)
        return 0

    if tokens[i] != "{":
        descriptor.submodel_name = tokens[i]
        return i + 1 - start

    i += 1 # consume "{"
    if i < tokens.size():
        descriptor.submodel_name = tokens[i]
        i += 1

    while i < tokens.size() and tokens[i] != "}":
        var token_lower:String = tokens[i].to_lower()
        if token_lower == "soundinc:":
            var result:Dictionary = _read_sound_field_value(tokens, i + 1, context, source_file, "soundinc")
            descriptor.sound_increase = result["value"]
            i += 1 + int(result["consumed"])
        elif token_lower == "sounddec:":
            var result:Dictionary = _read_sound_field_value(tokens, i + 1, context, source_file, "sounddec")
            descriptor.sound_decrease = result["value"]
            i += 1 + int(result["consumed"])
        else:
            i += 1
    if i < tokens.size():
        i += 1 # consume "}"

    return i - start


## Returns the position for a "soundN:"/"sound-N:" field label (e.g. "sound-3:" -> -3, "sound0:"
## -> 0), or null if `label` isn't in that shape (rules out "soundinc:"/"sounddec:"/"soundmain:"/
## "type:", none of which have a valid-int remainder after stripping "sound" and ":").
static func _parse_sound_position_label(label:String) -> Variant:
    if not label.begins_with("sound") or not label.ends_with(":"):
        return null
    var middle:String = label.substr(5, label.length() - 6)
    if middle.is_empty() or not middle.is_valid_int():
        return null
    return int(middle)


## Reads one sound field's value starting at tokens[i] (right after its "soundX:" label) - a bare
## filename, a bracketed random-choice list (resolved to one entry, frozen via
## context.random_choices exactly like MMD's random `include` lists), or a nested `{ ... }`
## sub-block (only its soundmain: is kept; other sub-fields are reported and discarded). Returns
## {"value": normalized filename ("" if absent), "consumed": token count NOT including tokens[i]
## itself - i.e. the caller's index should advance by 1 (the label) + this "consumed"}.
static func _read_sound_field_value(
        tokens:Array[String], i:int, context:MmdImportContext, source_file:String, field_key:String) -> Dictionary:
    if i >= tokens.size():
        return {"value": "", "consumed": 0}

    if tokens[i] == "[":
        var candidates:Array[String] = []
        var j:int = i + 1
        while j < tokens.size() and tokens[j] != "]":
            candidates.append(tokens[j])
            j += 1
        if j < tokens.size():
            j += 1 # consume "]"
        if candidates.is_empty():
            return {"value": "", "consumed": j - i}
        var choice_key:String = "%s#%s#%s" % [source_file, field_key, "|".join(candidates)]
        if not context.random_choices.has(choice_key):
            context.random_choices[choice_key] = candidates[randi() % candidates.size()]
        return {"value": _normalize_sound_filename(context.random_choices[choice_key]), "consumed": j - i}

    if tokens[i] == "{":
        var soundmain:String = ""
        var j:int = i + 1
        while j < tokens.size() and tokens[j] != "}":
            if tokens[j].to_lower() == "soundmain:" and j + 1 < tokens.size():
                soundmain = tokens[j + 1]
                j += 2
            else:
                j += 1
        if j < tokens.size():
            j += 1 # consume "}"
        context.add_diagnostic(
                "info", "MMD_ANIMATION_UNSUPPORTED",
                "Sound field '%s' uses a nested sub-block - only soundmain: is used, other parameters (amplitudefactor/range/etc.) are ignored" % field_key,
                source_file, 0, field_key)
        return {"value": _normalize_sound_filename(soundmain), "consumed": j - i}

    return {"value": _normalize_sound_filename(tokens[i]), "consumed": 1}


## Strips a trailing ".wav"/".ogg" - everything else (including a "[NNNN]" numeric prefix, which
## is confirmed to be part of the literal filename on disk) is kept verbatim.
static func _normalize_sound_filename(token:String) -> String:
    if token.is_empty():
        return ""
    var lower:String = token.to_lower()
    if lower.ends_with(".wav") or lower.ends_with(".ogg"):
        return token.substr(0, token.length() - 4)
    return token


## Strips ".t3d"/".e3d" and normalizes backslashes. Case-insensitive filesystem resolution
## (feasibility doc section 3.1's 59 non-matching-case models) is not attempted here - if the
## exact case doesn't resolve, E3DModelInstance.reload() simply fails to load and build_into()
## reports MMD_MODEL_NOT_FOUND, same as any other missing model.
##
## Real data (dynamic/pkp/st44_v2/st44.mmd.inc) has model tokens glued directly to a trailing
## "#" with no space (e.g. "main/(p1).t3d#") - strip it before touching the extension, or the
## ".t3d"/".e3d" suffix check below never matches and the resolved path is left corrupted.
static func _resolve_model_relpath(model_token:String) -> String:
    if model_token.is_empty() or model_token.to_lower() == "none":
        return ""
    var normalized:String = model_token.replace("\\", "/")
    if normalized.ends_with("#"):
        normalized = normalized.substr(0, normalized.length() - 1)
    var lower:String = normalized.to_lower()
    if lower.ends_with(".t3d") or lower.ends_with(".e3d"):
        normalized = normalized.substr(0, normalized.length() - 4)
    return normalized


## Indexes by node reference, not NodePath - the correct mesh_path (a path FROM the widget TO
## the submodel) can only be computed once the widget itself has a place in the tree, which
## happens later, in _wire_mesh_path().
##
## E3DNodesInstancer adds every submodel node as an INTERNAL child (INTERNAL_MODE_BACK, since
## `editable` is false at runtime) - get_children() without `true` silently returns none of them,
## making every lookup fail.
##
## Indexed by LOWERCASED name, matching the original engine's own TSubModel::GetFromName(search,
## i=true), which is case-insensitive by default. Confirmed necessary against real data: ST44's
## submodel names happen to match MMD's declared case exactly ("nastawnik"/"zasadniczy"), but
## su45_v2/kabina-su45-a.e3d's brake gauge submodels are actually "przglknob06"/"przglknob05"
## while 301d.mmd declares them "PrzGlKnob06"/"PrzGlKnob05" - a case-sensitive lookup silently
## fails to bind these, leaving those gauges dead with no diagnostic (matches.is_empty() still
## fires correctly, but only after realizing the exact-case assumption was wrong).
static func _index_submodels(node:Node, index:Dictionary) -> void:
    for child:Node in node.get_children(true):
        var child_name:String = child.name.to_lower()
        if not index.has(child_name):
            index[child_name] = []
        index[child_name].append(child)
        _index_submodels(child, index)


static func _build_widget(
        descriptor:MmdInstrumentDescriptor, controller:TrainController,
        cab_number:int, diagnostics:Array[Dictionary]) -> Node:
    var entry:Dictionary = MmdSemanticCatalog.get_entry(descriptor.label)
    var widget:Node = entry["widget_class"].new()
    widget.name = "%s_%s" % [descriptor.label, descriptor.submodel_name]

    for field_name:String in entry["fixed_fields"]:
        widget.set(field_name, entry["fixed_fields"][field_name])

    var config_max_property:String = entry.get("config_max_property", "")
    if config_max_property:
        var fallback:Variant = widget.get("switch_max_position")
        widget.set("switch_max_position", int(controller.config.get(config_max_property, fallback)))

    # "i-*:" indicator descriptors (see _parse_indicator()) never set animation_type - they have
    # no "rot"/"mov" shape at all, so there's nothing for _apply_animation_shape() to compute.
    if descriptor.animation_type:
        _apply_animation_shape(widget, descriptor, entry, controller, cab_number, diagnostics)
    _apply_sound(widget, descriptor)

    return widget


## Wires parsed MMD sound_increase/sound_decrease/sound_positions onto whichever sound fields the
## widget actually has (CabinButton: sound_on/sound_off; CabinSwitch: sound_increase_stream/
## sound_decrease_stream/sound_override/sound_override_negative) - duck-typed the same way
## mesh_path/target_mesh_path already are. CabinKnob/CabinGauge have no sound fields at all today
## (see mmd_semantic_catalog.gd's scope notes), so this is a no-op for those widget types.
static func _apply_sound(widget:Node, descriptor:MmdInstrumentDescriptor) -> void:
    if "sound_on" in widget:
        if descriptor.sound_increase:
            widget.set("sound_on", _build_audio_stream(descriptor.sound_increase))
        if descriptor.sound_decrease:
            widget.set("sound_off", _build_audio_stream(descriptor.sound_decrease))
        return

    if "sound_increase_stream" in widget:
        if descriptor.sound_increase:
            widget.set("sound_increase_stream", _build_audio_stream(descriptor.sound_increase))
        if descriptor.sound_decrease:
            widget.set("sound_decrease_stream", _build_audio_stream(descriptor.sound_decrease))
        if not descriptor.sound_positions.is_empty():
            var positive:Array[AudioStream] = []
            var negative:Array[AudioStream] = []
            for position:int in descriptor.sound_positions:
                var stream:AudioStream = _build_audio_stream(descriptor.sound_positions[position])
                if not stream:
                    continue
                if position > 0:
                    while positive.size() < position:
                        positive.append(null)
                    positive[position - 1] = stream
                elif position < 0:
                    var idx:int = -position - 1
                    while negative.size() <= idx:
                        negative.append(null)
                    negative[idx] = stream
            if not positive.is_empty():
                widget.set("sound_override", positive)
            if not negative.is_empty():
                widget.set("sound_override_negative", negative)


static func _build_audio_stream(filename:String) -> AudioStream:
    if filename.is_empty():
        return null
    var stream := MaszynaAudioStream.new()
    stream.file_path = filename
    return stream


## Sets the widget's mesh_rotation/mesh_position "full-swing" target directly from this vehicle's
## own MMD scale/animation_type - cabin geometry differs per vehicle, so this can't be a fixed
## per-label constant (see MmdSemanticCatalog's header comment). Matches the original engine's
## TGauge formula evaluated at value=1: `rot` -> rotation.y = scale*360 degrees; `mov` ->
## position.z = scale (MMD's own local Z axis convention). `offset` has no equivalent on these
## widgets (no baseline-shift concept) and is reported instead of silently dropped.
##
## `entry["animation_range_config_properties"]`, when present, is `[min_key, max_key]` into
## controller.config - MMD's scale is calibrated against MaSzyna's raw value domain for a
## property (e.g. TrainBrake's fBrakeCtrlPos), but the widget may be bound to an already-
## normalized (0..1) state_property instead (brakectrl: the command it sends,
## TrainBrake::brake_level_set, itself expects a normalized level, so the widget's value/command
## domain has to stay normalized even though that's not what MMD's scale assumes). Multiplying
## the raw-domain-derived degrees/position by (max-min) - the same range the state's own
## normalization divides by - converts it to the correct per-normalized-unit amount without
## having to change the widget's value domain (and therefore without breaking its command).
##
## `entry["mmd_scale_multiplier"]` (default 1.0) mirrors the original engine's own
## TGauge::Load(..., mul) parameter (vehicle/Gauge.cpp:182, `scale *= mul`): Train.cpp calls
## gauge.Load(..., 0.1) specifically for pressure-family gauge labels (brakepress, pipepress,
## scndpress, compressor, ...), while every other gauge label (confirmed: tachometer, oilpress)
## uses the implicit default of 1.0. This is the original engine's own fixed per-label
## correction factor, not a per-vehicle guess - confirmed by reading vehicle/Train.cpp directly.
static func _apply_animation_shape(
        widget:Node, descriptor:MmdInstrumentDescriptor, entry:Dictionary, controller:TrainController,
        cab_number:int, diagnostics:Array[Dictionary]) -> void:
    var range_scale:float = 1.0
    var range_properties:Array = entry.get("animation_range_config_properties", [])
    if range_properties.size() == 2:
        var range_min:float = float(controller.config.get(range_properties[0], 0.0))
        var range_max:float = float(controller.config.get(range_properties[1], 1.0))
        range_scale = range_max - range_min

    var mmd_scale:float = descriptor.scale * float(entry.get("mmd_scale_multiplier", 1.0))

    match descriptor.animation_type:
        "rot":
            if "mesh_rotation" in widget:
                var rotation_vec:Vector3 = widget.get("mesh_rotation")
                rotation_vec.y = mmd_scale * 360.0 * range_scale
                widget.set("mesh_rotation", rotation_vec)
            else:
                diagnostics.append(_diag(
                        "info", "MMD_ANIMATION_UNSUPPORTED",
                        "Label '%s' uses 'rot' but its widget has no mesh_rotation field" % descriptor.label,
                        cab_number, descriptor.label, descriptor.submodel_name))
        "mov":
            if "mesh_position" in widget:
                var position_vec:Vector3 = widget.get("mesh_position")
                position_vec.z = mmd_scale * range_scale
                widget.set("mesh_position", position_vec)
            else:
                diagnostics.append(_diag(
                        "info", "MMD_ANIMATION_UNSUPPORTED",
                        "Label '%s' uses 'mov' but its widget has no mesh_position field (e.g. CabinGauge)" % descriptor.label,
                        cab_number, descriptor.label, descriptor.submodel_name))
        _:
            diagnostics.append(_diag(
                    "info", "MMD_ANIMATION_UNSUPPORTED",
                    "Label '%s' uses unsupported animation type '%s'" % [descriptor.label, descriptor.animation_type],
                    cab_number, descriptor.label, descriptor.submodel_name))

    if not is_zero_approx(descriptor.offset):
        diagnostics.append(_diag(
                "info", "MMD_ANIMATION_UNSUPPORTED",
                "Label '%s' has a non-zero MMD offset (%s) which the reused cabin widgets cannot represent - ignored" % [descriptor.label, descriptor.offset],
                cab_number, descriptor.label, descriptor.submodel_name))


## `widget` must already be inside the tree (a child of the same generated_root as `model`'s
## submodels) before this is called - the mesh-path property is a NodePath FROM the widget TO
## the target. Field name differs per widget class: CabinButton/CabinSwitch/CabinKnob all use
## `mesh_path`, but CabinGauge uses `target_mesh_path` - MmdSemanticCatalog entries carry
## `mesh_path_field` precisely so this doesn't have to special-case by class.
static func _wire_mesh_path(
        widget:Node, descriptor:MmdInstrumentDescriptor, submodel_index:Dictionary,
        mesh_path_field:String, cab_number:int, diagnostics:Array[Dictionary]) -> void:
    var matches:Array = submodel_index.get(descriptor.submodel_name.to_lower(), [])
    if matches.size() == 1:
        widget.set(mesh_path_field, widget.get_path_to(matches[0]))
    elif matches.is_empty():
        diagnostics.append(_diag(
                "warning", "MMD_SUBMODEL_NOT_FOUND",
                "Submodel '%s' not found (label '%s')" % [descriptor.submodel_name, descriptor.label],
                cab_number, descriptor.label, descriptor.submodel_name))
    else:
        diagnostics.append(_diag(
                "warning", "MMD_SUBMODEL_AMBIGUOUS",
                "Submodel '%s' has %d matches (label '%s') - mesh not bound" % [descriptor.submodel_name, matches.size(), descriptor.label],
                cab_number, descriptor.label, descriptor.submodel_name))

    # Animation shape (mesh_rotation/max_value) comes from entry["fixed_fields"] above, not from
    # descriptor.scale/offset/friction - see mmd_semantic_catalog.gd's header comment for why.


## Builds one CabinSpotLight3D per matched "<base>_on"/"<base>_off" submodel INSTANCE, not just
## one widget total - some real cabins have multiple physical lamp housings sharing the same
## MMD-declared base name (confirmed real: sm_42_cabin.tscn's own hand-authored reference has 3
## "CzuwakOmni" lights for its one "i-security_aware:" label), unlike every other instrument label
## (which only ever has one real target mesh, so _wire_mesh_path() builds exactly one widget).
## on/off pairs are matched by index (real data doesn't guarantee they're declared in matching
## relative order - the best available heuristic without per-instance correlation data); an index
## missing one side just leaves that widget's corresponding target_path unset.
##
## "i-*:" labels declare a bare BASE name ("czuwak") that is never itself a real submodel - the
## original engine's own TButton::Init() (Button.cpp:32-33) always searches for "<name>_on" and
## "<name>_off" instead (confirmed real: SM42's own hand-authored cabin points its blinker at
## ".../czuwak_on" directly, and real-vehicle diagnostics confirmed the bare name is never found -
## EP09 uses base name "ca", so the real submodels there are "ca_on"/"ca_off").
static func _build_indicator_lights(
        descriptor:MmdInstrumentDescriptor, entry:Dictionary, controller:TrainController,
        submodel_index:Dictionary, generated_root:Node3D, cab_number:int, diagnostics:Array[Dictionary]) -> void:
    var base_name:String = descriptor.submodel_name.to_lower()
    var on_matches:Array = submodel_index.get(base_name + "_on", [])
    var off_matches:Array = submodel_index.get(base_name + "_off", [])
    var count:int = maxi(on_matches.size(), off_matches.size())

    if count == 0:
        diagnostics.append(_diag(
                "warning", "MMD_SUBMODEL_NOT_FOUND",
                "Submodel '%s_on'/'%s_off' not found (label '%s')" % [descriptor.submodel_name, descriptor.submodel_name, descriptor.label],
                cab_number, descriptor.label, descriptor.submodel_name))
        return

    for i in range(count):
        var widget:CabinSpotLight3D = entry["widget_class"].new()
        widget.name = "%s_%s_%d" % [descriptor.label, descriptor.submodel_name, i]
        for field_name:String in entry["fixed_fields"]:
            widget.set(field_name, entry["fixed_fields"][field_name])
        generated_root.add_child(widget)

        var on_node:Node3D = on_matches[i] if i < on_matches.size() else null
        var off_node:Node3D = off_matches[i] if i < off_matches.size() else null
        _position_at_submodel_instance(widget, on_node if on_node else off_node)
        if on_node:
            widget.on_target_path = widget.get_path_to(on_node)
        if off_node:
            widget.off_target_path = widget.get_path_to(off_node)
        widget.controller_path = widget.get_path_to(controller)


## Positions `widget` at `submodel`'s visual surface facing the driver, not its raw transform
## origin/pivot (frequently off to one side, e.g. its mounting point) and not its bare AABB center
## either (a light sitting exactly at a solid mesh's center is embedded inside the geometry,
## looking wrong) - the AABB center pushed forward along local +Z by half the AABB's own depth,
## landing it at the housing's front-facing surface instead. +Z (not -Z) because submodels share
## the cab model's own axis convention, where -Z is the cab's forward/travel direction, so a
## dashboard-mounted housing's driver-facing "open" side - the opposite of the cab's own front -
## is +Z.
static func _position_at_submodel_instance(widget:Node3D, submodel:Node3D) -> void:
    var target_transform:Transform3D = submodel.global_transform
    if submodel is VisualInstance3D:
        var aabb:AABB = (submodel as VisualInstance3D).get_aabb()
        var local_point:Vector3 = aabb.get_center() + Vector3(0.0, 0.0, aabb.size.z * 0.5)
        target_transform.origin = submodel.to_global(local_point)
    widget.global_transform = target_transform


static func _tokenize_file(abs_path:String, context:MmdImportContext, parameters:Dictionary = {}) -> Array[String]:
    context.include_depth += 1
    if context.include_depth > 32:
        context.add_diagnostic("error", "MMD_INCLUDE_CYCLE", "Include depth exceeded (circular include?): " + abs_path, abs_path)
        context.include_depth -= 1
        return []

    var file:FileAccess = FileAccess.open(abs_path, FileAccess.READ)
    if not file:
        context.add_diagnostic("error", "MMD_INCLUDE_NOT_FOUND", "Cannot open MMD file: " + abs_path, abs_path)
        context.include_depth -= 1
        return []
    var buffer:PackedByteArray = _strip_bom(file.get_buffer(file.get_length()))
    file.close()

    var p := MaszynaParser.new()
    p.initialize(buffer)
    if not parameters.is_empty():
        p.set_parameters(parameters)

    var dir:String = abs_path.get_base_dir()
    var tokens:Array[String] = []
    while not p.eof_reached():
        var token:String = p.next_token()
        if token.is_empty():
            continue
        # ":" is not a MaszynaParser stop char, so a label glued directly to its first value with
        # no space (real data: "radiostop_sw:radiostop") comes back as one token - split it into
        # the label (colon kept) and the remainder so the rest of this parser, which always
        # expects "label:" as its own token, still works.
        var colon_index:int = token.find(":")
        if colon_index != -1 and colon_index < token.length() - 1:
            tokens.append(token.substr(0, colon_index + 1))
            token = token.substr(colon_index + 1)
        if token.to_lower() == "include":
            tokens.append_array(_handle_include(p, dir, context, abs_path))
        else:
            tokens.append(token)

    context.include_depth -= 1
    return tokens


## `include filename p1 p2 ... end` or the random-file-set form `include [a.inc b.inc] end`.
static func _handle_include(p:MaszynaParser, dir:String, context:MmdImportContext, current_file:String) -> Array[String]:
    var first:String = p.next_token()
    var is_random:bool = first == _RANDOM_INCLUDE_OPEN
    var candidates:Array[String] = []
    var include_filename:String = first

    if is_random:
        var t:String = p.next_token()
        while not t.is_empty() and t != _RANDOM_INCLUDE_CLOSE:
            candidates.append(t)
            t = p.next_token()

    var params:Array[String] = []
    var t2:String = p.next_token()
    while not t2.is_empty() and t2.to_lower() != _INCLUDE_END_KEYWORD:
        params.append(t2)
        t2 = p.next_token()

    if is_random:
        if candidates.is_empty():
            context.add_diagnostic("error", "MMD_INVALID_CAB_DEFINITION", "Empty random include list", current_file)
            return []
        # Keyed by the include site's own content (not call order), so a later re-parse of the
        # same file with the same random_choices dict reproduces the same choice.
        var choice_key:String = "%s#%s" % [current_file, "|".join(candidates)]
        if not context.random_choices.has(choice_key):
            context.random_choices[choice_key] = candidates[randi() % candidates.size()]
        include_filename = context.random_choices[choice_key]

    if include_filename.is_empty():
        context.add_diagnostic("error", "MMD_INVALID_CAB_DEFINITION", "Empty include filename", current_file)
        return []

    var param_dict:Dictionary = {}
    for i in range(params.size()):
        param_dict["p%d" % (i + 1)] = params[i]
    for i in range(1, 10):
        var key:String = "p%d" % i
        if not param_dict.has(key):
            param_dict[key] = "none" # missing (pN) reference defaults to "none"

    return _tokenize_file(dir.path_join(include_filename), context, param_dict)


static func _strip_bom(buffer:PackedByteArray) -> PackedByteArray:
    if buffer.size() >= 3 and buffer[0] == 0xEF and buffer[1] == 0xBB and buffer[2] == 0xBF:
        return buffer.slice(3)
    return buffer
