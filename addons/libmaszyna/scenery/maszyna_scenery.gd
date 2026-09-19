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


func _clear_content() -> void:
    super._clear_content()
    first_train_id = ""


func _load_content() -> void:
    await super._load_content()
    var vehicles:Array[Node] = find_children("", "DynamicRailVehicle3D", true, false)
    if vehicles:
        first_train_id = (vehicles[0] as DynamicRailVehicle3D).train_id
    scenery_loaded.emit(first_train_id)
