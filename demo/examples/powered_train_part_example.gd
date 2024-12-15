extends PoweredTrainPart
var _t = 0.0


func _process_powered(delta):
    _t += delta
    if _t > 2.0:
        _t = 0.0
        log_debug("Powered train part example: POWERED")

func _process_unpowered(delta):
    _t += delta
    if _t > 2.0:
        _t = 0.0
        log_debug("Powered train part example: UNPOWERED")
