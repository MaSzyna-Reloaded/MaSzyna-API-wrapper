@tool
extends MaszynaIncludeNode
class_name MaszynaSceneryNode

## Emitted after every load with the first vehicle's train_id ("" when the scenery has none)
signal scenery_loaded(first_train_id:String)


@export var title:String = ""
@export var category:String = ""
@export_multiline var description:String = ""
@export_tool_button("Load") var load_action:Callable = self.load

## train_id of the first vehicle found in the loaded scenery
var first_train_id:String = ""


func _clear_content(budget_msec:int = 0) -> void:
    await super._clear_content(budget_msec)
    first_train_id = ""


func _load_content() -> void:
    await super._load_content()
    first_train_id = _find_driver_train_id(find_children("", "DynamicRailVehicle3D", true, false))
    scenery_loaded.emit(first_train_id)


## The player belongs in a vehicle with a driver, not in whatever vehicle the scenery declares
## first (DynamicRailVehicle3D.cabin_number: 1 = headdriver, -1 = reardriver, 0 = nobody)
func _find_driver_train_id(vehicles:Array[Node]) -> String:
    var reverse_driver_train_id:String = ""
    for node:Node in vehicles:
        var vehicle:DynamicRailVehicle3D = node as DynamicRailVehicle3D
        if vehicle.cabin_number == 1:
            return vehicle.train_id
        if vehicle.cabin_number == -1 and not reverse_driver_train_id:
            reverse_driver_train_id = vehicle.train_id
    if reverse_driver_train_id:
        return reverse_driver_train_id
    return (vehicles[0] as DynamicRailVehicle3D).train_id if vehicles else ""
