extends MaszynaGutTest


func test_estimate_period_detects_tick_train() -> void:
    var samples := SfxAutomationSyncAnalyzer.normalize_samples(_make_tick_signal(2000.0, 2.0, 0.2, 0.0))
    var period := SfxAutomationSyncAnalyzer.estimate_period(samples, 2000.0, 0.1, 0.3)

    assert_almost_eq(period, 0.2, 0.005, "Autocorrelation should recover the shared tick period")


func test_estimate_phase_offset_aligns_shifted_tick_train() -> void:
    var reference := SfxAutomationSyncAnalyzer.normalize_samples(_make_tick_signal(2000.0, 2.0, 0.2, 0.0))
    var shifted := SfxAutomationSyncAnalyzer.normalize_samples(_make_tick_signal(2000.0, 2.0, 0.2, 0.05))
    var offset := SfxAutomationSyncAnalyzer.estimate_phase_offset(reference, shifted, 2000.0, 0.2)

    assert_almost_eq(offset, 0.05, 0.005, "Cross-correlation should recover the target phase lag")


func test_estimate_loop_offset_finds_earliest_seam_after_leadin() -> void:
    var samples := SfxAutomationSyncAnalyzer.normalize_samples(_make_loop_candidate_signal(2000.0, 2.0, 0.2, 0.03))
    var loop_offset := SfxAutomationSyncAnalyzer.estimate_loop_offset(samples, 2000.0, 0.2, 0.03)

    assert_almost_eq(loop_offset, 0.03, 0.01, "Loop offset should land on the first seam that matches the stream tail")


func test_select_consensus_period_prefers_dominant_cluster_over_global_median() -> void:
    var estimates: Array[float] = [0.33548752834467, 0.49936507936508, 0.50011337868481, 0.22922902494331]
    var period := SfxAutomationSyncAnalyzer._select_consensus_period(estimates)

    assert_almost_eq(period, 0.499739229024945, 0.002, "Consensus period should follow the dominant cluster, not outliers")


func test_resolve_track_phase_offsets_accumulates_adjacent_pair_offsets() -> void:
    var phase_offsets := SfxAutomationSyncAnalyzer._resolve_track_phase_offsets(
        [
            {
                "active_samples": SfxAutomationSyncAnalyzer.normalize_samples(_make_tick_signal(2000.0, 2.0, 0.5, 0.0)),
                "analysis_rate": 2000.0,
            },
            {
                "active_samples": SfxAutomationSyncAnalyzer.normalize_samples(_make_tick_signal(2000.0, 2.0, 0.5, 0.1)),
                "analysis_rate": 2000.0,
            },
            {
                "active_samples": SfxAutomationSyncAnalyzer.normalize_samples(_make_tick_signal(2000.0, 2.0, 0.5, 0.2)),
                "analysis_rate": 2000.0,
            },
        ],
        0.5
    )

    assert_eq(phase_offsets.size(), 3, "Sequential phase resolution should return one offset per track")
    assert_almost_eq(phase_offsets[0], 0.0, 0.005, "The first track should remain the phase anchor")
    assert_almost_eq(phase_offsets[1], 0.1, 0.005, "The second track should preserve its adjacent lag")
    assert_almost_eq(phase_offsets[2], 0.2, 0.005, "The third track should accumulate from the previous track, not re-anchor to the first")


func test_estimate_loop_offset_prefers_cycle_stable_seam_when_multiple_matches_exist() -> void:
    var samples := SfxAutomationSyncAnalyzer.normalize_samples(
        _make_multi_seam_loop_candidate_signal(2000.0, 1.0, 0.2, [0.03, 0.2])
    )
    var loop_offset := SfxAutomationSyncAnalyzer.estimate_loop_offset(samples, 2000.0, 0.2, 0.03, 0.0, 0.8)

    assert_almost_eq(loop_offset, 0.2, 0.01, "Loop offset should prefer a seam that keeps the loop duration phase-stable")


func test_estimate_loop_offset_prefers_target_shared_loop_duration() -> void:
    var samples := SfxAutomationSyncAnalyzer.normalize_samples(
        _make_multi_seam_loop_candidate_signal(2000.0, 1.0, 0.2, [0.03, 0.1])
    )
    var loop_offset := SfxAutomationSyncAnalyzer.estimate_loop_offset(samples, 2000.0, 0.2, 0.03, 0.0, 0.9)

    assert_almost_eq(loop_offset, 0.1, 0.01, "Loop offset should prefer the seam that matches the shared loop duration")


func test_select_shared_loop_duration_uses_shortest_track() -> void:
    var shared_duration := SfxAutomationSyncAnalyzer._select_shared_loop_duration(
        [
            {
                "stream_duration": 2.01,
            },
            {
                "stream_duration": 1.993,
            },
            {
                "stream_duration": 2.016,
            },
        ]
    )

    assert_almost_eq(shared_duration, 1.993, 0.001, "Shared loop duration should match the shortest track, because other loops can only be shortened")


func test_select_shared_loop_duration_is_multiple_of_phase_period() -> void:
    var shared_duration := SfxAutomationSyncAnalyzer._select_shared_loop_duration(
        [
            {"stream_duration": 2.1},
            {"stream_duration": 2.2},
        ],
        0.4
    )
    # Shortest is 2.1. Multiples of 0.4 are 0.4, 0.8, 1.2, 1.6, 2.0.
    # Largest multiple <= 2.1 is 2.0.
    assert_almost_eq(shared_duration, 2.0, 0.001, "Shared loop duration should be a multiple of phase_period")


func test_estimate_loop_offset_ignores_search_limit_for_target_duration() -> void:
    # Track duration 5.0, target loop 2.0. Needs loop_offset 3.0.
    # Default search limit (LOOP_SEARCH_MAX) is 0.75.
    var loop_offset := SfxAutomationSyncAnalyzer.estimate_loop_offset(
        PackedFloat32Array([0,0,0,0,0,0,0,0,0,0]), # dummy samples
        10.0,
        0.1, # period
        0.0, # search_start
        0.0, # phase_offset
        2.0, # target_loop_duration
        5.0  # stream_duration
    )
    assert_almost_eq(loop_offset, 3.0, 0.01, "Should honor target duration even if it requires large offset")


func _make_tick_signal(sample_rate: float, duration_seconds: float, period_seconds: float, phase_seconds: float) -> PackedFloat32Array:
    var sample_count: int = int(round(sample_rate * duration_seconds))
    var period_samples: int = max(1, int(round(sample_rate * period_seconds)))
    var phase_samples: int = int(round(sample_rate * phase_seconds))
    var samples := PackedFloat32Array()
    samples.resize(sample_count)

    for center in range(phase_samples, sample_count, period_samples):
        for pulse_index in range(5):
            var sample_index := center + pulse_index
            if sample_index >= sample_count:
                break
            samples[sample_index] = 1.0 - (float(pulse_index) * 0.2)

    return samples


func _make_loop_candidate_signal(
    sample_rate: float,
    duration_seconds: float,
    period_seconds: float,
    loop_offset_seconds: float
) -> PackedFloat32Array:
    var sample_count: int = int(round(sample_rate * duration_seconds))
    var loop_offset_samples: int = int(round(sample_rate * loop_offset_seconds))
    var window_samples: int = int(round(sample_rate * min(period_seconds * 1.5, 0.25)))
    var tail_start: int = sample_count - window_samples
    var samples := PackedFloat32Array()
    samples.resize(sample_count)

    for index in range(sample_count):
        samples[index] = 0.15 * sin(float(index) * 0.071)

    for index in range(window_samples):
        var value := sin(float(index) * 0.11) + (0.35 * cos(float(index) * 0.037))
        samples[tail_start + index] = value
        samples[loop_offset_samples + index] = value

    return samples


func _make_multi_seam_loop_candidate_signal(
    sample_rate: float,
    duration_seconds: float,
    period_seconds: float,
    loop_offsets_seconds: Array[float]
) -> PackedFloat32Array:
    var sample_count: int = int(round(sample_rate * duration_seconds))
    var window_samples: int = int(round(sample_rate * min(period_seconds * 1.5, 0.25)))
    var tail_start: int = sample_count - window_samples
    var samples := PackedFloat32Array()
    samples.resize(sample_count)

    for index in range(sample_count):
        samples[index] = 0.12 * sin(float(index) * 0.053)

    for index in range(window_samples):
        var value := cos(float(index) * 0.09) + (0.28 * sin(float(index) * 0.041))
        samples[tail_start + index] = value
        for loop_offset_seconds in loop_offsets_seconds:
            var loop_offset_samples: int = int(round(sample_rate * loop_offset_seconds))
            samples[loop_offset_samples + index] = value

    return samples

