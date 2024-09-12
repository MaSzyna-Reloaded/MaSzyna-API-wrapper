extends Control

var _t:float = 0.0

@onready var train = $SM42_V1
@onready var brake = $SM42_V1/Brake
@onready var engine = $SM42_V1/StonkaDieselEngine
@onready var security = $SM42_V1/TrainSecuritySystem


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    $%TrainName.text = "%s (type: %s)" % [train.name, train.type_name]


func draw_dictionary(dict: Dictionary, target: DebugPanel):
    var lines = []
    for k in dict.keys():
        lines.append("%s=%s" % [k, dict[k]])
    target.text = "\n".join(lines)


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    _t += delta

    if(_t>0.1):
        _t = 0

        var train_state = train.get_mover_state()
        var bv = train_state.get("battery_voltage")

        $%CustomTrainPartCount.text = "%s" % train.state.get("custom_train_part_calls")

        $%BatteryProgressBar.value = bv
        $%BatteryValue.text = "%.2f V" % [bv]

        var security_state = security.get_mover_state()
        var brake_state = brake.get_mover_state()
        var engine_state = engine.get_mover_state()

        draw_dictionary(engine_state, $%DebugEngine)
        draw_dictionary(train_state, $%DebugTrain)
        draw_dictionary(brake_state, $%DebugBrake)
        draw_dictionary(security_state, $%DebugSecurity)

        $EngineRPM.value = engine_state.get("engine_rpm", 0.0) / 1400.0
        $EngineCurrent.value = engine_state.get("engine_current", 0.0) / 1500.0
        $OilPressure.value = engine_state.get("oil_pump_pressure", 0.0)
        $BrakeCylinderPressure.value = brake_state.get("brake_air_pressure", 0.0) / brake_state.get("brake_tank_volume", 1.0)
        $BrakePipePressure.value = brake_state.get("pipe_pressure", 0.0)  / 10.0


func _on_brake_level_value_changed(value):
    train.receive_command("brake_level_set", value)

func _on_main_decrease_button_up():
    train.receive_command("main_controller_decrease")

func _on_main_increase_button_up():
    train.receive_command("main_controller_increase")

func _on_reverse_button_up():
    train.receive_command("forwarder_decrease")

func _on_forward_button_up():
    train.receive_command("forwarder_increase")
