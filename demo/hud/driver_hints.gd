class_name DriverHints
extends Control

## The driver's hints to the player (the "Hints" of the original's scenario window,
## driveruipanels.cpp:280-294): the steps the driver of the player's vehicle would take, in the
## order it decided on them, translated - a step the vehicle already shows done in green until the
## driver's next update. On its left the key that does it in the cab of the moment
## (CabinLogic.get_action()); a hint no key does starts at the left edge. With no hints, or no
## driver, the tile says so. Read on a Timer while shown and the player has a vehicle; without one
## the tile is hidden, as the driving aid's (show_vehicle()).

## The hint whose text has a place for its parameter (`%.0f`)
const PARAMETER_MARK:String = "%"
const ROW:PackedScene = preload("driver_hint_row.tscn")

## The vehicle the player drives; an invalid RID while none
var vehicle:RID = RID()


func _ready() -> void:
    DrivingAid.apply_style(%HintsTile)


## The vehicle whose driver's hints are shown; an invalid RID hides the tile
func show_vehicle(p_vehicle:RID) -> void:
    vehicle = p_vehicle
    %HintsTile.visible = vehicle.is_valid()
    _on_visibility_changed()


func _on_visibility_changed() -> void:
    if is_visible_in_tree() and vehicle.is_valid():
        %RefreshTimer.start()
        _on_refresh_timer_timeout()
    else:
        %RefreshTimer.stop()


func _on_refresh_timer_timeout() -> void:
    var hints:Array = []
    var driver:RID = DriverSystem.vehicle_get_driver(vehicle)
    if driver.is_valid():
        hints = DriverSystem.driver_get_state(driver).get("hints", [])
    var cab_logic:CabinLogic = CabinSystem.vehicle_get_cab_logic(vehicle)
    var rows:Array[Node] = %HintList.get_children()
    for index:int in hints.size():
        var row:DriverHintRow
        if index < rows.size():
            row = rows[index] as DriverHintRow
        else:
            row = ROW.instantiate()
            %HintList.add_child(row)
        var hint:Dictionary = hints[index]
        var event:InputEvent = null
        if cab_logic and hint["control"]:
            var action:StringName = cab_logic.get_action(hint["control"], hint["gesture"])
            if action and InputMap.has_action(action) and InputMap.action_get_events(action):
                event = InputMap.action_get_events(action)[0]
        var text:String = tr(hint["text"])
        row.show_hint(event, text % hint["parameter"] if text.contains(PARAMETER_MARK) else text, hint["done"])
        row.visible = true
    for index:int in range(hints.size(), rows.size()):
        (rows[index] as Control).visible = false
    %EmptyText.visible = not hints
