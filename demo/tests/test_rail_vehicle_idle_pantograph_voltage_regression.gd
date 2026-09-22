extends MaszynaGutTest

## Regression test for the reported "na postoju elektrowozom flapuje napiecie z drutow i nie da
## sie uruchomic, bo wywala wylacznik szybki" bug. Drives a real RailVehicle3D + VehicleController
## + VehicleElectricSeriesEngine + a real overhead wire/power source (TractionPowerServer) through
## Godot's actual per-frame _process(), exactly like an electric locomotive sitting at a
## platform with its pantograph raised and main switch closed.
##
## This test passes as written and does NOT reproduce the symptom - this fixture's vehicle has
## no front/rear bogie nodes configured, so it never triggers
## RailVehicle3D::_update_track_transform()'s bogie-refined orientation branch, which is where
## test_rail_vehicle_idle_orientation_regression.gd found and fixed a real, confirmed sign bug
## (front/rear track-offset samples were swapped, flipping the whole vehicle 180 degrees the
## instant it moved - see that test's header comment and the RailVehicle3D.cpp fix). The
## pantograph/main-switch symptom is suspected to be downstream of that same flip (a flipped
## global_transform feeds _pantograph_frame_axes(), which _update_pantograph_power() uses to
## search TractionPowerServer for the overhead wire - a wrong axis could lose wire contact), but
## that has NOT been proven - this fixture would need bogies added, and the vehicle would need
## to actually move, to test that theory directly. Kept as a green regression test for the
## "stays parked, never touched, voltage/main switch should stay stable" case on its own merits.

var created_tracks:Array[RID] = []
var created_wires:Array[RID] = []
var created_power_sources:Array[RID] = []
var vehicle:RailVehicle3D
var controller:VehicleController
var engine:VehicleElectricSeriesEngine


func after_each() -> void:
    if is_instance_valid(vehicle):
        if vehicle.get_parent():
            vehicle.get_parent().remove_child(vehicle)
        vehicle.queue_free()
    vehicle = null
    if is_instance_valid(controller):
        if controller.get_parent():
            controller.get_parent().remove_child(controller)
        controller.queue_free()
    controller = null
    engine = null

    for wire_rid:RID in created_wires:
        TractionPowerServer.wire_free(wire_rid)
    created_wires.clear()
    for source_rid:RID in created_power_sources:
        TractionPowerServer.power_source_free(source_rid)
    created_power_sources.clear()

    for track_rid:RID in created_tracks:
        if TrackManager.track_exists(track_rid):
            TrackManager.track_free(track_rid)
    created_tracks.clear()
    TrackManager.topology_rebuild()


func test_parked_electric_locomotive_keeps_stable_wire_voltage_and_main_switch_closed() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 60.0)),
        null,
        TrackManager.TRACK_NORMAL,
        "start",
    )
    TrackManager.topology_rebuild()

    var source_rid:RID = TractionPowerServer.power_source_create()
    created_power_sources.append(source_rid)
    TractionPowerServer.power_source_set_params(
        source_rid, "test_power", 3000.0, 0.0, 0.2, 2000.0, 1.0, 3, 60.0, false)
    var wire_rid:RID = TractionPowerServer.wire_create()
    created_wires.append(wire_rid)
    TractionPowerServer.wire_set_params(
        wire_rid, Vector3(0.0, 5.5, -50.0), Vector3(0.0, 5.5, 100.0), "test_power", 3000.0, 2000.0, 0.01)
    TractionPowerServer.network_build()

    controller = VehicleController.new()
    controller.train_id = "test_idle_pantograph_train"
    controller.type_name = "test"
    controller.battery_voltage = 110.0
    add_child(controller)

    engine = MoverVehicleElectricSeriesEngine.new()
    engine.power_source = VehicleController.POWER_SOURCE_CURRENTCOLLECTOR
    engine.power_current_collector_physical_layout = 1
    engine.power_current_collector_max_voltage = 3600.0
    engine.power_current_collector_number_of_collectors = 1
    engine.cntrl_main_controller_position_count = 6
    controller.add_child(engine)

    vehicle = RailVehicle3D.new()
    vehicle.start_track_name = "start"
    vehicle.start_track_offset = 20.0
    vehicle.start_direction = TrackManager.DIRECTION_NORMAL
    vehicle.pantograph_collector_width = 0.5
    add_child(vehicle)
    vehicle.controller_path = vehicle.get_path_to(controller)
    await wait_idle_frames(2)

    controller.send_command("battery", true)
    controller.send_command("pantograph", VehicleElectricEngine.PANTOGRAPH_FIRST, true)
    var voltage_reached:bool = false
    for i in range(60):
        await wait_idle_frames(1)
        if float(controller.state.get("current_collector/pantograph_first_voltage", 0.0)) > 100.0:
            voltage_reached = true
            break
    assert_true(
        voltage_reached,
        "pantograph should read a real wire voltage from the overhead wire once raised, got %s" % [
                controller.state.get("current_collector/pantograph_first_voltage", 0.0)],
    )

    controller.send_command("direction_increase")
    controller.send_command("converter_fuse_reset")
    controller.send_command("fuse_reset")
    print("[before main_switch] %s" % [_dump(controller)])
    controller.send_command("main_switch", true)
    await wait_idle_frames(2)
    print("[after main_switch] %s" % [_dump(controller)])
    assert_true(
        bool(controller.state.get("main_switch_enabled", false)),
        "main switch should close once parked under a healthy, stationary overhead wire",
    )
    if not bool(controller.state.get("main_switch_enabled", false)):
        return

    # Now just sit parked for several real seconds - exactly the reported "postoj" scenario, no
    # notch advance, no driving - and make sure neither the wire voltage nor the main switch
    # ever drops out on their own.
    var min_voltage:float = INF
    var tripped:bool = false
    for i in range(300): # ~5s at 60fps
        await wait_idle_frames(1)
        var voltage:float = float(controller.state.get("current_collector/pantograph_first_voltage", 0.0))
        min_voltage = minf(min_voltage, voltage)
        if not bool(controller.state.get("main_switch_enabled", false)):
            tripped = true
            print(
                "main switch tripped at frame %d, voltage=%s"
                % [i, controller.state.get("current_collector/pantograph_first_voltage", 0.0)]
            )
            break

    assert_false(tripped, "main switch should not self-trip while the vehicle sits parked under the wire")
    assert_true(
        min_voltage > 100.0,
        "pantograph wire voltage should not flap/drop out while the vehicle sits parked, got min=%s" % min_voltage,
    )


func _dump(controller:VehicleController) -> String:
    var keys:Array[String] = [
        "main_switch_enabled", "relay_novolt", "relay_overvoltage", "relay_ground",
        "current_collector/pantograph_first_active", "current_collector/pantograph_first_voltage",
        "current_collector/voltage", "converter_enabled", "fuse_active", "converter_overload",
        "train_damage", "battery_voltage", "direction",
    ]
    var line:String = ""
    for key in keys:
        line += "%s=%s " % [key, controller.state.get(key, null)]
    return line


func _register_track(
    curve1:MaszynaTrackCurve,
    curve2:MaszynaTrackCurve = null,
    type:int = TrackManager.TRACK_NORMAL,
    name:String = "",
) -> RID:
    var track_rid:RID = TrackManager.track_create()
    created_tracks.append(track_rid)
    TrackManager.track_update_curves(track_rid, curve1, curve2)
    TrackManager.track_update(track_rid, type, name, 1.435)
    return track_rid


func _curve(p1:Vector3, p2:Vector3, roll1:float = 0.0, roll2:float = 0.0) -> MaszynaTrackCurve:
    var curve:MaszynaTrackCurve = MaszynaTrackCurve.new()
    curve.p1 = p1
    curve.p2 = p2
    curve.roll1 = roll1
    curve.roll2 = roll2
    return curve
