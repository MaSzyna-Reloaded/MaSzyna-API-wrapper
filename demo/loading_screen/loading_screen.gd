extends PanelContainer
signal fadein_finished

@export var autoclose_after_load:bool = true
func _ready():
    visible = true
    $VBoxContainer.visible = true
    #ActionQueue.action_requested.connect(_do_update)
    #ActionQueue.action_finished.connect(_do_update)
    #ActionQueue.queue_finished.connect(_on_scenery_loaded)
    $AnimationPlayer.play("fade_in")

func _do_update():
    pass
    #$%ProgressBar.value = ActionQueue.get_finished_items()
    #$%ProgressBar.max_value = ActionQueue.get_queue_size()
    #$%Message.text = SceneryResourceLoader.current_task

func _on_scenery_loaded():
    #ActionQueue.clear()
    _do_update()

    if visible and autoclose_after_load:
        $AnimationPlayer.play("dissolve")


func _on_animation_player_animation_finished(anim_name: StringName) -> void:
    if anim_name == "fade_in":
        fadein_finished.emit()
