class_name DriverHints
extends Control

## The driver's hints to the player (the "Hints" of the original's scenario window,
## driveruipanels.cpp:280-294): the steps the driver of the player's vehicle would take, in the
## order it decided on them, translated - a step the vehicle already shows done in green until the
## driver's next update. With no hints, or no driver, the tile says so. Read on a Timer while shown;
## the vehicle is the player's of the moment (PlayerServer), kept nowhere here.

## The hint whose text has a place for its parameter (`%.0f`)
const PARAMETER_MARK:String = "%"


func _ready() -> void:
    DrivingAid.apply_style(%HintsTile)


func _on_visibility_changed() -> void:
    if is_visible_in_tree():
        %RefreshTimer.start()
        _on_refresh_timer_timeout()
    else:
        %RefreshTimer.stop()


func _on_refresh_timer_timeout() -> void:
    var hints:Array = []
    var vehicle:RID = PlayerServer.player_get_vehicle()
    var driver:RID = DriverSystem.vehicle_get_driver(vehicle) if vehicle.is_valid() else RID()
    if driver.is_valid():
        hints = DriverSystem.driver_get_state(driver).get("hints", [])
    var labels:Array[Node] = %HintList.get_children()
    for index:int in hints.size():
        var label:Label
        if index < labels.size():
            label = labels[index] as Label
        else:
            label = Label.new()
            %HintList.add_child(label)
        var hint:Dictionary = hints[index]
        var text:String = tr(hint["text"])
        label.text = text % hint["parameter"] if text.contains(PARAMETER_MARK) else text
        label.theme_type_variation = &"DrivingAidHintDone" if hint["done"] else &"DrivingAidHint"
        label.visible = true
    for index:int in range(hints.size(), labels.size()):
        (labels[index] as Label).visible = false
    %EmptyText.visible = not hints
