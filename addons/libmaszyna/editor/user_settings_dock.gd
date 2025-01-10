@tool
extends Control


func _on_browse_button_up():
    %DirectorySelectorDialog.popup_centered()


func _refresh():
    UserSettings.load_config()
    %LineEdit.text = UserSettings.get_maszyna_game_dir()


func _ready():
    if OS.has_feature("release") and not OS.has_feature("editor"):
        $VBoxContainer/GameDirSection.visible = false
    _refresh()
    get_tree().root.focus_entered.connect(_refresh)


func _on_directory_selector_dialog_dir_selected(dir):
    %LineEdit.text = dir
    UserSettings.save_maszyna_game_dir(dir)


func _on_clear_cache_button_button_up():
    var fn = func():
        UserSettings.cache_clear_requested.emit()
        UserSettings.cache_cleared.emit()

    call_func_with_message_window("Clering caches...", "Please wait.\nClearing caches in progress...", fn)


func _on_reload_models_button_button_up():
    var fn = func():
        UserSettings.models_reload_requested.emit()
    call_func_with_message_window("Reloading models...", "Please wait.\nModels reloading in progress...", fn)


func _show_message_window(title:String, message: String):
    $InfoMessageWindow/FlowContainer/Label.text = message
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


func _on_settings_updater_timeout():
    _refresh()


func _on_fxaa_button_toggled(toggled_on):
    UserSettings
