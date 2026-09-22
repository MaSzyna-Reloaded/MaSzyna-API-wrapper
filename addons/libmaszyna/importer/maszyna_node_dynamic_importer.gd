@tool
extends RefCounted

const MAX_FIZ_INCLUDE_DEPTH:int = 4
## coupling::permanent (MOVER.h)
const COUPLING_PERMANENT:int = 128

## Ports deserialize_dynamic()'s placement math (simulationstateserializer.cpp) onto
## DynamicRailVehicle3D. Field order: datafolder, skinfile, mmdfile, [pathname - only when not
## inside a trainset, see below], offset, drivertype, [couplingdata - only inside a trainset],
## [velocity - only when not inside a trainset], loadcount, [loadtype if loadcount != 0],
## optional trailing destination, "enddynamic".
##
## Placement offset is NOT simply trainset_offset: the original computes
## `offset == -1.0 ? trainset.offset : trainset.offset - offset`, then decrements
## trainset.offset by the vehicle's own physical length (from its .fiz Dimensions: L=) for the
## NEXT vehicle in the consist - read directly and synchronously here (not via
## FizVehiclePhysicsNode's own async pipeline, which only finishes loading after scene
## construction, too late to affect this vehicle's own placement).
##
## offset == -1.0 is also, separately, the original's own sentinel for "place this vehicle
## reversed in the consist" - confirmed against simulationstateserializer.cpp:983
## (`vehicle->Init(..., ( offset == -1.0 ), params)`) and DynObj.cpp:1807
## (`iDirection = (Reversed ? 0 : 1)`). Previously only used here for the offset-math branch,
## never for direction - a real vehicle placed with offset: -1.0 (e.g. a reversed EZT member)
## silently always imported as DIRECTION_NORMAL.
func import(p:MaszynaParser, context: MaszynaImporterContext) -> DynamicRailVehicle3D:
    var data_folder:String = _resolve_data_path(p.next_token().replace("\\", "/").to_lower())
    var skin_file:String = p.next_token().to_lower()
    var mmd_file:String = p.next_token().to_lower()
    var path_name:String = context.trainset_track if context.trainset_open else p.next_token()
    var offset:float = float(p.next_token())
    var driver_type:String = p.next_token()
    var coupling_data:String = p.next_token() if context.trainset_open else "3"
    var velocity:float = context.trainset_velocity if context.trainset_open else float(p.next_token())
    var load_count:int = int(p.next_token())
    var _load_type:String = p.next_token() if load_count != 0 else ""

    var reversed:bool = is_equal_approx(offset, -1.0)
    var trainset_offset:float = context.trainset_offset if context.trainset_open else 0.0
    var start_offset:float = trainset_offset if reversed else trainset_offset - offset
    var length:float = _read_vehicle_length(data_folder, mmd_file, context)

    var vehicle := DynamicRailVehicle3D.new()
    vehicle.data_path = data_folder
    vehicle.file_name = mmd_file
    vehicle.skin = skin_file
    vehicle.start_track_name = path_name
    # start_offset marks the vehicle's front; start_track_offset is its center, which the
    # original gets the same way (DynObj.cpp:2308, fDist -= 0.5 * Dim.L).
    vehicle.start_track_offset = start_offset - 0.5 * length
    vehicle.start_direction = (
        TrackManager.DIRECTION_REVERSED if reversed else TrackManager.DIRECTION_NORMAL
    )
    vehicle.initial_velocity = velocity
    # DynObj.cpp:1812-1825 - headdriver occupies cab 1, reardriver cab 2 (-1), anything else none.
    vehicle.cabin_number = 1 if driver_type == "headdriver" else (-1 if driver_type == "reardriver" else 0)

    if context.trainset_open:
        context.trainset_offset -= length
        context.trainset_node.couplings.append(_parse_coupling(coupling_data, offset, reversed))

    var next_token:String = p.next_token()
    if not next_token == "enddynamic":
        # optional trailing destination parameter, not used yet
        p.get_tokens_until("enddynamic")

    return vehicle


## Coupling type with the next vehicle of the trainset (simulationstateserializer.cpp:921-934):
## the number before an optional "." parameter list, negative means a permanent coupling, and a
## vehicle placed further than 0.5 m from the previous one isn't coupled at all.
func _parse_coupling(coupling_data:String, offset:float, reversed:bool) -> int:
    var coupling:int = int(coupling_data.get_slice(".", 0))
    if coupling < 0:
        coupling = -coupling | COUPLING_PERMANENT
    if not reversed and absf(offset) > 0.5:
        coupling = 0
    return coupling


## Same convention as maszyna_node_model_importer.gd's data_path handling: the .scn token gives
## a path relative to the "dynamic" data root (e.g. "pkp/303e_v1"), not a full path - prepend
## "dynamic" when it isn't already there. Real game data paths are lowercase on disk even when
## the .scn token itself uses mixed/upper case (e.g. "PKP/303E_V1").
func _resolve_data_path(data_folder:String) -> String:
    var data_path_array:Array = data_folder.split("/")
    if not data_path_array or not data_path_array[0] == "dynamic":
        data_path_array.insert(0, "dynamic")
    return "/".join(data_path_array)


## Reads just the "Dimensions: L=..." value out of a vehicle's .fiz file - not the full FIZ
## import pipeline, just enough for trainset offset chaining. Uses MaszynaParser (the same
## whitespace-agnostic tokenizer every other .fiz/.scn reader in this addon uses) rather than
## splitting by line: some .fiz files (e.g. 303e-ep-tv.fiz) spell "include" across three separate
## lines ("include" / "303e-ep.fiz" / "end"), which a line-oriented reader silently misses
## entirely - the token itself doesn't care where the line breaks fall.
func _read_vehicle_length(data_path:String, file_name:String, context:MaszynaImporterContext) -> float:
    var abs_path:String = UserSettings.get_maszyna_game_dir().path_join(data_path).path_join(file_name + ".fiz")
    return _read_length_from_fiz_file(abs_path, 0, context)


func _read_length_from_fiz_file(abs_path:String, depth:int, context:MaszynaImporterContext) -> float:
    if depth > MAX_FIZ_INCLUDE_DEPTH:
        return 0.0
    var file := FileAccess.open(abs_path, FileAccess.READ)
    if not file:
        context.cacheable = false
        return 0.0
    context.register_dependency(abs_path, file.get_length())

    var base_dir:String = abs_path.get_base_dir()
    var parser := MaszynaParser.new()
    parser.initialize(file.get_buffer(file.get_length()), [])

    while not parser.eof_reached():
        var token:String = parser.next_token()
        if not token:
            break

        var lower_token:String = token.to_lower()
        if lower_token == "include":
            var include_filename:String = parser.next_token()
            var included_length:float = _read_length_from_fiz_file(
                base_dir.path_join(include_filename), depth + 1, context
            )
            if included_length > 0.0:
                return included_length
        elif lower_token == "dimensions:":
            var length:float = _read_dimensions_length(parser)
            if length > 0.0:
                return length

    return 0.0


## "Dimensions:" is followed by space-separated key=value tokens (L=, H=, W=, Cx=, ...) with no
## explicit terminator - stop at the first token that isn't itself a key=value pair, which marks
## the start of the next statement.
func _read_dimensions_length(parser:MaszynaParser) -> float:
    while not parser.eof_reached():
        var token:String = parser.next_token()
        if not token or not token.contains("="):
            break
        if token.to_lower().begins_with("l="):
            return token.substr(2).to_float()
    return 0.0
