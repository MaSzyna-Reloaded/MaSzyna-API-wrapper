@tool
extends SpinBox

@export var section: String
@export var key: String
@export var default: float = 0.0

func _ready():
	UserSettings.config_changed.connect(_load_from_settings)
	value_changed.connect(_on_value_changed)
	_load_from_settings()

func _load_from_settings():
	value = UserSettings.get_setting(section, key, default)

func _on_value_changed(new_value: float):
	UserSettings.save_setting(section, key, new_value)
