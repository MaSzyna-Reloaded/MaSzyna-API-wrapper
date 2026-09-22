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


func test_migrated_scene_properties_are_loaded() -> void:
    var scene: PackedScene = load("res://tests/sm42_controller.tscn")
    var train: VehicleController = scene.instantiate()
    var brake: VehicleBrake = train.get_node("Brake")
    var engine: VehicleDieselEngine = train.get_node("StonkaDieselEngine")
    var security_system: VehicleSecuritySystem = train.get_node("VehicleSecuritySystem")

    assert_eq(brake.valve_type, 20)
    assert_eq(brake.brake_force_max, 85.0)
    assert_eq(brake.compressor_cab_a_min_pressure, 7.0)
    assert_almost_eq(engine.oil_pump_pressure_minimum, 0.15, 0.000001)
    assert_true(security_system.aware_system_active)
    assert_eq(security_system.emergency_brake_delay, 2.5)

    train.free()
