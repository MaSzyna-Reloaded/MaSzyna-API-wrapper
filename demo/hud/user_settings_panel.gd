extends PanelContainer

@export_node_path("Skydome") var skydome_path: NodePath

@onready var _time_section: VBoxContainer = $VBoxContainer/TimeSection
@onready var _day_of_year_slider: HSlider = $VBoxContainer/TimeSection/DayOfYearRow/DayOfYearSlider
@onready var _day_of_year_value: Label = $VBoxContainer/TimeSection/DayOfYearRow/DayOfYearValue
@onready var _time_of_day_slider: HSlider = $VBoxContainer/TimeSection/TimeOfDayRow/TimeOfDaySlider
@onready var _time_of_day_value: Label = $VBoxContainer/TimeSection/TimeOfDayRow/TimeOfDayValue


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    _auto_user_settings_visibility()
    UserSettings.game_dir_changed.connect(_on_gamedir_changed)
    UserSettings.config_changed.connect(_update_render_settings)
    UserSettings.setting_changed.connect(_on_user_setting_changed)
    _day_of_year_slider.value_changed.connect(_on_day_of_year_changed)
    _time_of_day_slider.value_changed.connect(_on_time_of_day_changed)
    _sync_time_controls()
    _update_render_settings()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    pass
    

func _input(event):
    if event.is_action_pressed("ui_cancel"):
        visible = not visible


func _on_gamedir_changed():
    _auto_user_settings_visibility()
    E3DModelManager.clear_cache()
    MaterialManager.clear_cache()
    _reload_all_models()


func _auto_user_settings_visibility():
    var game_dir = UserSettings.get_maszyna_game_dir()
    visible = not game_dir or FileAccess.file_exists(game_dir)


func _update_render_settings():
    var viewport: Viewport = get_tree().root.get_viewport()
    var world3d: World3D = viewport.world_3d
    world3d.environment.sdfgi_enabled = UserSettings.get_setting("render", "sdfgi_enabled", true)
    world3d.environment.volumetric_fog_enabled = UserSettings.get_setting("render", "volumetric_fog_enabled", true)
    world3d.environment.ssao_enabled = UserSettings.get_setting("render", "ssao_enabled", true)
    world3d.environment.ssil_enabled = UserSettings.get_setting("render", "ssil_enabled", true)
    world3d.environment.ssr_enabled = UserSettings.get_setting("render", "ssr_enabled", true)
    viewport.anisotropic_filtering_level = UserSettings.get_setting("render", "anisotropic_filtering_level", 2)
    viewport.screen_space_aa = UserSettings.get_setting("render", "screen_space_aa", Viewport.SCREEN_SPACE_AA_FXAA)
    viewport.msaa_3d = UserSettings.get_setting("render", "msaa_3d", 2)
    viewport.use_taa = UserSettings.get_setting("render", "use_taa", true)
    viewport.fsr_sharpness = UserSettings.get_setting("render", "fsr_sharpness", 0.2)
    world3d.environment.glow_enabled = true
    DisplayServer.window_set_vsync_mode(
        DisplayServer.VSYNC_ENABLED
        if UserSettings.get_setting("window", "vsync_enabled", true)
        else DisplayServer.VSYNC_DISABLED
    )

func _on_user_setting_changed(section, key):
    if section == "e3d" and key == "use_alpha_transparency":
        _reload_all_models()


func _reload_all_models():
    var instances = get_tree().root.find_children("", "E3DModelInstance", true, false)
    for instance in instances:
        instance.reload()


func _on_visibility_changed() -> void:
    var game_dir = UserSettings.get_maszyna_game_dir()
    $VBoxContainer/GameDirNotSet.visible = not game_dir or FileAccess.file_exists(game_dir)
    _sync_time_controls()


func _get_skydome() -> Skydome:
    if skydome_path.is_empty():
        return null

    var node: Node = get_node_or_null(skydome_path)
    if node is Skydome:
        return node as Skydome

    return null


func _sync_time_controls() -> void:
    var skydome: Skydome = _get_skydome()
    if skydome == null:
        return

    _day_of_year_slider.set_value_no_signal(skydome.day_of_year)
    _time_of_day_slider.set_value_no_signal(skydome.time_of_day)
    _day_of_year_value.text = str(skydome.day_of_year)
    _time_of_day_value.text = _format_time_of_day(skydome.time_of_day)


func _on_day_of_year_changed(value: float) -> void:
    var skydome: Skydome = _get_skydome()
    if skydome == null:
        return

    skydome.use_system_time = false
    skydome.day_of_year = int(round(value))
    _day_of_year_value.text = str(skydome.day_of_year)


func _on_time_of_day_changed(value: float) -> void:
    var skydome: Skydome = _get_skydome()
    if skydome == null:
        return

    skydome.use_system_time = false
    skydome.time_of_day = value
    _time_of_day_value.text = _format_time_of_day(skydome.time_of_day)


func _format_time_of_day(value: float) -> String:
    var total_minutes: int = int(round(value * 60.0))
    var hours: int = total_minutes / 60
    var minutes: int = total_minutes % 60
    if hours >= 24 and minutes == 0:
        return "24:00"
    if hours >= 24:
        hours = 0
    return "%02d:%02d" % [hours, minutes]
