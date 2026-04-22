extends GenericTrainPart
class_name HornsRailVehicleModule

func _initialize() -> void:
    print("HERE JESERE ")


func _get_supported_commands():
    return {
        "horn": set_horn,
    }


func set_horn(p_state) -> void:
    self.update_state({"horn": p_state})
