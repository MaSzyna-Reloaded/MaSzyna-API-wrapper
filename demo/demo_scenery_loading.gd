extends Node3D

## Seconds of the music fade out after a scenery has loaded
const MUSIC_FADE_OUT_TIME: float = 1.0
## Music in the menu, and the little it gains while a scenery loads - 3 dB, not the jump to full
## scale the loading used to make
const MUSIC_MENU_VOLUME_DB: float = -6.0
const MUSIC_LOADING_VOLUME_DB: float = -3.0
## Seconds of the loading screen fade out into the game
const LOADING_FADE_OUT_TIME: float = 1.0
## Frames given to the cabin to instantiate before the loading screen fades out
const CABIN_SETTLE_FRAMES: int = 5
## Frames waited for the player to get its vehicle
const VEHICLE_WAIT_FRAMES: int = 120
## Seconds the loading screen waits for the streaming to fill in around the player before giving up
const STREAMING_WAIT_TIME: float = 30.0
## Seconds of each fade of "Exit to menu": game -> spinner -> scenario selector
const EXIT_FADE_TIME: float = 0.5
## Seconds the spinner stays after the scenery has been unloaded
const EXIT_SPINNER_HOLD_TIME: float = 0.5
## Seconds of the fade to black (and the music fade) before quitting
const QUIT_FADE_TIME: float = 0.5

var _music_tween: Tween


## Before _ready(): the children must not read a cache left by another build
func _enter_tree() -> void:
    MaszynaRuntime.check_build_version()


func _ready() -> void:
    if not $MaszynaSceneryNode.filename:
        $ScenerySelectorScreen.open()


func _on_scenery_selector_scenery_selected(
    filename: String, train_id: String, skin_overrides: Dictionary
) -> void:
    _play_music(MUSIC_LOADING_VOLUME_DB)
    $GameHud.visible = false
    # The camera moves to the selected vehicle only after loading; planning before that point
    # streams the empty menu position and puts irrelevant work ahead of the starting area.
    SceneryStreamingServer.set_camera(null)
    # the title from the .scn header ("//$n"), not the file name
    var info: MaszynaSceneryInfo = MaszynaSceneryInfo.read(filename)
    $LoadingScreen.show_loading(info.title if info.title else filename.get_basename())
    $MaszynaSceneryNode.filename = filename
    $MaszynaSceneryNode.skin_overrides.assign(skin_overrides)
    await $Player.clear_start_train()
    # the consist chosen in the selector; an empty one lets the scenery pick its own driver
    $Player.start_train_id = train_id
    await $MaszynaSceneryNode.load()
    await _wait_for_cabin()
    SceneryStreamingServer.set_camera($Player.get_camera())
    await _wait_for_streaming()
    var tween: Tween = create_tween()
    tween.tween_property($LoadingScreen, "modulate:a", 0.0, LOADING_FADE_OUT_TIME)
    await tween.finished
    $LoadingScreen.visible = false
    $LoadingScreen.modulate.a = 1.0
    $GameHud.visible = true


## The player gets its vehicle a few frames after the scenery is loaded, and the cabin is built
## a few frames later still - without this the game pops in half-built behind the loading screen
func _wait_for_cabin() -> void:
    var waited: int = 0
    while not $Player.controlled_vehicle and waited < VEHICLE_WAIT_FRAMES:
        await get_tree().process_frame
        waited += 1
    for frame: int in CABIN_SETTLE_FRAMES:
        await get_tree().process_frame


## Wait only for the chunk containing the camera. Neighbouring chunks and the rest of the draw
## distance keep streaming after the game appears.
func _wait_for_streaming() -> void:
    var deadline: float = Time.get_ticks_msec() + STREAMING_WAIT_TIME * 1000.0
    while SceneryStreamingServer.has_camera() and Time.get_ticks_msec() < deadline:
        if SceneryStreamingServer.is_area_ready(0):
            break
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
func _on_exit_to_menu_pressed() -> void:
    $ExitConfirmation.popup_centered()
    # the vigilance button is on space, which must not confirm leaving the scenery
    $ExitConfirmation.get_cancel_button().grab_focus()


func _exit_to_menu() -> void:
    _play_music(MUSIC_MENU_VOLUME_DB)
    await $SpinnerOverlay.fade_in(EXIT_FADE_TIME)
    $GameHud.visible = false
    SceneryStreamingServer.set_camera(null)
    await $Player.clear_start_train()
    $MaszynaSceneryNode.filename = ""
    await $MaszynaSceneryNode.load()
    await get_tree().create_timer(EXIT_SPINNER_HOLD_TIME).timeout
    $ScenerySelectorScreen.open()
    await $SpinnerOverlay.fade_out(EXIT_FADE_TIME)


## Music plays while no scenery is loaded (autoplay) and while a scenery loads, at the level the
## caller asks for
func _play_music(volume_db: float) -> void:
    if _music_tween:
        _music_tween.kill()
    $Music.volume_db = volume_db
    if not $Music.playing:
        $Music.play()


func _on_scenery_loaded(_first_train_id: String) -> void:
    _music_tween = create_tween()
    _music_tween.tween_property($Music, "volume_linear", 0.0, MUSIC_FADE_OUT_TIME)
    _music_tween.tween_callback($Music.stop)
