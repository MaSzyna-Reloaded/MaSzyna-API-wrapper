extends Node3D

## Seconds of the music fade out after a scenery has loaded
const MUSIC_FADE_OUT_TIME: float = 1.0
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


func _ready() -> void:
    if not $MaszynaSceneryNode.filename:
        $ScenerySelectorScreen.open()


func _on_scenery_selector_scenery_selected(
    filename: String, train_id: String, skin_overrides: Dictionary
) -> void:
    _play_music()
    $GameHud.visible = false
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


## A scenery is registered with SceneryStreamingServer, not built, so the world around the player
## is filled in afterwards. In game that means watching it pop in at a few fps, so it happens here
## instead - after _wait_for_cabin(), when the player is already in its vehicle and the streaming
## fills in the place the game actually starts at.
func _wait_for_streaming() -> void:
    var deadline: float = Time.get_ticks_msec() + STREAMING_WAIT_TIME * 1000.0
    var statistics: Dictionary = SceneryStreamingServer.get_statistics()
    while statistics["has_camera"] and Time.get_ticks_msec() < deadline:
        # passes == 0 means the first plan has not run yet, so an empty queue proves nothing
        if statistics["passes"] > 0 and statistics["pending_builds"] == 0:
            return
        await get_tree().process_frame
        statistics = SceneryStreamingServer.get_statistics()


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
    _play_music()
    await $SpinnerOverlay.fade_in(EXIT_FADE_TIME)
    $GameHud.visible = false
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
