extends Node3D

@onready var world = $WorldEnvironment


func _on_gamedir_changed():
    _auto_user_settings_visibility()
    E3DModelManager.clear_cache()
    MaterialManager.clear_cache()
    _reload_all_models()


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

func _on_user_setting_changed(section, key):
    if section == "e3d" and key == "use_alpha_transparency":
        _reload_all_models()

func _ready():
    var menu = $TopBar/HBoxContainer/MenuBar/PopupMenu as PopupMenu
    for child in $ControlWindows.get_children():
        var win = child as HUDWindow
        if win:
            win.visible = false
            menu.add_item(win.title)

    _auto_user_settings_visibility()
    UserSettings.game_dir_changed.connect(_on_gamedir_changed)
    UserSettings.config_changed.connect(_update_render_settings)
    UserSettings.setting_changed.connect(_on_user_setting_changed)
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
        $TopBar/HBoxContainer/ToggleAllControls.button_pressed = not $TopBar/HBoxContainer/ToggleAllControls.button_pressed

func _on_user_settings_panel_visibility_changed():
    var game_dir = UserSettings.get_maszyna_game_dir()
    $UserSettingsPanel/VBoxContainer/GameDirNotSet.visible = not game_dir or FileAccess.file_exists(game_dir)


func _on_loading_screen_fadein_finished() -> void:
    SceneryResourceLoader.enabled = true


func _on_popup_menu_index_pressed(index: int) -> void:
    var win = $ControlWindows.get_child(index) as HUDWindow
    if win:
        win.visible = not win.visible
        for child in win.get_children():
            if "train_controller" in child:
                if $Player.controlled_vehicle:
                    child.train_controller = child.get_path_to($Player.controlled_vehicle.get_controller())


func _on_show_all_controls_button_toggled(toggled_on: bool) -> void:
    for win in $ControlWindows.get_children():
        win.visible = toggled_on
        for child in win.get_children():
            if "train_controller" in child:
                if $Player.controlled_vehicle:
                    child.train_controller = child.get_path_to($Player.controlled_vehicle.get_controller())


func _reload_all_models():
    var instances = get_tree().root.find_children("", "E3DModelInstance", true, false)
    for instance in instances:
        instance.reload()
