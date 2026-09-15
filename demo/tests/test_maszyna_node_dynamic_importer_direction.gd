extends MaszynaGutTest

## Placement of .scn `dynamic` nodes, ported from deserialize_dynamic()
## (simulationstateserializer.cpp:960-988) and TDynamicObject::Init() (DynObj.cpp):
## - offset == -1.0 means "reversed in the consist" (`Init(..., (offset == -1.0), ...)`,
##   DynObj.cpp:1807).
## - the resulting distance marks the vehicle's FRONT; the vehicle's center sits half its
##   Dimensions L= behind it (DynObj.cpp:2308, `fDist -= 0.5 * Dim.L`), and the next vehicle in
##   the trainset starts a full L= further back. Placing the center at the front distance made
##   neighbouring vehicles of different lengths overlap.

const TEST_GAME_DIR:String = "user://gut/dynamic_importer_fixture"
const SHORT_LENGTH:float = 10.0
const LONG_LENGTH:float = 16.0

var importer:RefCounted
var _previous_game_dir:String
var _vehicles:Array[DynamicRailVehicle3D] = []


func before_each() -> void:
    importer = load("res://addons/libmaszyna/importer/maszyna_node_dynamic_importer.gd").new()
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    var fixture_dir:String = TEST_GAME_DIR.path_join("dynamic/fixtures")
    DirAccess.make_dir_recursive_absolute(fixture_dir)
    _write_fiz(fixture_dir.path_join("short.fiz"), SHORT_LENGTH)
    _write_fiz(fixture_dir.path_join("long.fiz"), LONG_LENGTH)
    UserSettings.save_maszyna_game_dir(TEST_GAME_DIR)


func after_each() -> void:
    for vehicle:DynamicRailVehicle3D in _vehicles:
        vehicle.free()
    _vehicles.clear()
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func _write_fiz(path:String, length:float) -> void:
    var file:FileAccess = FileAccess.open(path, FileAccess.WRITE)
    file.store_string("Dimensions: L=%s H=4.0 W=3.0 Cx=0.5\n" % length)
    file.close()


func _trainset_context(offset:float) -> MaszynaImporterContext:
    var context := MaszynaImporterContext.new()
    context.trainset_open = true
    context.trainset_track = "start"
    context.trainset_offset = offset
    context.trainset_velocity = 0.0
    return context


func _import(context:MaszynaImporterContext, text:String) -> DynamicRailVehicle3D:
    var parser := MaszynaParser.new()
    parser.initialize(text.to_utf8_buffer(), [])
    var vehicle:DynamicRailVehicle3D = importer.import(parser, context)
    _vehicles.append(vehicle)
    return vehicle


func test_offset_minus_one_sentinel_imports_as_reversed() -> void:
    var vehicle:DynamicRailVehicle3D = _import(
            _trainset_context(20.0), "fixtures skin short -1.0 headdriver 99 0 enddynamic")
    assert_eq(vehicle.start_direction, TrackManager.Direction.DIRECTION_REVERSED)
    assert_almost_eq(vehicle.start_track_offset, 20.0 - SHORT_LENGTH * 0.5, 0.001)


func test_normal_offset_imports_as_normal_direction() -> void:
    var vehicle:DynamicRailVehicle3D = _import(
            _trainset_context(20.0), "fixtures skin short 0 headdriver 99 0 enddynamic")
    assert_eq(vehicle.start_direction, TrackManager.Direction.DIRECTION_NORMAL)
    assert_almost_eq(vehicle.start_track_offset, 20.0 - SHORT_LENGTH * 0.5, 0.001)


func test_trainset_vehicles_of_different_length_touch_without_overlap() -> void:
    var context:MaszynaImporterContext = _trainset_context(20.0)
    var first:DynamicRailVehicle3D = _import(context, "fixtures skin short 0 headdriver 3 0 enddynamic")
    var second:DynamicRailVehicle3D = _import(context, "fixtures skin long 0 nobody 3 0 enddynamic")
    var third:DynamicRailVehicle3D = _import(context, "fixtures skin short 0 nobody 3 0 enddynamic")

    assert_almost_eq(first.start_track_offset, 20.0 - SHORT_LENGTH * 0.5, 0.001)
    assert_almost_eq(
            first.start_track_offset - second.start_track_offset, (SHORT_LENGTH + LONG_LENGTH) * 0.5, 0.001,
            "centers of neighbouring vehicles must be half their lengths apart")
    assert_almost_eq(
            second.start_track_offset - third.start_track_offset, (LONG_LENGTH + SHORT_LENGTH) * 0.5, 0.001,
            "centers of neighbouring vehicles must be half their lengths apart")
    assert_almost_eq(context.trainset_offset, 20.0 - SHORT_LENGTH - LONG_LENGTH - SHORT_LENGTH, 0.001)


## DynObj.cpp:1812-1825 - the driver type picks the occupied cab.
func test_driver_type_selects_occupied_cab() -> void:
    var context:MaszynaImporterContext = _trainset_context(20.0)
    var head:DynamicRailVehicle3D = _import(context, "fixtures skin short 0 headdriver 3 0 enddynamic")
    var rear:DynamicRailVehicle3D = _import(context, "fixtures skin short 0 reardriver 3 0 enddynamic")
    var nobody:DynamicRailVehicle3D = _import(context, "fixtures skin short 0 nobody 3 0 enddynamic")

    assert_eq(head.cabin_number, 1)
    assert_eq(rear.cabin_number, -1)
    assert_eq(nobody.cabin_number, 0)
