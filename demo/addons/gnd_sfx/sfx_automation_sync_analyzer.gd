@tool
extends RefCounted
class_name SfxAutomationSyncAnalyzer

const ANALYSIS_SAMPLE_RATE := 4000.0
const MIN_PHASE_PERIOD := 0.05
const MAX_PHASE_PERIOD := 0.5
const MIN_TRACK_DURATION := 0.1
const PERIOD_SEARCH_RATIO := 0.5
const LOOP_SEARCH_PERIODS := 4.0
const LOOP_SEARCH_MIN := 0.2
const LOOP_SEARCH_MAX := 0.75
const LOOP_WINDOW_PERIODS := 1.5
const LOOP_WINDOW_MIN := 0.05
const LOOP_WINDOW_MAX := 0.25
const PHASE_HINT_WEIGHT := 0.05
const EARLY_LOOP_WEIGHT := 0.02
const EARLY_LOOP_ACCEPT_RATIO := 0.98
const EARLY_LOOP_MIN_SCORE := 0.05
const LOOP_CYCLE_WEIGHT := 0.2
const SHARED_LOOP_WEIGHT := 0.35
const PERIOD_CLUSTER_ABSOLUTE_TOLERANCE := 0.02
const PERIOD_CLUSTER_RELATIVE_TOLERANCE := 0.08


static func analyze_automation(automation: SfxAutomation) -> Dictionary:
    var track_data: Array[Dictionary] = []
    var warnings: PackedStringArray = []

    for track in automation.tracks:
        if track == null or track.stream == null:
            continue

        var decoded := _decode_stream_to_mono(track.stream)
        if decoded.is_empty():
            warnings.append("Skipped a track with unreadable audio stream.")
            continue

        var full_analysis := resample_and_normalize(decoded["samples"], decoded["sample_rate"], ANALYSIS_SAMPLE_RATE)
        var active_analysis := trim_samples(
            full_analysis["samples"],
            full_analysis["sample_rate"],
            maxf(track.stream_offset, 0.0)
        )
        active_analysis = normalize_samples(active_analysis)

        var active_duration := float(active_analysis.size()) / maxf(full_analysis["sample_rate"], 1.0)
        if active_duration < MIN_TRACK_DURATION:
            warnings.append("Skipped a track with too little usable audio after stream_offset.")
            continue

        track_data.append({
            "track": track,
            "stream_duration": track.stream.get_length(),
            "analysis_rate": full_analysis["sample_rate"],
            "full_samples": full_analysis["samples"],
            "active_samples": active_analysis,
            "loop_supported": _supports_loop_offset(track.stream),
        })

    if track_data.size() < 2:
        return {
            "ok": false,
            "error": "Need at least two tracks with readable audio to auto-sync.",
            "warnings": warnings,
        }

    var period_estimates: Array[float] = []
    for data in track_data:
        var sample_rate: float = data["analysis_rate"]
        var samples: PackedFloat32Array = data["active_samples"]
        var max_period := minf(MAX_PHASE_PERIOD, (float(samples.size()) / sample_rate) * PERIOD_SEARCH_RATIO)
        var estimate := estimate_period(samples, sample_rate, MIN_PHASE_PERIOD, max_period)
        if estimate > 0.0:
            period_estimates.append(estimate)
            data["local_period"] = estimate
        else:
            data["local_period"] = 0.0

    if period_estimates.size() < 2:
        return {
            "ok": false,
            "error": "Could not find a stable shared phase period from the available tracks.",
            "warnings": warnings,
        }

    track_data.sort_custom(func(a: Dictionary, b: Dictionary) -> bool:
        var track_a: SfxTrack = a["track"]
        var track_b: SfxTrack = b["track"]
        return track_a.offset < track_b.offset
    )

    var phase_period := _select_consensus_period(period_estimates)
    var phase_offsets := _resolve_track_phase_offsets(track_data, phase_period)
    var shared_loop_duration := _select_shared_loop_duration(track_data, phase_period)
    var results: Array[Dictionary] = []

    for index in range(track_data.size()):
        var data: Dictionary = track_data[index]
        var track: SfxTrack = data["track"]
        var phase_offset: float = phase_offsets[index]

        var track_result := {
            "track": track,
            "phase_offset": phase_offset,
        }

        if data["loop_supported"]:
            var loop_period: float = data["local_period"] if data["local_period"] > 0.0 else phase_period
            track_result["loop_offset"] = estimate_loop_offset(
                data["full_samples"],
                data["analysis_rate"],
                loop_period,
                maxf(track.stream_offset, 0.0),
                phase_offset,
                shared_loop_duration,
                data["stream_duration"]
            )
        else:
            warnings.append("Track %s uses a stream type without editable loop_offset." % [track.stream.resource_path])

        results.append(track_result)

    return {
        "ok": true,
        "phase_period": phase_period,
        "track_results": results,
        "warnings": warnings,
    }


static func normalize_samples(samples: PackedFloat32Array) -> PackedFloat32Array:
    var normalized := PackedFloat32Array()
    normalized.resize(samples.size())
    if samples.is_empty():
        return normalized

    var mean := 0.0
    for sample in samples:
        mean += sample
    mean /= float(samples.size())

    var peak := 0.0
    for index in range(samples.size()):
        var centered := samples[index] - mean
        normalized[index] = centered
        peak = maxf(peak, absf(centered))

    if peak <= 0.000001:
        return normalized

    for index in range(normalized.size()):
        normalized[index] /= peak
    return normalized


static func resample_and_normalize(
    samples: PackedFloat32Array,
    input_rate: float,
    target_rate: float
) -> Dictionary:
    if samples.is_empty():
        return {"samples": PackedFloat32Array(), "sample_rate": target_rate}

    var step := 1
    if input_rate > target_rate and target_rate > 0.0:
        step = max(1, int(round(input_rate / target_rate)))
    var output_rate := input_rate / float(step)
    var resized := PackedFloat32Array()
    resized.resize(int(ceil(float(samples.size()) / float(step))))

    for out_index in range(resized.size()):
        var start := out_index * step
        var end := min(start + step, samples.size())
        var value := 0.0
        for index in range(start, end):
            value += samples[index]
        resized[out_index] = value / maxf(float(end - start), 1.0)

    return {
        "samples": normalize_samples(resized),
        "sample_rate": output_rate,
    }


static func trim_samples(samples: PackedFloat32Array, sample_rate: float, start_seconds: float) -> PackedFloat32Array:
    var start_index := clampi(int(round(start_seconds * sample_rate)), 0, samples.size())
    return slice_samples(samples, start_index, samples.size() - start_index)


static func slice_samples(samples: PackedFloat32Array, start: int, length: int) -> PackedFloat32Array:
    var safe_start := clampi(start, 0, samples.size())
    var safe_end := clampi(safe_start + max(length, 0), safe_start, samples.size())
    var sliced := PackedFloat32Array()
    sliced.resize(safe_end - safe_start)
    for index in range(sliced.size()):
        sliced[index] = samples[safe_start + index]
    return sliced


static func estimate_period(
    samples: PackedFloat32Array,
    sample_rate: float,
    min_period_seconds: float,
    max_period_seconds: float
) -> float:
    if samples.size() < 4 or sample_rate <= 0.0:
        return 0.0

    var min_lag: int = max(1, int(round(min_period_seconds * sample_rate)))
    var max_lag: int = min(int(round(max_period_seconds * sample_rate)), (samples.size() / 2) - 1)
    if max_lag <= min_lag:
        return 0.0

    var best_lag := min_lag
    var best_score := -INF

    for lag in range(min_lag, max_lag + 1):
        var overlap := samples.size() - lag
        var numerator := 0.0
        var lhs_energy := 0.0
        var rhs_energy := 0.0
        for index in range(overlap):
            var lhs := samples[index]
            var rhs := samples[index + lag]
            numerator += lhs * rhs
            lhs_energy += lhs * lhs
            rhs_energy += rhs * rhs
        var denominator := sqrt(lhs_energy * rhs_energy)
        if denominator <= 0.000001:
            continue
        var score := numerator / denominator
        if score > best_score:
            best_score = score
            best_lag = lag

    return float(best_lag) / sample_rate


static func estimate_phase_offset(
    reference_samples: PackedFloat32Array,
    target_samples: PackedFloat32Array,
    sample_rate: float,
    period_seconds: float
) -> float:
    var period_samples: int = max(1, int(round(period_seconds * sample_rate)))
    if reference_samples.is_empty() or target_samples.is_empty() or period_samples <= 1:
        return 0.0

    var window: int = min(reference_samples.size(), target_samples.size(), period_samples * 4)
    if window <= 4:
        return 0.0

    var best_lag := 0
    var best_score := -INF

    for lag in range(period_samples):
        var numerator := 0.0
        var lhs_energy := 0.0
        var rhs_energy := 0.0
        for index in range(window):
            var lhs := reference_samples[index]
            var rhs := target_samples[(index + lag) % target_samples.size()]
            numerator += lhs * rhs
            lhs_energy += lhs * lhs
            rhs_energy += rhs * rhs
        var denominator := sqrt(lhs_energy * rhs_energy)
        if denominator <= 0.000001:
            continue
        var score := numerator / denominator
        if score > best_score:
            best_score = score
            best_lag = lag

    return float(best_lag) / sample_rate


static func estimate_loop_offset(
    samples: PackedFloat32Array,
    sample_rate: float,
    period_seconds: float,
    search_start_seconds := 0.0,
    phase_offset_seconds := 0.0,
    target_loop_duration_seconds := -1.0,
    stream_duration_seconds := -1.0
) -> float:
    if samples.is_empty() or sample_rate <= 0.0:
        return maxf(search_start_seconds, 0.0)

    var duration_seconds := stream_duration_seconds if stream_duration_seconds > 0.0 else float(samples.size()) / sample_rate
    var window_seconds := clampf(period_seconds * LOOP_WINDOW_PERIODS, LOOP_WINDOW_MIN, minf(LOOP_WINDOW_MAX, duration_seconds * 0.25))
    var window_samples: int = max(8, int(round(window_seconds * sample_rate)))
    if window_samples >= samples.size():
        return maxf(search_start_seconds, 0.0)

    var tail_start: int = samples.size() - window_samples
    var tail_window := slice_samples(samples, tail_start, window_samples)

    var search_start: int = clampi(int(round(search_start_seconds * sample_rate)), 0, max(0, tail_start - window_samples))
    var search_duration := clampf(period_seconds * LOOP_SEARCH_PERIODS, LOOP_SEARCH_MIN, LOOP_SEARCH_MAX)
    var search_end: int = clampi(
        int(round((search_start_seconds + search_duration) * sample_rate)),
        search_start,
        max(0, tail_start - window_samples)
    )

    var best_index := search_start
    var best_score := -INF
    var candidate_scores: Array[Dictionary] = []
    var target_offset_seconds := duration_seconds - target_loop_duration_seconds

    if target_loop_duration_seconds > 0.0:
        var target_offset_samples := int(round(target_offset_seconds * sample_rate))
        return clampf(float(target_offset_samples) / sample_rate, 0.0, duration_seconds - window_seconds)

    for candidate in range(search_start, search_end + 1):
        var candidate_window := slice_samples(samples, candidate, window_samples)
        var seam_score := _normalized_dot(candidate_window, tail_window)
        var total_score := seam_score
        var candidate_seconds := float(candidate) / sample_rate
        var loop_duration := maxf(duration_seconds - candidate_seconds, 0.0)
        var duration_delta := INF
        if period_seconds > 0.0:
            var cycle_distance := _circular_distance(loop_duration, 0.0, period_seconds)
            var cycle_score := 1.0 - (cycle_distance / maxf(period_seconds * 0.5, 0.000001))
            total_score += maxf(cycle_score, 0.0) * LOOP_CYCLE_WEIGHT
            var circular_distance := _circular_distance(candidate_seconds, phase_offset_seconds, period_seconds)
            var phase_score := 1.0 - (circular_distance / maxf(period_seconds * 0.5, 0.000001))
            total_score += maxf(phase_score, 0.0) * PHASE_HINT_WEIGHT
            total_score -= candidate_seconds * EARLY_LOOP_WEIGHT
            if target_loop_duration_seconds > 0.0:
                duration_delta = absf(loop_duration - target_loop_duration_seconds)
                var shared_score := 1.0 - (duration_delta / maxf(period_seconds * 0.5, 0.000001))
                total_score += maxf(shared_score, 0.0) * SHARED_LOOP_WEIGHT
        candidate_scores.append({
            "index": candidate,
            "seconds": candidate_seconds,
            "seam_score": seam_score,
            "duration_delta": duration_delta,
            "score": total_score,
        })
        if total_score > best_score:
            best_score = total_score
            best_index = candidate

    var acceptable_score := maxf(EARLY_LOOP_MIN_SCORE, best_score * EARLY_LOOP_ACCEPT_RATIO)
    if target_loop_duration_seconds > 0.0:
        var quantized_target_offset: float = round(target_offset_seconds * sample_rate) / sample_rate
        return clampf(quantized_target_offset, float(search_start) / sample_rate, float(search_end) / sample_rate)

    for candidate in candidate_scores:
        if candidate["score"] >= acceptable_score:
            return candidate["seconds"]

    return float(best_index) / sample_rate


static func _decode_stream_to_mono(stream: AudioStream) -> Dictionary:
    if stream == null or stream.get_length() <= 0.0:
        return {}

    var playback := stream.instantiate_playback()
    if playback == null:
        return {}

    var sample_rate := AudioServer.get_mix_rate()
    var frame_count := max(1, int(ceil(stream.get_length() * sample_rate)))
    playback.start(0.0)
    var mixed: Array = playback.mix_audio(1.0, frame_count)
    var samples := PackedFloat32Array()
    samples.resize(mixed.size())
    for index in range(mixed.size()):
        var frame: Vector2 = mixed[index]
        samples[index] = 0.5 * (frame.x + frame.y)

    return {
        "samples": samples,
        "sample_rate": sample_rate,
    }


static func _supports_loop_offset(stream: AudioStream) -> bool:
    if stream == null:
        return false
    return stream.has_method("set_loop") and stream.has_method("set_loop_offset")


static func _normalized_dot(lhs: PackedFloat32Array, rhs: PackedFloat32Array) -> float:
    var size := min(lhs.size(), rhs.size())
    if size == 0:
        return -INF

    var numerator := 0.0
    var lhs_energy := 0.0
    var rhs_energy := 0.0
    for index in range(size):
        var lhs_value := lhs[index]
        var rhs_value := rhs[index]
        numerator += lhs_value * rhs_value
        lhs_energy += lhs_value * lhs_value
        rhs_energy += rhs_value * rhs_value

    var denominator := sqrt(lhs_energy * rhs_energy)
    if denominator <= 0.000001:
        return -INF
    return numerator / denominator


static func _circular_distance(lhs: float, rhs: float, period: float) -> float:
    if period <= 0.0:
        return 0.0
    var delta := fposmod(lhs - rhs, period)
    return minf(delta, period - delta)


static func _median(values: Array[float]) -> float:
    if values.is_empty():
        return 0.0
    var sorted := values.duplicate()
    sorted.sort()
    var middle := sorted.size() / 2
    if sorted.size() % 2 == 1:
        return sorted[middle]
    return 0.5 * (sorted[middle - 1] + sorted[middle])


static func _select_consensus_period(values: Array[float]) -> float:
    if values.is_empty():
        return 0.0
    if values.size() <= 2:
        return _median(values)

    var best_cluster: Array[float] = []
    var best_variance := INF
    var best_center := INF

    for seed in values:
        var tolerance := maxf(PERIOD_CLUSTER_ABSOLUTE_TOLERANCE, absf(seed) * PERIOD_CLUSTER_RELATIVE_TOLERANCE)
        var cluster: Array[float] = []
        for value in values:
            if absf(value - seed) <= tolerance:
                cluster.append(value)

        if cluster.is_empty():
            continue

        var center := _median(cluster)
        var variance := 0.0
        for value in cluster:
            variance += pow(value - center, 2.0)
        variance /= float(cluster.size())

        if cluster.size() > best_cluster.size():
            best_cluster = cluster
            best_variance = variance
            best_center = center
            continue

        if cluster.size() == best_cluster.size():
            if variance < best_variance - 0.000001:
                best_cluster = cluster
                best_variance = variance
                best_center = center
                continue
            if absf(variance - best_variance) <= 0.000001 and center < best_center:
                best_cluster = cluster
                best_variance = variance
                best_center = center

    if best_cluster.is_empty():
        return _median(values)
    return _median(best_cluster)


static func _resolve_track_phase_offsets(track_data: Array[Dictionary], phase_period: float) -> Array[float]:
    var phase_offsets: Array[float] = []
    if track_data.is_empty():
        return phase_offsets

    phase_offsets.resize(track_data.size())
    phase_offsets[0] = 0.0

    for index in range(1, track_data.size()):
        var previous: Dictionary = track_data[index - 1]
        var current: Dictionary = track_data[index]
        var relative_offset := estimate_phase_offset(
            previous["active_samples"],
            current["active_samples"],
            previous["analysis_rate"],
            phase_period
        )
        phase_offsets[index] = fposmod(phase_offsets[index - 1] + relative_offset, phase_period)

    return phase_offsets


static func _select_shared_loop_duration(track_data: Array[Dictionary], phase_period: float = 0.0) -> float:
    if track_data.is_empty():
        return 0.0

    var shortest_duration := INF
    for data in track_data:
        var duration: float = data["stream_duration"] if data.has("stream_duration") else 0.0
        if duration > 0.0:
            shortest_duration = minf(shortest_duration, duration)

    if shortest_duration == INF:
        return 0.0

    if phase_period > 0.0:
        var cycles := floorf(shortest_duration / phase_period)
        if cycles > 0:
            return cycles * phase_period

    return shortest_duration
