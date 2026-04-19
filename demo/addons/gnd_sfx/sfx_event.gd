@tool
extends Resource
class_name SfxEvent

@export var tracks: Array[SfxTrack] = []:
    set(value):
        tracks = value
        emit_changed()

@export var polyphony_enabled := true:
    set(value):
        polyphony_enabled = value
        emit_changed()

@export var adsr_enabled := false:
    set(value):
        adsr_enabled = value
        emit_changed()

@export_range(0.0, 30.0, 0.01) var attack := 0.0:
    set(value):
        attack = value
        emit_changed()

@export_range(0.0, 30.0, 0.01) var decay := 0.0:
    set(value):
        decay = value
        emit_changed()

@export_range(0.0, 1.0, 0.01) var sustain := 1.0:
    set(value):
        sustain = value
        emit_changed()

@export_range(0.0, 30.0, 0.01) var release := 0.0:
    set(value):
        release = value
        emit_changed()

@export var name: StringName = "":
    set(value):
        name = value
        emit_changed()

@export var automations: Array[SfxAutomation] = []:
    set(value):
        automations = value
        emit_changed()


func get_automation(automation_name:StringName) -> SfxAutomation:
    for automation in automations:
        if automation.parameter_name == automation_name:
            return automation
    return null
