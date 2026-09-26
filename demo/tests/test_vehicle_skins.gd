extends MaszynaGutTest

## VehicleSkins: the skins of a vehicle come from its textures.txt, as in the launcher of the
## original (launcher/textures_scanner.cpp); without one, from the .mat files next to it.


func test_lists_skins_of_the_vehicle_from_textures_index() -> void:
    var skins: Array[String] = VehicleSkins.list_skins(
            ProjectSettings.globalize_path("res://tests/fixtures/skins"), "p160dc")

    assert_eq(skins, ["146_226-6", "480_002-9"] as Array[String])


func test_skips_skins_of_other_vehicles_and_rule_lines() -> void:
    var skins: Array[String] = VehicleSkins.list_skins(
            ProjectSettings.globalize_path("res://tests/fixtures/skins"), "F140MS")

    assert_eq(skins, ["e186_240"] as Array[String])


func test_lists_materials_without_textures_index() -> void:
    var skins: Array[String] = VehicleSkins.list_skins(
            ProjectSettings.globalize_path("res://tests/fixtures/skins_without_index"), "anything")

    assert_eq(skins, ["first", "second"] as Array[String])
