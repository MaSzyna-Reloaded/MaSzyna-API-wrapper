extends MaszynaGutTest

## The driver's door hints at the closing of the doors (Doors(), Driver.cpp:4380-4395): the doors
## closed first, then the permits revoked - closing the doors revokes the permits (Mover.cpp:
## 8745-8749), so the revoking hints have no key: a push permit button only grants
## (Train.cpp:7213-7220). On an EN57's driving trailer: a permit needed (DoorNeedPermit), no permit
## presets, the doors closed from the cab (CloseCtrl=DriverCtrl).

const CONTROL_CAR_PATH:String = "res://tests/fixtures/dynamic/pkp/en57-2000_v1/6bs.fiz"
## Long enough for the doors to open or close [s]
const DOORS_TIMEOUT:float = 20.0
const SETTLE_FRAMES:int = 2

var _vehicle:RID


func before_each() -> void:
    var description:VehicleController = FizVehicleBuilder.build_description_at(CONTROL_CAR_PATH)
    _vehicle = build_vehicle("DoorHintsTest", description, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD).get_rid()
    # the unit's converter feeds the trailer's low voltage (BatteryStart=Disabled); alone, its own
    # battery does
    var power_supply:RailVehiclePowerSupply = RailVehicleServer.vehicle_component_get(
            _vehicle, RailVehicleComponentType.COMPONENT_POWER_SUPPLY) as RailVehiclePowerSupply
    power_supply.cntrl_battery_start_mode = RailVehicleController.START_MODE_MANUAL
    await wait_idle_frames(SETTLE_FRAMES)
    # the doors are worked from the cab only with the low voltage (Mover.cpp:8669-8673)
    VehicleServer.vehicle_send_command(_vehicle, "battery", true)
    await wait_until(func() -> bool: return VehicleServer.vehicle_dump_state(_vehicle)["power24_available"], DOORS_TIMEOUT)
    VehicleServer.vehicle_send_command(_vehicle, "cab_activation", true)
    VehicleServer.vehicle_send_command(_vehicle, "doors_left_permit", true)
    VehicleServer.vehicle_send_command(_vehicle, "doors_right_permit", true)
    VehicleServer.vehicle_send_command(_vehicle, "doors_left", true)
    await wait_until(func() -> bool: return _doors().get_left_open(), DOORS_TIMEOUT)


func after_each() -> void:
    StationServer.dispatch_cancel(_vehicle)
    PlayerServer.player_leave_vehicle()


func test_a_revoking_hint_has_no_key_a_granting_one_has() -> void:
    for hint:MaszynaLegacyDriverHints.Hint in [
            MaszynaLegacyDriverHints.Hint.DOOR_LEFT_PERMIT_OFF, MaszynaLegacyDriverHints.Hint.DOOR_RIGHT_PERMIT_OFF]:
        assert_false(MaszynaLegacyDriverHints.CONTROLS.has(hint), "a permit is revoked by closing the doors")
    for hint:MaszynaLegacyDriverHints.Hint in [
            MaszynaLegacyDriverHints.Hint.DOOR_LEFT_PERMIT_ON, MaszynaLegacyDriverHints.Hint.DOOR_RIGHT_PERMIT_ON]:
        assert_true(MaszynaLegacyDriverHints.CONTROLS.has(hint), "a permit is granted by its button")


func test_closing_the_doors_revokes_their_permit() -> void:
    assert_true(_doors().get_left_open(), "the left doors open with their permit")
    VehicleServer.vehicle_send_command(_vehicle, "doors_left", false)
    await wait_until(func() -> bool: return not _doors().get_left_open_permit(), DOORS_TIMEOUT)
    assert_false(_doors().get_left_open_permit(), "closing the doors revokes their permit")


func test_a_player_is_hinted_to_close_the_doors_before_the_permits() -> void:
    var driver:RID = get_vehicle_driver(_vehicle)
    DriverSystem.driver_attach_delegate(driver, MaszynaLegacyAIDriver.new())
    PlayerServer.player_take_over_vehicle(_vehicle)
    # the hints look at the doors of the trainset the driver has checked
    await wait_until(func() -> bool: return DriverSystem.driver_get_state(driver).get("trainset_vehicles", []).size() > 0,
            DOORS_TIMEOUT)
    var cars:Array[RID] = [_vehicle]
    StationServer.dispatch_start(_vehicle, cars)
    StationServer.dispatch_depart(_vehicle)
    await wait_until(func() -> bool: return _hints(driver).has(MaszynaLegacyDriverHints.Hint.DOOR_LEFT_PERMIT_OFF),
            DOORS_TIMEOUT)
    var hints:Array[MaszynaLegacyDriverHints.Hint] = _hints(driver)
    assert_has(hints, MaszynaLegacyDriverHints.Hint.DOOR_LEFT_CLOSE, "the open doors are to be closed")
    assert_lt(hints.find(MaszynaLegacyDriverHints.Hint.DOOR_LEFT_CLOSE),
            hints.find(MaszynaLegacyDriverHints.Hint.DOOR_LEFT_PERMIT_OFF), "the doors first, then the permit")
    assert_lt(hints.find(MaszynaLegacyDriverHints.Hint.DOOR_LEFT_CLOSE),
            hints.find(MaszynaLegacyDriverHints.Hint.DOOR_RIGHT_PERMIT_OFF), "the doors first, then the permits")
    DriverSystem.driver_attach_delegate(driver, null)


func _doors() -> RailVehicleDoors:
    return VehicleServer.vehicle_component_get(_vehicle, VehicleComponentType.COMPONENT_DOORS) as RailVehicleDoors


func _hints(driver:RID) -> Array[MaszynaLegacyDriverHints.Hint]:
    var hints:Array[MaszynaLegacyDriverHints.Hint] = []
    for entry:Dictionary in DriverSystem.driver_get_state(driver).get("hints", []):
        hints.append(entry["hint"])
    return hints
