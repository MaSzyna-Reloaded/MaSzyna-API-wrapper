extends MaszynaGutTest

## CabinSystem is the one place a cab talks to the vehicle servers from, and it hands the same
## vehicle dump to every element that asks within a frame - a cab is dozens of them. What that
## cache must not do is outlive a command: a control reports its manipulation and reads the result
## in the very same frame, so a dump built before the command would show the cab what it looked
## like a moment ago. It is the same trap the server's own dump cache had (see `FINDINGS.md`,
## 2026-09-23), one layer up.

var _controller:VehicleController = null


func before_each() -> void:
    _controller = build_vehicle("cabin_state_test")
    _controller.type_name = "test"
    CabinSystem.register_vehicle("cabin_state_test")
    await wait_idle_frames(2)


func after_each() -> void:
    _controller = null


func test_two_elements_asking_in_one_frame_are_given_the_same_dump() -> void:
    var first:Dictionary = CabinSystem.vehicle_state("cabin_state_test")
    var second:Dictionary = CabinSystem.vehicle_state("cabin_state_test")
    assert_eq(first, second, "the dump is built once a frame, not once an element")
    assert_true(first.has("velocity"), "and it is the real dump")


func test_a_command_shows_through_without_waiting_for_the_next_frame() -> void:
    var before:int = int(CabinSystem.vehicle_state("cabin_state_test").get("radio_channel", -1))
    _controller.send_command("radio_channel_set", before + 1)
    assert_eq(
            int(CabinSystem.vehicle_state("cabin_state_test").get("radio_channel", -1)),
            before + 1,
            "a control reads the result of its own manipulation in the frame it made it")


func test_a_vehicle_that_left_is_not_still_being_described() -> void:
    assert_true(CabinSystem.vehicle_state("cabin_state_test").has("velocity"))
    CabinSystem._on_train_unregistered("cabin_state_test")
    assert_eq(
            CabinSystem.vehicle_state("cabin_state_test").size(), 0,
            "the dump of a vehicle that is gone is not handed out for the rest of the frame")
