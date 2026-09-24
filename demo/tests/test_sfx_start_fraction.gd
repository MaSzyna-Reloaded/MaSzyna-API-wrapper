extends GutTest

## Every vehicle of a consist plays the same running noise, and it is an automation crossfaded
## between speed chunks. Each vehicle's shift has to reach every chunk it swaps in, or the copies
## play in step and ring metallic (DynObj.cpp:6511, audiorenderer.cpp:99).

const SAMPLE_RATE:int = 1000
const CHUNK_SPEED:float = 50.0
const MAX_SPEED:float = 100.0
const START_FRACTION:float = 0.5
const TICK:float = 0.016
const VOICE_SLOTS:int = 4
## a speed inside the first chunk and one inside the second
const FIRST_CHUNK_SPEED:float = 10.0
const SECOND_CHUNK_SPEED:float = 70.0


func _chunk(offset:float) -> SfxClip:
    var stream:AudioStreamWAV = AudioStreamWAV.new()
    stream.format = AudioStreamWAV.FORMAT_16_BITS
    stream.mix_rate = SAMPLE_RATE
    var data:PackedByteArray = PackedByteArray()
    data.resize(SAMPLE_RATE * 2)
    stream.data = data
    stream.loop_mode = AudioStreamWAV.LOOP_FORWARD
    stream.loop_end = SAMPLE_RATE
    var clip:SfxClip = SfxClip.new()
    clip.stream = stream
    clip.offset = offset
    clip.length = CHUNK_SPEED
    return clip


func _play(start_fraction:float, speed:float) -> SfxPlaybackRuntime:
    var automation:SfxAutomation = SfxAutomation.new()
    automation.parameter_name = &"speed"
    automation.max_domain = MAX_SPEED
    var clips:Array[SfxClip] = [_chunk(0.0), _chunk(CHUNK_SPEED)]
    automation.clips = clips
    var event:SfxEvent = SfxEvent.new()
    event.name = &"outer_noise"
    var automations:Array[SfxAutomation] = [automation]
    event.automations = automations
    var runtime:SfxPlaybackRuntime = SfxPlaybackRuntime.new()
    runtime.set_slot_capacity(VOICE_SLOTS)
    runtime.play(event, 0.0, {&"speed": speed}, start_fraction)
    runtime.update(TICK)
    return runtime


func _start_position(runtime:SfxPlaybackRuntime) -> float:
    var slot:SfxVoiceSlot = runtime.get_slots()[0]
    return slot.start_position / slot.stream.get_length()


func test_automation_voice_starts_at_the_start_fraction() -> void:
    assert_almost_eq(_start_position(_play(START_FRACTION, FIRST_CHUNK_SPEED)), START_FRACTION, 0.001)


func test_chunk_swapped_in_keeps_the_start_fraction() -> void:
    var runtime:SfxPlaybackRuntime = _play(START_FRACTION, FIRST_CHUNK_SPEED)
    runtime.modulate(&"outer_noise", {&"speed": SECOND_CHUNK_SPEED})
    runtime.update(TICK)
    assert_almost_eq(_start_position(runtime), START_FRACTION, 0.001)


func test_automation_voice_without_fraction_starts_at_the_stream_offset() -> void:
    assert_almost_eq(_start_position(_play(0.0, FIRST_CHUNK_SPEED)), 0.0, 0.001)
