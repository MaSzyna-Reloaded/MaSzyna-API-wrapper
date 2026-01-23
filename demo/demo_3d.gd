extends Node3D

@onready var world = $WorldEnvironment

func _auto_user_settings_visibility():
    var game_dir = UserSettings.get_maszyna_game_dir()
    $UserSettingsPanel.visible = not game_dir or FileAccess.file_exists(game_dir)

func _update_render_settings():
    var viewport = get_tree().root.get_viewport()
    var world3d:World3D = viewport.world_3d
    world3d.environment.sdfgi_enabled = UserSettings.get_setting("render", "sdfgi_enabled", true)
    world3d.environment.volumetric_fog_enabled = UserSettings.get_setting("render", "volumetric_fog_enabled", true)
    world3d.environment.ssao_enabled = UserSettings.get_setting("render", "ssao_enabled", true)
    world3d.environment.ssil_enabled = UserSettings.get_setting("render", "ssil_enabled", true)
    world3d.environment.ssr_enabled = UserSettings.get_setting("render", "ssr_enabled", true)
    viewport.anisotropic_filtering_level = UserSettings.get_setting("render", "anisotropic_filtering_level", 2)
    viewport.screen_space_aa = UserSettings.get_setting("render", "screen_space_aa", Viewport.SCREEN_SPACE_AA_FXAA)
    viewport.msaa_3d = UserSettings.get_setting("render", "msaa_3d", 2)
    viewport.fsr_sharpness = UserSettings.get_setting("render", "fsr_sharpness", 0.2)
    world3d.environment.glow_enabled = true
    DisplayServer.window_set_vsync_mode(
        DisplayServer.VSYNC_ENABLED
        if UserSettings.get_setting("window", "vsync_enabled", true)
        else DisplayServer.VSYNC_DISABLED
    )

func _ready():
    _auto_user_settings_visibility()
    UserSettings.game_dir_changed.connect(_auto_user_settings_visibility)
    UserSettings.config_changed.connect(_update_render_settings)
    _update_render_settings()
    SceneryResourceLoader.enabled = false
    SceneryResourceLoader.loading_request.connect(_on_loading_started)
    SceneryResourceLoader.scenery_loaded.connect(_on_loading_finished)

func _on_loading_started():
    $LoadingLabel.visible = true
    await get_tree().process_frame
    await get_tree().process_frame

func _on_loading_finished():
    $LoadingLabel.visible = false

func _input(event):
    if event.is_action_pressed("ui_cancel"):
        $UserSettingsPanel.visible = not $UserSettingsPanel.visible

    if event.is_action_pressed("hud_toggle"):
        $DebugHUD.visible = not $DebugHUD.visible

func _on_user_settings_panel_visibility_changed():
    var game_dir = UserSettings.get_maszyna_game_dir()
    $UserSettingsPanel/VBoxContainer/GameDirNotSet.visible = not game_dir or FileAccess.file_exists(game_dir)


func _on_loading_screen_fadein_finished() -> void:
    SceneryResourceLoader.enabled = true
