extends MaszynaGutTest

## Placement of .scn `dynamic` nodes, ported from deserialize_dynamic()
## (simulationstateserializer.cpp:960-988) and TDynamicObject::Init() (DynObj.cpp):
## - offset == -1.0 means "reversed in the trainset" (`Init(..., (offset == -1.0), ...)`,
##   DynObj.cpp:1807).
## - inside a trainset the vehicle stands where the trainset puts it: the trainset takes the
##   track and the offset of `trainset:`, and the vehicle's `offset` as its gap (TrainSet3D).
## - outside one the distance marks the vehicle's FRONT; its center sits half its Dimensions L=
##   behind it (DynObj.cpp:2308, `fDist -= 0.5 * Dim.L`).

const TEST_GAME_DIR:String = "user://gut/dynamic_importer_fixture"
const SHORT_LENGTH:float = 10.0
const LONG_LENGTH:float = 16.0

var importer:RefCounted
var _previous_game_dir:String
var _vehicles:Array[MaszynaRailVehicle3D] = []
var _trainsets:Array[TrainSet3D] = []


func before_each() -> void:
    importer = load("res://addons/libmaszyna/legacy/scenery/maszyna_node_dynamic_importer.gd").new()
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    var fixture_dir:String = TEST_GAME_DIR.path_join("dynamic/fixtures")
    DirAccess.make_dir_recursive_absolute(fixture_dir)
    _write_fiz(fixture_dir.path_join("short.fiz"), SHORT_LENGTH)
    _write_fiz(fixture_dir.path_join("long.fiz"), LONG_LENGTH)
    UserSettings.save_maszyna_game_dir(TEST_GAME_DIR)


func after_each() -> void:
    for vehicle:MaszynaRailVehicle3D in _vehicles:
        vehicle.free()
    _vehicles.clear()
    for trainset:TrainSet3D in _trainsets:
        trainset.free()
    _trainsets.clear()
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func _write_fiz(path:String, length:float) -> void:
    var file:FileAccess = FileAccess.open(path, FileAccess.WRITE)
    file.store_string("Dimensions: L=%s H=4.0 W=3.0 Cx=0.5\n" % length)
    file.close()


func _trainset_context(offset:float) -> MaszynaImporterContext:
    var context := MaszynaImporterContext.new()
    var parser := MaszynaParser.new()
    parser.initialize(("trainset start %s 0" % offset).to_utf8_buffer(), [])
    var trainset_importer:RefCounted = load("res://addons/libmaszyna/legacy/scenery/maszyna_trainset_importer.gd").new()
    _trainsets.append_array(trainset_importer.import(parser, context))
    return context


func _import(context:MaszynaImporterContext, text:String) -> MaszynaRailVehicle3D:
    var parser := MaszynaParser.new()
    parser.initialize(text.to_utf8_buffer(), [])
    var vehicle:MaszynaRailVehicle3D = importer.import(parser, context)
    _vehicles.append(vehicle)
    return vehicle


func test_offset_minus_one_sentinel_imports_as_reversed() -> void:
    var context:MaszynaImporterContext = _trainset_context(20.0)
    var vehicle:MaszynaRailVehicle3D = _import(context, "fixtures skin short -1.0 headdriver 99 0 enddynamic")
    assert_eq(vehicle.start_direction, TrackServer.DIRECTION_REVERSED)
    assert_eq(context.trainset_node.vehicle_gaps, [0.0] as Array[float], "a reversed vehicle stands right behind")


func test_normal_offset_imports_as_normal_direction() -> void:
    var context:MaszynaImporterContext = _trainset_context(20.0)
    var vehicle:MaszynaRailVehicle3D = _import(context, "fixtures skin short 2.5 headdriver 99 0 enddynamic")
    assert_eq(vehicle.start_direction, TrackServer.DIRECTION_NORMAL)
    assert_eq(context.trainset_node.vehicle_gaps, [2.5] as Array[float], "its offset is its gap in the trainset")


func test_the_trainset_places_its_vehicles_not_the_vehicles_themselves() -> void:
    var context:MaszynaImporterContext = _trainset_context(20.0)
    var first:MaszynaRailVehicle3D = _import(context, "fixtures skin short 0 headdriver 3 0 enddynamic")
    var second:MaszynaRailVehicle3D = _import(context, "fixtures skin long 0 nobody 3 0 enddynamic")

    var trainset:TrainSet3D = context.trainset_node
    assert_eq(trainset.start_track_name, "start")
    assert_almost_eq(trainset.start_track_offset, 20.0, 0.001, "the trainset's front is the offset of trainset:")
    assert_eq(first.start_track_name, "", "a vehicle of a trainset has no start track of its own")
    assert_eq(second.start_track_name, "")
    assert_eq(trainset.couplings, [3, 3] as Array[int], "the couplingdata of every vehicle")


func test_a_vehicle_outside_a_trainset_stands_with_its_center_behind_its_front() -> void:
    var context := MaszynaImporterContext.new()
    var vehicle:MaszynaRailVehicle3D = _import(context, "fixtures skin short start -20.0 headdriver 0 0 enddynamic")
    assert_eq(vehicle.start_track_name, "start")
    assert_almost_eq(vehicle.start_track_offset, 20.0 - SHORT_LENGTH * 0.5, 0.001)


## DynObj.cpp:1812-1825 - the driver type picks the occupied cab.
func test_driver_type_selects_occupied_cab() -> void:
    var context:MaszynaImporterContext = _trainset_context(20.0)
    var head:MaszynaRailVehicle3D = _import(context, "fixtures skin short 0 headdriver 3 0 enddynamic")
    var rear:MaszynaRailVehicle3D = _import(context, "fixtures skin short 0 reardriver 3 0 enddynamic")
    var nobody:MaszynaRailVehicle3D = _import(context, "fixtures skin short 0 nobody 3 0 enddynamic")

    assert_eq(head.driver_type, VehicleController.DRIVER_HEAD)
    assert_eq(rear.driver_type, VehicleController.DRIVER_REAR)
    assert_eq(nobody.driver_type, VehicleController.DRIVER_NOBODY)


## What the scenery loaded the vehicle with. The count comes first and the cargo's name only
## follows it when the count is not zero; a count with no name behind it is not a load at all
## (simulationstateserializer.cpp:1031), which is how a `dynamic` ending right there reads.
func test_the_load_a_dynamic_declares_reaches_the_vehicle() -> void:
    var context:MaszynaImporterContext = _trainset_context(20.0)
    var loaded:MaszynaRailVehicle3D = _import(
            context, "fixtures skin short 0 nobody 3 24 coal enddynamic")
    var empty:MaszynaRailVehicle3D = _import(context, "fixtures skin short 0 nobody 3 0 enddynamic")
    var unnamed:MaszynaRailVehicle3D = _import(context, "fixtures skin short 0 nobody 3 24 enddynamic")

    assert_eq(loaded.load_name, "coal", "the cargo is named as the scenery names it")
    assert_almost_eq(loaded.load_amount, 24.0, 0.001, "and carried in the amount it declares")
    assert_eq(empty.load_name, "", "a count of zero carries nothing")
    assert_almost_eq(empty.load_amount, 0.0, 0.001)
    assert_eq(unnamed.load_name, "", "a count with no cargo named behind it is not a load")
    assert_almost_eq(unnamed.load_amount, 0.0, 0.001)
