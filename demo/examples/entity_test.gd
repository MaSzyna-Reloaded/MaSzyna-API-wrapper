extends Node2D

@onready var train:EntityNode = $Train

func dict_to_str(x:Dictionary):
    var lines = []
    for key in x.keys():
        lines.append(key+"="+str(x[key]))
    return "\n".join(lines)


func _ready():
    print("Brakes ", train.find_components("brakes"))
    var brake:Component = train.get_component_by_index("brakes", 0)
    if brake:
        $BrakePanel/Label.text = dict_to_str(brake.get_state())
    $TrainPanel/Label.text = dict_to_str(train.state)
