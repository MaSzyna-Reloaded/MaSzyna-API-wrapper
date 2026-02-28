extends TrainSound

@export_node_path("TrainController") var controller_path:NodePath = NodePath("../SM42v1"):
    set(x):
        if not x == controller_path:
            controller_path = x
            _controller = null
  
var engine_sounds
var _controller:TrainController

func _ready() -> void:
    sound_root_path = UserSettings.get_maszyna_game_dir() + "/sounds/"
    _controller = $"..".get_controller()
    #TODO: Play it on actual engine start
    play_sound_type(TrainSound.SOUND_ENGINE)

func _process(delta: float) -> void:
    update_sound_type(TrainSound.SOUND_ENGINE, _controller.state.get("engine_rpm", 0.0))
    pass
