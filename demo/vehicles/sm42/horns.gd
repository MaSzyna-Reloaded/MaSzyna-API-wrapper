extends GenericTrainPart

var _state: Dictionary = {
    "horn": false,
    "horn1": false,
    "horn2": false,
}

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    register_command("horn1", set_horn1)
    register_command("horn2", set_horn2)
    register_command("horn", set_horn)

func set_horn1(p_state: Variant) -> void:
    _state["horn1"] = bool(float(p_state)) if p_state else false

func set_horn2(p_state: Variant) -> void:
    _state["horn2"] = bool(float(p_state)) if p_state else false

func set_horn(p_state: Variant) -> void:
    var value: float = float(p_state) if p_state else 0.0
    _state["horn"] = value
    _state["horn1"] = value > 0.0
    _state["horn2"] = value < 0.0

func _get_train_part_state() -> Dictionary:
    return _state
