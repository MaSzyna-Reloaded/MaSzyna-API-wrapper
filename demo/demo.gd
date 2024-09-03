extends Control

var _t:float = 0.0

@onready var train = $SM42_V1

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    $%TrainName.text = "%s (type: %s)" % [train.name, train.type_name]

func draw_dictionary(dict: Dictionary, target: DebugPanel):
    var lines = []
    for k in dict.keys():
        lines.append("%s=%s" % [k, dict[k]])
    target.text = "\n".join(lines)


func get_state_by_path(path):
    var _p = train.get_node(path) as TrainPart
    if _p:
        return _p.get_mover_state(train)
    else:
        return {}

func get_state_by_method(method):
    var _p = Callable(train, "get_%s" % method).call() as TrainPart
    if _p:
        return _p.get_mover_state(train)
    else:
        return {}

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    _t += delta

    if(_t>0.1):
        _t = 0

        var train_state = train.get_mover_state()
        var bv = train_state.get("battery_voltage")

        $%BatteryProgressBar.value = bv
        $%BatteryValue.text = "%.2f V" % [bv]

        var security_state = get_state_by_method("security_system")
        var brake_state = get_state_by_method("brake")
        var engine_state = get_state_by_method("engine")

        draw_dictionary(engine_state, $%DebugEngine)
        draw_dictionary(train_state, $%DebugTrain)
        draw_dictionary(brake_state, $%DebugBrake)
        draw_dictionary(security_state, $%DebugSecurity)


func _on_main_decrease_button_up():
    train.main_controller_decrease()


func _on_main_increase_button_up():
    train.main_controller_increase()


func _on_reverse_button_up():
    train.forwarder_decrease()


func _on_forward_button_up():
    train.forwarder_increase()


func _on_brake_level_value_changed(value):
    var b = train.get_brake() as TrainBrake
    if b:
        b.brake_level = value
