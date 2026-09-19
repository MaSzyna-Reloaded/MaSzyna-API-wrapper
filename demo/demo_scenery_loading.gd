extends Node3D

## Seconds of the music fade out after a scenery has loaded
const MUSIC_FADE_OUT_TIME: float = 1.0
## Seconds of the loading screen fade out into the game
const LOADING_FADE_OUT_TIME: float = 1.0
## Frames given to the cabin to instantiate before the loading screen fades out
const CABIN_SETTLE_FRAMES: int = 5
## Frames waited for the player to get its vehicle
const VEHICLE_WAIT_FRAMES: int = 120
## Seconds of each fade of "Exit to menu": game -> spinner -> scenario selector
const EXIT_FADE_TIME: float = 0.5
## Seconds the spinner stays after the scenery has been unloaded
const EXIT_SPINNER_HOLD_TIME: float = 0.5
## Seconds of the fade to black (and the music fade) before quitting
const QUIT_FADE_TIME: float = 0.5

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
    await _wait_for_cabin()
    var tween: Tween = create_tween()
    tween.tween_property($LoadingScreen, "modulate:a", 0.0, LOADING_FADE_OUT_TIME)
    await tween.finished
    $LoadingScreen.visible = false
    $LoadingScreen.modulate.a = 1.0
    $TopBar.visible = true
    $ControlWindows.visible = true


## The player gets its vehicle a few frames after the scenery is loaded, and the cabin is built
## a few frames later still - without this the game pops in half-built behind the loading screen
func _wait_for_cabin() -> void:
    var waited: int = 0
    while not $Player.controlled_vehicle and waited < VEHICLE_WAIT_FRAMES:
        await get_tree().process_frame
        waited += 1
    for frame: int in CABIN_SETTLE_FRAMES:
        await get_tree().process_frame


## Escape in the scenario selector: fade the screen to black and the music out, then quit
func _on_scenery_selector_quit_requested() -> void:
    if _music_tween:
        _music_tween.kill()
    var tween: Tween = create_tween().set_parallel()
    tween.tween_property($FadeLayer/Black, "modulate:a", 1.0, QUIT_FADE_TIME)
    tween.tween_property($Music, "volume_linear", 0.0, QUIT_FADE_TIME)
    await tween.finished
    get_tree().quit()


## Unloading a scenery stalls the main thread (thousands of nodes), so it happens behind the
## spinner: the game fades into it, and it fades into the scenario selector
func _exit_to_menu() -> void:
    _play_music()
    await $SpinnerOverlay.fade_in(EXIT_FADE_TIME)
    $TopBar.visible = false
    $ControlWindows.visible = false
    await $Player.clear_start_train()
    $MaszynaSceneryNode.filename = ""
    await $MaszynaSceneryNode.load()
    await get_tree().create_timer(EXIT_SPINNER_HOLD_TIME).timeout
    $ScenerySelectorScreen.open()
    await $SpinnerOverlay.fade_out(EXIT_FADE_TIME)


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
