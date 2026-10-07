extends MaszynaGutTest

const Order = MaszynaLegacyAIDriver.Order
const DEPARTURE:float = 12.0 + 40.0 / 60.0
const ARRIVAL:float = 13.0
const NO_TIME:float = -1.0
const MASS:float = 210000.0
const VEHICLES:int = 5
const UNCOUPLED_VEHICLES:int = 3
## The texts are compared untranslated, whatever language the game runs in
const TEST_LOCALE:String = "en"

var _locale:String = ""


func before_all() -> void:
    _locale = TranslationServer.get_locale()
    TranslationServer.set_locale(TEST_LOCALE)


func after_all() -> void:
    TranslationServer.set_locale(_locale)
    super()


func test_an_order_is_told_as_the_original_tells_it() -> void:
    assert_eq(MaszynaLegacyAIDriver.order_text(Order.WAIT_FOR_ORDERS, 0, false), "Wait for orders")
    assert_eq(MaszynaLegacyAIDriver.order_text(Order.PREPARE_ENGINE, 0, false), "Start the engine")
    assert_eq(MaszynaLegacyAIDriver.order_text(Order.OBEY_TRAIN, 0, false),
            "Drive according to signals and timetable")
    assert_eq(MaszynaLegacyAIDriver.order_text(Order.SHUNT | Order.CHANGE_DIRECTION, 0, false), "Change direction",
            "a change of direction first")
    assert_eq(MaszynaLegacyAIDriver.order_text(Order.CONNECT | Order.CHANGE_DIRECTION, 0, true),
            "Couple to consist ahead", "but not while coupling")
    assert_eq(MaszynaLegacyAIDriver.order_text(Order.DISCONNECT, 0, false), "Uncouple the engine")
    assert_eq(MaszynaLegacyAIDriver.order_text(Order.DISCONNECT, 1, false),
            "Uncouple the engine plus the next vehicle")
    assert_eq(MaszynaLegacyAIDriver.order_text(Order.DISCONNECT, UNCOUPLED_VEHICLES, false),
            "Uncouple the engine plus 3 next vehicles")
    assert_eq(MaszynaLegacyAIDriver.order_text(Order.DISCONNECT, -1, false), "Wait for orders",
            "done with uncoupling")


func test_a_train_is_told_by_its_timetable_and_order() -> void:
    var vehicles:Array[RID] = []
    vehicles.resize(VEHICLES)
    var task:ScenarioTask = ScenarioTask.compose("SU45-123", {
        "order": Order.PREPARE_ENGINE,
        "order_text": "Start the engine",
        "trainset_vehicles": vehicles,
        "trainset_mass": MASS,
    }, {"timetable": _timetable(), "station_index": 0})

    assert_eq(task.title, "Scenario progress for SU45-123")
    assert_eq(task.paragraphs, PackedStringArray([
        "You drive train Os 34229 from Kalisz Pomorski to Szczecinek. Vehicles in the trainset: 5, mass 210 t.",
        "Start station: Kalisz Pomorski, departure at 12:40.",
        "Stops on the way: Wierzchowo, Szczecinek.",
        "Szczecinek: change of direction - uncouple the engine and shunt according to signals.",
        "Current task: Start the engine",
    ]))


func test_a_vehicle_without_a_driver_or_a_task_has_no_scenario() -> void:
    assert_eq(ScenarioTask.compose("SM42-1", {}, {}).title, "No scenario set for SM42-1", "no driver")
    var waiting:ScenarioTask = ScenarioTask.compose("SM42-1",
            {"order": Order.WAIT_FOR_ORDERS, "order_text": "Wait for orders"}, {})
    assert_eq(waiting.title, "No scenario set for SM42-1", "no timetable and nothing to do")
    assert_eq(waiting.paragraphs, PackedStringArray())
    assert_eq(ScenarioTask.describe(RID()).title, "No scenario set", "no vehicle")


func _timetable() -> Timetable:
    var entries:Array[TimetableEntry] = [
        _entry("Kalisz_Pomorski", NO_TIME, DEPARTURE, ""),
        _entry("Wierzchowo", ARRIVAL, ARRIVAL, ""),
        _entry("Przelot", NO_TIME, NO_TIME, ""),
        _entry("Szczecinek", ARRIVAL, NO_TIME, "@"),
    ]
    var timetable:Timetable = Timetable.new()
    timetable.train_category = "Os"
    timetable.train_name = "34229"
    timetable.relation_from = "Kalisz_Pomorski"
    timetable.relation_to = "Szczecinek"
    timetable.entries = entries
    return timetable


func _entry(station:String, arrival:float, departure:float, facilities:String) -> TimetableEntry:
    var entry:TimetableEntry = TimetableEntry.new()
    entry.station_name = station
    entry.arrival = arrival
    entry.departure = departure
    entry.facilities = facilities
    return entry
