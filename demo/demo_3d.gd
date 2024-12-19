extends Node3D

@onready var world = $WorldEnvironment

func _auto_user_settings_visibility():
    var game_dir = UserSettings.get_maszyna_game_dir()
    $UserSettingsPanel.visible = not game_dir or FileAccess.file_exists(game_dir)

func _on_console_toggled(opened: bool):
    $Camera3D.enabled = not opened

func _update_render_settings():
    var viewport = get_tree().root.get_viewport()
    var world3d:World3D = viewport.world_3d
    world3d.environment.sdfgi_enabled = UserSettings.get_setting("render", "sdfgi")
    world3d.environment.volumetric_fog_enabled = UserSettings.get_setting("render", "volumetric_fog")
    viewport.screen_space_aa = viewport.SCREEN_SPACE_AA_FXAA if UserSettings.get_setting("render", "fxaa") else viewport.SCREEN_SPACE_AA_DISABLED

func _ready():
    _auto_user_settings_visibility()
    UserSettings.game_dir_changed.connect(_auto_user_settings_visibility)
    UserSettings.config_changed.connect(_update_render_settings)
    Console.console_toggled.connect(_on_console_toggled)

func _input(event):
    if event.is_action_pressed("ui_cancel"):
        $UserSettingsPanel.visible = not $UserSettingsPanel.visible

func _process(delta):
    var state = TrainSystem.get_train_state("sm42v1")
    $Stonka.position += Vector3.FORWARD * delta * state.get("velocity", 0.0)

func _on_user_settings_panel_visibility_changed():
    var game_dir = UserSettings.get_maszyna_game_dir()
    $UserSettingsPanel/VBoxContainer/GameDirNotSet.visible = not game_dir or FileAccess.file_exists(game_dir)
