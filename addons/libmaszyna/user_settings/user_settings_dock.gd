@tool
extends Control

# These render options only take effect on the Forward+/Mobile renderers;
# on Compatibility (OpenGL3) they do nothing and just spam engine warnings.
const FORWARD_PLUS_ONLY_CONTROL_PATHS := [
    "VBoxContainer/HBoxContainer3/FXAAButton",
    "VBoxContainer/HBoxContainer13/UseTAA",
    "VBoxContainer/HBoxContainer5/SDFGI",
    "VBoxContainer/HBoxContainer6/SSIL",
    "VBoxContainer/HBoxContainer7/SSAO",
    "VBoxContainer/HBoxContainer8/SSR",
    "VBoxContainer/HBoxContainer4/VolumetricFog",
]


func _on_browse_button_up():
    %DirectorySelectorDialog.popup_centered()


func _update_forward_plus_only_controls_state():
    var supported := UserSettings.get_current_graphics_backend() != "opengl3"
    var tooltip := "" if supported else "Requires the Forward+/Mobile renderer (Vulkan, D3D12 or Metal)."

    for path in FORWARD_PLUS_ONLY_CONTROL_PATHS:
        var control: Control = get_node(path)
        control.disabled = not supported
        control.tooltip_text = tooltip


func _refresh():
    if visible and is_inside_tree() and UserSettings:
        UserSettings.load_config()
        %LineEdit.text = UserSettings.get_maszyna_game_dir()


func _ready():
    if OS.has_feature("release") and not OS.has_feature("editor"):
        $VBoxContainer/GameDirSection.visible = false
    UserSettings.graphics_backend_changed.connect(_on_graphics_backend_changed)
    _update_forward_plus_only_controls_state()
    _refresh()

func _enter_tree():
    get_tree().root.focus_entered.connect(_refresh)


func _exit_tree():
    get_tree().root.focus_entered.disconnect(_refresh)


func _on_directory_selector_dialog_dir_selected(dir):
    %LineEdit.text = dir
    UserSettings.save_maszyna_game_dir(dir)


func _on_clear_cache_button_button_up():
    var fn = func():
        E3DModelManager.clear_cache()
        MaterialManager.clear_cache()

    call_func_with_message_window("Clering caches...", "Please wait.\nClearing caches in progress...", fn)


func _on_graphics_backend_changed():
    _show_message_window(
        "Restart required",
        "The graphics backend change will take effect after restarting the game."
    )


func _on_reload_models_button_button_up():
    var fn = func():
        _reload_e3d_models()
    call_func_with_message_window("Reloading models...", "Please wait.\nModels reloading in progress...", fn)


func _show_message_window(title:String, message: String):
    $InfoMessageWindow/VBoxContainer/Label.text = message
    $InfoMessageWindow.title = title
    $InfoMessageWindow.popup_centered()

func _close_message_window():
    $InfoMessageWindow.hide()

func call_func_with_message_window(title: String, message: String, callable: Callable):
    # A tricky method to display properly popup window with the message

    _show_message_window(title, message)

    var do_call = func():
        # Waits until the window is fully rendered (2 frames)
        #
        # The Godot Editor is written in the Godot Engine, so when a long-running task
        # is executed in a signal handler, the execution of main_loop() / _process()
        # will be blocked and the editor UI will not be rendered correctly.
        await Engine.get_main_loop().process_frame
        await Engine.get_main_loop().process_frame
        callable.call()
        _close_message_window()

    # Yes, must be deferred call here.
    do_call.call_deferred()

func _on_line_edit_text_changed(new_text):
    UserSettings.save_maszyna_game_dir(new_text)


func _reload_e3d_models():
    var instances = get_tree().root.find_children(
        "", "E3DModelInstance", true, false)
    for instance in instances:
        instance.reload()
