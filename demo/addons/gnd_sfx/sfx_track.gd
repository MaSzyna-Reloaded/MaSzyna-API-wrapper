@tool
extends Resource
class_name SfxTrack

enum TriggerMode {
    TRIGGER_TIMELINE,
    TRIGGER_SUSTAIN
}

@export var stream : AudioStream
@export var generator_playback: SfxGeneratorPlayback
@export var offset := 0.0
@export var length := 0.0
@export var stream_offset := 0.0
@export var phase_offset := 0.0
@export var adsr_enabled := false
@export_range(0.0, 30.0, 0.01) var attack := 0.0
@export_range(0.0, 30.0, 0.01) var decay := 0.0
@export_range(0.0, 1.0, 0.01) var sustain := 1.0
@export_range(0.0, 30.0, 0.01) var release := 0.0
@export var fade_in_curve: Curve
@export var fade_out_curve: Curve
@export var audio_bus : StringName
@export var pitch_curve: Curve
@export var trigger_mode: TriggerMode = TriggerMode.TRIGGER_TIMELINE
