extends MaszynaGutTest

## The cab's own lights (LegacyCabinCabLights): the cab light, its dimmer (cablightdim_sw,
## TTrain::OnCommand_interiorlightdimenable/disable, Train.cpp:6291-6340) and the instrument light
## belong to the cab they are switched in, never to the vehicle. The instrument light's kind - the
## cab's lamp label - says what powers and switches it, and the dashboard and timetable lights share
## its power (Train.cpp:9562-9574, 11755-11779).

const CAB:int = 1
const OTHER_CAB:int = -1
const TOLERANCE:float = 0.001

var train:VehicleController
var lights:LegacyCabinCabLights


func before_each():
    train = build_vehicle("TestCabLights")
    train.add_component(build_power_supply(110.0))
    train.apply_configuration()
    await wait_idle_frames(2)
    train.send_command("battery", true)
    await wait_idle_frames(2)
    lights = LegacyCabinCabLights.new(LegacyCabinCabLights.InstrumentLightType.STANDARD)
    lights.register(train.get_rid(), CAB)


func _lights_of_kind(kind:LegacyCabinCabLights.InstrumentLightType) -> void:
    lights.unregister()
    lights = LegacyCabinCabLights.new(kind)
    lights.register(train.get_rid(), CAB)
    await wait_idle_frames(2)


func after_each():
    lights.unregister()


func test_the_cab_light_lights_only_the_cab_it_is_switched_in():
    watch_signals(CabinSystem)

    CabinSystem.act(train.get_rid(), CAB, LegacyCabinCabLights.CAB_LIGHT, &"toggle", true)

    var level:float = CabinSystem.cab_get_light_level(train.get_rid(), CAB)
    assert_gt(level, 0.0, "the cab switched in is lit")
    assert_eq(CabinSystem.cab_get_light_level(train.get_rid(), OTHER_CAB), 0.0, "the other cab is not")
    assert_signal_emitted_with_parameters(CabinSystem, "cab_light_level_changed", [train.get_rid(), CAB, level])


func test_the_dimmer_dims_the_cab_light():
    CabinSystem.act(train.get_rid(), CAB, LegacyCabinCabLights.CAB_LIGHT, &"toggle", true)
    var full:float = CabinSystem.cab_get_light_level(train.get_rid(), CAB)

    CabinSystem.act(train.get_rid(), CAB, LegacyCabinCabLights.CAB_LIGHT_DIM, &"toggle", true)
    assert_almost_eq(CabinSystem.cab_get_light_level(train.get_rid(), CAB),
            full * LegacyCabinCabLights.DIMMED_LEVEL, TOLERANCE)

    CabinSystem.act(train.get_rid(), CAB, LegacyCabinCabLights.CAB_LIGHT_DIM, &"toggle", false)
    assert_almost_eq(CabinSystem.cab_get_light_level(train.get_rid(), CAB), full, TOLERANCE)


func test_the_instrument_light_is_the_cabs_own():
    CabinSystem.act(train.get_rid(), CAB, LegacyCabinCabLights.INSTRUMENT_LIGHT, &"toggle", true)

    assert_true(CabinSystem.cab_get_instrument_light_enabled(train.get_rid(), CAB))
    assert_false(CabinSystem.cab_get_instrument_light_enabled(train.get_rid(), OTHER_CAB))


func test_the_vehicle_has_no_cab_light_of_its_own():
    CabinSystem.act(train.get_rid(), CAB, LegacyCabinCabLights.CAB_LIGHT, &"toggle", true)

    assert_false(train.get_state().has("roof_light_enabled"))
    assert_false(train.get_state().has("devices_light_enabled"))


func test_an_always_on_instrument_light_needs_no_switch():
    await _lights_of_kind(LegacyCabinCabLights.InstrumentLightType.ALWAYS)
    assert_true(CabinSystem.cab_get_instrument_light_enabled(train.get_rid(), CAB), "lit with the low voltage")


func test_a_converter_instrument_light_needs_the_110_v():
    await _lights_of_kind(LegacyCabinCabLights.InstrumentLightType.CONVERTER)
    CabinSystem.act(train.get_rid(), CAB, LegacyCabinCabLights.INSTRUMENT_LIGHT, &"toggle", true)
    var power110:bool = bool(train.get_state().get("power110_available", false))
    assert_eq(CabinSystem.cab_get_instrument_light_enabled(train.get_rid(), CAB), power110,
            "lit only with the converter's 110 V")


func test_the_dashboard_and_timetable_lights_are_switched_on_their_own():
    CabinSystem.act(train.get_rid(), CAB, LegacyCabinCabLights.DASHBOARD_LIGHT, &"toggle", true)
    assert_true(CabinSystem.cab_get_dashboard_light_enabled(train.get_rid(), CAB))
    assert_false(CabinSystem.cab_get_timetable_light_enabled(train.get_rid(), CAB))
    CabinSystem.act(train.get_rid(), CAB, LegacyCabinCabLights.TIMETABLE_LIGHT, &"toggle", true)
    assert_true(CabinSystem.cab_get_timetable_light_enabled(train.get_rid(), CAB))
