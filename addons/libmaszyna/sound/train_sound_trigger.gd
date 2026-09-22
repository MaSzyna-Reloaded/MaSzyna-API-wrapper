extends Node
class_name TrainSoundTrigger

## CHANGE plays the event once whenever the state value changes (an event counter, e.g. coupler sounds)
enum TriggerMode { TOGGLE, CONTINUOUS, CHANGE }

@export var state_property:String = ""
@export var trigger_mode:TriggerMode = TriggerMode.TOGGLE
@export var trigger_threshold_min:float = 0.0
@export var trigger_threshold_max:float = 1.0
@export var sound_event:StringName = &""
@export var sound_parameter:StringName = &""
@export var sound_placement:StringName = &"general"

@export_node_path("VehiclePhysicsNode") var controller_path:NodePath = NodePath("")

var _sfxplayer:SfxPlayer3D
var _trigger_id:int = 0


func _ready() -> void:
    _sfxplayer = get_parent() as SfxPlayer3D
    var vehicle:RailVehicle3D = _find_vehicle()
    var physics_node:VehiclePhysicsNode = get_node_or_null(controller_path) if controller_path else null
    var controller:VehicleController = physics_node.get_controller() if physics_node else null
    if not sound_placement == &"general":
        var event:SfxEvent = _sfxplayer.bank.get_event(sound_event)
        if event:
            MmdSoundEventBuilder.add_soundproofing_modulations(event)
    _trigger_id = TrainSoundSystem.register_trigger(_sfxplayer, {
        "id": get_instance_id(),
        "vehicle": vehicle,
        "controller": controller,
        "state_property": state_property,
        "trigger_mode": trigger_mode,
        "trigger_threshold_min": trigger_threshold_min,
        "trigger_threshold_max": trigger_threshold_max,
        "sound_event": sound_event,
        "sound_parameter": sound_parameter,
        "sound_placement": sound_placement,
    })


func _exit_tree() -> void:
    if _sfxplayer and _trigger_id:
        TrainSoundSystem.unregister_trigger(_sfxplayer, _trigger_id)


func _find_vehicle() -> RailVehicle3D:
    var node:Node = get_parent()
    while node:
        if node is RailVehicle3D:
            return node as RailVehicle3D
        node = node.get_parent()
    return null
