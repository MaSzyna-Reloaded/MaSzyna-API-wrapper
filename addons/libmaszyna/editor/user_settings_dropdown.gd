@tool
extends OptionButton

@export var section:String = ""
@export var key:String = ""
@export var default:int = 2

func _ready():
    item_selected.connect(_save_settings)
    UserSettings.config_changed.connect(_load_from_settings)
    _load_from_settings()


func _load_from_settings():
    if section and key:
        selected = int(UserSettings.get_setting(section, key, default))


func _save_settings(state):
    if section and key:
        UserSettings.save_setting(section, key, state)
