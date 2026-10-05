extends MaszynaGutTest

## The player hands the running EP07-424 of td.scn to its AI driver (Shift+Q) and takes it back (Q):
## the vehicle runs on as it was - the line breaker and the converter stay on (report 2026-10-05:
## both dropped after the take-back). Only the roles in the cabin change (PlayerServer).
##
## The player is in the scene before the scenery and takes the first vehicle that has its simulation
## (MaszynaPlayer.auto_start) - while the scenery is still loading, before its drivers get their AI:
## the AI was given to whoever sat at the controls then - the player - and drove with the player,
## as the player (the cause of the report).

## EP07-424 of td.scn on a cut of its line, with the EP07's own .fiz and .mmd (demo/tests/fixtures)
const FIXTURES_GAME_DIR:String = "res://tests/fixtures"
const SCENERY:String = "ep07.scn"
## Simulated time the AI drives before the player takes the vehicle back [s]
const AI_DRIVING_SECONDS:float = 3.0
## Time the line breaker and the converter take to settle once switched on [s]
const SETTLE_SECONDS:float = 2.0

var _previous_game_dir:String = ""
var scenery:MaszynaSceneryNode
var player:MaszynaPlayer
var vehicle_rid:RID


func before_each():
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    UserSettings.save_maszyna_game_dir(FIXTURES_GAME_DIR)
    player = load("res://addons/libmaszyna/player/player.tscn").instantiate()
    add_child(player)
    scenery = MaszynaSceneryNode.new()
    scenery.filename = SCENERY
    add_child(scenery)
    for i in range(30):
        vehicle_rid = VehicleServer.vehicle_get_rid_by_name("EP07-424")
        if VehicleServer.vehicle_is_simulation_ready(vehicle_rid):
            break
        await wait_seconds(0.5)
    for i in range(30):
        if DriverSystem.vehicle_get_driver(vehicle_rid).is_valid():
            break
        await wait_seconds(0.5)
    await wait_idle_frames(10)


func after_each():
    player.free()
    scenery.free()
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func _main_switch_enabled() -> bool:
    return bool(VehicleServer.vehicle_dump_state(vehicle_rid).get("main_switch_enabled", false))


func _converter_enabled() -> bool:
    var power_supply:RailVehiclePowerSupply = RailVehicleServer.vehicle_component_get(
            vehicle_rid, RailVehicleComponentType.COMPONENT_POWER_SUPPLY) as RailVehiclePowerSupply
    return power_supply.get_converter_enabled()


## The locomotive running: low voltage, pantograph at the wire, the line breaker and the converter on
func _power_up() -> void:
    VehicleServer.vehicle_send_command(vehicle_rid, "battery", true)
    await wait_idle_frames(2)
    VehicleServer.vehicle_send_command(vehicle_rid, "security_acknowledge", true)
    VehicleServer.vehicle_send_command(vehicle_rid, "security_acknowledge", false)
    VehicleServer.vehicle_send_command(vehicle_rid, "pantograph", RailVehicleEnginePowerSource.PANTOGRAPH_FIRST, true)
    for i in range(20):
        await wait_seconds(0.5)
        if VehicleServer.vehicle_dump_state(vehicle_rid).get("current_collector/pantograph_first_voltage", 0.0) > 100.0:
            break
    # the ground relay only resets with a direction set (Mover.cpp:6038-6051)
    VehicleServer.vehicle_send_command(vehicle_rid, "direction_increase")
    VehicleServer.vehicle_send_command(vehicle_rid, "converter_fuse_reset")
    VehicleServer.vehicle_send_command(vehicle_rid, "fuse_reset")
    await wait_idle_frames(2)
    VehicleServer.vehicle_send_command(vehicle_rid, "main_switch", true)
    await wait_idle_frames(5)
    VehicleServer.vehicle_send_command(vehicle_rid, "converter", true)
    await wait_seconds(SETTLE_SECONDS)


func test_handed_to_the_ai_and_taken_back_the_locomotive_runs_on() -> void:
    assert_true(VehicleServer.vehicle_is_simulation_ready(vehicle_rid), "EP07-424 should exist")
    assert_eq(PlayerServer.player_get_vehicle(), vehicle_rid, "the player drives EP07-424")
    var ai_driver:RID = DriverSystem.vehicle_get_driver(vehicle_rid)
    assert_true(ai_driver.is_valid(), "the scenery's driver rides along")
    assert_ne(ai_driver, PlayerServer.player_get_person(), "and it is not the player")
    assert_false(DriverSystem.driver_get_rids().has(PlayerServer.player_get_person()), "the player thinks for itself")
    assert_false(DriverSystem.vehicle_is_control_active(vehicle_rid), "the AI touches nothing while the player drives")
    await _power_up()
    assert_true(_main_switch_enabled(), "the line breaker is on")
    assert_true(_converter_enabled(), "the converter runs")

    PlayerServer.player_hand_over_vehicle()
    assert_true(DriverSystem.vehicle_is_control_active(vehicle_rid), "the AI drives")
    await wait_seconds(AI_DRIVING_SECONDS)
    assert_true(_main_switch_enabled(), "the line breaker stays on while the AI drives")
    assert_true(_converter_enabled(), "and so does the converter")

    PlayerServer.player_take_over_vehicle(vehicle_rid)
    assert_eq(VehicleServer.person_get_role(PlayerServer.player_get_person()),
            VehiclePersonRole.VEHICLE_PERSON_ROLE_DRIVER, "the player drives again")
    assert_false(DriverSystem.vehicle_is_control_active(vehicle_rid), "and the AI rides along")
    await wait_seconds(SETTLE_SECONDS)
    assert_true(_main_switch_enabled(), "the line breaker stays on after the take-back")
    assert_true(_converter_enabled(), "and so does the converter")
