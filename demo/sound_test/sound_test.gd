extends Node3D
var engine_sounds: LayeredSoundResource = null
@onready var player_1 = $AudioStreamPlayer3D
@onready var player_2 = $AudioStreamPlayer3D2
@onready var sounds = $TrainSound
@export var MAX_VOLUME_DB: float = 0.0
@export var MIN_VOLUME_DB = -80.0
func _ready():
    var _sounds = LayeredSoundResource.new()
    _sounds.starting_sounds = ["697_sm42-start-v2.ogg"]
    _sounds.ending_sounds = ["697_sm42-stop.ogg"]
    _sounds.range = 150
    _sounds.amplitude_factor = 0.8
    _sounds.amplitude_offset = 1.2
    _sounds.sound_table = {
        495: "697_sm42-idle-1.ogg",
        600: "697_sm42-idle-2.ogg",
        700: "697_sm42-idle-3.ogg",
        850: "697_sm42-idle-4.ogg",
        1000: "697_sm42-idle-5.ogg"
    }
    
    _sounds.pitch_table = {
        495: 495,
        600: 600,
        700: 700,
        850: 850,
        1000: 1000
    }
    _sounds.cross_fade = 90.0
    engine_sounds = _sounds
    sounds.sound_root_path = UserSettings.get_maszyna_game_dir() + "sounds/"
func _process(delta: float) -> void:
    #TODO: Gather AudioStreamPlayer from LayeredSoundResource and make a copy of it as stream_player_2 - WIP
    #TODO: Apply pitch_table properly
    var rpm = $HSlider.value
    $Label.text = "RPM: %s" % rpm
    sounds.update_audio_streams_and_volumes(rpm, engine_sounds, player_1, player_2)
