extends RefCounted
class_name MmdInstrumentDescriptor

## One parsed MMD instrument/manipulator line, in the raw shape shared by every label:
## `label: submodel animation scale offset friction` (or the equivalent `{ ... }` block form).
## Duplicate labels (e.g. two `tachometer:` entries driving two needles) are kept as separate
## descriptors in MmdCabinDefinition.instruments, in file order - this class never deduplicates.

var label:String = ""
var submodel_name:String = ""
## "rot" or "mov" - wip/dgt/rotvar/movvar are out of Etap A+B scope and are parsed as "rot"/"mov"
## with their trailing endvalue/endscale tokens ignored, rather than rejected outright.
var animation_type:String = ""
var scale:float = 0.0
var offset:float = 0.0
var friction:float = 0.0
## "return"/"impulse"/"push"/"toggle"/"pushtoggle"/"delayed", or "" when the label has no
## explicit `type:` field (plain 5-token form).
var button_type:String = ""
var source_file:String = ""
var line:int = 0
