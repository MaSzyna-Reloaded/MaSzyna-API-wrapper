extends MaszynaGutTest

## The AI driver's lights (TController::CheckVehicles(), control_lights(), Driver.cpp:2349-2525,
## 6389-6428; the headcode hints, driverhints.cpp:1238-1308): the trainset put out, then the
## headcode of the order on the front vehicle's leading end and the last vehicle's trailing end.
## The trainset is two engines and a wagon behind them.

const ENGINE_PATH:String = "res://tests/fixtures/test_vehicle.fiz"
const WAGON_PATH:String = "res://tests/fixtures/test_wagon.fiz"
const VEHICLE_COUNT:int = 3
const WAGON:int = 2
## Original engine: coupling::coupler (MOVER.h:161)
const NO_HINTS:Vector2i = Vector2i(-1, -1)
const LAMPS:PackedStringArray = [
    "headlight_upper", "headlight_left", "headlight_right", "redmarker_left", "redmarker_right",
]
const PC1:PackedStringArray = ["headlight_upper", "headlight_left", "headlight_right"]
const RED_MARKERS:PackedStringArray = ["redmarker_left", "redmarker_right"]

var controllers:Array[VehicleController] = []
var vehicles:Array[RID] = []


func before_each() -> void:
    controllers.clear()
    vehicles.clear()
    var engine:VehicleController = FizVehicleBuilder.build_description_at(ENGINE_PATH)
    var wagon:VehicleController = FizVehicleBuilder.build_description_at(WAGON_PATH)
    for index:int in range(VEHICLE_COUNT):
        # the first vehicle is driven - an unmanned one is not simulated (FINDINGS, 09-23)
        var controller:VehicleController = build_vehicle("LightsVehicle%d" % index,
                wagon if index == WAGON else engine, 0.0,
                MaszynaDynamicData.DriverType.DRIVER_HEAD if index == 0 else MaszynaDynamicData.DriverType.DRIVER_NOBODY)
        controllers.append(controller)
        vehicles.append(controller.get_rid())
    await wait_idle_frames(2)
    for index:int in range(1, VEHICLE_COUNT):
        controllers[index - 1].couple(controllers[index], RailVehicleController.COUPLER_END_REAR,
                RailVehicleController.COUPLER_END_FRONT, RailVehicleController.COUPLING_FLAG_COUPLER)


func after_each() -> void:
    controllers.clear()
    vehicles.clear()


func _lit(index:int, end:String) -> PackedStringArray:
    var lit:PackedStringArray = []
    var state:Dictionary = VehicleServer.vehicle_dump_state(vehicles[index])
    for lamp:String in LAMPS:
        if state.get("lights/%s_%s_enabled" % [end, lamp], false):
            lit.append(lamp)
    return lit


func test_a_train_shows_pc1_at_its_head_and_red_markers_at_its_tail():
    MaszynaLegacyDriverLights.check_vehicles(vehicles[0], 1, MaszynaLegacyAIDriver.Order.OBEY_TRAIN, NO_HINTS)
    assert_eq(_lit(0, "front"), PC1)
    assert_eq(_lit(WAGON, "rear"), RED_MARKERS)
    assert_eq(_lit(0, "rear"), PackedStringArray())
    assert_eq(_lit(WAGON, "front"), PackedStringArray())


func test_the_vehicles_inside_the_trainset_are_put_out():
    VehicleServer.vehicle_send_command(vehicles[1], "light", "front_headlight_left", true)
    MaszynaLegacyDriverLights.check_vehicles(vehicles[0], 1, MaszynaLegacyAIDriver.Order.OBEY_TRAIN, NO_HINTS)
    assert_eq(_lit(1, "front"), PackedStringArray())
    assert_eq(_lit(1, "rear"), PackedStringArray())


# driving the other way the head is the wagon's rear end, and the tail an engine with no direction
# set - it shows plates, no lamp (RaLightsSet(), DynObj.cpp:7266-7278)
func test_the_head_follows_the_way_it_drives():
    MaszynaLegacyDriverLights.check_vehicles(vehicles[WAGON], -1, MaszynaLegacyAIDriver.Order.OBEY_TRAIN, NO_HINTS)
    assert_eq(_lit(WAGON, "rear"), PC1)
    assert_eq(_lit(0, "front"), PackedStringArray())


# headcodetb1 (driverhints.cpp:1265): one white lamp at each end, diagonally
func test_shunting_shows_tb1():
    MaszynaLegacyDriverLights.check_vehicles(vehicles[0], 1, MaszynaLegacyAIDriver.Order.SHUNT, NO_HINTS)
    assert_eq(_lit(0, "front"), PackedStringArray(["headlight_right"]))
    assert_eq(_lit(WAGON, "rear"), PackedStringArray(["headlight_left"]))


# SetLights: the scenery's pattern in place of Pc1 - here Pc2 (driverhints.cpp:1252)
func test_a_train_shows_the_pattern_the_scenery_asked_for():
    var pc2:int = MaszynaLegacyDriverLights.REDMARKER_LEFT | MaszynaLegacyDriverLights.HEADLIGHT_RIGHT \
            | MaszynaLegacyDriverLights.HEADLIGHT_UPPER
    MaszynaLegacyDriverLights.check_vehicles(
            vehicles[0], 1, MaszynaLegacyAIDriver.Order.OBEY_TRAIN, Vector2i(pc2, -1))
    assert_eq(_lit(0, "front"), PackedStringArray(["headlight_upper", "headlight_right", "redmarker_left"]))
    assert_eq(_lit(WAGON, "rear"), RED_MARKERS)


func test_lights_off_puts_out_the_head_and_the_tail():
    MaszynaLegacyDriverLights.check_vehicles(vehicles[0], 1, MaszynaLegacyAIDriver.Order.OBEY_TRAIN, NO_HINTS)
    MaszynaLegacyDriverLights.off(vehicles[0], 1)
    assert_eq(_lit(0, "front"), PackedStringArray())
    assert_eq(_lit(WAGON, "rear"), PackedStringArray())
