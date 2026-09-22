@tool
extends Resource
class_name VehicleStructure

## What a vehicle's `.mmd` says it is built from, read once and cached on disk.
##
## Everything here is a property of the vehicle *type*, so it is shared by every vehicle of that
## data_path/file_name/skin: the exterior body model the MMD names in its own `models:` line, the
## optional low-poly interior and passengers models, the resolved skin slots, the wiper prefix and
## the cab. What says which *instance* a vehicle is - its train_id, its speed, who occupies it,
## where it stands - is not here; it is applied to the vehicle that is built from this.
##
## Reading an MMD is the expensive part and a scenery repeats the same file across a whole
## consist, so this turns an O(vehicle count) parse into O(distinct types). The nodes themselves
## are cheap and are built per vehicle, rather than packed into a PackedScene and instantiated:
## a vehicle is not a scene, it is a model plus a cab plus a physics handle.

## Normalized, with the leading slash MaterialManager's search path needs.
@export var data_path:String = ""

## The base name the vehicle's .fiz, .mmd and .e3d share - what the `.scn` calls the mmdfile.
@export var file_name:String = ""

## From the MMD's own `models:` line - NOT the .fiz/.mmd base name (dynamic/pkp/st44_v2's body
## model is not named "st44-700"), and resolved to the case the file system actually has.
@export var body_model_filename:String = ""

## The lower-detail interior seen through the windows from outside. Most MMDs declare none.
@export var low_poly_model_filename:String = ""

## The passenger visualization from the MMD's `loads:` block. Most MMDs declare none.
@export var passengers_model_filename:String = ""

## The skin expanded to its numbered material slots, one entry per slot.
@export var skins:PackedStringArray = PackedStringArray()

## Submodel name prefix of the wiper arms, "" when the vehicle models no wipers.
@export var wiper_prefix:String = ""

## The MMD's `jointcabs:` - both cabs of this vehicle are the same cab.
@export var joint_cabs:bool = false

## The cab is genuinely a tree of widgets, so it stays a scene of its own.
@export var cabin_scene:PackedScene = null
