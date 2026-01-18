extends PanelContainer
signal fadein_finished

@export var autoclose_after_load:bool = true

func _ready():
    visible = true
    $VBoxContainer.visible = true
    SceneryResourceLoader.loading_request.connect(_do_update)
    SceneryResourceLoader.loading_finished.connect(_do_update)
    SceneryResourceLoader.scenery_loaded.connect(_on_scenery_loaded)
    $AnimationPlayer.play("fade_in")

func _do_update():
    $%ProgressBar.value = SceneryResourceLoader.files_loaded
    $%ProgressBar.max_value = SceneryResourceLoader.files_to_load
    $%Message.text = SceneryResourceLoader.current_task

func _on_scenery_loaded():
    SceneryResourceLoader.reset()
    _do_update()

    if visible and autoclose_after_load:
        $AnimationPlayer.play("dissolve")


func _on_animation_player_animation_finished(anim_name: StringName) -> void:
    if anim_name == "fade_in":
        fadein_finished.emit()
