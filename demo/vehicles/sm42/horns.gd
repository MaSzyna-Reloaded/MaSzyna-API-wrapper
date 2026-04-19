extends GenericTrainPart
class_name HornsRailVehicleModule

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    register_command("horn", set_horn)


func set_horn(p_state) -> void:
    self.update_state({"horn": p_state})
