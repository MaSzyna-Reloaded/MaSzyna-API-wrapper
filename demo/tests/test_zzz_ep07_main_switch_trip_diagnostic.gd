extends MaszynaGutTest

## Regression test for the reported "wylacznik szybki wybija po paru sekundach na drugiej pozycji
## nastawnika" (main switch trips a couple seconds after the second controller notch, can't
## accelerate) bug - drives the real EP07-424 from td.scn (dynamic/pkp/303e_v1) through the
## operator's own in-game command sequence and asserts Mains stays closed and DamageFlag stays
## clear while advancing the controller.
##
## Root cause (confirmed via this test's own diagnostic dumps before the fix): FizTrainElectric
## SeriesEngineParser.apply_engine_fields() divided "nmax" by 60 in GDScript AND
## TrainElectricSeriesEngine::_do_update_internal_mover divided by 60 again in C++ - nmax ended up
## 3600x too small. Mover's own motor-overspeed damage check (Mover.cpp:446, FuzzyLogic(abs(enrot),
## nmax*1.11, p_elengproblem)) then had a chance to fire at a tiny fraction of a km/h instead of
## near the real max speed, latching DamageFlag's dtrain_engine bit almost immediately once any
## current flowed - which then keeps re-tripping Mains (Mover.cpp:440-443) for as long as the
## motor draws current, matching the reported symptom exactly. Fixed by not pre-dividing in
## GDScript (fiz_train_electric_series_engine_parser.gd) since the C++ layer already does the
## conversion, mirroring the original engine's own single LoadFIZ_Engine "nmax /= 60.0"
## (Mover.cpp:10884).
##
## Modeled on test_zzz_ep07_pantograph_power_smoke.gd's real-scenery pattern.

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


func _find_train_controller(root:Node, vehicle_name:String) -> TrainController:
    var dynamic_vehicle:Node = root.find_child(vehicle_name, true, false)
    if not dynamic_vehicle:
        return null
    var rail_vehicle:RailVehicle3D = dynamic_vehicle.find_child("RailVehicle3D", false, false) as RailVehicle3D
    if not rail_vehicle:
        return null
    return rail_vehicle.get_controller()


func _dump_diagnostic_state(controller:TrainController, label:String) -> void:
    var keys:Array[String] = [
        "main_switch_enabled", "relay_novolt", "relay_overvoltage", "relay_ground",
        "current_collector/voltage", "current_collector/pantograph_first_active",
        "current_collector/pantograph_first_voltage", "compressor_pressure",
        "current_collector/pantograph_tank_pressure",
        "current_collector/pantograph_pressure_switch_armed", "feed_pipe_pressure",
        "brake_air_pressure", "converter_enabled", "engine_current", "Im", "Mm", "Ft",
        "controller_main_position", "main_no_power_pos", "main_switch_time",
        "line_breaker_delay", "line_breaker_initial_delay", "converter_overload", "fuse_active",
        "train_damage", "engine_damage", "velocity", "speed", "engine_rpm_count",
        "circuit_imax", "circuit_nmax_rpm",
    ]
    var line:String = "[%s] " % label
    for key in keys:
        line += "%s=%s " % [key, controller.state.get(key, null)]
    print(line)


func test_ep07_main_switch_stays_closed_while_advancing_controller() -> void:
    var controller:TrainController = null
    for i in range(10):
        controller = _find_train_controller(scenery, "EP07-424")
        if controller:
            break
        await wait_seconds(0.5)
    assert_not_null(controller, "EP07-424 should exist somewhere under the loaded scenery")
    if not controller:
        return

    controller.send_command("security_acknowledge", true)
    controller.send_command("security_acknowledge", false)
    controller.send_command("brake_level_set", 0.25)
    controller.send_command("brake_releaser", true)
    controller.send_command("pantograph", TrainElectricEngine.PANTOGRAPH_FIRST, true)
    await wait_idle_frames(2)
    _dump_diagnostic_state(controller, "after pantograph")
    controller.send_command("direction_increase")
    await wait_idle_frames(2)
    _dump_diagnostic_state(controller, "after direction_increase")
    controller.send_command("converter_fuse_reset")
    controller.send_command("fuse_reset")
    await wait_idle_frames(2)
    _dump_diagnostic_state(controller, "after fuse resets")
    controller.send_command("main_switch", true)
    await wait_idle_frames(1)
    _dump_diagnostic_state(controller, "1 frame after main_switch true")
    await wait_idle_frames(4)
    _dump_diagnostic_state(controller, "5 frames after main_switch true")
    controller.send_command("converter", true)
    await wait_seconds(0.5)
    _dump_diagnostic_state(controller, "0.5s after converter true")
    await wait_seconds(4.5)
    _dump_diagnostic_state(controller, "5s after converter true")
    controller.send_command("compressor", true)
    await wait_seconds(0.5)
    _dump_diagnostic_state(controller, "0.5s after compressor true")
    await wait_seconds(4.5)

    _dump_diagnostic_state(controller, "before any notch")
    assert_true(
            controller.state.get("main_switch_enabled", false),
            "main switch should be closed before advancing the controller")

    var tripped:bool = false
    for notch in range(1, 6):
        controller.send_command("main_controller_increase")
        print("-- sent main_controller_increase #%d (controller_main_position now %s) --" % [
                notch, controller.state.get("controller_main_position", null)])
        # Poll every single idle frame (not just every 0.5s) so the exact frame Mains flips is
        # caught, instead of a coarser 0.5s snapshot that could miss a one-frame relay blip that
        # already self-recovered by the next sample.
        var prev_damage:int = controller.state.get("train_damage", 0)
        for i in range(180): # ~3s at 60fps
            var was_enabled:bool = controller.state.get("main_switch_enabled", false)
            await wait_idle_frames(1)
            var now_enabled:bool = controller.state.get("main_switch_enabled", false)
            var now_damage:int = controller.state.get("train_damage", 0)
            if now_damage != prev_damage:
                _dump_diagnostic_state(
                        controller, "DAMAGE CHANGE notch %d, frame %d (%d -> %d)" % [
                                notch, i, prev_damage, now_damage])
                prev_damage = now_damage
            if was_enabled and not now_enabled:
                _dump_diagnostic_state(controller, "TRIP FRAME notch %d, frame %d" % [notch, i])
                tripped = true
                break
        if tripped:
            break

    _dump_diagnostic_state(controller, "final (tripped=%s)" % tripped)
    assert_false(tripped, "main switch should not self-trip while advancing the controller")
    assert_eq(
            controller.state.get("train_damage", 0), 0,
            "no engine damage should latch from normal acceleration")
    assert_true(
            controller.state.get("velocity", 0.0) > 2.0,
            "vehicle should have accelerated past 2 m/s across 5 controller notches")
