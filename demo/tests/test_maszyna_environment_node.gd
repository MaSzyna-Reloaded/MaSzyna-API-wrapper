extends MaszynaGutTest

var _previous_season: MaszynaEnvironment.Season
var _previous_weather: MaszynaEnvironment.Weather


func before_each() -> void:
    _previous_season = MaterialManager.season
    _previous_weather = MaterialManager.weather


func after_each() -> void:
    MaterialManager.season = _previous_season
    MaterialManager.weather = _previous_weather


func test_creates_internal_environment_hierarchy() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()

    assert_true(environment_node._sky_environment is MaszynaSkyEnvironment)
    assert_true(environment_node._sky_environment is GndSkydomeMaszynaEnvironment)
    assert_same(environment_node._sky_environment.environment_node, environment_node)
    assert_eq(environment_node.get_child_count(), 0)
    assert_eq(environment_node.get_child_count(true), 1)

    var world_environment: WorldEnvironment = (
        environment_node.get_child(0, true) as WorldEnvironment
    )
    assert_not_null(world_environment)
    assert_not_null(world_environment.environment)
    assert_eq(world_environment.get_child_count(), 0)
    assert_eq(world_environment.get_child_count(true), 3)
    assert_not_null(world_environment.get_node_or_null("SunLight") as DirectionalLight3D)
    assert_not_null(world_environment.get_node_or_null("Skydome") as Skydome)
    assert_not_null(world_environment.get_node_or_null("Weather") as WeatherNode)
    _assert_children_have_no_owner(environment_node)


func test_does_not_duplicate_generated_environment() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()

    environment_node._ensure_environment()
    environment_node._ensure_environment()

    assert_eq(environment_node.get_child_count(true), 1)


func test_generated_environment_is_not_packed() -> void:
    var scene_root: Node = add_child_autofree(Node.new())
    var environment_node: MaszynaEnvironmentNode = MaszynaEnvironmentNode.new()
    var packed_scene: PackedScene = PackedScene.new()

    scene_root.add_child(environment_node)
    environment_node.owner = scene_root

    assert_eq(packed_scene.pack(scene_root), OK)
    assert_eq(packed_scene.get_state().get_node_count(), 2)


func test_maps_cloudiness_and_wind_to_weather() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()
    var skydome_environment: GndSkydomeMaszynaEnvironment = (
        _get_skydome_environment(environment_node)
    )

    environment_node.cloudiness = 0.8
    environment_node.wind_direction = PI / 2.0
    environment_node._process(0.0)

    assert_almost_eq(skydome_environment.weather.cloud_density, 0.8, 0.000001)
    assert_almost_eq(skydome_environment.skydome.clouds_wind_direction.x, 0.0, 0.000001)
    assert_almost_eq(skydome_environment.skydome.clouds_wind_direction.y, 1.0, 0.000001)
    assert_eq(skydome_environment.weather.skydome_path, NodePath("../Skydome"))


func test_weather_preset_sets_weather_controls() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()
    var weather: WeatherNode = _get_skydome_environment(environment_node).weather

    environment_node.weather = MaszynaEnvironment.Weather.WEATHER_RAIN
    environment_node._process(0.0)

    assert_almost_eq(environment_node.precipitation, 0.8, 0.000001)
    assert_almost_eq(environment_node.cloudiness, 0.9, 0.000001)
    assert_almost_eq(environment_node.fog_density, 0.3, 0.000001)
    assert_almost_eq(environment_node.wind_strength, 0.6, 0.000001)
    assert_almost_eq(weather.precipitation_intensity, 0.8, 0.000001)
    assert_eq(MaterialManager.weather, MaszynaEnvironment.Weather.WEATHER_RAIN)

    environment_node.weather = MaszynaEnvironment.Weather.WEATHER_SNOW
    environment_node._process(0.0)

    assert_almost_eq(weather.precipitation_intensity, 0.0, 0.000001)
    assert_eq(MaterialManager.weather, MaszynaEnvironment.Weather.WEATHER_SNOW)


func test_applies_rendering_light_settings_on_ready() -> void:
    var settings: Dictionary = {
        MaszynaSkyEnvironment.SHADOW_ENABLED_SETTING: false,
        MaszynaSkyEnvironment.SHADOW_MODE_SETTING: DirectionalLight3D.SHADOW_ORTHOGONAL,
        MaszynaSkyEnvironment.SHADOW_BLUR_SETTING: 0.5,
        MaszynaSkyEnvironment.SHADOW_BIAS_SETTING: 0.2,
        MaszynaSkyEnvironment.SHADOW_NORMAL_BIAS_SETTING: 1.5,
        MaszynaSkyEnvironment.SHADOW_EXTERIOR_MAX_DISTANCE_SETTING: 250.0,
        MaszynaSkyEnvironment.VOLUMETRIC_FOG_ENERGY_SETTING: 12.5,
    }
    var previous: Dictionary = {}
    for setting: StringName in settings:
        previous[setting] = ProjectSettings.get_setting(setting)
        ProjectSettings.set_setting(setting, settings[setting])

    var environment_node: MaszynaEnvironmentNode = _create_environment_node()
    var sun_light: DirectionalLight3D = _get_skydome_environment(environment_node).sun_light

    for setting: StringName in previous:
        ProjectSettings.set_setting(setting, previous[setting])

    assert_false(sun_light.shadow_enabled)
    assert_eq(sun_light.directional_shadow_mode, DirectionalLight3D.SHADOW_ORTHOGONAL)
    assert_almost_eq(sun_light.shadow_blur, 0.5, 0.000001)
    assert_almost_eq(sun_light.shadow_bias, 0.2, 0.000001)
    assert_almost_eq(sun_light.shadow_normal_bias, 1.5, 0.000001)
    assert_almost_eq(sun_light.directional_shadow_max_distance, 250.0, 0.000001)
    assert_almost_eq(sun_light.light_volumetric_fog_energy, 12.5, 0.000001)

func test_maps_rain_intensity_to_storm_overcast_and_rainbow() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()
    var skydome_environment: GndSkydomeMaszynaEnvironment = (
        _get_skydome_environment(environment_node)
    )

    environment_node.fog_enabled = false
    environment_node.cloudiness = 0.0
    environment_node.precipitation = 0.8
    environment_node._process(0.0)

    assert_almost_eq(skydome_environment.weather.precipitation_intensity, 0.8, 0.000001)
    assert_almost_eq(skydome_environment.weather.cloud_overcast_intensity, 0.8, 0.000001)
    assert_almost_eq(skydome_environment.weather.storm_intensity, 2.0 / 3.0, 0.000001)
    assert_almost_eq(skydome_environment.weather.storm_fog_intensity, 0.375, 0.000001)
    assert_almost_eq(skydome_environment.skydome.rainbow_intensity, 0.12, 0.000001)


func test_maps_wind_strength_to_weather_global_wind() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()
    var weather: WeatherNode = _get_skydome_environment(environment_node).weather

    environment_node.wind_strength = 1.0
    environment_node._process(0.0)

    assert_almost_eq(weather.global_wind_speed, 3.0, 0.000001)
    assert_almost_eq(weather.global_wind_strength, 5.0, 0.000001)


func test_applies_skydome_and_weather_project_settings() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()
    var skydome: Skydome = _get_skydome_environment(environment_node).skydome

    assert_almost_eq(skydome.day_light_energy, 2.0, 0.000001)
    assert_eq(skydome.clouds_color_shadow, Color(0.8515625, 0.8515625, 0.8515625, 1.0))
    assert_almost_eq(_get_skydome_environment(environment_node).weather.visual_intensity, 1.0, 0.000001)
    assert_almost_eq(
        _get_skydome_environment(environment_node).weather.precipitation_wind_strength, 10.0, 0.000001
    )


func test_maps_fog_controls_to_skydome_and_weather() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()
    var skydome: Skydome = _get_skydome_environment(environment_node).skydome
    var weather: WeatherNode = _get_skydome_environment(environment_node).weather

    environment_node.fog_density = 0.3
    environment_node.fog_distance = 235.0
    environment_node._process(0.0)

    assert_true(environment_node._environment.fog_enabled)
    assert_almost_eq(skydome.day_fog_density, 0.01, 0.000001)
    assert_almost_eq(skydome.night_fog_density, 0.04, 0.000001)
    assert_almost_eq(skydome.day_fog_distance, 235.0, 0.000001)
    # twice the reference opacity at half the reference distance: four times the tuned extinction
    assert_almost_eq(
        skydome.night_vol_fog_density, float(SkydomeSettings.get_value(&"night_vol_fog_density")) * 4.0, 0.000001)
    assert_almost_eq(
        skydome.night_fog_distance, 235.0 * MaszynaSkyEnvironment.FOG_NIGHT_DISTANCE_FACTOR_DEFAULT, 0.000001)
    assert_eq(
        environment_node._environment.volumetric_fog_enabled,
        bool(UserSettings.get_setting("render", "volumetric_fog_enabled", true))
    )
    assert_almost_eq(weather.storm_fog_intensity, 0.3, 0.000001)
    assert_eq(skydome.fog_mode, Skydome.FogModeOverride.DEPTH)
    assert_eq(environment_node._environment.fog_mode, Environment.FOG_MODE_DEPTH)


func test_fog_switch_disables_all_fog_layers() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()

    environment_node.fog_enabled = false
    environment_node._process(0.0)

    assert_false(environment_node._environment.fog_enabled)
    assert_false(environment_node._environment.volumetric_fog_enabled)


func test_applies_time_and_location_as_solar_time() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()
    var skydome_environment: GndSkydomeMaszynaEnvironment = (
        _get_skydome_environment(environment_node)
    )

    environment_node.latitude = 50.271
    environment_node.longitude = 15.0
    environment_node.timezone_offset = 2
    environment_node.current_time = 12.0
    environment_node.set_date(2026, 2, 1)
    environment_node._process(0.0)

    assert_almost_eq(skydome_environment.skydome.latitude, 50.271, 0.000001)
    assert_eq(skydome_environment.skydome.day_of_year, 32)
    assert_almost_eq(skydome_environment.skydome.time_of_day, 11.0, 0.000001)
    assert_eq(skydome_environment.skydome.world_environment_path, NodePath(".."))
    assert_eq(skydome_environment.skydome.directional_light_path, NodePath("../SunLight"))


func test_process_advances_time_with_simulation_speed() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()
    var skydome_environment: GndSkydomeMaszynaEnvironment = (
        _get_skydome_environment(environment_node)
    )

    environment_node.simulation_speed = 3600.0
    environment_node.current_time = 23.5
    environment_node.set_date(2026, 12, 31)
    environment_node._process(0.0)

    skydome_environment.process(1.0)

    assert_almost_eq(skydome_environment.get_current_time(), 0.5, 0.000001)
    assert_eq(skydome_environment.get_date(), Vector3i(2027, 1, 1))


func test_changing_simulation_speed_keeps_running_time() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()

    environment_node.simulation_speed = 3600.0
    environment_node.current_time = 8.0
    environment_node._process(0.0)
    environment_node._process(1.0)
    environment_node.simulation_speed = 1.0
    environment_node._process(0.0)

    assert_almost_eq(environment_node.current_time, 9.0, 0.000001)


func test_proxies_season_and_weather_to_material_manager() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()

    environment_node.season = MaszynaEnvironment.Season.SEASON_WINTER

    assert_eq(MaterialManager.season, MaszynaEnvironment.Season.SEASON_WINTER)

    environment_node.precipitation = 0.0
    environment_node._process(0.0)

    assert_eq(MaterialManager.weather, MaszynaEnvironment.Weather.WEATHER_CLEAR)

    environment_node.precipitation = 0.5
    environment_node._process(0.0)

    assert_eq(MaterialManager.weather, MaszynaEnvironment.Weather.WEATHER_RAIN)


func test_sets_season_from_manual_date_thresholds() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()
    var cases: Array[Array] = [
        [5, 3, 2026, MaszynaEnvironment.Season.SEASON_WINTER],
        [7, 3, 2026, MaszynaEnvironment.Season.SEASON_SPRING],
        [7, 6, 2026, MaszynaEnvironment.Season.SEASON_SPRING],
        [8, 6, 2026, MaszynaEnvironment.Season.SEASON_SUMMER],
        [9, 9, 2026, MaszynaEnvironment.Season.SEASON_SUMMER],
        [10, 9, 2026, MaszynaEnvironment.Season.SEASON_AUTUMN],
        [7, 12, 2026, MaszynaEnvironment.Season.SEASON_AUTUMN],
        [8, 12, 2026, MaszynaEnvironment.Season.SEASON_WINTER],
    ]

    for case_data: Array in cases:
        environment_node.day = case_data[0]
        environment_node.month = case_data[1]
        environment_node.year = case_data[2]
        environment_node._process(0.0)

        assert_eq(environment_node.season, case_data[3])
        assert_eq(MaterialManager.season, case_data[3])


func test_set_date_normalizes_invalid_date() -> void:
    var environment_node: MaszynaEnvironmentNode = _create_environment_node()

    environment_node.set_date(2025, 4, 31)

    assert_eq(environment_node.day, 1)
    assert_eq(environment_node.month, 5)
    assert_eq(environment_node.year, 2025)


func _create_environment_node() -> MaszynaEnvironmentNode:
    return add_child_autofree(MaszynaEnvironmentNode.new()) as MaszynaEnvironmentNode


func _get_skydome_environment(
    environment_node: MaszynaEnvironmentNode
) -> GndSkydomeMaszynaEnvironment:
    return environment_node._sky_environment as GndSkydomeMaszynaEnvironment


func _assert_children_have_no_owner(parent: Node) -> void:
    for child: Node in parent.get_children(true):
        assert_null(child.owner)
        _assert_children_have_no_owner(child)
