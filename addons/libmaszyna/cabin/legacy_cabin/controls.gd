extends RefCounted
class_name LegacyCabinControls

## The controls one cab has, read from its MMD rather than from the widgets built of it - what the
## cab logic asks about (LegacyCabinLogic): which controls there are, the kind of switch each is
## (the MMD `type:`, TGaugeType) and the vehicle command it is wired to (MmdSemanticCatalog). The
## same list serves a cab the player sits in and one only the AI drives, which has no widgets.
##
## Only a descriptor of a class that reports manipulations counts - the widgets that carry a
## control_id (MmdCabinInstancer._build_widget); lamps and lights only show the vehicle.

## The key fields of a catalog entry
const ACTION_FIELDS:Array[String] = ["action", "action_increase", "action_decrease"]
## Jumps the brake handle to its driving position - no MMD label, every dynamically built cab has it
## (DynamicTrainCabin takes its key)
const BRAKE_LEVEL_DRIVE:StringName = &"brake_level_drive"

## control_id -> the widget class of its catalog entry
var _classes:Dictionary[StringName, Variant] = {}
## control_id -> the fixed fields of its catalog entry
var _fields:Dictionary[StringName, Dictionary] = {}
var _button_types:Dictionary[StringName, CabinButton.ButtonType] = {}


## The controls of the cab the MMD defines as `cab` (1, 0, or -1 for the rear one)
static func from_mmd(cab:int, abs_mmd_path:String) -> LegacyCabinControls:
    # Train.cpp:8684 (InitializeCab) - the rear cab is cab2definition:
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(abs_mmd_path, 2 if cab < 0 else cab, {})
    return from_definition(definition)


static func from_definition(definition:MmdCabinDefinition) -> LegacyCabinControls:
    var controls:LegacyCabinControls = LegacyCabinControls.new()
    for descriptor:MmdInstrumentDescriptor in definition.instruments:
        if not MmdSemanticCatalog.has_label(descriptor.label):
            continue
        var entry:Dictionary = MmdSemanticCatalog.get_entry(descriptor.label)
        var control_id:StringName = StringName(descriptor.label)
        # the widget classes of MmdSemanticCatalog that are controls
        var is_control:bool = entry["widget_class"] in [CabinButton, CabinSwitch, CabinKnob, CabinGauge]
        # a label repeated in a cab is one control: its first descriptor is it (the quirk in
        # MmdCabinInstancer.build_into - the later widgets only follow the cabin state)
        if not is_control or controls.has_control(control_id):
            continue
        controls.add_control(
                control_id, entry["widget_class"], entry["fixed_fields"],
                MmdCabinInstancer.BUTTON_TYPES.get(descriptor.button_type, CabinButton.ButtonType.TOGGLE))
    controls.add_control(
            BRAKE_LEVEL_DRIVE, CabinCommand, {"command": "brake_level_set_position", "command_param": "drive"})
    return controls


func add_control(
    control_id:StringName, widget_class:Variant, fields:Dictionary,
    button_type:CabinButton.ButtonType = CabinButton.ButtonType.TOGGLE
) -> void:
    _classes[control_id] = widget_class
    _fields[control_id] = fields
    _button_types[control_id] = button_type


func has_control(control_id:StringName) -> bool:
    return _classes.has(control_id)


## The kind of switch the cab has for this control; one it does not have is a toggle, as an
## undefined TGauge is (Gauge.h:89)
func button_type(control_id:StringName) -> CabinButton.ButtonType:
    return _button_types.get(control_id, CabinButton.ButtonType.TOGGLE)


## The vehicle command the control is wired to (LegacyCabinForwardCommands.wiring())
func wiring(control_id:StringName) -> Dictionary:
    return LegacyCabinForwardCommands.wiring(_classes[control_id], _fields[control_id]) if has_control(control_id) else {}


func get_control_ids() -> Array[StringName]:
    var control_ids:Array[StringName] = []
    control_ids.assign(_classes.keys())
    return control_ids


## The keys the cab's controls take
func get_actions() -> Dictionary[String, bool]:
    var actions:Dictionary[String, bool] = {}
    for fields:Dictionary in _fields.values():
        for field:String in ACTION_FIELDS:
            if fields.get(field, ""):
                actions[fields[field]] = true
    return actions
