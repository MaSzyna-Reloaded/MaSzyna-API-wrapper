extends MaszynaGutTest

## The geometry half of the pantograph: which span is overhead, and how far the collector reaches.
## Built from bare wires rather than from a scenery, so it says nothing about voltage - only about
## where contact is kept and where it is lost.
##
## Both cases here were real losses of contact reported on zwierzyniec_tlk: an EP08 dropping the
## line voltage a few times per run while crossing wire junctions. In that scenery 126 span ends
## have three neighbours each - four spans meeting over a switch - and only two ends anywhere have
## a gap wider than the joining tolerance, so the spans are not the problem; reaching the next one
## is.

## Half of a 1.4 m slider, which is what every electric locomotive of the datapack declares
## (CSW=1.4 in 134 of 135 .fiz files).
const SLIDER_HALF_WIDTH:float = 0.7
## DynObj.cpp:93 fWidthExtra - the guide horn beyond the slider.
const HORN_WIDTH:float = 0.381
const WIRE_HEIGHT:float = 5.8
const SPAN_LENGTH:float = 20.0

const UP:Vector3 = Vector3(0.0, 1.0, 0.0)
const FORWARD:Vector3 = Vector3(0.0, 0.0, 1.0)
const LEFT:Vector3 = Vector3(1.0, 0.0, 0.0)

var _wires:Array[RID] = []


func after_each() -> void:
    for wire:RID in _wires:
        TractionPowerServer.wire_free(wire)
    _wires.clear()


func _add_wire(from:Vector3, to:Vector3) -> RID:
    var wire:RID = TractionPowerServer.wire_create()
    _wires.append(wire)
    TractionPowerServer.wire_set_params(wire, from, to, "test_power", 3000.0, 2000.0, 0.01)
    return wire


## Running off the end of a span is not a loss of contact - the next span is reached along the
## chain, in the same frame, the way vehicle_table::update_traction() does (DynObj.cpp:8742).
func test_a_pantograph_crossing_a_span_junction_keeps_contact() -> void:
    var first:RID = _add_wire(
            Vector3(0.0, WIRE_HEIGHT, 0.0), Vector3(0.0, WIRE_HEIGHT, SPAN_LENGTH))
    var second:RID = _add_wire(
            Vector3(0.0, WIRE_HEIGHT, SPAN_LENGTH), Vector3(0.0, WIRE_HEIGHT, 2.0 * SPAN_LENGTH))
    TractionPowerServer.network_build()

    var held:RID = first
    var lost_at:PackedFloat32Array = PackedFloat32Array()
    var offset:float = 1.0
    while offset <= 2.0 * SPAN_LENGTH - 1.0:
        var found:Dictionary = TractionPowerServer.wire_follow_above(
                held, Vector3(0.0, 0.0, offset), UP, FORWARD, LEFT, SLIDER_HALF_WIDTH, HORN_WIDTH)
        var wire:RID = found["rid"]
        if wire.is_valid():
            held = wire
        else:
            lost_at.append(offset)
        offset += 0.5

    assert_eq(lost_at.size(), 0, "contact should never be lost while crossing the junction")
    assert_eq(held, second, "and the pantograph should end up on the second span")


## What following the chain buys over searching the area every frame: the pantograph stays on the
## wire it is under. The original only looks around once its own span stops being usable
## (DynObj.cpp:8796), so a lower wire crossing overhead - which over a switch is the diverging
## span - does not steal the contact.
func test_a_lower_crossing_wire_does_not_steal_the_contact() -> void:
    var first:RID = _add_wire(
            Vector3(0.0, WIRE_HEIGHT, 0.0), Vector3(0.0, WIRE_HEIGHT, SPAN_LENGTH))
    _add_wire(
            Vector3(0.0, WIRE_HEIGHT - 0.2, 0.0), Vector3(0.0, WIRE_HEIGHT - 0.2, SPAN_LENGTH))
    TractionPowerServer.network_build()

    var found:Dictionary = TractionPowerServer.wire_follow_above(
            first, Vector3(0.0, 0.0, 10.0), UP, FORWARD, LEFT, SLIDER_HALF_WIDTH, HORN_WIDTH)
    assert_eq(found["rid"], first, "the span being followed keeps the contact")
    assert_almost_eq(float(found["height"]), WIRE_HEIGHT, 0.001, "at its own height")


## The horn catches a wire the slider alone would miss, and reports it as higher than a wire
## straight overhead, so a span properly above still wins (scene.cpp:105-112).
func test_a_wire_over_the_horn_is_still_caught_and_reads_higher() -> void:
    var overhang:float = 0.5 * HORN_WIDTH
    var wire:RID = _add_wire(
            Vector3(SLIDER_HALF_WIDTH + overhang, WIRE_HEIGHT, 0.0),
            Vector3(SLIDER_HALF_WIDTH + overhang, WIRE_HEIGHT, SPAN_LENGTH))
    TractionPowerServer.network_build()

    var found:Dictionary = TractionPowerServer.wire_find_above_with_height(
            Vector3(0.0, 0.0, 10.0), UP, FORWARD, LEFT, SLIDER_HALF_WIDTH, HORN_WIDTH)
    assert_eq(found["rid"], wire, "a wire on the horn is in reach")
    assert_gt(float(found["height"]), WIRE_HEIGHT, "and counts as higher than one on the slider")


func test_a_wire_beyond_the_horn_is_out_of_reach() -> void:
    _add_wire(
            Vector3(SLIDER_HALF_WIDTH + HORN_WIDTH + 0.1, WIRE_HEIGHT, 0.0),
            Vector3(SLIDER_HALF_WIDTH + HORN_WIDTH + 0.1, WIRE_HEIGHT, SPAN_LENGTH))
    TractionPowerServer.network_build()

    var found:Dictionary = TractionPowerServer.wire_find_above_with_height(
            Vector3(0.0, 0.0, 10.0), UP, FORWARD, LEFT, SLIDER_HALF_WIDTH, HORN_WIDTH)
    assert_false(RID(found["rid"]).is_valid(), "past the horn there is nothing to collect from")
