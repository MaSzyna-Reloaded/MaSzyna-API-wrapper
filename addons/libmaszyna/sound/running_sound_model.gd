extends RefCounted
class_name RunningSoundModel

## Running sounds of one vehicle bank: the original per-frame gain/pitch formulas, driven by the
## controller state/config and the track under the vehicle. It only decides what each event should
## do; TrainSoundSystem plays it. Combined (chunked) sounds get the chunk selector as `point`,
## single samples get the playback `pitch` (sound.cpp:478 compute_combined_point()).

## A clatter axle without a click this update has no result, so its playing one-shot runs to its end.
enum Action {STOP, LOOP, ONE_SHOT}

const GRAVITY:float = 9.81
## Track3D.TrackEnvironment
const _ENVIRONMENT_BRIDGE:int = 1
const _ENVIRONMENT_TUNNEL:int = 2

## {"event": StringName, "source": MmdSoundSourceDefinition}; a chunked wheel clatter axle has no
## event of its own but one one-shot event per chunk in "chunk_events", in sorted_chunks() order
var sources:Array[Dictionary] = []

var _motor_momentum:float = 0.0
var _motor_volume:float = 0.0
var _rail_length:float = 0.0
var _axle_distances:Dictionary[StringName, float] = {}
## looping events started and not stopped yet - only these get a STOP
var _playing:Dictionary[StringName, bool] = {}
var _labels:Dictionary[String, bool] = {}


## Returns event name -> {"action": Action, "parameters": Dictionary, "source": definition}, only
## for events that play or have to stop. `outer_noise_audible` is false for the consist the
## listener drives from a cab (DynObj.cpp:4632-4640).
func update(
        controller:VehicleController, state:Dictionary, delta:float,
        outer_noise_audible:bool) -> Dictionary:
    if not _labels:
        for entry:Dictionary in sources:
            _labels[(entry["source"] as MmdSoundSourceDefinition).label] = true
    var speed:float = float(state.get("speed", 0.0))
    # a standing vehicle with still wheels and fan plays nothing - every formula below gives silence
    if (speed <= 0.0 and not _playing and absf(float(state.get("wheel_rotation_speed_rps", 0.0))) <= 0.01
            and float(state.get("resistor_fan_rotation", 0.0)) <= 0.1):
        _motor_volume = 0.0
        return {}
    var config:Dictionary = controller.config
    # the track is only read by sounds of a moving vehicle, the curve radius only above 5 km/h
    var shape:Dictionary = {}
    if speed > 0.0 and (_labels.has("wheel_clatter") or _labels.has("curve")
            or _labels.has("outernoise") or _labels.has("runningnoise")):
        shape = RailVehicleServer.vehicle_get_track_position(controller.get_rid())
    if speed > 5.0 and _labels.has("curve"):
        shape.merge(RailVehicleServer.vehicle_get_curve(
                controller.get_rid(), float(config.get("bogie_pivot_spacing", 0.0))))
    var track_rid:RID = shape.get("track_rid", RID())
    var quality_volume:float = lerpf(
            0.8, 1.2, clampf(TrackManager.track_get_quality_flag(track_rid) / 20.0, 0.0, 1.0))

    var results:Dictionary = {}
    var levels:Dictionary = {}
    for entry:Dictionary in sources:
        var source:MmdSoundSourceDefinition = entry["source"]
        var event_name:StringName = entry["event"]
        if source.label == "wheel_clatter":
            _wheel_clatter(entry, state, delta, shape, quality_volume, results)
            continue
        if not levels.has(source.label):
            levels[source.label] = _level(source, state, config, delta, shape, quality_volume, outer_noise_audible)
        var level:Array = levels[source.label]
        if level:
            _playing[event_name] = true
            results[event_name] = _result(source, level)
        elif _playing.has(event_name):
            _playing.erase(event_name)
            results[event_name] = {"action": Action.STOP, "parameters": {}, "source": source}
    return results


## [frequency, volume], or an empty array when the sound is stopped.
func _level(
        source:MmdSoundSourceDefinition, state:Dictionary, config:Dictionary, delta:float,
        shape:Dictionary, quality_volume:float, outer_noise_audible:bool) -> Array:
    match source.label:
        "tractionmotor": return _traction_motor(source, state, config, delta)
        "ventilator": return _ventilator(source, state, config)
        "curve": return _curve(source, state, config, shape)
        "outernoise": return _outer_noise(source, state, config, quality_volume, outer_noise_audible)
        "runningnoise": return _running_noise(source, state, config, quality_volume)
    return []


## DynObj.cpp:7933-8010, amplitude divisor from DynObj.cpp:5714
func _traction_motor(
        source:MmdSoundSourceDefinition, state:Dictionary, config:Dictionary, delta:float) -> Array:
    var power:float = float(config.get("power", 0.0))
    var wheel_revolutions:float = absf(float(state.get("wheel_rotation_speed_rps", 0.0)))
    if power <= 0.0 or wheel_revolutions <= 0.01:
        _motor_volume = 0.0
        return []
    var max_rpm:float = float(state.get("circuit_nmax_rpm", 0.0))
    var engine_power:float = float(state.get("engine_power", 0.0))
    var engine_type:int = int(state.get("engine_type", VehicleEngine.NONE))
    # combined motor sound selects its chunks in motor rpm
    var normalizer:float = 60.0 * 0.01 if _is_combined(source) else 1.0
    var motor_revolutions:float = wheel_revolutions * float(config.get("transmission_ratio", 1.0))
    var frequency:float = source.frequency_offset + source.frequency_factor * motor_revolutions * normalizer
    var amplitude_factor:float = source.amplitude_factor / (max_rpm + power * 3.0)
    var volume:float = source.amplitude_offset + amplitude_factor * motor_revolutions * 60.0
    if engine_type == VehicleEngine.ELECTRIC_INDUCTION_MOTOR:
        volume = source.amplitude_offset + amplitude_factor * (engine_power + motor_revolutions * 2.0)
    elif engine_type == VehicleEngine.ELECTRIC_SERIES_MOTOR:
        volume = source.amplitude_offset + amplitude_factor * (engine_power + motor_revolutions * 60.0)
    if engine_type == VehicleEngine.ELECTRIC_SERIES_MOTOR:
        if volume < 1.0 and engine_power < 100.0:
            var variation:float = (
                    randf_range(0.0, 100.0) * float(state.get("engine_rpm_count", 0.0))
                    / (1.0 + max_rpm / 60.0))
            if variation < 2.0:
                volume += variation / 200.0
        if bool(state.get("dynamic_brake_active", false)) and engine_power > 0.1:
            volume += 0.8
    _motor_momentum = clampf(
            _motor_momentum - delta + absf(float(state.get("Mm", 0.0))) / 60.0 * delta, 0.0, 1.25)
    volume *= maxf(0.25, _motor_momentum)
    _motor_volume = lerpf(_motor_volume, volume, 0.25)
    if _motor_volume < 0.05:
        return []
    return [frequency, _motor_volume]


## DynObj.cpp:8081-8092, divisors from DynObj.cpp:5806
func _ventilator(source:MmdSoundSourceDefinition, state:Dictionary, config:Dictionary) -> Array:
    var rotation:float = float(state.get("resistor_fan_rotation", 0.0))
    var max_rpm:float = float(config.get("resistor_fan_max_rpm", 0.0))
    if rotation <= 0.1 or max_rpm <= 0.0:
        return []
    return [
        source.frequency_offset + source.frequency_factor / max_rpm * rotation,
        source.amplitude_offset + source.amplitude_factor / max_rpm * rotation,
    ]


## DynObj.cpp:4735-4763. AccN = V^2/R + g*dHrail/TrackW (Mover.cpp:1312); the cant always lifts the
## outer rail, so it is taken against the centripetal part by magnitude.
func _curve(
        source:MmdSoundSourceDefinition, state:Dictionary, config:Dictionary, shape:Dictionary) -> Array:
    var speed:float = float(state.get("speed", 0.0))
    var radius:float = absf(float(shape.get("radius", 0.0)))
    if speed <= 5.0 or radius * radius <= 1.0 or radius >= 15000.0:
        return []
    var velocity:float = float(state.get("velocity", 0.0))
    var track_width:float = maxf(float(config.get("track_width", 1.435)), 0.001)
    var lateral_acceleration:float = absf(
            velocity * velocity / radius - GRAVITY * absf(float(shape.get("cant", 0.0))) / track_width)
    var volume:float = lateral_acceleration * lerpf(0.5, 1.0, clampf(speed / 40.0, 0.0, 1.0))
    var track_rid:RID = shape.get("track_rid", RID())
    if TrackManager.track_is_switch(track_rid) and radius < 1500.0:
        volume *= 100.0
    if volume <= 0.05:
        return []
    var frequency:float = (
            speed * 0.01 if _is_combined(source)
            else source.frequency_offset + source.frequency_factor)
    return [frequency, 2.5 * volume]


## DynObj.cpp:4630-4720, divisors from DynObj.cpp:6129
func _outer_noise(
        source:MmdSoundSourceDefinition, state:Dictionary, config:Dictionary,
        quality_volume:float, outer_noise_audible:bool) -> Array:
    var speed:float = float(state.get("speed", 0.0))
    if speed <= 0.5 or not outer_noise_audible:
        return []
    var max_speed:float = float(config.get("max_speed", 0.0))
    var combined:bool = _is_combined(source)
    var normalizer:float = max_speed * 0.01 if combined else 1.0
    var frequency:float = source.frequency_offset + source.frequency_factor / (1.0 + max_speed) * speed * normalizer
    var volume:float = source.amplitude_offset + source.amplitude_factor / (1.0 + max_speed) * speed
    # DynObj.cpp:4412-4420 - the ratio is only non-zero for a braking, moving vehicle
    if float(state.get("brake_unit_force", 0.0)) > 10.0 and speed > 0.05:
        volume *= 1.0 + 0.125 * float(state.get("brake_force_ratio", 0.0))
    volume *= quality_volume
    if not combined:
        volume *= clampf(speed / 40.0, 0.0, 1.0)
    if volume <= 0.05:
        return []
    return [frequency, volume]


## Train.cpp:8272-8282 and update_sounds_runningnoise(), frequency divisor from Train.cpp:8636
func _running_noise(
        source:MmdSoundSourceDefinition, state:Dictionary, config:Dictionary, quality_volume:float) -> Array:
    var speed:float = float(state.get("speed", 0.0))
    if speed <= 0.5:
        return []
    var max_speed:float = float(config.get("max_speed", 0.0))
    var combined:bool = _is_combined(source)
    var normalizer:float = max_speed * 0.01 if combined else 1.0
    var frequency:float = source.frequency_offset + source.frequency_factor / (1.0 + max_speed) * speed * normalizer
    var volume:float = source.amplitude_offset + source.amplitude_factor * lerpf(speed / (1.0 + max_speed), 1.0, 0.5)
    if absf(float(state.get("wheel_rotation_speed_rps", 0.0))) > 0.01:
        volume *= 1.0 + 0.125 * float(state.get("brake_force_ratio", 0.0))
    volume *= quality_volume
    if not combined:
        volume *= clampf(speed / 25.0, 0.0, 1.0)
    if volume <= 0.05:
        return []
    return [frequency, volume]


## DynObj.cpp:3477-3545 - each axle counts its way along the rail and clicks when it passes a joint.
## The axles start phased by their position on the track (DynObj.cpp:2283), a later rail length
## change re-phases them by their offsets.
func _wheel_clatter(
        entry:Dictionary, state:Dictionary, delta:float, shape:Dictionary, quality_volume:float,
        results:Dictionary) -> void:
    var source:MmdSoundSourceDefinition = entry["source"]
    var event_name:StringName = entry["event"]
    var track_rid:RID = shape.get("track_rid", RID())
    var sound_distance:float = TrackManager.track_get_sound_distance(track_rid)
    if is_equal_approx(sound_distance, -1.0):
        return
    if not is_equal_approx(sound_distance, _rail_length):
        for axle_entry:Dictionary in sources:
            var axle_offset:float = (axle_entry["source"] as MmdSoundSourceDefinition).offset.z
            if not (axle_entry["source"] as MmdSoundSourceDefinition).label == "wheel_clatter":
                continue
            if _rail_length > 0.0:
                _axle_distances[axle_entry["event"]] = axle_offset
            elif sound_distance > 0.0:
                _axle_distances[axle_entry["event"]] = fposmod(
                        axle_offset + float(shape.get("along", 0.0)), sound_distance)
        _rail_length = sound_distance
    var speed:float = float(state.get("speed", 0.0))
    if _rail_length <= 0.0 or speed <= 0.0:
        return
    var distance:float = float(_axle_distances.get(event_name, 0.0)) + float(state.get("velocity", 0.0)) * delta
    _axle_distances[event_name] = distance
    if distance >= 0.0 and distance <= _rail_length:
        return
    _axle_distances[event_name] = fposmod(distance, _rail_length)
    if speed <= 0.1:
        return
    var volume:float = quality_volume
    match TrackManager.track_get_environment(track_rid):
        _ENVIRONMENT_TUNNEL: volume *= 1.1
        _ENVIRONMENT_BRIDGE: volume *= 1.2
    if not _is_combined(source):
        results[event_name] = {
            "action": Action.ONE_SHOT,
            "parameters": {&"gain": clampf(volume, 0.0, 2.0), &"pitch": 1.0},
            "source": source,
        }
        return
    # the combined clatter is selected by the speed (DynObj.cpp:3525)
    var chunk_events:Array = entry["chunk_events"]
    for chunk:Dictionary in chunk_levels(source, combined_point(speed * 0.01)):
        results[chunk_events[chunk["index"]]] = {
            "action": Action.ONE_SHOT,
            "parameters": {&"gain": clampf(volume * chunk["gain"], 0.0, 2.0), &"pitch": chunk["pitch"]},
            "source": source,
        }


func _result(source:MmdSoundSourceDefinition, level:Array) -> Dictionary:
    var frequency:float = level[0]
    var parameters:Dictionary = {&"gain": clampf(level[1], 0.0, 2.0)}
    if _is_combined(source):
        parameters[&"point"] = combined_point(frequency)
    else:
        parameters[&"pitch"] = frequency
    return {"action": Action.LOOP, "parameters": parameters, "source": source}


## Chunks sorted by threshold, with the fade in/out points of sound_source::deserialize()
## (sound.cpp:56-85, Chunkrange 100).
static func sorted_chunks(source:MmdSoundSourceDefinition) -> Array[Dictionary]:
    var chunks:Array[Dictionary] = source.chunks.duplicate(true)
    chunks.sort_custom(func(a:Dictionary, b:Dictionary) -> bool: return int(a["threshold"]) < int(b["threshold"]))
    for idx:int in range(chunks.size()):
        var threshold:float = float(chunks[idx]["threshold"])
        if idx == 0:
            chunks[idx]["fadein"] = maxf(0.0, threshold)
        else:
            var previous_threshold:float = float(chunks[idx - 1]["threshold"])
            chunks[idx]["fadein"] = threshold - 0.01 * source.crossfade_percent * (threshold - previous_threshold)
        chunks[idx]["fadeout"] = (
                float(chunks[idx + 1]["threshold"]) if idx < chunks.size() - 1 else maxf(100.0, threshold))
    return chunks


## Chunks a combined sound starts at `point`, with the per-chunk gain and pitch ratio of
## sound_source::play_combined()/update_crossfade() (sound.cpp:427-475, 761-850).
static func chunk_levels(source:MmdSoundSourceDefinition, point:float) -> Array[Dictionary]:
    var levels:Array[Dictionary] = []
    var chunks:Array[Dictionary] = sorted_chunks(source)
    for idx:int in range(chunks.size()):
        var chunk:Dictionary = chunks[idx]
        if point < float(chunk["fadein"]):
            break
        if point >= float(chunk["fadeout"]):
            continue
        var threshold:float = float(chunk["threshold"])
        var pitch:float = 1.0
        if point < threshold:
            if idx > 0:
                var previous:Dictionary = chunks[idx - 1]
                pitch = lerpf(
                        _chunk_pitch(previous) / _chunk_pitch(chunk), 1.0,
                        clampf((point - float(previous["threshold"])) / (threshold - float(previous["threshold"])), 0.0, 1.0))
        elif idx < chunks.size() - 1:
            var following:Dictionary = chunks[idx + 1]
            pitch = lerpf(
                    1.0, _chunk_pitch(following) / _chunk_pitch(chunk),
                    clampf((point - threshold) / (float(following["threshold"]) - threshold), 0.0, 1.0))
        var gain:float = 1.0
        if source.crossfade_percent > 0:
            if idx > 0 and point < threshold:
                var fade_in:float = clampf((point - float(chunk["fadein"])) / (threshold - float(chunk["fadein"])), 0.0, 1.0)
                gain = fade_in / (1.0 + (1.0 - fade_in) * -0.57)
            elif idx < chunks.size() - 1:
                var fadeout_start:float = float(chunks[idx + 1]["fadein"])
                if point > fadeout_start:
                    var fade_out:float = clampf(
                            (point - fadeout_start) / (float(chunk["fadeout"]) - fadeout_start), 0.0, 1.0)
                    gain = (1.0 - fade_out) / (1.0 + fade_out * -0.57)
        levels.append({"index": idx, "gain": gain, "pitch": pitch})
    return levels


## a chunk without pitchN: plays at its own pitch
static func _chunk_pitch(chunk:Dictionary) -> float:
    var pitch:float = float(chunk.get("pitch", 0.0))
    return pitch if pitch > 0.0 else 1.0


func _is_combined(source:MmdSoundSourceDefinition) -> bool:
    return not not source.chunks


## sound.cpp:478 - values up to 1.0 are a 0-1 fraction of the 0-100 chunk range
static func combined_point(frequency:float) -> float:
    return (clampf(frequency, 0.0, 0.99) if frequency <= 1.0 else maxf(0.0, frequency)) * 100.0
