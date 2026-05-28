extends MaszynaGutTest

const MATERIALS_GAME_DIR = "res://tests/materials"
const MATERIAL_NAME = "seasonal_manager"
const NONTRANSPARENT_MATERIAL_NAME = "nontransparent_manager"
const NORMALMAP_SHADER_PATH = "res://addons/libmaszyna/materials/types/normalmap.gdshader"
const SHADOWLESS_SHADER_PATH = "res://addons/libmaszyna/materials/types/shadowlessnormalmap.gdshader"
const WATER_SHADER_PATH = "res://addons/libmaszyna/materials/types/water.gdshader"

var _previous_game_dir: String = ""
var _previous_season: MaterialManager.Season
var _previous_weather: MaterialManager.Weather


func _is_leap_year(value: int) -> bool:
    return ((value % 4) == 0 and not (value % 100) == 0) or (value % 400) == 0


func _get_year_day(value_day: int, value_month: int, value_year: int) -> int:
    var month_lengths: Array[int] = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    var year_day: int = value_day
    var month_index: int = 0

    if _is_leap_year(value_year):
        month_lengths[1] = 29

    while month_index < value_month - 1:
        year_day += month_lengths[month_index]
        month_index += 1

    return year_day


func before_each() -> void:
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    _previous_season = MaterialManager.season
    _previous_weather = MaterialManager.weather
    UserSettings.save_maszyna_game_dir(MATERIALS_GAME_DIR)
    MaterialManager.season = MaterialManager.Season.SEASON_SUMMER
    MaterialManager.weather = MaterialManager.Weather.WEATHER_CLEAR
    MaterialManager.clear_cache()


func after_each() -> void:
    MaterialManager.season = _previous_season
    MaterialManager.weather = _previous_weather
    UserSettings.save_maszyna_game_dir(_previous_game_dir)
    MaterialManager.clear_cache()


func test_parse_stores_default_shader_on_default_variant() -> void:
    var material: MaszynaMaterial = MaterialManager.load_material("", MATERIAL_NAME)

    assert_eq(material.default.shader, "normalmap")


func test_variant_inherits_default_shader_when_season_has_no_shader() -> void:
    var material: MaszynaMaterial = MaterialManager.load_material("", MATERIAL_NAME)
    var variant: MaszynaMaterial.MaszynaMaterialVariant = material.get_variant(
        MaterialManager.Season.SEASON_SPRING,
        MaterialManager.Weather.WEATHER_CLEAR
    )

    assert_eq(variant.shader, "normalmap")
    assert_eq(variant.get_texture_path("diffuse"), "spring_diffuse")


func test_weather_shader_overrides_season_shader() -> void:
    var material: MaszynaMaterial = MaterialManager.load_material("", MATERIAL_NAME)
    var variant: MaszynaMaterial.MaszynaMaterialVariant = material.get_variant(
        MaterialManager.Season.SEASON_WINTER,
        MaterialManager.Weather.WEATHER_RAIN
    )

    assert_eq(variant.shader, "water")


func test_weather_variant_merges_after_season_variant() -> void:
    var material: MaszynaMaterial = MaterialManager.load_material("", MATERIAL_NAME)
    var variant: MaszynaMaterial.MaszynaMaterialVariant = material.get_variant(
        MaterialManager.Season.SEASON_SPRING,
        MaterialManager.Weather.WEATHER_CLOUDY
    )

    assert_eq(variant.get_texture_path("diffuse"), "cloudy_diffuse")


func test_get_material_updates_existing_material_in_place_when_season_changes() -> void:
    var material: ShaderMaterial = MaterialManager.get_material("", MATERIAL_NAME) as ShaderMaterial
    var original_rid: RID = material.get_rid()

    MaterialManager.season = MaterialManager.Season.SEASON_WINTER

    assert_same(material, MaterialManager.get_material("", MATERIAL_NAME))
    assert_eq(material.get_rid(), original_rid)
    assert_eq(material.shader.resource_path, SHADOWLESS_SHADER_PATH)


func test_get_material_updates_existing_material_in_place_when_weather_changes() -> void:
    var material: ShaderMaterial = MaterialManager.get_material("", MATERIAL_NAME) as ShaderMaterial
    var original_rid: RID = material.get_rid()

    MaterialManager.season = MaterialManager.Season.SEASON_WINTER
    MaterialManager.weather = MaterialManager.Weather.WEATHER_RAIN

    assert_same(material, MaterialManager.get_material("", MATERIAL_NAME))
    assert_eq(material.get_rid(), original_rid)
    assert_eq(material.shader.resource_path, WATER_SHADER_PATH)


func test_refresh_prunes_dead_managed_material_entries() -> void:
    var cache_hash: String = MaterialManager._compute_cache_hash(
        "",
        MATERIAL_NAME,
        MaterialManager.Transparency.Disabled,
        Color(1.0, 1.0, 1.0)
    )
    var material: ShaderMaterial = MaterialManager.get_material("", MATERIAL_NAME) as ShaderMaterial
    var material_ref: WeakRef = weakref(material)

    material = null
    MaterialManager.season = MaterialManager.Season.SEASON_WINTER

    assert_null(material_ref.get_ref())
    assert_false(cache_hash in MaterialManager._managed_materials)


func test_environment_node_proxies_season_and_weather_to_material_manager() -> void:
    var environment_node := MaszynaEnvironmentNode.new()

    environment_node.season = MaterialManager.Season.SEASON_WINTER
    environment_node.weather = MaterialManager.Weather.WEATHER_RAIN

    assert_eq(MaterialManager.season, MaterialManager.Season.SEASON_WINTER)
    assert_eq(MaterialManager.weather, MaterialManager.Weather.WEATHER_RAIN)
    environment_node.free()


func test_environment_node_sets_season_from_manual_date_thresholds() -> void:
    var environment_node: MaszynaEnvironmentNode = add_child_autofree(MaszynaEnvironmentNode.new())
    var time_of_day: TimeOfDay = TimeOfDay.new()
    var cases: Array[Array] = [
        [5, 3, 2026, MaterialManager.Season.SEASON_WINTER],
        [7, 3, 2026, MaterialManager.Season.SEASON_SPRING],
        [7, 6, 2026, MaterialManager.Season.SEASON_SPRING],
        [8, 6, 2026, MaterialManager.Season.SEASON_SUMMER],
        [9, 9, 2026, MaterialManager.Season.SEASON_SUMMER],
        [10, 9, 2026, MaterialManager.Season.SEASON_AUTUMN],
        [7, 12, 2026, MaterialManager.Season.SEASON_AUTUMN],
        [8, 12, 2026, MaterialManager.Season.SEASON_WINTER],
    ]

    environment_node.add_child(time_of_day)
    environment_node.time_of_day_path = NodePath("TimeOfDay")
    for case_data: Array in cases:
        environment_node.day = case_data[0]
        environment_node.month = case_data[1]
        environment_node.year = case_data[2]
        environment_node._process(0.0)

        assert_eq(environment_node.season, case_data[3])
        assert_eq(MaterialManager.season, case_data[3])


func test_environment_node_cleans_manual_date_via_time_of_day() -> void:
    var environment_node: MaszynaEnvironmentNode = add_child_autofree(MaszynaEnvironmentNode.new())
    var time_of_day: TimeOfDay = TimeOfDay.new()

    environment_node.add_child(time_of_day)
    environment_node.time_of_day_path = NodePath("TimeOfDay")
    environment_node.day = 31
    environment_node.month = 4
    environment_node.year = 2025
    environment_node._process(0.0)

    assert_eq(environment_node.day, 1)
    assert_eq(environment_node.month, 5)
    assert_eq(environment_node.year, 2025)


func test_environment_node_uses_time_of_day_date_when_system_time_enabled() -> void:
    var environment_node: MaszynaEnvironmentNode = add_child_autofree(MaszynaEnvironmentNode.new())
    var time_of_day: TimeOfDay = TimeOfDay.new()
    var system_date: Dictionary = Time.get_datetime_dict_from_system()

    time_of_day.set_from_datetime_dict(system_date)
    environment_node.add_child(time_of_day)
    environment_node.time_of_day_path = NodePath("TimeOfDay")
    environment_node.day = 1
    environment_node.month = 1
    environment_node.year = 2026
    environment_node.use_system_time = true
    environment_node._process(0.0)

    assert_eq(environment_node.day, time_of_day.day)
    assert_eq(environment_node.month, time_of_day.month)
    assert_eq(environment_node.year, time_of_day.year)
    assert_eq(
        environment_node.season,
        environment_node._season_from_year_day(
            _get_year_day(system_date.day, system_date.month, system_date.year)
        )
    )


func test_get_material_uses_default_shader_before_variant_override() -> void:
    var material: ShaderMaterial = MaterialManager.get_material("", MATERIAL_NAME) as ShaderMaterial

    assert_eq(material.shader.resource_path, NORMALMAP_SHADER_PATH)


func test_texture_transparency_suffix_sets_material_transparent() -> void:
    var material: MaszynaMaterial = MaterialManager.load_material("", MATERIAL_NAME)

    assert_true(material.transparent)


func test_create_sets_alpha_scissor_for_transparent_material() -> void:
    var mmat: MaszynaMaterial = MaterialManager.load_material("", MATERIAL_NAME)
    var material: ShaderMaterial = MaterialFactory.create(mmat) as ShaderMaterial

    assert_eq(material.get_shader_parameter("transparency"), MaterialManager.Transparency.AlphaScissor)
    assert_eq(material.get_shader_parameter("alpha_scissor_threshold"), 0.5)


func test_create_uses_diffuse_color_for_default_shader_without_texture() -> void:
    var mmat := MaszynaMaterial.new()
    var diffuse_color := Color(0.25, 0.5, 0.75, 1.0)
    var material: ShaderMaterial = MaterialFactory.create(
        mmat,
        "",
        MaterialManager.Season.SEASON_SUMMER,
        MaterialManager.Weather.WEATHER_CLEAR,
        diffuse_color
    ) as ShaderMaterial

    assert_eq(material.get_shader_parameter("albedo"), diffuse_color)


func test_create_uses_diffuse_color_for_parallax_shader_without_texture() -> void:
    var mmat := MaszynaMaterial.new()
    mmat.default.shader = "parallax"
    var diffuse_color := Color(0.3, 0.4, 0.5, 1.0)
    var material: ShaderMaterial = MaterialFactory.create(
        mmat,
        "",
        MaterialManager.Season.SEASON_SUMMER,
        MaterialManager.Weather.WEATHER_CLEAR,
        diffuse_color
    ) as ShaderMaterial

    assert_eq(material.get_shader_parameter("albedo_multiplier"), diffuse_color)


func test_apply_updates_existing_material_transparency_state() -> void:
    var transparent_mmat: MaszynaMaterial = MaterialManager.load_material("", MATERIAL_NAME)
    var nontransparent_mmat: MaszynaMaterial = MaterialManager.load_material("", NONTRANSPARENT_MATERIAL_NAME)
    var material: ShaderMaterial = MaterialFactory.create(transparent_mmat) as ShaderMaterial

    MaterialFactory.apply(
        material,
        nontransparent_mmat,
        "",
        MaterialManager.Season.SEASON_SUMMER,
        MaterialManager.Weather.WEATHER_CLEAR
    )

    assert_eq(material.get_shader_parameter("transparency"), MaterialManager.Transparency.Disabled)
    assert_eq(material.get_shader_parameter("alpha_scissor_threshold"), 0.5)
