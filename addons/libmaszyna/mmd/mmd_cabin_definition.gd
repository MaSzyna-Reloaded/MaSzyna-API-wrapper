extends RefCounted
class_name MmdCabinDefinition

## Neutral, parsed representation of one MMD file's cabin data, for one selected cab number.
## Produced by MmdCabinInstancer.parse() and consumed by MmdCabinInstancer.build_into() - this
## class knows nothing about Cabin3D, E3DModelInstance, or TrainController.

## 1 or -1, matching TrainController.state["cabin_occupied"] (cab1/cab2).
var cab_number:int = 1

## Raw MMD camera bounds - RailVehicle3D.enter_cabin() adds the +0.5/+1.8 Y offset itself,
## do not add it again here.
var bounds_min:Vector3 = Vector3.ZERO
var bounds_max:Vector3 = Vector3.ZERO
var driver_pos:Vector3 = Vector3.ZERO
## Parsed but not wired to anything yet - Cabin3D has no seated-camera mode to hand it to.
var driver_sitpos:Vector3 = Vector3.ZERO
var driver_angle:Vector2 = Vector2.ZERO

## Original MaSzyna camera spring parameters from the MMD preamble.
var shake_spring_stiffness:float = 125.0
var shake_spring_damping:float = 0.002
var shake_jolt_scale:Vector3 = Vector3(0.2, 0.2, 0.1)
var shake_jolt_limit:float = 0.15
var shake_angle_scale:Vector2 = Vector2(0.05, 0.1)
var engine_shake_scale:float = 2.0
var engine_shake_fade_in_rpm:float = 90.0
var engine_shake_fade_in_factor:float = 0.3
var engine_shake_fade_out_rpm:float = 600.0
var engine_shake_fade_out_factor:float = 0.5

## Cab model, relative to the MMD's own directory, with ".t3d" already swapped for ".e3d" and
## backslashes normalized - still needs case-insensitive filesystem resolution by the builder.
var model_relpath:String = ""

## Ordered, duplicate-preserving list of every instrument/manipulator line found between this
## cab's `cabNdefinition:` and the following `cab0definition:`/EOF.
var instruments:Array[MmdInstrumentDescriptor] = []

## Parse-time diagnostics (severity/code/source_file/line/cabin_number/mmd_label/
## submodel_name/message), collected while building this definition. MmdCabinInstancer.
## build_into() appends its own build-time diagnostics to the same shape separately.
var diagnostics:Array[Dictionary] = []
