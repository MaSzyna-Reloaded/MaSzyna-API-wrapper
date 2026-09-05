extends RefCounted
class_name MmdSoundCatalog

## MMD `sounds:` label -> gnd-sfx event name + TrainSoundTrigger wiring, evidence-based the same
## way MmdSemanticCatalog is: state_property names are copied from demo/vehicles/sm42/sm_42.tscn's
## already-working hand-authored TrainSoundTrigger wiring (oil_pump_active/engine_rpm) or from the
## C++ TrainPart state each other label's own property is confirmed to expose (fuel_pump_active -
## TrainDieselEngine.cpp:180, battery_enabled - TrainController.cpp:428, compressor_enabled -
## TrainEngine.cpp:197/TrainElectricEngine.cpp:169, horn_low_active/horn_high_active/whistle_active
## - TrainHorns.cpp). Any MMD sound label not listed here is parsed (so the token stream stays
## aligned) but produces no bank event and no trigger - same "nothing built rather than something
## wrong" discipline as MmdSemanticCatalog.
##
## v1 is Tier 1 (exact parity with the proven SM42 reference: oilpump/fuelpump/horn1/horn2/horn3/
## engine) + Tier 2 (same shapes, additional labels with confirmed wrapper state: battery/
## compressor). Every other label surveyed in dynamic/pkp/ (curve/brake/tractionmotor/turbo/
## wheel_clatter/door family/announcements/...) is deliberately absent - each still needs its own
## wrapper-state cross-reference before it can be added, same discipline as the cabin catalog.

static var _catalog:Dictionary = {}
static var _built:bool = false


static func _ensure_built() -> void:
    if _built:
        return
    _built = true

    _catalog = {
        "oilpump": {
            "event_name": &"oil_pump",
            "state_property": "oil_pump_active",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
        },
        "fuelpump": {
            "event_name": &"fuel_pump",
            "state_property": "fuel_pump_active",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
        },
        "battery": {
            "event_name": &"battery",
            "state_property": "battery_enabled",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
        },
        "compressor": {
            "event_name": &"compressor",
            "state_property": "compressor_enabled",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
        },
        # horn1/horn2/horn3 map onto TrainHorns' low/high/whistle bits, in that fixed order -
        # confirmed via the original engine's Train.cpp (OnCommand_hornlowactivate/
        # OnCommand_hornhighactivate/OnCommand_whistleactivate) and DynObj.cpp's per-frame
        # WarningSignal bit 1/2/4 -> sHorn1/sHorn2/sHorn3 dispatch, NOT by the sample names
        # vehicles happen to give the files (dynamic/pkp/sm42_v1's horn3 samples are literally
        # named "...-klakson-..." - a klaxon-style third horn tone, not a train whistle, but it's
        # still driven by the whistle_activate command/WarningSignal bit 4 in the engine).
        "horn1": {
            "event_name": &"horn1",
            "state_property": "horn_low_active",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
        },
        "horn2": {
            "event_name": &"horn2",
            "state_property": "horn_high_active",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
        },
        "horn3": {
            "event_name": &"horn3",
            "state_property": "whistle_active",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
        },
        "engine": {
            "event_name": &"engine",
            "state_property": "engine_rpm",
            "trigger_mode": TrainSoundTrigger.TriggerMode.CONTINUOUS,
            "sound_parameter": &"rpm",
            # Matches sm_42.tscn's own Engine TrainSoundTrigger thresholds - a chunk-based
            # automation is only meaningful once the engine is actually turning, and RPM has no
            # natural upper bound worth clamping below in practice.
            "trigger_threshold_min": 10.0,
            "trigger_threshold_max": 10000.0,
        },
    }


static func has_label(label:String) -> bool:
    _ensure_built()
    return _catalog.has(label)


static func get_entry(label:String) -> Dictionary:
    _ensure_built()
    return _catalog.get(label, {})
