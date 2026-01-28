extends PanelContainer

@export var autoclose_after_load:bool = true

const SCENE_PATH: String = "res://demo_3d.tscn"
var progress: Array[float] = []
func _ready():
    ResourceLoader.load_threaded_request(SCENE_PATH)
    visible = true
    $VBoxContainer.visible = true

func _process(delta: float) -> void:
    var status = ResourceLoader.load_threaded_get_status(SCENE_PATH, progress)
    match status:
        ResourceLoader.THREAD_LOAD_IN_PROGRESS:
            var pct = progress[0] * 100
            $%ProgressBar.value = pct
        ResourceLoader.THREAD_LOAD_LOADED:
            var scene = ResourceLoader.load_threaded_get(SCENE_PATH)
            get_tree().change_scene_to_packed(scene)
