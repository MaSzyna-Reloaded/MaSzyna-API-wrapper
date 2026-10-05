extends MaszynaGutTest

## Cabin layer (#94) on the REAL EP07-424 from td.scn with the player in the cab: cabin controls go
## through CabinSystem, whose callbacks are registered by LegacyCabinLogic. The line breaker
## closes only after main_on_bt is held for InitialCtrlDelay and then released (Train.cpp:2976-3070,
## 6779-6827), while the vehicle-level "main_switch" command still acts immediately and returns its
## result (#43).

## EP07-424 of td.scn on a cut of its line, with the EP07's own .fiz and .mmd (demo/tests/fixtures)
const FIXTURES_GAME_DIR:String = "res://tests/fixtures"
const SCENERY:String = "ep07.scn"

var _previous_game_dir:String = ""
var scenery:MaszynaSceneryNode
var player:MaszynaPlayer
var controller:VehicleController
var train_id:String
var vehicle_rid:RID


func before_each():
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    UserSettings.save_maszyna_game_dir(FIXTURES_GAME_DIR)
    scenery = MaszynaSceneryNode.new()
    scenery.filename = SCENERY
    add_child(scenery)
    for i in range(30):
        vehicle_rid = VehicleServer.vehicle_get_rid_by_name("EP07-424")
        if VehicleServer.vehicle_is_simulation_ready(vehicle_rid):
            break
        await wait_seconds(0.5)
    controller = VehicleServer.vehicle_get_controller(vehicle_rid)
    train_id = controller.vehicle_id if controller else ""
    player = load("res://addons/libmaszyna/player/player.tscn").instantiate()
    player.start_vehicle_id = train_id
    add_child(player)
    await wait_idle_frames(10)


func after_each():
    player.free()
    scenery.free()
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


## The cabin the player drives from
func _cabin() -> RID:
    return RailVehicleServer.vehicle_get_driver_cabin(vehicle_rid)


func _power_up() -> void:
    # the player's battery switch, not the vehicle's command (Train.cpp:2891-3070)
    CabinSystem.act(_cabin(), &"battery_sw", &"set", true)
    await wait_idle_frames(2)
    VehicleServer.vehicle_send_command(vehicle_rid, "security_acknowledge", true)
    VehicleServer.vehicle_send_command(vehicle_rid, "security_acknowledge", false)
    VehicleServer.vehicle_send_command(vehicle_rid, "pantograph", RailVehicleEnginePowerSource.PANTOGRAPH_FIRST, true)
    for i in range(20):
        await wait_seconds(0.5)
        if controller.get_state().get("current_collector/pantograph_first_voltage", 0.0) > 100.0:
            break
    # the ground relay only resets with a direction set (Mover.cpp:6038-6051)
    VehicleServer.vehicle_send_command(vehicle_rid, "direction_increase")
    VehicleServer.vehicle_send_command(vehicle_rid, "converter_fuse_reset")
    VehicleServer.vehicle_send_command(vehicle_rid, "fuse_reset")
    await wait_idle_frames(2)


func test_main_switch_needs_hold_and_release() -> void:
    assert_not_null(controller, "EP07-424 should exist")
    if not controller:
        return
    await _power_up()
    var power_supply:RailVehiclePowerSupply = RailVehicleServer.vehicle_component_get(
            vehicle_rid, RailVehicleComponentType.COMPONENT_POWER_SUPPLY) as RailVehiclePowerSupply
    assert_true(power_supply and power_supply.get_power24_available(), "the cab's battery switch gives the low voltage")
    assert_true(controller.get_state().get("main_switch_closable", false), "main switch should be closable once powered")

    CabinSystem.act(_cabin(), &"main_on_bt", &"hold")
    await wait_seconds(0.2)
    CabinSystem.act(_cabin(), &"main_on_bt", &"release")
    await wait_idle_frames(2)
    assert_false(controller.get_state().get("main_switch_enabled", true), "a short press must not close the breaker")

    CabinSystem.act(_cabin(), &"main_on_bt", &"hold")
    await wait_seconds(0.8)
    assert_false(controller.get_state().get("main_switch_enabled", true), "the breaker closes on release, not while held")
    CabinSystem.act(_cabin(), &"main_on_bt", &"release")
    await wait_idle_frames(2)
    assert_true(controller.get_state().get("main_switch_enabled", false), "hold >= IniCDelay and release closes it")

    CabinSystem.act(_cabin(), &"main_off_bt", &"hold")
    CabinSystem.act(_cabin(), &"main_off_bt", &"release")
    await wait_idle_frames(2)
    assert_false(controller.get_state().get("main_switch_enabled", true), "main_off_bt opens the breaker")


func test_main_switch_hold_without_power_does_nothing() -> void:
    assert_not_null(controller, "EP07-424 should exist")
    if not controller:
        return
    CabinSystem.act(_cabin(), &"main_on_bt", &"hold")
    await wait_seconds(0.8)
    CabinSystem.act(_cabin(), &"main_on_bt", &"release")
    await wait_idle_frames(2)
    assert_false(controller.get_state().get("main_switch_enabled", true), "no power - the hold timer must not run")


func test_cabin_controls_are_registered_and_forwarded() -> void:
    assert_not_null(controller, "EP07-424 should exist")
    if not controller:
        return
    var cabin:RID = _cabin()
    assert_true(CabinSystem.has_control(cabin, &"battery_sw"), "battery_sw should be registered")
    var controls:Array = CabinSystem.get_controls(cabin)
    assert_has(controls, &"battery_sw")
    assert_has(controls, &"main_on_bt")
    # the console "cabin <train> controls" listing formats these arrays
    assert_string_contains("\n".join(controls), "main_on_bt")
    assert_string_contains(", ".join(CabinSystem.ACTIONS), "hold")
    CabinSystem.act(cabin, &"battery_sw", &"toggle", true)
    await wait_idle_frames(2)
    assert_true(controller.get_state().get("battery_enabled", false), "battery_sw through CabinSystem switches the battery")
    var power_supply:RailVehiclePowerSupply = RailVehicleServer.vehicle_component_get(
            vehicle_rid, RailVehicleComponentType.COMPONENT_POWER_SUPPLY) as RailVehiclePowerSupply
    assert_true(power_supply and power_supply.get_power24_available(), "and the battery gives the low voltage")
    assert_eq(CabinSystem.get_control(cabin, &"battery_sw"), true)
    assert_null(CabinSystem.act(cabin, &"no_such_control", &"hold"), "unknown control returns null")

    PlayerServer.player_leave_vehicle()
    await wait_idle_frames(5)
    # the cab logic is the vehicle's: it stays registered for the cabin while somebody drives from
    # it (CabinSystem.vehicle_attach_cab_logic())
    var crewed:bool = RailVehicleServer.vehicle_get_driver_cabin(vehicle_rid) == cabin
    assert_eq(CabinSystem.has_control(cabin, &"battery_sw"), crewed,
            "leaving the cab unregisters the callbacks, unless a driver is aboard")
    assert_eq(CabinSystem.get_control(cabin, &"battery_sw"), true, "cabin state survives leaving the cab")


## #43 - the vehicle-level command returns whether it was accepted.
func test_send_command_returns_result() -> void:
    assert_not_null(controller, "EP07-424 should exist")
    if not controller:
        return
    await _power_up()
    assert_true(VehicleServer.vehicle_send_command(vehicle_rid, "main_switch", true), "closing a closable breaker returns true")
    assert_false(VehicleServer.vehicle_send_command(vehicle_rid, "main_switch", true), "closing it again changes nothing")
    assert_false(VehicleServer.vehicle_get_rid_by_name("no_such_train").is_valid(), "an unknown name reaches no vehicle")


## Console "cabin <train> toggle battery_sw" - no value flips the control, and a cabin widget of
## it shows the new position without reporting it back. The fixture has no cab model to build
## widgets from, so the widget is the test's own, bound as the cab instancer binds one: inside the
## Cabin3D of the cabin it belongs to.
func test_toggle_without_value_flips_control_and_widget() -> void:
    assert_not_null(controller, "EP07-424 should exist")
    if not controller:
        return
    var cabin:RID = _cabin()
    var cabin_node:Cabin3D = Cabin3D.new()
    cabin_node.set_cabin(cabin)
    add_child_autofree(cabin_node)
    var widget:CabinButton = CabinButton.new()
    widget.control_id = &"battery_sw"
    cabin_node.add_child(widget)
    widget.set_vehicle_rid(vehicle_rid)
    # the widget reports its own pose on its first frame, as a cab's widgets do when it is built
    await wait_idle_frames(2)

    CabinSystem.act(cabin, &"battery_sw", &"toggle")
    await wait_idle_frames(2)
    assert_true(controller.get_state().get("battery_enabled", false), "toggle without value switches the battery on")
    assert_true(widget.pushed, "the widget follows the cabin state")

    CabinSystem.act(cabin, &"battery_sw", &"toggle")
    await wait_idle_frames(2)
    assert_false(controller.get_state().get("battery_enabled", true), "second toggle switches it off")
    assert_false(widget.pushed, "the widget follows the cabin state")
