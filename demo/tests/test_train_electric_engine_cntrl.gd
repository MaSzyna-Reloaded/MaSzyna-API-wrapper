extends MaszynaGutTest

var train: VehicleController
var engine: VehicleElectricSeriesEngine

func before_each():
    train = build_vehicle("TestTrain")

    engine = MoverVehicleElectricSeriesEngine.new()
    engine.power_source = VehicleController.POWER_SOURCE_ACCUMULATOR
    train.add_component(engine)
    await wait_idle_frames(2)

func test_defaults():
    assert_false(engine.cntrl_pantograph_auto_valve)
    assert_eq(engine.cntrl_main_switch_start_mode, VehicleEngine.START_MODE_MANUAL)

func test_round_trip_and_update_without_crashing():
    engine.cntrl_converter_overload_relay_start_mode = VehicleEngine.START_MODE_CONVERTER
    engine.cntrl_converter_overload_relay_off_when_main_is_off = true
    engine.cntrl_pantograph_compressor_start_mode = VehicleEngine.START_MODE_AUTOMATIC
    engine.cntrl_pantograph_auto_valve = true
    engine.cntrl_main_switch_start_mode = VehicleEngine.START_MODE_AUTOMATIC
    await wait_idle_frames(2)

    assert_true(engine.cntrl_pantograph_auto_valve)
    assert_true(train.state.has("main_switch_enabled"), "VehicleElectricEngine should keep functioning after configuring the Cntrl. section")


func _pantograph_vehicle(master_valve_start:VehicleEngine.StartMode) -> VehicleController:
    var vehicle:VehicleController = build_vehicle("TestPantographValves")
    vehicle.battery_voltage = 110.0
    var electric := MoverVehicleElectricSeriesEngine.new()
    electric.power_source = VehicleController.POWER_SOURCE_CURRENTCOLLECTOR
    electric.power_current_collector_number_of_collectors = 1
    electric.cntrl_pantographs_valve_start_mode = master_valve_start
    vehicle.add_component(electric)
    vehicle.apply_configuration()
    await wait_idle_frames(2)
    vehicle.send_command("battery", true)
    await wait_idle_frames(2)
    return vehicle


# LoadFIZ_Cntrl (Mover.cpp:10930) - without PantEPValveStart the master valve opens by itself with
# low voltage, which is what lets an EP07 raise a pantograph from its own switch alone
func test_the_pantographs_master_valve_is_automatic_by_default():
    var vehicle:VehicleController = await _pantograph_vehicle(VehicleEngine.START_MODE_AUTOMATIC)
    assert_true(vehicle.state["current_collector/valve_active"])


# PantEPValveStart=Manual (dynamic/pkp/e186_v2) - it waits for the pantograph lever
func test_a_manual_master_valve_waits_for_the_lever():
    var vehicle:VehicleController = await _pantograph_vehicle(VehicleEngine.START_MODE_MANUAL)
    assert_false(vehicle.state["current_collector/valve_active"])
