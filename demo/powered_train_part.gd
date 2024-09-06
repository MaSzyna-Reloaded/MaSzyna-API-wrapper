extends GenericTrainPart
class_name PoweredTrainPart


func _process_train_part(delta):
    var state = get_train_state()
    var power_avail = state.get("power24_available") or state.get("power110_available")
    if power_avail and has_method("_process_powered"):
        call("_process_powered", delta)
    elif not power_avail and has_method("_process_unpowered"):
        call("_process_unpowered", delta)
