extends MaszynaGutTest

## End-to-end smoke test for the pantograph power layer, using the actual td.scn scenery and its
## real EP07-424 vehicle (node "EP07-424" in td.scn, dynamic/pkp/303e_v1) - not a synthetic
## VehicleController like test_traction_power_pantograph.gd. Named test_zzz_* (like the existing
## test_zzz_scenery_scene_smoke.gd/test_zzz_trainset_diagnostic.gd) so it runs last: it's slow
## (loads the whole scenery) and exists specifically to catch breaks in the RailVehicle3D
## geometry/wire-lookup path that a synthetic-controller test can't reach.

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


func _find_train_controller(root:Node, vehicle_name:String) -> VehicleController:
    var dynamic_vehicle:Node = root.find_child(vehicle_name, true, false)
    if not dynamic_vehicle:
        return null
    var rail_vehicle:RailVehicle3D = dynamic_vehicle.find_child("RailVehicle3D", false, false) as RailVehicle3D
    if not rail_vehicle:
        return null
    return rail_vehicle.get_controller()


func test_ep07_pantograph_draws_wire_voltage_from_td_scn() -> void:
    var controller:VehicleController = null
    for i in range(10):
        controller = _find_train_controller(scenery, "EP07-424")
        if controller:
            break
        await wait_seconds(0.5)
    assert_not_null(controller, "EP07-424 should exist somewhere under the loaded scenery")
    if not controller:
        return

    # Same startup sequence a real driver uses on a cold EP07: battery, then the (main) compressor
    # to actually build air pressure - PantPress mirrors ScndPipePress (Mover.cpp's
    # UpdatePantVolume, bPantKurek3 branch), so without this the reservoir never fills and the
    # pantograph raise simulation added in RailVehicle3D._update_pantograph_raise_state() would
    # never see enough pressure to move at all, regardless of "pantograph" being sent. No separate
    # master pantograph-valve command - this vehicle's cabin has no such switch (confirmed against
    # its .mmd) and nothing in this wrapper sends one via keybind either. If this ever needs a
    # fourth command again, that's a real regression, not a missing test setup step - see
    # VehicleElectricEngine::pantograph()'s own comment for why it's otherwise self-contained.
    controller.send_command("battery", true)
    await wait_seconds(0.5)
    controller.send_command("compressor", true)
    await wait_seconds(0.5)
    controller.send_command("pantograph", VehicleElectricEngine.PANTOGRAPH_FIRST, true)

    var voltage:float = 0.0
    var active:bool = false
    for i in range(60):
        await wait_seconds(0.5)
        active = controller.state.get("current_collector/pantograph_first_active", false)
        voltage = controller.state.get("current_collector/pantograph_first_voltage", 0.0)
        if active and voltage > 100.0:
            break

    assert_true(active, "pantograph should report raised once battery+valves are on")
    assert_true(
            voltage > 100.0,
            "pantograph should read a real wire voltage once raised and placed over td.scn's electrified track, got %s" % voltage)
