extends Node3D
class_name BaseCabinTool3D

## A cabin element knows which vehicle it sits in and nothing else about it: every read and every
## manipulation goes through CabinSystem, which holds the vehicle's handle and is the only place
## that talks to the vehicle servers. There is deliberately no path to a controller here - the
## cabin root is told which vehicle it belongs to and passes that down.

var _train_id:String = ""
var _dirty:bool = false
var _applying_control_value:bool = false

signal train_id_changed
## Emitted while the previous vehicle is still the one connected, for disconnecting from it.
signal train_id_changing

## Cabin control id (MMD label) - manipulations are reported to CabinSystem under this id; the
## registered cabin logic decides what they do to the vehicle.
@export var control_id:StringName = &""


## The vehicle this element sits in, as the cabin root hands it down.
func set_train_id(train_id:String) -> void:
    if _train_id == train_id:
        return
    train_id_changing.emit()
    _train_id = train_id
    _dirty = true
    if _train_id:
        train_id_changed.emit()


func get_train_id() -> String:
    return _train_id


## One named value of the vehicle's state - what a control reads, being driven by a property name
## out of the MMD. The dump behind it is built once a frame for the whole cab.
func _vehicle_state_value(key:String, default_value:Variant = null) -> Variant:
    return CabinSystem.vehicle_state_value(_train_id, key, default_value)


## The whole of it, for the few places that genuinely read several unrelated values at once.
func _vehicle_state() -> Dictionary:
    return CabinSystem.vehicle_state(_train_id)


func _vehicle_config() -> Dictionary:
    return CabinSystem.vehicle_config(_train_id)


## Reports a manipulation of this control to CabinSystem, for the occupied cab of its vehicle.
func _act(action:StringName, value:Variant = null) -> Variant:
    if _applying_control_value or not _train_id or not control_id:
        return null
    return CabinSystem.act(_train_id, CabinSystem.occupied_cab(_train_id), control_id, action, value)


# _notification runs on every class of the hierarchy, unlike _ready/_enter_tree overridden below.
func _notification(what:int) -> void:
    if what == NOTIFICATION_ENTER_TREE:
        CabinSystem.control_changed.connect(_on_cabin_control_changed)
    elif what == NOTIFICATION_EXIT_TREE:
        CabinSystem.control_changed.disconnect(_on_cabin_control_changed)


## Shows a control value set in CabinSystem (e.g. from the console) without reporting it back.
func _on_cabin_control_changed(train_id:String, _cab:int, p_control_id:StringName, value:Variant) -> void:
    if not p_control_id == control_id or not _train_id or not train_id == _train_id:
        return
    _applying_control_value = true
    _apply_control_value(value)
    _applying_control_value = false


func _apply_control_value(_value:Variant) -> void:
    pass


func _exit_tree() -> void:
    set_train_id("")
    _dirty = true


func _process_dirty(delta):
    pass


func _process_tool(delta):
    pass


func _process(delta):
    if _dirty:
        _dirty = false
        _process_dirty(delta)
    _process_tool(delta)
