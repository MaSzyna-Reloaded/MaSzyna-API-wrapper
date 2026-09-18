extends MaszynaGutTest

## Regression test for a real porting gap found while investigating the reported
## "EZT ma odwrocone czlony" bug: the original engine's offset == -1.0 sentinel means "place
## this vehicle reversed in the consist", not just "use the trainset's own running offset"
## (simulationstateserializer.cpp:983's `vehicle->Init(..., ( offset == -1.0 ), params)`,
## DynObj.cpp:1807's `iDirection = (Reversed ? 0 : 1)`). maszyna_node_dynamic_importer.gd already
## checked is_equal_approx(offset, -1.0) for the offset-math branch, but never used it to set
## DynamicRailVehicle3D.start_direction - every imported vehicle silently became
## DIRECTION_NORMAL regardless of this sentinel.

var importer:RefCounted


func before_each() -> void:
    importer = load("res://addons/libmaszyna/importer/maszyna_node_dynamic_importer.gd").new()


func _parser_for(text:String) -> MaszynaParser:
    var parser := MaszynaParser.new()
    parser.initialize(text.to_utf8_buffer(), [])
    return parser


func test_offset_minus_one_sentinel_imports_as_reversed() -> void:
    var context := MaszynaImporterContext.new()
    context.trainset_open = true
    context.trainset_track = "start"
    context.trainset_offset = 20.0
    context.trainset_velocity = 0.0

    var parser:MaszynaParser = _parser_for(
        "pkp\\sm42_v1 6d-907 6da -1.0 headdriver 99 0 enddynamic"
    )
    var vehicle:DynamicRailVehicle3D = importer.import(parser, context)

    assert_not_null(vehicle)
    assert_eq(
        vehicle.start_direction, TrackManager.Direction.DIRECTION_REVERSED,
        "offset == -1.0 should place the vehicle reversed, matching the original engine",
    )
    assert_eq(vehicle.start_track_offset, 20.0, "reversed placement should use the trainset's own running offset")
    vehicle.free()


func test_normal_offset_imports_as_normal_direction() -> void:
    var context := MaszynaImporterContext.new()
    context.trainset_open = true
    context.trainset_track = "start"
    context.trainset_offset = 20.0
    context.trainset_velocity = 0.0

    var parser:MaszynaParser = _parser_for(
        "PKP\\303E_V1 303E-EP-TV-424-HIST 303E-EP-TV 0 headdriver 35.WH25 0 enddynamic"
    )
    var vehicle:DynamicRailVehicle3D = importer.import(parser, context)

    assert_not_null(vehicle)
    assert_eq(
        vehicle.start_direction, TrackManager.Direction.DIRECTION_NORMAL,
        "a real (non-sentinel) offset should keep the vehicle at DIRECTION_NORMAL",
    )
    assert_eq(vehicle.start_track_offset, 20.0, "a zero offset should not shift the trainset's own running offset")
    vehicle.free()
