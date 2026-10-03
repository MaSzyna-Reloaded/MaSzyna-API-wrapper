@tool
extends HBoxContainer

## One trainset of the edited scene: its name, "Show" and the side views of its vehicles in the
## order they stand

## "Show" was pressed: the 3D view is to go to the trainset
signal show_requested(trainset_id:int)

## Height of a vehicle's side view [px]
const PROFILE_HEIGHT:float = 32.0

## The trainset's instance id - a scenery loaded again frees it
var _trainset_id:int = 0


func set_trainset(trainset:TrainSet3D) -> void:
    _trainset_id = trainset.get_instance_id()
    %Name.text = trainset.name


## The place of the next vehicle's side view, empty until the view is rendered
func add_vehicle_profile(tooltip:String) -> TextureRect:
    var profile:TextureRect = TextureRect.new()
    profile.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
    profile.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
    profile.custom_minimum_size = Vector2(PROFILE_HEIGHT, PROFILE_HEIGHT)
    profile.tooltip_text = tooltip
    %Vehicles.add_child(profile)
    return profile


## The side view in its place, as wide as the vehicle is long
static func show_profile(profile:TextureRect, texture:Texture2D) -> void:
    profile.texture = texture
    profile.custom_minimum_size = Vector2(
            PROFILE_HEIGHT * float(texture.get_width()) / float(texture.get_height()), PROFILE_HEIGHT)


func _on_show_pressed() -> void:
    show_requested.emit(_trainset_id)
