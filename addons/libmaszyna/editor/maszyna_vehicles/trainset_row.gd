@tool
extends HBoxContainer

## One trainset of the edited scene: its name, "Show" and the side views of its vehicles in the
## order they stand

## "Show" was pressed: the 3D view is to go to the trainset's vehicles (RailVehicleServer's)
signal show_requested(vehicles:Array[RID])

## Height of a vehicle's side view [px]
const PROFILE_HEIGHT:float = 32.0

## The trainset's vehicles, in the order they stand
var _vehicles:Array[RID] = []


func set_trainset(trainset_name:String, vehicles:Array[RID]) -> void:
    %Name.text = trainset_name
    _vehicles = vehicles


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
    show_requested.emit(_vehicles)
