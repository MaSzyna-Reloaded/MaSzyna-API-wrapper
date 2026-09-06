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
## compressor) + Tier 3 (brake-related labels - `entry["controller"] == &"brake"` marks these;
## MmdSoundBankInstancer routes them to BrakeSfxEventFactory instead of building a TrainSoundTrigger
## from `state_property`/`trigger_mode` the way Tier 1/2 entries do - see each label's own comment
## for its DynObj.cpp/Train.cpp source and BrakeSfxEventFactory for how gating/shaping is actually
## built). Every other label surveyed in dynamic/pkp/ (curve/tractionmotor/turbo/wheel_clatter/door
## family/announcements/...) is deliberately absent - each still needs its own wrapper-state
## cross-reference before it can be added, same discipline as the cabin catalog.

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
        # buzzer:/buzzershp: (internaldata:, not sounds: - see MmdSoundSourceParser.
        # parse_internal_data()) drive a LOOPING sound while the alerter is actively unacknowledged
        # (Train.cpp:10111-10151: dsbBuzzer/dsbBuzzerShp play() while is_beeping()/
        # is_cabsignal_beeping(), stop() otherwise) - a SEPARATE, later-triggered stage from the
        # light's own on/off click (TrainSecuritySystem::is_beeping(), Mover.cpp:186:
        # `alert_timer > SoundSignalDelay` - the buzzer only starts SoundSignalDelay seconds after
        # the light already began blinking, not simultaneously).
        "buzzer": {
            "event_name": &"buzzer",
            "state_property": "beeping",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
        },
        "buzzershp": {
            "event_name": &"buzzershp",
            "state_property": "cabsignal_beeping",
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
        # Brake-related labels only carry event_name/controller - BrakeSfxEventFactory (not this
        # catalog) decides gating/shaping/hardware wiring, built once from TrainController.config +
        # each MmdSoundSourceDefinition's own amplitude/frequency constants (see the brake-sound
        # redesign plan). event_name is a many-to-one COMPOSITION MAP: labels that are begin/
        # middle/end phases or same-physical-effect variants of one sound share one event_name, so
        # MmdSoundBankInstancer groups them before handing them to the factory, instead of building
        # one event per label. "brake" (squeal, DynObj.cpp:4437-4458 rsPisk) + "brakesound"
        # (friction rumble, DynObj.cpp:4411-4428 rsBrake) compose into "brake_shoe" - both keyed off
        # the same brakeforceratio in the original engine. The cab-local copy of "brakesound"
        # is routed to a separate bank, so it keeps the same label and event name without either
        # definition being overwritten.
        "brake": {"event_name": &"brake_shoe", "controller": &"brake"},
        "brakesound": {"event_name": &"brake_shoe", "controller": &"brake"},
        # Brake cylinder release hiss (DynObj.cpp:4351-4372, rsUnbrake) - unique driving parameter
        # (rate of brake_air_pressure dropping), no natural counterpart to compose with.
        "unbrake": {"event_name": &"brake_release_hiss", "controller": &"brake"},
        # Emergency valve dump hiss (DynObj.cpp:4332-4349) - unique driving parameter
        # (EmergencyValveFlow).
        "emergencybrake": {"event_name": &"emergency_brake_hiss", "controller": &"brake"},
        # Wheel-slip squeal (DynObj.cpp:4387-4401, rsSlippery) - unique driving parameter
        # (slipping_wheels).
        "slipperysound": {"event_name": &"wheel_slip_squeal", "controller": &"brake"},
        # Main pipe pneumatic hiss family, cab-only (Train.cpp:8129-8229, rsHiss/rsHissU/rsHissE/
        # rsHissX/rsHissT - one function, one physical valve system) - all five compose into one
        # "pipe_hiss" event; BrakeSfxEventFactory picks FV4a-handle-specific or generic dpMainValve-
        # derived sub-signals for each, once, from config.
        "airsound": {"event_name": &"pipe_hiss", "controller": &"brake"},
        "airsound2": {"event_name": &"pipe_hiss", "controller": &"brake"},
        "airsound3": {"event_name": &"pipe_hiss", "controller": &"brake"},
        "airsound4": {"event_name": &"pipe_hiss", "controller": &"brake"},
        "airsound5": {"event_name": &"pipe_hiss", "controller": &"brake"},
        # Auxiliary/independent (loco) brake cylinder hiss, cab-only (Train.cpp:8090-8127,
        # rsSBHiss/rsSBHissU) - release/engage halves of one effect, compose into "local_brake_hiss".
        "localbrakesound": {"event_name": &"local_brake_hiss", "controller": &"brake"},
        "localbrakesound2": {"event_name": &"local_brake_hiss", "controller": &"brake"},
        # Brake cylinder piston advance/recede clicks (DynObj.cpp:4258-4287) and EP brake pressure
        # clicks (DynObj.cpp:4289-4330) - each pair composes into one event with two boolean-pulse
        # automations (one per direction), matching the original engine's own coupled clicks; see
        # BrakeSfxEventFactory for the pulse-automation shape (a domain like [0.5,1.5] IS the right
        # model for a genuinely discrete one-shot trigger, same idea as horn1/horn2's own begin/
        # sustain/end shape in demo/vehicles/sm42/sounds - not every parameter needs to be
        # continuous, just never a pre-combined "gain" value).
        "brakecylinderinc": {"event_name": &"brake_cylinder_click", "controller": &"brake"},
        "brakecylinderdec": {"event_name": &"brake_cylinder_click", "controller": &"brake"},
        "epbrakeinc": {"event_name": &"ep_brake_click", "controller": &"brake"},
        "epbrakedec": {"event_name": &"ep_brake_click", "controller": &"brake"},
        # Quick-release accelerator valve one-shot ("przyspieszacz") - unique, no counterpart,
        # single boolean-pulse automation.
        "brakeacc": {"event_name": &"brake_accelerator", "controller": &"brake"},
        # Release valve ("odluzniacz") hiss, driven by its raw mover state.
        "releaser": {"event_name": &"brake_releaser", "controller": &"brake"},
        # Spring/parking brake activation and release use the high and low domains of the same raw
        # spring_brake/active parameter.
        "springbrake": {"event_name": &"springbrake", "controller": &"brake"},
        "springbrakeoff": {"event_name": &"springbrake", "controller": &"brake"},
    }


static func has_label(label:String) -> bool:
    _ensure_built()
    return _catalog.has(label)


static func get_entry(label:String) -> Dictionary:
    _ensure_built()
    return _catalog.get(label, {})
