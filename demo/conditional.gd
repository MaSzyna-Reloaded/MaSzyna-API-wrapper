extends AudioStreamPlayer3D
class_name ConditionalAudioStreamPlayer3D

@export_node_path("TrainController") var train_controller_path:NodePath = ""

#@export var stream:AudioStreamInteractive
@export var amplitude_offset:float = 0.0
@export var amplitude_factor:float = 1.0

@export var sound_attenuation_model:AudioStreamPlayer3D.AttenuationModel = AudioStreamPlayer3D.AttenuationModel.ATTENUATION_DISABLED:
    set(x):
        sound_attenuation_model = x
        _dirty = true

@export var sound_unit_size:float = 10.0:
    set(x):
        sound_unit_size = x
        _dirty = true
@export_range(0.0, 1.0, 0.01) var sound_panning_strength:float = 0.5:
    set(x):
        sound_panning_strength = x
        _dirty = true
@export var pitch_current = 0.0
@export var pitch_next = 0.0
@export var pitch_previous = 0.0
var _current = "start"
var _current_index = -1
var _train
var _dirty = false
var _ranges_to_index = []  # (start, end, clip_index)
var _starting = false
var _stopping = false
#var playing = false
var _last_parameter = 0.0
var _real_player:AudioStreamPlayer3D

@export var threshold_current = 0.0
@export var threshold_previous = 0.0
@export var threshold_next = -1.0

func _ready():
    _train = get_node(train_controller_path)

    #_real_player = self.duplicate(0)
    _real_player = self
    #add_child(_real_player)
    # FIXME: copy other params :/

    _real_player.connect("finished", _on_stream_playback_finished)
    var streams = stream as AudioStreamInteractive

    var sort_thresholds = func(a, b):
        return a[0] < b[0]

    if streams:
        var thresholds = []
        for i in streams.clip_count:
            var name = streams.get_clip_name(i)
            if name.is_valid_float():
                var param = name.to_float()
                thresholds.append([param, i])
        thresholds.sort_custom(sort_thresholds)
        var prev_from = 0.0
        var prev_clip = -1
        for x in thresholds:
            if prev_clip >= 0:
                _ranges_to_index.append([prev_from, x[0], prev_clip, x[0]])
            prev_clip = x[1]
            prev_from = x[0]
        if prev_clip >= 0:
            _ranges_to_index.append([prev_from, null, prev_clip, 1.0])


func _process(delta):
    if _dirty:
        _dirty = false
        #_real_player.attenuation_model = sound_attenuation_model
        #_real_player.unit_size = sound_unit_size
        #aaas_real_player.panning_strength = sound_panning_strength

func update(parameter:float):
    _last_parameter = parameter

    var streams = stream as AudioStreamInteractive
    if playing and not _starting and not _stopping and streams:

        var idx = 0
        for x in _ranges_to_index:
            var from = x[0]
            var to = x[1]
            var clip_index = x[2]
            var pitch = x[3]
            if parameter >= from and (to == null or parameter < to):
                if not _current_index == clip_index:
                    threshold_current = to
                    pitch_current = pitch
                    pitch_previous = _ranges_to_index[idx-1][3] if idx > 0 else 1.0
                    pitch_next = _ranges_to_index[idx-1][3] if idx < _ranges_to_index.size()-1 else 1.0
                    threshold_previous = _ranges_to_index[idx-1][1] if idx > 0 else 1.0
                    threshold_next = _ranges_to_index[idx-1][1] if idx < _ranges_to_index.size()-1 else null

                    _play_clip_by_index(clip_index)
    if _train:
        if not _starting and not _stopping:
            var _s = _train.state

            # example calc for engine sound

            # generic freq calc for non-combined samples
            #var freq = 0 + 1.0 * abs(_s.get("engine_rpm_count", 0.0)) * (60.0 * 0.01)
            var freq = 1.0

            if not threshold_current == null and parameter < threshold_current:
                freq = lerpf(
                    pitch_previous/pitch_current,
                    1.0,
                    clampf(
                        (parameter-threshold_previous)/(threshold_current-threshold_previous),
                        0.0,
                        1.0)
                    )
            elif not threshold_current == null and parameter > threshold_current:
                freq = lerpf(
                    1.0,
                    pitch_next/pitch_current,
                    clampf(
                        (parameter-threshold_current)/(threshold_next-threshold_current),
                        0.0,
                        1.0)
                    )
            else:
                freq = 1.0

            var vol = amplitude_offset + amplitude_factor * (0.25 * (_s.get("engine_power", 0.0) / _train.power) + 0.75 * _s.get("engine_rpm_ratio", 0.0))
            _real_player.pitch_scale = freq
            _real_player.max_db = vol * 4.0
            _real_player.volume_db = vol * 10.0 - 10.0


func _play_clip_by_name(name):
    # print("requested play clip by name: ", name)
    var streams = stream as AudioStreamInteractive
    if streams:
        for i in streams.clip_count:
            var x = streams.get_clip_name(i)
            if x == name:
                _play_clip_by_index(i)
                break

func _play_clip_by_index(index):
    var streams = stream as AudioStreamInteractive
    if streams:
        #_real_player.stream = streams.get_clip_stream(index)
        streams.instantiate_playback()
        if _real_player.stream:
            _current = streams.get_clip_name(index)
            _real_player.play(0.0)
            _current_index = index
        else:
            push_error("No stream associated for index ", index)


func start_playing():
    if playing and not _stopping:
        return
    _starting = true
    _stopping = false
    playing = true
    #print("starting...")
    #set("parameters/switch_to_clip", "start")
    _play_clip_by_name("start")


func stop_playing():
    if not playing or _stopping:
        return

    _stopping = true
    #print(self, " stopping...")
    #set("parameters/switch_to_clip", "stop")
    _play_clip_by_name("stop")
    _current_index = -1


func _on_stream_playback_finished():
    #print("finished playing ", _current)
    if _starting:
        #print("started.")
        _starting = false
        update(_last_parameter)
    elif _stopping:
        #print("stopped.")
        _stopping = false
        playing = false
