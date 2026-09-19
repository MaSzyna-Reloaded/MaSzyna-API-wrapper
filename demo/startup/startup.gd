extends Node

## First scene: shows the EU07 spinner while the main scene loads in a background thread and is
## added below. Then the spinner fades out to the background and the background fades out,
## fading the main scene (scenario selector) in.

const MAIN_SCENE: String = "res://demo_scenery_loading.tscn"
## Seconds the spinner stays after the main scene has loaded
const EXTRA_DISPLAY_TIME: float = 2.0
const FADE_TIME: float = 0.5


func _ready() -> void:
    ResourceLoader.load_threaded_request(MAIN_SCENE)


func _process(_delta: float) -> void:
    match ResourceLoader.load_threaded_get_status(MAIN_SCENE):
        ResourceLoader.THREAD_LOAD_LOADED:
            set_process(false)
            _show_main_scene(ResourceLoader.load_threaded_get(MAIN_SCENE))
        ResourceLoader.THREAD_LOAD_FAILED, ResourceLoader.THREAD_LOAD_INVALID_RESOURCE:
            set_process(false)
            push_error("Cannot load " + MAIN_SCENE)


## The main scene is added right away: its _ready() and first frames (shader compilation) stall
## the main thread while the spinner is still shown, not between the fades
func _show_main_scene(scene: PackedScene) -> void:
    var main_scene: Node = scene.instantiate()
    get_tree().root.add_child(main_scene)
    get_tree().current_scene = main_scene
    await get_tree().create_timer(EXTRA_DISPLAY_TIME).timeout
    await $SpinnerOverlay.fade_out_spinner(FADE_TIME)
    await $SpinnerOverlay.fade_out(FADE_TIME)
    queue_free()
