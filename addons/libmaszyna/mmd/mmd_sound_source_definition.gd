extends RefCounted
class_name MmdSoundSourceDefinition

## One parsed label from the MMD's own vehicle-wide `sounds:`...`endsounds` section, in the
## neutral shape the original engine's audio/sound.cpp `sound_source` class settles every one of
## its ~46 label syntaxes into: an optional single/begin/end sound (all three normalized,
## extension-less filenames, "" when absent) and/or a set of numbered `soundN:`/`pitchN:` chunks.
## `soundset:` (a single random-set value containing "a|b|c", confirmed real:
## dynamic/pkp/303e_v1/303e-ep-ic.mmd's compressor:) is unpacked into sound_begin/sound_main/
## sound_end at parse time, same as if the source had used soundbegin:/soundmain:/soundend:
## directly - MmdSoundEventBuilder only ever looks at the fields below, never at which source
## syntax produced them.

var label:String = ""
var sound_main:String = ""
var sound_begin:String = ""
var sound_end:String = ""
## {threshold:int, filename:String, pitch:float}, in file order (MmdSoundEventBuilder sorts by
## threshold - matches the original engine's own sound_source::deserialize() sort step).
var chunks:Array[Dictionary] = []
## Percentage (0-100) of the gap between adjacent chunk thresholds that the crossfade porting in
## MmdSoundEventBuilder blends over - mirrors m_crossfaderange, "crossfade:" in MMD.
var crossfade_percent:int = 0
var amplitude_factor:float = 1.0
var amplitude_offset:float = 0.0
var frequency_factor:float = 1.0
var frequency_offset:float = 0.0
var range:float = 50.0
var range_defined:bool = false
var placement:StringName = &"general"
var placement_defined:bool = false
var offset:Vector3 = Vector3.ZERO
var soundproofing:PackedFloat32Array = PackedFloat32Array()
var pitch_variation:float = 0.0
var start_offset:float = 0.0
var source_file:String = ""
