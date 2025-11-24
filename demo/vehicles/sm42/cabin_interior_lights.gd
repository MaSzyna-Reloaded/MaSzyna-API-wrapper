extends TrainPartNode

var roof_light_enabled:bool = false
var devices_light_enabled:bool = false
var _powered:bool = false
var _state = {}

func _ready():
    var train = get_train_node()
    train.power_changed.connect(_on_power_changed)
    train.register_command("roof_light", set_roof_light)
    train.register_command("devices_light", set_devices_light)

func _on_power_changed(train_is_powered):
    _powered = train_is_powered

func set_roof_light(p_enabled):
    roof_light_enabled = true if p_enabled else false

func set_devices_light(p_enabled):
    devices_light_enabled = true if p_enabled else false

func _get_train_part_state():
    _state["roof_light_enabled"] = roof_light_enabled and _powered
    _state["devices_light_enabled"] = devices_light_enabled and _powered
    return _state
