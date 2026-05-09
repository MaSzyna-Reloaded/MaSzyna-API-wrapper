extends Node
class_name MaszynaTimeNode

@export var start_time:float = 12.00

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    var skydomes = get_tree().root.find_children("*", "Skydome")
    if skydomes:
        var skydome = skydomes[0] as Skydome
        if skydome:
            skydome.time_of_day = start_time
            skydome.use_system_time = false
