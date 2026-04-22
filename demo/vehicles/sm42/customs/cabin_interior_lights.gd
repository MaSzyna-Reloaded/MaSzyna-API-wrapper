extends GenericTrainPart

var roof_light_enabled:bool = false
var devices_light_enabled:bool = false

func _get_supported_commands():
    return {
        "roof_light": set_roof_light,
        "devices_light": set_devices_light,
        }

func set_roof_light(p_enabled):
    roof_light_enabled = true if p_enabled else false

func set_devices_light(p_enabled):
    devices_light_enabled = true if p_enabled else false

func _get_train_part_state():
    var state = get_train_state()
    var power_avail = state.get("power24_available") or state.get("power110_available")
    return {
        "roof_light_enabled": roof_light_enabled and power_avail,
        "devices_light_enabled": devices_light_enabled and power_avail,
    }
