@tool
extends Resource
class_name SfxAutomation

enum CrossfadeMode {
    LINEAR,
    EQUAL_POWER
}

@export var crossfade_mode: CrossfadeMode = CrossfadeMode.LINEAR:
    set(value):
        crossfade_mode = value
        emit_changed()

@export var parameter_name : StringName = "":
    set(value):
        parameter_name = value
        emit_changed()

@export var tracks : Array[SfxTrack]:
    set(value):
        tracks = value

@export var audio_bus : StringName:
    set(value):
        audio_bus = value
        emit_changed()

@export var min_domain = 0.0:
    set(value):
        min_domain = value

@export var max_domain = 1.0:
    set(value):
        max_domain = value

@export var pitch_curve: Curve:
    set(value):
        pitch_curve = value

@export var phase_locked := false:
    set(value):
        phase_locked = value

@export var phase_period := 0.0:
    set(value):
        phase_period = value
