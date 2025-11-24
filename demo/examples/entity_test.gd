extends Node2D

@onready var n:EntityNode = $EntityNode

func dict_to_str(x:Dictionary):
    var lines = []
    for key in x.keys():
        lines.append(key+"="+str(x[key]))
    return "\n".join(lines)


func _ready():
    print("Brakes ", n.find_components("brakes"))
    var brake:Component = n.get_component_by_index("brakes", 0)
    $Panel/Label.text = dict_to_str(brake.get_state())
