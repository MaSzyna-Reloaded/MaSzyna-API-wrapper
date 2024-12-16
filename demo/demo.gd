extends Control

var _t:float = 0.0

@onready var train = $SM42
@onready var brake = $SM42/Brake
@onready var engine = $SM42/StonkaDieselEngine
@onready var security = $SM42/TrainSecuritySystem
@onready var doors = $SM42/TrainDoors


const rich_print_loglevel_colors = {
    LogSystem.LogLevel.DEBUG: "#777",
    LogSystem.LogLevel.ERROR: "red",
    LogSystem.LogLevel.WARNING: "orange",
    }

const loglevel_names = {
    LogSystem.LogLevel.DEBUG: "DEBUG",
    LogSystem.LogLevel.INFO: "INFO",
    LogSystem.LogLevel.WARNING: "WARNING",
    LogSystem.LogLevel.ERROR: "ERROR",
    }
# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    $%TrainName.text = "%s (type: %s)" % [train.name, train.type_name]
    LogSystem.log_updated.connect(print_log_entry_to_godot_console)


func _colorize_loglevel(loglevel, line):
    var color = rich_print_loglevel_colors.get(loglevel)
    if color:
        return "[color=%s]%s[/color]" % [color, line]
    else:
        return line

func print_log_entry_to_godot_console(loglevel, line):
    print_rich(_colorize_loglevel(loglevel, "%s: %s" % [loglevel_names[loglevel], line]))


func print_train_log_entry_to_godot_console(train, loglevel, line):
    print_rich(_colorize_loglevel(loglevel, "LOG: [%s][%s] %s" % [train, loglevel, line]))

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

        var train_state = train.get_state()
        var bv = train_state.get("battery_voltage")

        $%BatteryProgressBar.value = bv
        $%BatteryValue.text = "%.2f V" % [bv]

        var security_state = security.get_mover_state()
        var brake_state = brake.get_mover_state()
        var engine_state = engine.get_mover_state()
        var door_state = doors.get_mover_state()

        draw_dictionary(engine_state, $%DebugEngine)
        draw_dictionary(train_state, $%DebugTrain)
        draw_dictionary(brake_state, $%DebugBrake)
        draw_dictionary(security_state, $%DebugSecurity)
        draw_dictionary(door_state, %DebugDoor)

        $UI/Gauges/EngineRPM.value = engine_state.get("engine_rpm", 0.0) / 1400.0
        $UI/Gauges/EngineCurrent.value = engine_state.get("engine_current", 0.0) / 1500.0
        $UI/Gauges/OilPressure.value = engine_state.get("oil_pump_pressure", 0.0)
        $UI/Gauges/BrakeCylinderPressure.value = brake_state.get("brake_air_pressure", 0.0) / brake_state.get("brake_tank_volume", 1.0)
        $UI/Gauges/BrakePipePressure.value = brake_state.get("pipe_pressure", 0.0)  / 10.0
        $UI/Gauges/Speed.value = train_state.get("speed", 0.0) / 100.0
        $%SecurityLight.enabled = true if train_state.get("blinking") else false
        $%SHPLight.enabled = true if train_state.get("cabsignal_blinking") else false
        $"%DoorsLocked".enabled = true if train_state.get("doors_locked") else false
        $%Forward.modulate = Color.GREEN if train_state.get("direction", 0) > 0 else Color.WHITE
        $%Reverse.modulate = Color.GREEN if train_state.get("direction", 0) < 0 else Color.WHITE
        $"%MainCtrlPos".text = str(train_state.get("controller_main_position", 0))

        $"%LeftDoorsOpenLight".color_active = Color.ORANGE if train_state.get("doors_left_operating") else Color.LIME_GREEN
        $"%LeftDoorsOpenLight".enabled = train_state.get("doors_left_open") or train_state.get("doors_left_operating")
        $"%RightDoorsOpenLight".color_active = Color.ORANGE if train_state.get("doors_right_operating") else Color.LIME_GREEN
        $"%RightDoorsOpenLight".enabled = train_state.get("doors_right_open") or train_state.get("doors_right_operating")

func _on_brake_level_value_changed(value):
    TrainSystem.broadcast_command("brake_level_set", value, null)

func _on_main_decrease_button_up():
    train.send_command("main_controller_decrease")

func _on_main_increase_button_up():
    train.send_command("main_controller_increase")

func _on_reverse_button_up():
    train.send_command("direction_decrease")

func _on_forward_button_up():
    train.send_command("direction_increase")


func _on_sm_42_mover_initialized():
    print("Mover initialized. Train config: ", $SM42.config)
