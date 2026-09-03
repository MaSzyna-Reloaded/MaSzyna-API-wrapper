@tool
extends Node
class_name FizTrainControllerInstancer

## Top-level FIZ file orchestrator, mirroring scenery_instancer.gd's role for scenery files.
##
## FIZ's key=value micro-language is detected by line-PREFIX match (mirrors MaSzyna's own
## `issection()`, Mover.cpp:9035-9039), not by the exact-token `register_handler`/`parse()`
## loop the scenery importers use - a section header and its first key=value pair share one
## token (e.g. `Cntrl.BCPN=6`), so section identification happens per physical line, and only
## the remainder of that line (and, for table sections, the following lines up to an end
## marker) gets tokenized.

const _INCLUDE_KEYWORD := "include"
const _INCLUDE_END_KEYWORD := "end"

## Ordered (longest-prefix-first where ambiguity is possible) table of recognized FIZ section
## headers. `parser` is a section parser instance (see fiz_train_*_parser.gd) exposing
## `parse(p: MaszynaParser, context, prefix)` and, for table sections, `parse_row(p, context)` +
## `end_table(context)` - `p` is a throwaway MaszynaParser scoped to exactly one line (header
## line with its prefix stripped, or one table row), mirroring the original LoadFIZ_* code's
## habit of constructing "a fresh cParser parser(line)" per line/row. `table_end` is the
## literal end-of-table token for table sections, "" for plain scalar sections. `parser ==
## null` means "recognized but not yet implemented / has no Godot class" - the section (and
## any table rows up to table_end, if set) is skipped with a single warning per vehicle.
static var _sections: Array[Dictionary] = []
static var _sections_built := false


static func _ensure_sections() -> void:
    if _sections_built:
        return
    _sections_built = true

    var controller_parser := FizTrainControllerParser.new()
    var wheels_parser := FizTrainWheelsParser.new()
    var doors_parser := FizTrainDoorsParser.new()
    var buff_coupl_parser := FizTrainBuffCouplParser.new()
    var brake_parser := FizTrainBrakeParser.new()
    var cntrl_parser := FizTrainCntrlParser.new(controller_parser, brake_parser)
    var lighting_parser := FizTrainLightingParser.new()
    var heating_parser := FizTrainHeatingParser.new()
    var power_parser := FizTrainPowerParser.new()
    var engine_parser := FizTrainEngineParser.new()
    var electric_series_parser := engine_parser.electric_series_parser

    _sections = [
        # controller-mapped
        {"prefix": "Param.", "parser": controller_parser, "table_end": ""},
        {"prefix": "Dimensions:", "parser": controller_parser, "table_end": ""},
        {"prefix": "Load:", "parser": null, "table_end": ""}, # TrainLoad - not yet implemented
        {"prefix": "Wheels:", "parser": wheels_parser, "table_end": ""},
        # brake family (Cntrl. also starts the BPT table)
        {"prefix": "Brake:", "parser": brake_parser, "table_end": ""},
        {"prefix": "Cntrl.", "parser": cntrl_parser, "table_end": ""}, # BPT rows start after this line, see below
        {"prefix": "SpringBrake:", "parser": null, "table_end": ""},
        {"prefix": "Blending:", "parser": null, "table_end": ""},
        {"prefix": "DCEMUED:", "parser": null, "table_end": ""},
        {"prefix": "CompressorList:", "parser": brake_parser, "table_end": "endCL"},
        # doors / couplers
        {"prefix": "Doors:", "parser": doors_parser, "table_end": ""},
        {"prefix": "BuffCoupl1.", "parser": buff_coupl_parser, "table_end": ""},
        {"prefix": "BuffCoupl2.", "parser": buff_coupl_parser, "table_end": ""},
        {"prefix": "BuffCoupl.", "parser": buff_coupl_parser, "table_end": ""},
        # lighting / heating / power
        {"prefix": "Headlights:", "parser": lighting_parser, "table_end": ""},
        {"prefix": "LightsList:", "parser": null, "table_end": "endL"}, # rows not decoded yet, see fiz_train_lighting_parser.gd
        {"prefix": "Light:", "parser": lighting_parser, "table_end": ""},
        {"prefix": "Clima:", "parser": heating_parser, "table_end": ""},
        {"prefix": "Power:", "parser": power_parser, "table_end": ""},
        {"prefix": "SpeedControl:", "parser": null, "table_end": ""},
        {"prefix": "Switches:", "parser": null, "table_end": ""},
        {"prefix": "DimmerList:", "parser": null, "table_end": "endDimmerList"},
        {"prefix": "AI:", "parser": null, "table_end": ""},
        {"prefix": "Security:", "parser": null, "table_end": ""},
        {"prefix": "WiperList:", "parser": null, "table_end": "endwl"},
        {"prefix": "UCList:", "parser": null, "table_end": "END-UCL"},
        # engine family
        {"prefix": "Engine:", "parser": engine_parser, "table_end": ""},
        # MotorParamTable0: is what ElectricSeriesMotor vehicles actually use (confirmed
        # against the wiki and real files); MotorParamTable: (no "0") rows are still
        # unmapped - it's a diesel-mechanical-transmission-specific format not implemented yet.
        {"prefix": "MotorParamTable0:", "parser": electric_series_parser, "table_end": "END-MPT"},
        {"prefix": "MotorParamTable:", "parser": null, "table_end": "END-MPT"},
        {"prefix": "Circuit:", "parser": electric_series_parser, "table_end": ""},
        {"prefix": "RList:", "parser": electric_series_parser, "table_end": "END-RL"},
        {"prefix": "DList:", "parser": null, "table_end": "END-DL"},
        {"prefix": "DMList:", "parser": null, "table_end": "END-DML"},
        {"prefix": "HTCList:", "parser": null, "table_end": "END-HTCL"},
        {"prefix": "PmaxList:", "parser": null, "table_end": "END-PML"},
        {"prefix": "WWList:", "parser": null, "table_end": "END-WWL"},
        # sections with no Godot-class home yet (recognized so their rows aren't misparsed)
        {"prefix": "TurboPos:", "parser": null, "table_end": ""},
        {"prefix": "ffBrakeList:", "parser": null, "table_end": "endff"},
        {"prefix": "ffList:", "parser": null, "table_end": "endff"},
        {"prefix": "V2NList:", "parser": null, "table_end": "END-V2NL"},
    ]


## Populates an EXISTING TrainController (`target`) from a FIZ file: root-level properties
## (Param./Dimensions:/Cntrl. general subset/...) are applied directly to `target`, and its
## parsed TrainPart children are added under it. `target` must have no children of its own
## yet - the caller is responsible for clearing any previous FIZ-sourced children first (see
## FIZTrainController._reload()). `fiz_path` must already be a fully resolved, openable path
## (res://, user://, or absolute) - e.g. UserSettings.get_maszyna_game_dir().path_join(
## "pkp/eu04_v1/eu04-01.fiz"). `include` directives inside the file resolve relative to its
## own containing directory.
static func build_into(target: TrainController, fiz_path: String) -> void:
    _ensure_sections()
    var context := FizImportContext.new()
    context.base_dir = fiz_path.get_base_dir()
    context.controller = target

    var table_state: Dictionary = {"prefix": "", "parser": null, "end": ""}
    _parse_file(fiz_path, context.base_dir, context, table_state)

    if table_state["parser"] != null:
        table_state["parser"].end_table(context)

    for part_name: String in context.parts:
        var node: Node = context.parts[part_name]
        node.name = part_name
        target.add_child(node)


## Builds a new, unparented TrainController + children from a FIZ file.
static func build(fiz_path: String) -> TrainController:
    var controller := TrainController.new()
    build_into(controller, fiz_path)
    return controller


static func build_scene(fiz_path: String) -> PackedScene:
    var root := build(fiz_path)
    # PackedScene.pack() only includes nodes whose `owner` is set - without this, the packed
    # scene would contain just the root TrainController and silently drop every TrainPart child.
    for child: Node in root.get_children():
        child.owner = root
    var scene := PackedScene.new()
    var err: Error = scene.pack(root)
    root.free()
    if err != OK:
        push_error("Could not pack FIZ scene for: " + fiz_path)
        return null
    return scene


## Reads one logical line off a MaszynaParser's byte stream, mirroring the original
## `getToken<std::string>(false, "\n\r")` line reader (Mover.cpp:9661) - a raw, un-tokenized
## line, stopping at \n or \r. Unlike MaszynaParser.get_line() (which treats \r and \n as two
## independent terminators, producing a spurious empty "line" for every CRLF pair), this
## consumes a \n that immediately follows a \r as part of the same terminator, so blank-line
## detection (used to end the brake-position table) isn't corrupted by CRLF encoding.
static func _read_fiz_line(p: MaszynaParser) -> String:
    var bytes := PackedByteArray()
    while not p.eof_reached():
        var c: int = p.get8()
        if c == -1 or c == 10: # EOF or \n
            break
        if c == 13: # \r - swallow a paired \n, if any
            if not p.eof_reached():
                var c2: int = p.get8()
                if c2 != 10 and c2 != -1:
                    bytes.append(c2) # not a CRLF pair (lone CR); keep the byte we peeked
            break
        bytes.append(c)
    return bytes.get_string_from_utf8()


static func _parse_file(abs_path: String, dir: String, context: FizImportContext, table_state: Dictionary) -> void:
    context.include_depth += 1
    if context.include_depth > 32:
        push_error("FIZ include depth exceeded (circular include?): " + abs_path)
        context.include_depth -= 1
        return

    var file := FileAccess.open(abs_path, FileAccess.READ)
    if not file:
        push_error("Cannot open FIZ file: " + abs_path)
        context.include_depth -= 1
        return

    var p := MaszynaParser.new()
    p.initialize(file.get_buffer(file.get_length()))
    file.close()

    while not p.eof_reached():
        var raw_line: String = _read_fiz_line(p)
        var line: String = raw_line.strip_edges(true, false)

        # FIZ-specific: a line containing an unescaped '#' anywhere is fully ignored. This is
        # distinct from `//`/`/* */`, which MaszynaParser's tokenizer already strips on its
        # own - a `//`-only line naturally yields zero tokens below, no special-casing needed.
        if line.find("#") != -1:
            if table_state["prefix"] == "BPT":
                table_state["prefix"] = ""
                table_state["parser"] = null
            continue

        if line.is_empty():
            if table_state["prefix"] == "BPT":
                table_state["parser"].end_table(context)
                table_state["prefix"] = ""
                table_state["parser"] = null
                table_state["end"] = ""
            continue

        var line_parser := MaszynaParser.new()
        line_parser.initialize(line.to_utf8_buffer())
        var first_token: String = line_parser.next_token()
        if first_token.is_empty():
            continue # line was entirely a `//`/`/* */` comment

        # include <file> [params...] end - splices the referenced file's lines in place,
        # sharing the same table_state so an in-progress table can (rarely) continue across
        # the include boundary, matching the original cParser's transparent splicing.
        if first_token == _INCLUDE_KEYWORD:
            var include_filename: String = line_parser.next_token()
            if not include_filename.is_empty():
                var include_path: String = dir.path_join(include_filename)
                _parse_file(include_path, include_path.get_base_dir(), context, table_state)
            continue
        if first_token == _INCLUDE_END_KEYWORD:
            continue

        var matched_section: Dictionary = {}
        for section: Dictionary in _sections:
            if line.begins_with(section["prefix"]):
                matched_section = section
                break

        if not matched_section.is_empty():
            _dispatch_header(matched_section, line, context, table_state)
            continue

        if table_state["end"] != "" and line.strip_edges() == table_state["end"]:
            if table_state["parser"] != null:
                table_state["parser"].end_table(context)
            table_state["prefix"] = ""
            table_state["parser"] = null
            table_state["end"] = ""
            continue

        if table_state["parser"] != null:
            var row_parser := MaszynaParser.new()
            row_parser.initialize(line.to_utf8_buffer())
            table_state["parser"].parse_row(row_parser, context)
        # else: unrecognized line outside any table - ignored, matching original tolerance.

    context.include_depth -= 1


static func _dispatch_header(
        section: Dictionary, line: String, context: FizImportContext, table_state: Dictionary) -> void:
    var prefix: String = section["prefix"]
    var line_parser := MaszynaParser.new()
    line_parser.initialize(line.substr(prefix.length()).to_utf8_buffer())

    if section["parser"] == null:
        context.warn_unmapped_section(prefix)
    else:
        section["parser"].parse(line_parser, context, prefix)

    # Cntrl. additionally opens the brake-position table (only when BrakeSystem != Individual,
    # decided by Brake:/Cntrl. themselves inside TrainBrake's own parser). Any other recognized
    # header - including one encountered while a table (BPT or otherwise) is still active -
    # ends whatever table was active, matching header-match precedence over table-row fallback.
    if prefix == "Cntrl." and section["parser"] != null and section["parser"].wants_bpt_table(context):
        table_state["prefix"] = "BPT"
        table_state["parser"] = section["parser"]
        table_state["end"] = ""
    elif section["table_end"] != "":
        table_state["prefix"] = prefix
        table_state["parser"] = section["parser"]
        table_state["end"] = section["table_end"]
    else:
        if table_state["parser"] != null:
            table_state["parser"].end_table(context)
        table_state["prefix"] = ""
        table_state["parser"] = null
        table_state["end"] = ""
