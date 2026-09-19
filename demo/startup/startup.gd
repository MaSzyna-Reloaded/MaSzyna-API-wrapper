extends Control

## First scene: shows the EU07 spinner while the main scene loads in a background thread.

const MAIN_SCENE: String = "res://demo_scenery_loading.tscn"
## Seconds the spinner stays after the main scene has loaded
const EXTRA_DISPLAY_TIME: float = 2.0


func _ready() -> void:
    ResourceLoader.load_threaded_request(MAIN_SCENE)


func _process(_delta: float) -> void:
    match ResourceLoader.load_threaded_get_status(MAIN_SCENE):
        ResourceLoader.THREAD_LOAD_LOADED:
            set_process(false)
            var scene: PackedScene = ResourceLoader.load_threaded_get(MAIN_SCENE)
            await get_tree().create_timer(EXTRA_DISPLAY_TIME).timeout
            get_tree().change_scene_to_packed(scene)
        ResourceLoader.THREAD_LOAD_FAILED, ResourceLoader.THREAD_LOAD_INVALID_RESOURCE:
            set_process(false)
            push_error("Cannot load " + MAIN_SCENE)
