extends Control
var engine_sounds: LayeredSoundResource = null
@onready var player_1 = $AudioStreamPlayer3D
@onready var player_2 = $AudioStreamPlayer3D2
@onready var sounds = $TrainSound
@export var MAX_VOLUME_DB: float = 0.0
@export var MIN_VOLUME_DB = -80.0
func _ready():
    engine_sounds = sounds.get_engine_sounds()
    
func _process(delta: float) -> void:
    #TODO: Gather AudioStreamPlayer from LayeredSoundResource and make a copy of it as stream_plauyer_2
    var rpm = $HSlider.value
    $Label.text = "RPM: %s" % rpm
    _update_audio_streams_and_volumes(rpm, engine_sounds.sound_table, engine_sounds.sound_table.keys(), engine_sounds.cross_fade)

func _update_audio_streams_and_volumes(param_value: float, audio_map: Dictionary, sorted_parameter_keys: Array, crossfade_threshold_percent: int):
    if sorted_parameter_keys.size() == 0: return
    var idx_min = -1
    for i in range(sorted_parameter_keys.size() - 1, -1, -1):
        if param_value >= sorted_parameter_keys[i]:
            idx_min = i
            break

    if idx_min == -1:
        idx_min = 0

    var param_min = sorted_parameter_keys[idx_min]
    var stream_min = sounds.get_audio_stream_for_file(UserSettings.get_maszyna_game_dir() + "sounds/" + str(audio_map[int(param_min)]), true)
    var param_max: float
    var stream_max = null # null means max rpm scope
    var idx_max = idx_min + 1
    if idx_max < sorted_parameter_keys.size():
        param_max = sorted_parameter_keys[idx_max]
        stream_max = sounds.get_audio_stream_for_file(UserSettings.get_maszyna_game_dir() + "sounds/" + str(audio_map[int(param_max)]), true)
    else:
        param_max = param_min + 1

    var p_min: AudioStreamPlayer2D
    var p_max: AudioStreamPlayer2D
    if idx_min % 2 == 0:
        p_min = player_1
        p_max = player_2
    else:
        p_min = player_2
        p_max = player_1

    if p_min.stream != stream_min:
        p_min.stream = stream_min
        print_verbose("Assigned stream_min (%s) to %s" % [stream_min.resource_path.get_file(), p_min.name])
        if not p_min.playing: p_min.play()

    if stream_max != null:
        if p_max.stream != stream_max:
            p_max.stream = stream_max
            print_verbose("Assigned stream_max (%s) to %s" % [stream_max.resource_path.get_file(), p_max.name])
            if not p_max.playing: p_max.play()
    else:
        if p_max.stream != null:
            p_max.stream = null
            print_verbose("Cleared stream from %s (highest RPM range)" % p_max.name)

    var progress: float = 0.0
    if stream_max != null and param_max > param_min:
        if (crossfade_threshold_percent > 100 || crossfade_threshold_percent < 0):
            push_error("[TrainSound] Crossfade treshold must be within a range between 0 and 100")
            return
        var crossfade_start = param_min + (param_max - param_min) * ((1-crossfade_threshold_percent) / 100.0)
        if (param_value > crossfade_start):
            progress = clamp(inverse_lerp(param_min, param_max, param_value), 0.0, 1.0)
            var vol_min_linear: float = 1.0 - progress
            var vol_max_linear: float = progress
            p_min.volume_db = linear_to_db(vol_min_linear)
            p_max.volume_db = linear_to_db(vol_max_linear)
    else:
        p_min.volume_db = MAX_VOLUME_DB
        p_max.volume_db = MIN_VOLUME_DB
    #print("RPM: %.1f | Range: [%d, %s] | Idx: %d | Progress: %.2f | p_min(%s): %.1f dB | p_max(%s): %.1f dB" % [current_rpm, rpm_min, str(rpm_max) if stream_max else "inf", idx_min, progress, p_min.name, p_min.volume_db, p_max.name, p_max.volume_db])
