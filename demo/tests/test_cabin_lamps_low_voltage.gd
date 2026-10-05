extends MaszynaGutTest

## A cab's lamps are dark without the low voltage, whatever they show: lowvoltagepower
## (Power24vIsAvailable || Power110vIsAvailable, Train.cpp:8843) is handed to every lamp
## (TButton::Update(Power), Button.cpp:126; the btLampka block, Train.cpp:9022, 9242). The radio
## lamp of the fixtures' EP07 follows its radio switch, lit only while the battery gives the
## low voltage.

const EP07_PATH:String = "res://tests/fixtures/dynamic/pkp/303e_v1/303e-ep-tv.fiz"
## Long enough for a few simulation steps and the lamps' own refresh (0.1 s)
const SETTLE_SECONDS:float = 0.5

var train:VehicleController
var indicator:CabinIndicator3D
var spot_light:CabinSpotLight3D


func before_each() -> void:
    # a driven vehicle is simulated (FINDINGS, 09-23); the test drives it, no AI sits aboard
    train = build_vehicle("TestLampsLowVoltage", FizVehicleBuilder.build_description_at(EP07_PATH), 0.0,
            MaszynaDynamicData.DriverType.DRIVER_HEAD)
    indicator = CabinIndicator3D.new()
    indicator.state_property = "radio_enabled"
    add_child_autofree(indicator)
    indicator.set_vehicle_rid(train.get_rid())
    spot_light = CabinSpotLight3D.new()
    spot_light.state_property = "radio_enabled"
    add_child_autofree(spot_light)
    spot_light.set_vehicle_rid(train.get_rid())
    await wait_idle_frames(2)


func _send(command:String, value:Variant) -> void:
    VehicleServer.vehicle_send_command(train.get_rid(), command, value)
    await wait_seconds(SETTLE_SECONDS)


func test_lamps_are_dark_without_the_low_voltage() -> void:
    await _send("battery", false)
    await _send("radio", true)
    assert_true(VehicleServer.vehicle_dump_state(train.get_rid())["radio_enabled"], "the radio is switched on")
    assert_false(indicator.enabled, "no battery, no lamp")
    assert_false(spot_light.enabled, "no battery, no lamp")
    await _send("battery", true)
    assert_true(VehicleServer.vehicle_dump_state(train.get_rid())["power24_available"]
            or VehicleServer.vehicle_dump_state(train.get_rid())["power110_available"], "the battery gives the low voltage")
    assert_true(indicator.enabled, "lit with the low voltage")
    assert_true(spot_light.enabled, "lit with the low voltage")
    await _send("battery", false)
    assert_false(indicator.enabled, "dark again without it")
    assert_false(spot_light.enabled, "dark again without it")
