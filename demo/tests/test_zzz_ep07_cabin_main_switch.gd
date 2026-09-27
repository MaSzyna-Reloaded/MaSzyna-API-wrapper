extends MaszynaGutTest

## Cabin layer (#94) on the REAL EP07-424 from td.scn with the player in the cab: cabin controls go
## through CabinSystem, whose callbacks are registered by LegacyCabinLogic. The line breaker
## closes only after main_on_bt is held for InitialCtrlDelay and then released (Train.cpp:2976-3070,
## 6779-6827), while the vehicle-level "main_switch" command still acts immediately and returns its
## result (#43).

var scenery:MaszynaSceneryNode
var player:MaszynaPlayer
var controller:VehicleController
var train_id:String
var vehicle_rid:RID


func before_each():
    scenery = MaszynaSceneryNode.new()
    scenery.filename = "td.scn"
    add_child(scenery)
    for i in range(20):
        if scenery.get_child_count() > 0:
            break
        await wait_seconds(0.5)
    for i in range(10):
        var dynamic_vehicle:Node = scenery.find_child("EP07-424", true, false)
        var rail_vehicle:RailVehicle3D = (
                dynamic_vehicle.find_child("RailVehicle3D", false, false) as RailVehicle3D if dynamic_vehicle else null)
        controller = rail_vehicle.get_controller() if rail_vehicle else null
        if controller:
            break
        await wait_seconds(0.5)
    train_id = controller.train_id if controller else ""
    vehicle_rid = controller.get_rid() if controller else RID()
    player = load("res://addons/libmaszyna/player/player.tscn").instantiate()
    player.start_train_id = train_id
    add_child(player)
    await wait_idle_frames(10)


func after_each():
    player.free()
    scenery.free()


func _cab() -> int:
    return int(controller.state.get("cabin_occupied", 1))


func _power_up() -> void:
    RailVehicleServer.vehicle_send_command(vehicle_rid, "battery", true)
    await wait_idle_frames(2)
    RailVehicleServer.vehicle_send_command(vehicle_rid, "security_acknowledge", true)
    RailVehicleServer.vehicle_send_command(vehicle_rid, "security_acknowledge", false)
    RailVehicleServer.vehicle_send_command(vehicle_rid, "pantograph", VehicleElectricEngine.PANTOGRAPH_FIRST, true)
    for i in range(20):
        await wait_seconds(0.5)
        if controller.state.get("current_collector/pantograph_first_voltage", 0.0) > 100.0:
            break
    # the ground relay only resets with a direction set (Mover.cpp:6038-6051)
    RailVehicleServer.vehicle_send_command(vehicle_rid, "direction_increase")
    RailVehicleServer.vehicle_send_command(vehicle_rid, "converter_fuse_reset")
    RailVehicleServer.vehicle_send_command(vehicle_rid, "fuse_reset")
    await wait_idle_frames(2)


func test_main_switch_needs_hold_and_release() -> void:
    assert_not_null(controller, "EP07-424 should exist")
    if not controller:
        return
    await _power_up()
    assert_true(controller.state.get("main_switch_closable", false), "main switch should be closable once powered")

    CabinSystem.act(vehicle_rid, _cab(), &"main_on_bt", &"hold")
    await wait_seconds(0.2)
    CabinSystem.act(vehicle_rid, _cab(), &"main_on_bt", &"release")
    await wait_idle_frames(2)
    assert_false(controller.state.get("main_switch_enabled", true), "a short press must not close the breaker")

    CabinSystem.act(vehicle_rid, _cab(), &"main_on_bt", &"hold")
    await wait_seconds(0.8)
    assert_false(controller.state.get("main_switch_enabled", true), "the breaker closes on release, not while held")
    CabinSystem.act(vehicle_rid, _cab(), &"main_on_bt", &"release")
    await wait_idle_frames(2)
    assert_true(controller.state.get("main_switch_enabled", false), "hold >= IniCDelay and release closes it")

    CabinSystem.act(vehicle_rid, _cab(), &"main_off_bt", &"hold")
    CabinSystem.act(vehicle_rid, _cab(), &"main_off_bt", &"release")
    await wait_idle_frames(2)
    assert_false(controller.state.get("main_switch_enabled", true), "main_off_bt opens the breaker")


func test_main_switch_hold_without_power_does_nothing() -> void:
    assert_not_null(controller, "EP07-424 should exist")
    if not controller:
        return
    CabinSystem.act(vehicle_rid, _cab(), &"main_on_bt", &"hold")
    await wait_seconds(0.8)
    CabinSystem.act(vehicle_rid, _cab(), &"main_on_bt", &"release")
    await wait_idle_frames(2)
    assert_false(controller.state.get("main_switch_enabled", true), "no power - the hold timer must not run")


func test_cabin_controls_are_registered_and_forwarded() -> void:
    assert_not_null(controller, "EP07-424 should exist")
    if not controller:
        return
    var cab:int = _cab()
    assert_true(CabinSystem.has_control(vehicle_rid, cab, &"battery_sw"), "battery_sw should be registered")
    var controls:Array = CabinSystem.get_controls(vehicle_rid, cab)
    assert_has(controls, &"battery_sw")
    assert_has(controls, &"main_on_bt")
    # the console "cabin <train> controls" listing formats these arrays
    assert_string_contains("\n".join(controls), "main_on_bt")
    assert_string_contains(", ".join(CabinSystem.ACTIONS), "hold")
    CabinSystem.act(vehicle_rid, cab, &"battery_sw", &"toggle", true)
    await wait_idle_frames(2)
    assert_true(controller.state.get("battery_enabled", false), "battery_sw through CabinSystem switches the battery")
    assert_eq(CabinSystem.get_control(vehicle_rid, cab, &"battery_sw"), true)
    assert_null(CabinSystem.act(vehicle_rid, cab, &"no_such_control", &"hold"), "unknown control returns null")

    player.start_train_id = ""
    await wait_idle_frames(5)
    # the cab logic is the vehicle's: a crewed vehicle keeps it for its driver (SceneryInstancer._build_drivers())
    var crewed:bool = DriverSystem.vehicle_get_driver(vehicle_rid).is_valid()
    assert_eq(CabinSystem.has_control(vehicle_rid, cab, &"battery_sw"), crewed,
            "leaving the cab unregisters the callbacks, unless a driver is aboard")
    assert_eq(CabinSystem.get_control(vehicle_rid, cab, &"battery_sw"), true, "cabin state survives leaving the cab")


## #43 - the vehicle-level command returns whether it was accepted.
func test_send_command_returns_result() -> void:
    assert_not_null(controller, "EP07-424 should exist")
    if not controller:
        return
    await _power_up()
    assert_true(RailVehicleServer.vehicle_send_command(vehicle_rid, "main_switch", true), "closing a closable breaker returns true")
    assert_false(RailVehicleServer.vehicle_send_command(vehicle_rid, "main_switch", true), "closing it again changes nothing")
    assert_false(RailVehicleServer.vehicle_get_rid_by_name("no_such_train").is_valid(), "an unknown name reaches no vehicle")


## Console "cabin <train> toggle battery_sw" - no value flips the control, and the cabin widget
## shows the new position without reporting it back.
func test_toggle_without_value_flips_control_and_widget() -> void:
    assert_not_null(controller, "EP07-424 should exist")
    if not controller:
        return
    var cab:int = _cab()
    var widget:CabinButton = null
    for node:Node in player.get_camera().get_parent().find_children("*", "CabinButton", true, false):
        if node.control_id == &"battery_sw":
            widget = node
    assert_not_null(widget, "EP07 cab should have a battery_sw widget")
    if not widget:
        return

    CabinSystem.act(vehicle_rid, cab, &"battery_sw", &"toggle")
    await wait_idle_frames(2)
    assert_true(controller.state.get("battery_enabled", false), "toggle without value switches the battery on")
    assert_true(widget.pushed, "the widget follows the cabin state")

    CabinSystem.act(vehicle_rid, cab, &"battery_sw", &"toggle")
    await wait_idle_frames(2)
    assert_false(controller.state.get("battery_enabled", true), "second toggle switches it off")
    assert_false(widget.pushed, "the widget follows the cabin state")
