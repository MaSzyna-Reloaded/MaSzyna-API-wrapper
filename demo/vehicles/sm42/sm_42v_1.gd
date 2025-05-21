extends TrainController

var _powered:bool = false

func _on_train_lightning_selector_position_changed(position: int) -> void:
    print("New light selectior position is " + str(position))
