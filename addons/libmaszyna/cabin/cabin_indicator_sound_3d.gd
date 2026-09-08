extends AudioStreamPlayer3D
class_name CabinIndicatorSound3D

@export var sound_on:AudioStream
@export var sound_off:AudioStream

func set_active(active:bool) -> void:
    stream = sound_on if active else sound_off
    if stream:
        play()
