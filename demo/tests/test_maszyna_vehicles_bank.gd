extends MaszynaGutTest

## MaszynaVehiclesBank: every vehicle of dynamic/ with its skins, from the textures.txt of each
## vehicle directory, as the launcher of the original gathers them (launcher/textures_scanner.cpp)

const GAME_DIR: String = "res://tests/fixtures/vehicles_bank"


func _scan() -> Array[MaszynaVehiclesBank.Vehicle]:
    return MaszynaVehiclesBank.scan(ProjectSettings.globalize_path(GAME_DIR))


func test_lists_vehicles_of_every_index_sorted_by_directory_and_name() -> void:
    var names: Array[String] = []
    for vehicle: MaszynaVehiclesBank.Vehicle in _scan():
        names.append(vehicle.data_path.path_join(vehicle.file_name))

    # a directory without a textures.txt has no vehicles, as in the launcher
    assert_eq(names, [
            "/dynamic/pkp/e186_v1/f140ms",
            "/dynamic/pkp/e186_v1/p160dc",
            "/dynamic/test/wagon_v1/wagon"] as Array[String])


func test_gathers_skins_of_a_vehicle_in_index_order_whatever_the_case_of_its_name() -> void:
    var wagon: MaszynaVehiclesBank.Vehicle = _scan()[2]

    assert_eq(wagon.skins, ["red", "blue"] as Array[String])


func test_skins_match_the_skins_listed_for_the_vehicle() -> void:
    var p160dc: MaszynaVehiclesBank.Vehicle = _scan()[1]
    var vehicle_dir: String = ProjectSettings.globalize_path(GAME_DIR).path_join(p160dc.data_path)

    assert_eq(p160dc.skins, MaszynaVehicleSkins.list_skins(vehicle_dir, p160dc.file_name))
