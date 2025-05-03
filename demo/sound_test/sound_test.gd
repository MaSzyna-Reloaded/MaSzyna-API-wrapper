extends Control
var _engine_audio_stream = false
func _ready():
    var sounds = $TrainSound
    var stream_player: AudioStreamPlayer2D = $AudioStreamPlayer3D
    var stream_player_2: AudioStreamPlayer2D = $AudioStreamPlayer3D2
    var engine_sound: LayeredSoundResource = sounds.get_engine_sounds()
    var rpm_495 = UserSettings.get_maszyna_game_dir() + "sounds/" + str(engine_sound.sound_table[495])
    var rpm_600 = UserSettings.get_maszyna_game_dir() + "sounds/" + str(engine_sound.sound_table[600])
    stream_player.stream = sounds.get_audio_stream_for_file(rpm_495, true)
    stream_player.play()
    stream_player.volume_db = -80.0
    stream_player_2.stream = sounds.get_audio_stream_for_file(rpm_600, true)
    stream_player_2.play()
    stream_player_2.volume_db = -80.0
    
func _process(delta: float) -> void:
    var stream_player: AudioStreamPlayer2D = $AudioStreamPlayer3D
    var stream_player_2: AudioStreamPlayer2D = $AudioStreamPlayer3D2
    var rpm = $HSlider.value
    var next_rpm = $HSlider.max_value
    var vol_1 = linear_to_db((next_rpm - rpm)/next_rpm)
    var vol_2 = linear_to_db(rpm/next_rpm)
    print("vol_1: %s, vol_2: %s" % [vol_1, vol_2])
    stream_player.volume_db = vol_1
    stream_player_2.volume_db = vol_2
    

func calc_crossfade(current, next, treshold):
    if (treshold == 100 || treshold == 0):
        return current
    if (treshold > 100 || treshold < 0):
        push_error("[TrainSound] Crossfade treshold must be within a range between 0 and 100")
        return
    return (next-current)+((next-current)*(treshold/100))
