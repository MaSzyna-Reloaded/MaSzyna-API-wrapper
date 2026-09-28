extends MaszynaGutTest

## The cab light's dimming (cablightdim_sw, TTrain::OnCommand_interiorlightdimenable/disable,
## Train.cpp:6291-6340) reaches the vehicle's state.

var train: VehicleController


func before_each():
    train = build_vehicle("TestCabLightDimming")
    train.add_component(MoverRailVehicleLighting.new())
    train.apply_configuration()
    await wait_idle_frames(2)


func test_the_cab_light_starts_undimmed():
    assert_false(train.state["roof_light_dimmed"])


func test_the_dim_command_dims_and_undims_the_cab_light():
    train.send_command("roof_light_dim", true)
    assert_true(train.state["roof_light_dimmed"])
    train.send_command("roof_light_dim", false)
    assert_false(train.state["roof_light_dimmed"])
