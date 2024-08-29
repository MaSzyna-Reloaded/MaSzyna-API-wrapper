extends Node2D

var _t:float = 0.0

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    pass

func draw_dictionary(dict: Dictionary, target: DebugPanel):
    var lines = []
    for k in dict.keys():
        lines.append("%s=%s" % [k, dict[k]])
    target.text = "\n".join(lines)


func get_state_by_path(path):
    var _p = $Train.get_node(path) as TrainPart
    if _p:
        return _p.get_mover_state($Train)
    else:
        return {}

func get_state_by_method(method):
    var _p = Callable($Train, "get_%s" % method).call() as TrainPart
    if _p:
        return _p.get_mover_state($Train)
    else:
        return {}

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    _t += delta

    if(_t>0.1):
        _t = 0

        var train = $Train
        var train_state = train.get_mover_state()
        var bv = train_state.get("battery_voltage")

        $%BatteryProgressBar.value = bv
        $%BatteryValue.text = "%.2f V" % [bv]


		var security_state = get_state_by_method("security_system")
		var brake_state = get_state_by_method("brake")
		var engine_state = get_state_by_method("engine")

		draw_dictionary(engine_state, $DebugEngine)
		draw_dictionary(train_state, $DebugTrain)
		draw_dictionary(brake_state, $DebugBrake)
		draw_dictionary(security_state, $DebugSecurity)
		draw_dictionary(mainswitch_state, $DebugMainSwitch)


func _on_oil_pump_toggled(toggled_on):
    $Train.set("switches/oil_pump_enabled", toggled_on)


func _on_main_switch_toggled(toggled_on):
    $Train/Guziki/TrainMainSwitch.pushed = toggled_on


func _on_fuel_pump_toggled(toggled_on):
    $Train.set("switches/fuel_pump_enabled", toggled_on)


func _on_battery_switch_toggled(toggled_on):
    $Train.set("switches/battery_enabled", toggled_on)
