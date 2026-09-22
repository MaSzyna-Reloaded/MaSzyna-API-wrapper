extends MaszynaGutTest

## Regression test for the reported "na postoju pojazdy sa odwrocone domyslnie, a jak ruszysz to
## sie odwraca / w koncu gasnie i nie drgnie" bug - drives the REAL EP07-424 from td.scn
## (dynamic/pkp/303e_v1), asserting its actual global orientation stays stable while parked and
## while being driven.
##
## Root cause (confirmed with this test before the fix, still printed as a running dump for
## future debugging): RailVehicle3D::_update_track_transform()'s bogie-refined orientation
## branch sampled "front" and "rear" track-offset distances swapped (RailVehicleServer's
## offset-distance sign convention is rear-relative, not what the naive +0.5/-0.5 sampling
## assumed), so body_forward pointed opposite the vehicle's real forward direction. That branch
## only runs once bogies are resolved *and* the vehicle has visibly moved - while parked the
## correct coarse transform is all that's applied, so the vehicle looked fine until it moved,
## at which point the very first recompute flipped it 180 degrees. Fixed in RailVehicle3D.cpp by
## swapping which distance sample is "front" vs "rear". See
## test_rail_vehicle_idle_orientation_regression.gd for the isolated, asset-free reproduction of
## the same bug (that one is faster to run and easier to read; keep this real-scenery test too,
## since a synthetic fixture can miss real-world specifics like async E3D bogie resolution).

var scenery:MaszynaSceneryNode


func before_each():
    scenery = MaszynaSceneryNode.new()
    scenery.filename = "td.scn"
    add_child(scenery)
    for i in range(20):
        if scenery.get_child_count() > 0:
            break
        await wait_seconds(0.5)


func after_each():
    scenery.free()


func _find_rail_vehicle(root:Node, vehicle_name:String) -> RailVehicle3D:
    var dynamic_vehicle:Node = root.find_child(vehicle_name, true, false)
    if not dynamic_vehicle:
        return null
    return dynamic_vehicle.find_child("RailVehicle3D", false, false) as RailVehicle3D


func _forward(vehicle:RailVehicle3D) -> Vector3:
    return -vehicle.global_basis.z.normalized()


func _dump_orientation(label:String, vehicle:RailVehicle3D, controller:VehicleController) -> void:
    print(
        "[%s] forward=%s pos=%s start_direction=%s velocity=%s main_switch_enabled=%s" % [
            label,
            _forward(vehicle),
            vehicle.global_position,
            vehicle.start_direction,
            controller.state.get("velocity", null),
            controller.state.get("main_switch_enabled", null),
        ]
    )


func test_ep07_orientation_stays_stable_while_parked_and_while_driving() -> void:
    var rail_vehicle:RailVehicle3D = null
    for i in range(10):
        rail_vehicle = _find_rail_vehicle(scenery, "EP07-424")
        if rail_vehicle:
            break
        await wait_seconds(0.5)
    assert_not_null(rail_vehicle, "EP07-424 should exist somewhere under the loaded scenery")
    if not rail_vehicle:
        return
    var controller:VehicleController = rail_vehicle.get_controller()
    assert_not_null(controller, "EP07-424's RailVehicle3D should have a controller")
    if not controller:
        return

    print(
        "placement: start_track_name=%s start_track_offset=%s start_direction=%s front_bogie_path=%s rear_bogie_path=%s" % [
            rail_vehicle.start_track_name,
            rail_vehicle.start_track_offset,
            rail_vehicle.start_direction,
            rail_vehicle.front_bogie_path,
            rail_vehicle.rear_bogie_path,
        ]
    )

    # Sit parked for a while right after spawn, exactly as a player would see before boarding.
    for i in range(10):
        await wait_idle_frames(1)
        _dump_orientation("parked frame %d" % i, rail_vehicle, controller)
    var forward_while_parked:Vector3 = _forward(rail_vehicle)
    # td.scn's trainset velocity is 0.0 - the original spawns such a vehicle cold
    # (Mover.cpp:8943 only turns Battery on for a vehicle ready to depart).
    assert_false(
        controller.state.get("battery_enabled", true),
        "EP07-424 on td.scn should spawn with its battery off, like in the original",
    )

    # Battery on arms the cab signal (Mover.cpp:131), so acknowledge only once it is powered.
    controller.send_command("battery", true)
    await wait_idle_frames(2)
    controller.send_command("security_acknowledge", true)
    controller.send_command("security_acknowledge", false)
    controller.send_command("brake_level_set", 0.25)
    controller.send_command("brake_releaser", true)
    controller.send_command("pantograph", VehicleElectricEngine.PANTOGRAPH_FIRST, true)
    for i in range(20):
        await wait_seconds(0.5)
        if controller.state.get("current_collector/pantograph_first_voltage", 0.0) > 100.0:
            break
    controller.send_command("direction_increase")
    controller.send_command("converter_fuse_reset")
    controller.send_command("fuse_reset")
    controller.send_command("main_switch", true)
    await wait_idle_frames(2)
    controller.send_command("converter", true)
    await wait_seconds(2.0)
    controller.send_command("compressor", true)
    await wait_seconds(2.0)
    _dump_orientation("just before first notch", rail_vehicle, controller)

    controller.send_command("main_controller_increase")
    var tripped:bool = false
    for i in range(120): # ~2s at 60fps
        await wait_idle_frames(1)
        if i % 10 == 0:
            _dump_orientation("driving frame %d" % i, rail_vehicle, controller)
        if not controller.state.get("main_switch_enabled", false):
            _dump_orientation("MAIN SWITCH DROPPED at frame %d" % i, rail_vehicle, controller)
            tripped = true
            break
        var forward_now:Vector3 = _forward(rail_vehicle)
        if forward_while_parked.distance_to(forward_now) > 0.1:
            _dump_orientation("ORIENTATION FLIPPED at frame %d" % i, rail_vehicle, controller)
            assert_true(
                false,
                "vehicle orientation flipped while driving: parked=%s now=%s" % [forward_while_parked, forward_now],
            )
            return

    _dump_orientation("final", rail_vehicle, controller)
    assert_false(tripped, "main switch should not self-trip while accelerating away from a stop")
    assert_true(
        float(controller.state.get("velocity", 0.0)) > 0.0,
        "vehicle should have actually started moving",
    )
