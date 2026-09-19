extends Node3D

## Seconds of the music fade out after a scenery has loaded
const MUSIC_FADE_OUT_TIME: float = 1.0

## Menu order snapshot - HUDWindow.move_to_front() reorders ControlWindows children.
var _windows: Array[HUDWindow] = []
var _music_tween: Tween


func _ready() -> void:
    var menu: PopupMenu = $TopBar/HBoxContainer/MenuBar/PopupMenu as PopupMenu
    for child: Node in $ControlWindows.get_children():
        var win: HUDWindow = child as HUDWindow
        win.visible = false
        menu.add_item(win.title)
        _windows.append(win)
    menu.add_item("Exit to menu")
    if not $MaszynaSceneryNode.filename:
        $ScenerySelectorScreen.open()


func _input(event: InputEvent) -> void:
    if event.is_action_pressed("hud_toggle"):
        $TopBar/HBoxContainer/ToggleAllControls.button_pressed = not $TopBar/HBoxContainer/ToggleAllControls.button_pressed
    if event.is_action_pressed("toggle_weather_controls"):
        $ControlWindows/WeatherAndTime.visible = not $ControlWindows/WeatherAndTime.visible


func _on_popup_menu_index_pressed(index: int) -> void:
    if index == _windows.size():
        # "Exit to menu", after the windows
        _exit_to_menu()
        return
    var win: HUDWindow = _windows[index]
    win.visible = not win.visible
    _bind_train_controller(win)


func _on_show_all_controls_button_toggled(toggled_on: bool) -> void:
    for win: Node in $ControlWindows.get_children():
        win.visible = toggled_on
        _bind_train_controller(win)


func _on_scenery_selector_scenery_selected(filename: String) -> void:
    _play_music()
    $TopBar.visible = false
    $ControlWindows.visible = false
    $LoadingScreen.show_loading(filename.get_basename())
    $MaszynaSceneryNode.filename = filename
    await $Player.clear_start_train()
    await $MaszynaSceneryNode.load()
    $LoadingScreen.visible = false
    $TopBar.visible = true
    $ControlWindows.visible = true


## Unloads the scenery and shows the scenario selector
func _exit_to_menu() -> void:
    _play_music()
    $TopBar.visible = false
    $ControlWindows.visible = false
    $ScenerySelectorScreen.open()
    await $Player.clear_start_train()
    $MaszynaSceneryNode.filename = ""
    await $MaszynaSceneryNode.load()


## Music plays while no scenery is loaded (autoplay) and while a scenery loads
func _play_music() -> void:
    if _music_tween:
        _music_tween.kill()
    $Music.volume_linear = 1.0
    if not $Music.playing:
        $Music.play()


func _on_scenery_loaded(_first_train_id: String) -> void:
    _music_tween = create_tween()
    _music_tween.tween_property($Music, "volume_linear", 0.0, MUSIC_FADE_OUT_TIME)
    _music_tween.tween_callback($Music.stop)


func _bind_train_controller(win: Node) -> void:
    if not $Player.controlled_vehicle:
        return
    for child: Node in win.get_children():
        if "train_controller" in child:
            child.train_controller = child.get_path_to($Player.controlled_vehicle.get_controller())
