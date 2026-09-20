extends MaszynaGutTest

## maszyna_node_track_importer.gd - the ".scn track node" reader.

const TrackImporter = preload("res://addons/libmaszyna/importer/maszyna_node_track_importer.gd")

## A "normal" track whose trackbed slot holds the format's "no texture" sentinel, followed by a
## marker token so the parser position after the node can be checked.
const NONE_TRACKBED_SOURCE:String = """
normal 6.092 1.435 0.15 25 25 0 flat vis
rail_screw_used1 4.55 none 0.6 0.9 0.9
-15992.16 0.603 -2902.416 0
0 0 0
0 0 0
-15997.885 0.63 -2900.332 0
0
velocity 80
endtrack
MARKER
"""
## A track type the wrapper does not build yet.
const ROAD_SOURCE:String = """
road 10.0 8.0 0.15 25 25 0 flat vis
road_asphalt 4.0 none 0.0 0.0 0.0
0 0 0 0
0 0 0
0 0 0
10 0 0 0
0
endtrack
MARKER
"""


func _import(source:String) -> Array:
    var parser:MaszynaParser = MaszynaParser.new()
    parser.initialize(source.to_utf8_buffer())
    var track:MaszynaTrackData = TrackImporter.new().import(parser, MaszynaImporterContext.new())
    return [track, parser]


func test_none_trackbed_is_not_a_material_name() -> void:
    var result:Array = _import(NONE_TRACKBED_SOURCE)
    var track:MaszynaTrackData = result[0]

    assert_not_null(track)
    assert_eq(track.material1, "rail_screw_used1")
    assert_eq(track.material2, "", "\"none\" is the format's sentinel, not a texture")


func test_unsupported_track_type_is_skipped_whole() -> void:
    var result:Array = _import(ROAD_SOURCE)
    var parser:MaszynaParser = result[1]

    assert_null(result[0])
    assert_eq(parser.next_token(), "MARKER", "the rejected node must be consumed up to endtrack")
