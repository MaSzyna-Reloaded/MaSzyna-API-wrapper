class_name ScenarioTask
extends RefCounted

## What the player is to do in the vehicle they drive, told in sentences for the scenario panel:
## the train and its relation, the trainset, the station it starts from or drives to, the stops
## left, the stations it turns at and the driver's current order. Scenarios seldom describe the
## task, so it is read from what the driver of the vehicle knows - its timetable and the orders
## the scenario's events have given it (the original shows only the current order,
## driveruipanels.cpp:226-235). Translated when made.

## A station where the train changes direction (`@`, the orders of OrdersInit(), Driver.cpp:5238)
const CHANGE_DIRECTION_MARK:String = "@"

## The heading: the vehicle's task, or that it has none
var title:String = ""
## The sentences, a paragraph each
var paragraphs:PackedStringArray = []


## The task of `vehicle`, read from its driver; a vehicle without a driver has none
static func describe(vehicle:RID) -> ScenarioTask:
    if not vehicle.is_valid():
        var task:ScenarioTask = ScenarioTask.new()
        task.title = TranslationServer.translate("No scenario set")
        return task
    var driver:RID = DriverSystem.vehicle_get_driver(vehicle)
    return compose(VehicleServer.vehicle_get_name(vehicle),
            DriverSystem.driver_get_state(driver) if driver.is_valid() else {},
            DriverSystem.driver_get_timetable_state(driver) if driver.is_valid() else {})


## The task told from a driver's state and its timetable state (DriverSystem.driver_get_state(),
## driver_get_timetable_state()); without a timetable and with no order but waiting there is none
static func compose(vehicle_name:String, driver_state:Dictionary, timetable_state:Dictionary) -> ScenarioTask:
    var task:ScenarioTask = ScenarioTask.new()
    var timetable:Timetable = timetable_state.get("timetable")
    var order:int = driver_state.get("order", MaszynaLegacyAIDriver.Order.WAIT_FOR_ORDERS)
    if not driver_state or (not timetable and order == MaszynaLegacyAIDriver.Order.WAIT_FOR_ORDERS):
        task.title = TranslationServer.translate("No scenario set for %s") % vehicle_name
        return task
    task.title = TranslationServer.translate("Scenario progress for %s") % vehicle_name
    var train:PackedStringArray = []
    if timetable:
        var name:String = ("%s %s" % [timetable.train_category, timetable.train_name]).strip_edges()
        if timetable.relation_from and timetable.relation_to:
            train.append(TranslationServer.translate("You drive train %s from %s to %s.") % [
                name, _station(timetable.relation_from), _station(timetable.relation_to)])
        else:
            train.append(TranslationServer.translate("You drive train %s.") % name)
    var vehicles:Array = driver_state.get("trainset_vehicles", [])
    if vehicles:
        train.append(TranslationServer.translate("Vehicles in the trainset: %d, mass %d t.") % [
            vehicles.size(), roundi(driver_state.get("trainset_mass", 0.0) / VehicleCard.KILOGRAMS_PER_TONNE)])
    if train:
        task.paragraphs.append(" ".join(train))
    var entries:Array = timetable.entries if timetable else []
    var index:int = timetable_state.get("station_index", 0)
    if index < entries.size():
        var entry:TimetableEntry = entries[index]
        var station:String = TranslationServer.translate("Start station: %s" if index == 0 else "Next station: %s") \
                % _station(entry.station_name)
        if entry.departure >= 0.0:
            station += TranslationServer.translate(", departure at %s") % TimetableRow.format_time(entry.departure)
        task.paragraphs.append(station + ".")
    var stops:PackedStringArray = []
    for later:int in range(index + 1, entries.size()):
        if (entries[later] as TimetableEntry).is_stop():
            stops.append(_station((entries[later] as TimetableEntry).station_name))
    if stops:
        task.paragraphs.append(TranslationServer.translate("Stops on the way: %s.") % ", ".join(stops))
    for later:int in range(index, entries.size()):
        var entry:TimetableEntry = entries[later]
        if entry.facilities.contains(CHANGE_DIRECTION_MARK):
            task.paragraphs.append(TranslationServer.translate(
                    "%s: change of direction - uncouple the engine and shunt according to signals.")
                    % _station(entry.station_name))
    var current:String = driver_state.get("order_text", "")
    if current:
        task.paragraphs.append("%s %s" % [TranslationServer.translate("Current task:"), current])
    return task


## A station's name as the timetable panel shows it
static func _station(name:String) -> String:
    return name.replace("_", " ")
