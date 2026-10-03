extends Node3D

## Seconds of the music fade out after a scenery has loaded
const MUSIC_FADE_OUT_TIME: float = 1.0
## Music in the menu, and the little it gains while a scenery loads - 3 dB, not the jump to full
## scale the loading used to make
const MUSIC_MENU_VOLUME_DB: float = -6.0
const MUSIC_LOADING_VOLUME_DB: float = -3.0
## Seconds of the loading screen fade out into the game
const LOADING_FADE_OUT_TIME: float = 1.0
## Seconds into the loading screen fade out before the world starts - the simulation and its sound
## starting together with the fade make its first frames stutter
const SIMULATION_START_DELAY: float = 0.5
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
## Command line of the original's eu07.exe, as its Starter.exe gives it: "-s <scenery>.scn" starts
## that scenery of scenery/ without the selector, "-v <vehicle>" puts the player in that vehicle.
## An exported game leaves "-s <scenery>" alone (its templates run no script from the command line,
## main.cpp), but takes "-v" for itself as --verbose and leaves only its value - so a vehicle
## without its "-v" is the one argument that is neither a flag nor the scenery. The editor's binary
## (the editor, tests: "-s addons/gut/gut_cmdln.gd") runs "-s" as a script, so there the engine's
## arguments are never read and the two come after Godot's "--" (OS.get_cmdline_user_args()).
const ARG_SCENERY: String = "-s"
const ARG_VEHICLE: String = "-v"
## The 3D world of a scenery - made when one is chosen, freed in the menu, which renders nothing
## behind it
const WORLD_SCENE: PackedScene = preload("world/world.tscn")

## A scenery started at once, without the selector - as "-s" on the command line does
@export var scenery: String = ""

var _music_tween: Tween
## The world of the scenery being played, null in the menu
var _world: SceneryWorld = null
## The trainset chosen in the selector, handed to the player once the scenery is loaded
var _chosen_train_id: String = ""


## Before _ready(): the children must not read a cache left by another build
func _enter_tree() -> void:
    GameDataServer.build_check_version()


func _ready() -> void:
    # the dialog's own buttons, like the card it is drawn as (timetable_theme.tres)
    $ExitConfirmation.get_ok_button().theme_type_variation = &"CardButtonDefault"
    $ExitConfirmation.get_cancel_button().theme_type_variation = &"CardButton"
    var exported: bool = OS.has_feature("template")
    var args: PackedStringArray = OS.get_cmdline_user_args()
    if exported:
        args = OS.get_cmdline_args() + args
    var scenery_at: int = args.find(ARG_SCENERY)
    if scenery_at >= 0 and scenery_at + 1 < args.size():
        var train_id: String = ""
        var vehicle_at: int = args.find(ARG_VEHICLE)
        if vehicle_at >= 0 and vehicle_at + 1 < args.size():
            train_id = args[vehicle_at + 1]
        elif exported and OS.is_stdout_verbose():
            for at: int in args.size():
                if not at == scenery_at + 1 and not args[at].begins_with("-"):
                    train_id = args[at]
                    break
        start_scenery(args[scenery_at + 1], train_id, {})
    elif scenery:
        start_scenery(scenery, "", {})
    else:
        $ScenerySelectorScreen.open()
        $BugReport.show_edge_button()


## Loads scenery/<filename> and puts the player in train_id (none: the scenery's own driver) -
## chosen in the selector, or given on the command line
func start_scenery(filename: String, train_id: String, skin_overrides: Dictionary) -> void:
    _play_music(MUSIC_LOADING_VOLUME_DB)
    # the world starts while the loading screen fades out, not when it is built, and at the wall
    # clock's speed whatever the last one ran at
    SimulationServer.simulation_pause()
    SimulationServer.simulation_reset_speed()
    HUDServer.hud_set_visible(false)
    $BugReport.hide_edge_button()
    # The camera moves to the selected vehicle only after loading; planning before that point
    # streams the empty menu position and puts irrelevant work ahead of the starting area.
    SceneryStreamingServer.streaming_set_camera(null)
    var info: MaszynaSceneryInfo = MaszynaSceneryInfo.read(filename)
    $GameHud.show_scenario(info, train_id)
    # the name the scenery list gave it ("//$l", "//$n" and the file name)
    $LoadingScreen.show_loading(MaszynaSceneryInfo.read_display_name(filename))
    # the selector dissolves into the loading screen and hides once it is done; the loading below
    # blocks the main thread, so it waits for the dissolve not to stutter
    if $ScenerySelectorScreen.visible:
        await $ScenerySelectorScreen.hidden
    # made under the loading screen: its environment and first frames stall the main thread
    _world = WORLD_SCENE.instantiate() as SceneryWorld
    _world.load_progress.connect($LoadingScreen.set_progress)
    _world.load_files_parsed.connect($LoadingScreen.set_files)
    _world.scenery_loaded.connect(_on_scenery_loaded)
    add_child(_world)
    $GameHud.attach_environment(_world.get_environment())
    $BugReport.attach_world(_world)
    _chosen_train_id = train_id
    await _world.load_scenery(filename, skin_overrides)
    await _wait_for_cabin()
    SceneryStreamingServer.streaming_set_camera(get_viewport().get_camera_3d())
    await _wait_for_streaming()
    var tween: Tween = create_tween()
    tween.tween_property($LoadingScreen, "modulate:a", 0.0, LOADING_FADE_OUT_TIME)
    tween.parallel().tween_callback(SimulationServer.simulation_unpause).set_delay(SIMULATION_START_DELAY)
    await tween.finished
    $LoadingScreen.visible = false
    $LoadingScreen.modulate.a = 1.0
    HUDServer.hud_set_visible(true)
    $BugReport.show_edge_button()


## The player gets its vehicle a few frames after the scenery is loaded, and the cabin is built
## a few frames later still - without this the game pops in half-built behind the loading screen
func _wait_for_cabin() -> void:
    var waited: int = 0
    while not PlayerServer.player_get_vehicle().is_valid() and waited < VEHICLE_WAIT_FRAMES:
        await get_tree().process_frame
        waited += 1
    for frame: int in CABIN_SETTLE_FRAMES:
        await get_tree().process_frame


## Wait only for the chunk containing the camera. Neighbouring chunks and the rest of the draw
## distance keep streaming after the game appears.
func _wait_for_streaming() -> void:
    var deadline: float = Time.get_ticks_msec() + STREAMING_WAIT_TIME * 1000.0
    while SceneryStreamingServer.streaming_has_camera() and Time.get_ticks_msec() < deadline:
        if SceneryStreamingServer.area_is_ready(0):
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
    # the world stops once the spinner covers it, and stays stopped until the next scenery shows;
    # the menu is heard at the wall clock's speed (TrainSoundSystem)
    SimulationServer.simulation_pause()
    SimulationServer.simulation_reset_speed()
    HUDServer.hud_set_visible(false)
    $BugReport.hide_edge_button()
    SceneryStreamingServer.streaming_set_camera(null)
    # the scenery's script context and environment go with the world
    $GameHud.attach_script_context(RID())
    $GameHud.attach_environment(null)
    $BugReport.attach_world(null)
    await _world.unload_scenery()
    _world.queue_free()
    _world = null
    # the world is gone a frame later, and the allocator keeps what it held until asked
    await get_tree().process_frame
    ProcessMemory.release_unused()
    SceneryLoadMeasurement.print_process("SceneryMemory", "menu")
    await get_tree().create_timer(EXIT_SPINNER_HOLD_TIME).timeout
    $ScenerySelectorScreen.open()
    $BugReport.show_edge_button()
    await $SpinnerOverlay.fade_out(EXIT_FADE_TIME)


## Music plays while no scenery is loaded (autoplay) and while a scenery loads, at the level the
## caller asks for
func _play_music(volume_db: float) -> void:
    if _music_tween:
        _music_tween.kill()
    $Music.volume_db = volume_db
    if not $Music.playing:
        $Music.play()


## The player takes the trainset only now: the trainsets are coupled, so the cab it activates on
## entering reaches every car of its unit (CabActivisation() sends to the coupled ones, Mover.cpp:2905).
## The trainset chosen in the selector; none chosen, the scenery's own driver.
func _on_scenery_loaded(first_train_id: String) -> void:
    _world.start_player(_chosen_train_id if _chosen_train_id else first_train_id)
    $GameHud.attach_script_context(_world.get_script_context())
    _music_tween = create_tween()
    _music_tween.tween_property($Music, "volume_linear", 0.0, MUSIC_FADE_OUT_TIME)
    _music_tween.tween_callback($Music.stop)
