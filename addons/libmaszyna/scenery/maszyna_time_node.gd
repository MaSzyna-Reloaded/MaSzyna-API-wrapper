@tool
extends Node
class_name MaszynaTimeNode

## Marker produced by a scenery's "time:" token - applies its scenario start time to whichever
## MaszynaEnvironmentNode is live in the scene. Original engine's deserialize_time()
## (simulationstateserializer.cpp) only ever carried an HH:MM scenario clock; sunrise/sunset
## fields it also used to read are dead ("no longer used, calculated dynamically" per that same
## function's own comment), so start_time is the only thing worth porting.
@export var start_time:float = 12.0


func _ready() -> void:
    var environment_nodes:Array = get_tree().root.find_children("*", "MaszynaEnvironmentNode")
    if environment_nodes:
        var environment_node:MaszynaEnvironmentNode = environment_nodes[0]
        environment_node.use_system_time = false
        environment_node.current_time = start_time
