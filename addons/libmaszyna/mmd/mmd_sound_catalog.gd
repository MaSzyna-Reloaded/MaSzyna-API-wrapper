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
## compressor) + Tier 3 (brake-related: brake/brakesound/brakesound_cab/unbrake/emergencybrake/
## slipperysound/airsound(2-5)/localbrakesound(2), wired to TrainBrake/TrainWheels state - see
## each entry's own comment for its DynObj.cpp/Train.cpp source). Every other label surveyed in
## dynamic/pkp/ (curve/tractionmotor/turbo/wheel_clatter/door family/announcements/...) is
## deliberately absent - each still needs its own wrapper-state cross-reference before it can be
## added, same discipline as the cabin catalog.

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
        # Brake shoe squeal (DynObj.cpp:4437-4458, rsPisk) - original engine gates on Vel > 2.5
        # (km/h, same unit as TrainController.cpp's "speed") and modulates by brakeforceratio
        # (UnitBrakeForce vs. the speed-derated theoretical max, see TrainBrake.cpp's new
        # brake_force_ratio). Exterior-only (DynObj-owned), audible from outside the vehicle.
        "brake": {
            "event_name": &"brake_squeal",
            "controller": &"brake",
            "state_property": "brake_force_ratio",
            "trigger_mode": TrainSoundTrigger.TriggerMode.CONTINUOUS,
            "sound_parameter": &"ratio",
            "gate_state_property": "speed",
            "gate_threshold_min": 2.5,
        },
        # Brake shoe/pad friction rumble, exterior copy (DynObj.cpp:4411-4428, rsBrake) - gates on
        # UnitBrakeForce > 10 && Vel > 0.05; approximated here via brake_force_ratio's own floor
        # (trigger_threshold_min) plus the same speed gate, rather than adding a second numeric
        # gate to TrainSoundTrigger for the raw force cutoff.
        "brakesound": {
            "event_name": &"brake_friction",
            "controller": &"brake",
            "state_property": "brake_force_ratio",
            "trigger_mode": TrainSoundTrigger.TriggerMode.CONTINUOUS,
            "sound_parameter": &"ratio",
            "trigger_threshold_min": 0.02,
            "gate_state_property": "speed",
            "gate_threshold_min": 0.05,
        },
        # Same physical effect, cab-local copy (Train.cpp:8231-8253) - identical gating/formula to
        # "brakesound" above, but only audible while the player occupies this vehicle's cab
        # (Train.cpp mutes it in free-fly/external view; requires_occupied reproduces that here).
        # Relabeled from "brakesound" to "brakesound_cab" by MmdSoundBankInstancer before this
        # lookup, since both copies can share the same MMD label text in the same file
        # (dynamic/pkp/su45_v2/301d.mmd:62 vs. :119).
        "brakesound_cab": {
            "event_name": &"brake_friction_cab",
            "controller": &"brake",
            "state_property": "brake_force_ratio",
            "trigger_mode": TrainSoundTrigger.TriggerMode.CONTINUOUS,
            "sound_parameter": &"ratio",
            "trigger_threshold_min": 0.02,
            "gate_state_property": "speed",
            "gate_threshold_min": 0.05,
            "requires_occupied": true,
        },
        # Brake cylinder release hiss (DynObj.cpp:4351-4372, rsUnbrake) - driven by the RATE of
        # brake_air_pressure dropping (m_brakepressurechange in the original), not its raw level.
        # Exterior-only.
        "unbrake": {
            "event_name": &"brake_release_hiss",
            "controller": &"brake",
            "state_property": "brake_air_pressure",
            "trigger_mode": TrainSoundTrigger.TriggerMode.CONTINUOUS,
            "sound_parameter": &"rate",
            "trigger_threshold_min": -INF,
            "trigger_threshold_max": -0.05,
        },
        # Emergency valve dump hiss (DynObj.cpp:4332-4349) - driven by EmergencyValveFlow, off
        # below flow 0.015. Exterior-only.
        "emergencybrake": {
            "event_name": &"emergency_brake_hiss",
            "controller": &"brake",
            "state_property": "brake_emergency_valve_flow",
            "trigger_mode": TrainSoundTrigger.TriggerMode.CONTINUOUS,
            "sound_parameter": &"flow",
            "trigger_threshold_min": 0.015,
            "trigger_threshold_max": INF,
        },
        # Wheel-slip squeal (DynObj.cpp:4387-4401, rsSlippery) - gated on SlippingWheels alone
        # here (TrainWheels.cpp's slipping_wheels); the original's extra UnitBrakeForce>100/
        # Velocity>1 gates are dropped as a deliberate simplification (real slip in the underlying
        # physics already implies nontrivial speed/force). Exterior-only.
        "slipperysound": {
            "event_name": &"wheel_slip_squeal",
            "controller": &"brake",
            "state_property": "slipping_wheels",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
        },
        # Main pipe pneumatic hiss, cab-only (Train.cpp:8129-8229, rsHiss/rsHissU) - generic
        # (non-FV4a-handle) fallback reads dpMainValve's sign directly (brake_main_valve_flow);
        # airsound=fill (positive part), airsound2=release (negative part).
        "airsound": {
            "event_name": &"pipe_hiss_fill",
            "controller": &"brake",
            "state_property": "brake_main_valve_flow",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
            "trigger_threshold_min": 0.01,
            "trigger_threshold_max": INF,
            "requires_occupied": true,
        },
        "airsound2": {
            "event_name": &"pipe_hiss_release",
            "controller": &"brake",
            "state_property": "brake_main_valve_flow",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
            "trigger_threshold_min": -INF,
            "trigger_threshold_max": -0.01,
            "requires_occupied": true,
        },
        # airsound3/4/5 (rsHissE/X/T) are FV4a/FVel6 handle-family-specific synthetic signals in
        # the original engine (Handle->GetSound(s_fv4a_e/x/t)), with no generic Mover-level
        # fallback documented for other handle types - approximated here by reusing the same
        # brake_main_valve_flow signal with distinct thresholds rather than exposing the brake
        # handle valve model's internals. Cab-only. Deliberately approximate; tune thresholds
        # against real playback if they prove too eager/silent.
        "airsound3": {
            "event_name": &"pipe_hiss_e",
            "controller": &"brake",
            "state_property": "brake_main_valve_flow",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
            "trigger_threshold_min": 0.2,
            "trigger_threshold_max": INF,
            "requires_occupied": true,
        },
        "airsound4": {
            "event_name": &"pipe_hiss_x",
            "controller": &"brake",
            "state_property": "brake_main_valve_flow",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
            "trigger_threshold_min": -INF,
            "trigger_threshold_max": -0.2,
            "requires_occupied": true,
        },
        "airsound5": {
            "event_name": &"pipe_hiss_t",
            "controller": &"brake",
            "state_property": "brake_main_valve_flow",
            "trigger_mode": TrainSoundTrigger.TriggerMode.TOGGLE,
            "trigger_threshold_min": 0.5,
            "trigger_threshold_max": INF,
            "requires_occupied": true,
        },
        # Auxiliary/independent (loco) brake cylinder hiss, cab-only (Train.cpp:8090-8127,
        # rsSBHiss/rsSBHissU) - driven by the RATE of brake_loco_pressure changing, same shape as
        # "unbrake" but on the independent-brake cylinder rather than the train-brake cylinder.
        "localbrakesound": {
            "event_name": &"local_brake_hiss",
            "controller": &"brake",
            "state_property": "brake_loco_pressure",
            "trigger_mode": TrainSoundTrigger.TriggerMode.CONTINUOUS,
            "sound_parameter": &"rate",
            "trigger_threshold_min": -INF,
            "trigger_threshold_max": -0.05,
            "requires_occupied": true,
        },
        "localbrakesound2": {
            "event_name": &"local_brake_hiss2",
            "controller": &"brake",
            "state_property": "brake_loco_pressure",
            "trigger_mode": TrainSoundTrigger.TriggerMode.CONTINUOUS,
            "sound_parameter": &"rate",
            "trigger_threshold_min": 0.05,
            "trigger_threshold_max": INF,
            "requires_occupied": true,
        },
        "brakecylinderinc": {
            "event_name": &"brake_cylinder_increase",
            "controller": &"brake",
        },
        "brakecylinderdec": {
            "event_name": &"brake_cylinder_decrease",
            "controller": &"brake",
        },
        "epbrakeinc": {
            "event_name": &"ep_brake_increase",
            "controller": &"brake",
        },
        "epbrakedec": {
            "event_name": &"ep_brake_decrease",
            "controller": &"brake",
        },
        "brakeacc": {
            "event_name": &"brake_accelerator",
            "controller": &"brake",
        },
        "releaser": {
            "event_name": &"brake_releaser",
            "controller": &"brake",
        },
        "springbrake": {
            "event_name": &"spring_brake_activate",
            "controller": &"brake",
        },
        "springbrakeoff": {
            "event_name": &"spring_brake_release",
            "controller": &"brake",
        },
    }


static func has_label(label:String) -> bool:
    _ensure_built()
    return _catalog.has(label)


static func get_entry(label:String) -> Dictionary:
    _ensure_built()
    return _catalog.get(label, {})
