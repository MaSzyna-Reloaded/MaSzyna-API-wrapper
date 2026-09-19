extends PanelContainer

@export var autoclose_after_load:bool = true
## Scene loaded in the background and switched to once loaded; empty = overlay shown by show_loading()
@export var scene_path:String = ""

var progress: Array[float] = [0]

## Hidden without scene_path - an in-scene visible override is lost when the editor saves the
## scene (the RESET animation keys ".:visible" true)
func _ready():
    $VBoxContainer.visible = true
    visible = false
    if scene_path:
        ResourceLoader.load_threaded_request(scene_path)
        visible = true

func _process(delta: float) -> void:
    if not scene_path:
        return
    var status = ResourceLoader.load_threaded_get_status(scene_path, progress)
    match status:
        ResourceLoader.THREAD_LOAD_IN_PROGRESS:
            var pct = progress[0] * 100
            $%ProgressBar.value = pct
        ResourceLoader.THREAD_LOAD_LOADED:
            var scene = ResourceLoader.load_threaded_get(scene_path)
            get_tree().change_scene_to_packed(scene)

func show_loading(p_name: String) -> void:
    %Title.text = "Loading %s..." % p_name
    visible = true

func set_progress(p_progress: float, p_message: String) -> void:
    %ProgressBar.value = p_progress * 100.0
    %Message.text = p_message
