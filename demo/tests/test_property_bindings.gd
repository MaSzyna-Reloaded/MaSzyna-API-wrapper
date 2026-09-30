extends MaszynaGutTest

const BOUND_CLASSES: Array[StringName] = [
    &"RailVehicleBrakePressureTableItem",
    &"RailVehicleCompressorListItem",
    &"VehicleCurvePointItem",
    &"RailVehicleDimmerListItem",
    &"E3DModel",
    &"E3DSubModel",
    &"RailVehicleLightListItem",
    &"RailVehicleLoadListItem",
    &"RailVehicleMotorParameter",
    &"RailVehicleRelayListItem",
    &"RailVehicleThrottlePositionItem",
    &"RailVehicleAIHints",
    &"RailVehicleBrake",
    &"RailVehicleBuffCoupl",
    &"VehicleController",
    &"RailVehicleDieselElectricEngine",
    &"RailVehicleDieselEngine",
    &"RailVehicleDoors",
    &"RailVehicleElectricEngine",
    &"RailVehicleElectricInductionEngine",
    &"RailVehicleElectricSeriesEngine",
    &"RailVehicleElectroPneumaticDynamicBrake",
    &"RailVehicleEngine",
    &"RailVehicleHeating",
    &"RailVehicleHorns",
    &"RailVehicleLighting",
    &"RailVehicleLoad",
    &"RailVehicleSecuritySystem",
    &"RailVehicleSpeedControl",
    &"RailVehicleSpringBrake",
    &"RailVehicleSwitches",
    &"RailVehicleUniversalController",
    &"RailVehicleWheels",
    &"RailVehicleWipers",
    &"RailVehicleUniversalControllerListItem",
    &"RailVehicleWWListItem",
    &"RailVehicleWiperListItem",
]


func test_bound_properties_use_canonical_names_and_accessors() -> void:
    for bound_class in BOUND_CLASSES:
        var properties: Array[Dictionary] = ClassDB.class_get_property_list(bound_class, true)
        for property in properties:
            var usage: int = int(property["usage"])
            if bool(usage & PROPERTY_USAGE_GROUP) or bool(usage & PROPERTY_USAGE_SUBGROUP):
                continue

            var property_name: StringName = StringName(property["name"])
            var setter: StringName = ClassDB.class_get_property_setter(bound_class, property_name)
            var getter: StringName = ClassDB.class_get_property_getter(bound_class, property_name)
            if not setter or not getter:
                continue

            assert_false(String(property_name).contains("/"), "%s.%s contains a slash" % [bound_class, property_name])
            assert_eq(
                setter,
                StringName("set_" + String(property_name)),
                "%s.%s has an inconsistent setter" % [bound_class, property_name],
            )
            assert_eq(
                getter,
                StringName("get_" + String(property_name)),
                "%s.%s has an inconsistent getter" % [bound_class, property_name],
            )


func test_properties_are_available_through_direct_gdscript_access() -> void:
    var brake: RailVehicleBrake = MoverRailVehicleBrake.new()
    brake.brake_force_max = 85.0
    assert_eq(brake.brake_force_max, 85.0)

    var electric_engine: RailVehicleElectricEngine = MoverRailVehicleElectricSeriesEngine.new()
    electric_engine.power_cable_source = RailVehicleController.POWER_TYPE_STEAM
    assert_eq(electric_engine.power_cable_source, RailVehicleController.POWER_TYPE_STEAM)

    var lights: RailVehicleLightListItem = RailVehicleLightListItem.new()
    lights.cabin_a_left_white_signal = false
    lights.cabin_a_right_white_signal = true
    assert_false(lights.cabin_a_left_white_signal)
    assert_true(lights.cabin_a_right_white_signal)


func test_group_paths_do_not_change_public_property_names() -> void:
    var current_group: String = ""
    var current_subgroup: String = ""
    var properties: Array[Dictionary] = ClassDB.class_get_property_list(&"RailVehicleElectricEngine", true)
    for property in properties:
        var usage: int = int(property["usage"])
        if bool(usage & PROPERTY_USAGE_GROUP):
            current_group = property["name"]
            current_subgroup = ""
        elif bool(usage & PROPERTY_USAGE_SUBGROUP):
            current_subgroup = property["name"]
        elif property["name"] == &"power_cable_source":
            assert_eq(current_group, "Power")
            assert_eq(current_subgroup, "Power Cable")
            return

    fail_test("power_cable_source was not found")


## Authored configuration has to survive into the built vehicle. It used to be authored as a
## scene of component nodes; a component is not a node any more, so it is authored as a
## VehicleModel - and this asserts the same thing through it.
func test_authored_configuration_reaches_the_built_vehicle() -> void:
    var brake_model := VehicleComponentModel.new()
    brake_model.implementation = &"MoverRailVehicleBrake"
    brake_model.properties = {
        "valve_type": 20,
        "brake_force_max": 85.0,
        "compressor_cab_a_min_pressure": 7.0,
    }
    var engine_model := VehicleComponentModel.new()
    engine_model.implementation = &"MoverRailVehicleDieselElectricEngine"
    engine_model.properties = {"oil_pump_pressure_minimum": 0.15}
    var security_model := VehicleComponentModel.new()
    security_model.implementation = &"MoverRailVehicleSecuritySystem"
    security_model.properties = {"aware_system_active": true, "emergency_brake_delay": 2.5}

    var model := VehicleModel.new()
    model.properties = {"train_id": "PropertyBindingsTest", "mass": 74000.0}
    var components:Array[VehicleComponentModel] = [brake_model, engine_model, security_model]
    model.components = components

    var vehicle := VehiclePhysicsNode.new()
    add_child_autofree(vehicle)
    RailVehicleServer.vehicle_attach(vehicle.get_vehicle_rid())
    vehicle.set_model(model)

    var train: VehicleController = vehicle.get_controller()
    var brake: RailVehicleBrake = train.get_rail_component(RailVehicleComponentType.COMPONENT_BRAKES)
    var engine: RailVehicleDieselEngine = train.get_component(VehicleComponentType.COMPONENT_ENGINE)
    var security_system: RailVehicleSecuritySystem = train.get_rail_component(RailVehicleComponentType.COMPONENT_SECURITY)

    assert_eq(train.mass, 74000.0, "the vehicle's own properties too")
    assert_eq(brake.valve_type, 20)
    assert_eq(brake.brake_force_max, 85.0)
    assert_eq(brake.compressor_cab_a_min_pressure, 7.0)
    assert_almost_eq(engine.oil_pump_pressure_minimum, 0.15, 0.000001)
    assert_true(security_system.aware_system_active)
    assert_eq(security_system.emergency_brake_delay, 2.5)
