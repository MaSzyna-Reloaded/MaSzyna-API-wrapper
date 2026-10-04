class_name SettingToggle
extends SettingRow

## A setting that is on or off: a switch


## Enter switches it, there is no value to step
func is_edited_by_steps() -> bool:
    return false


func _make_compact() -> void:
    %Switch.set_compact(true)


func _enable_control(enabled: bool) -> void:
    %Switch.disabled = not enabled
    %Switch.modulate.a = 1.0 if enabled else UNAVAILABLE_ALPHA


func _show(value: Variant) -> void:
    %Switch.button_pressed = bool(value)


func _step_up() -> void:
    %Switch.button_pressed = true


func _step_down() -> void:
    %Switch.button_pressed = false


func _activate() -> void:
    %Switch.button_pressed = not %Switch.button_pressed


func _on_switch_toggled(toggled_on: bool) -> void:
    _store(toggled_on)
