extends MaszynaGutTest

const BOUND_CLASSES: Array[StringName] = [
    &"BrakePressureTableItem",
    &"CompressorListItem",
    &"CurvePointItem",
    &"DimmerListItem",
    &"E3DModel",
    &"E3DSubModel",
    &"LightListItem",
    &"LoadListItem",
    &"MotorParameter",
    &"RelayListItem",
    &"ThrottlePositionItem",
    &"VehicleAIHints",
    &"VehicleBrake",
    &"VehicleBuffCoupl",
    &"VehicleController",
    &"VehicleDieselElectricEngine",
    &"VehicleDieselEngine",
    &"VehicleDoors",
    &"VehicleElectricEngine",
    &"VehicleElectricInductionEngine",
    &"VehicleElectricSeriesEngine",
    &"VehicleElectroPneumaticDynamicBrake",
    &"VehicleEngine",
    &"VehicleHeating",
    &"VehicleHorns",
    &"VehicleLighting",
    &"VehicleLoad",
    &"VehicleSecuritySystem",
    &"VehicleSpeedControl",
    &"VehicleSpringBrake",
    &"VehicleSwitches",
    &"VehicleUniversalController",
    &"VehicleWheels",
    &"VehicleWipers",
    &"UniversalControllerListItem",
    &"WWListItem",
    &"WiperListItem",
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
    var brake: VehicleBrake = MoverVehicleBrake.new()
    brake.brake_force_max = 85.0
    assert_eq(brake.brake_force_max, 85.0)

    var electric_engine: VehicleElectricEngine = MoverVehicleElectricSeriesEngine.new()
    electric_engine.power_cable_source = VehicleController.POWER_TYPE_STEAM
    assert_eq(electric_engine.power_cable_source, VehicleController.POWER_TYPE_STEAM)

    var lights: LightListItem = LightListItem.new()
    lights.cabin_a_left_white_signal = false
    lights.cabin_a_right_white_signal = true
    assert_false(lights.cabin_a_left_white_signal)
    assert_true(lights.cabin_a_right_white_signal)

    brake.free()
    electric_engine.free()


func test_group_paths_do_not_change_public_property_names() -> void:
    var current_group: String = ""
    var current_subgroup: String = ""
    var properties: Array[Dictionary] = ClassDB.class_get_property_list(&"VehicleElectricEngine", true)
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
    brake_model.type = VehicleComponentType.COMPONENT_BRAKES
    brake_model.implementation = &"MoverVehicleBrake"
    brake_model.properties = {
        "valve_type": 20,
        "brake_force_max": 85.0,
        "compressor_cab_a_min_pressure": 7.0,
    }
    var engine_model := VehicleComponentModel.new()
    engine_model.type = VehicleComponentType.COMPONENT_ENGINE
    engine_model.implementation = &"MoverVehicleDieselElectricEngine"
    engine_model.properties = {"oil_pump_pressure_minimum": 0.15}
    var security_model := VehicleComponentModel.new()
    security_model.type = VehicleComponentType.COMPONENT_SECURITY
    security_model.implementation = &"MoverVehicleSecuritySystem"
    security_model.properties = {"aware_system_active": true, "emergency_brake_delay": 2.5}

    var model := VehicleModel.new()
    model.properties = {"train_id": "PropertyBindingsTest", "mass": 74000.0}
    var components:Array[VehicleComponentModel] = [brake_model, engine_model, security_model]
    model.components = components

    var vehicle := VehiclePhysicsNode.new()
    add_child_autofree(vehicle)
    vehicle.set_model(model)

    var train: VehicleController = vehicle.get_controller()
    var brake: VehicleBrake = train.get_component(VehicleComponentType.COMPONENT_BRAKES)
    var engine: VehicleDieselEngine = train.get_component(VehicleComponentType.COMPONENT_ENGINE)
    var security_system: VehicleSecuritySystem = train.get_component(VehicleComponentType.COMPONENT_SECURITY)

    assert_eq(train.mass, 74000.0, "the vehicle's own properties too")
    assert_eq(brake.valve_type, 20)
    assert_eq(brake.brake_force_max, 85.0)
    assert_eq(brake.compressor_cab_a_min_pressure, 7.0)
    assert_almost_eq(engine.oil_pump_pressure_minimum, 0.15, 0.000001)
    assert_true(security_system.aware_system_active)
    assert_eq(security_system.emergency_brake_delay, 2.5)
