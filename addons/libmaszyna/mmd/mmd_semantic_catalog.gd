extends RefCounted
class_name MmdSemanticCatalog

## Etap A+B's supported MMD label -> cabin widget mapping. command/state_property/action strings
## are copied from demo/vehicles/sm42/sm_42_cabin.tscn's already-shipped, already-working
## hand-authored wiring - those are wrapper-API choices (command names, state keys), not visual
## data, so they're the same for every vehicle regardless of cabin appearance.
##
## Animation SHAPE (how far a lever rotates/slides) is deliberately NOT stored here - it is
## computed per vehicle from that vehicle's own MMD `rot`/`mov` line by
## MmdCabinInstancer._apply_animation_shape(), because cabin geometry differs between vehicles
## (confirmed: ST44 and SM42 use different scale values for the same label) and a single fixed
## constant can only ever be right for the one vehicle it was copied from. The original engine's
## own TGauge formula (`scaled = value*scale + offset`, then `rot` applies `scaled*360°`) is
## evaluated at value=1 for a fixed "pushed"/"per-unit" target - confirmed against real data
## (section 4.3 of the feasibility doc: "instrumentlight_sw ... rot -0.2" means -72° at value 1,
## i.e. -0.2*360). This holds for any state property whose domain matches what MMD assumes
## (raw switch positions, physical pressures/speeds) - the one confirmed exception is
## brakectrl, bound to our wrapper's own normalized (0..1) brake position, a different numeric
## domain than MaSzyna's raw brake-handle units MMD's scale is calibrated against, so its
## MMD-derived shape may not be visually exact until the wrapper exposes a raw equivalent.
##
## Any MMD label not listed here gets no widget and no animation (see
## MmdImportContext.warn_unsupported_label) - there is no correct state source to guess from,
## so nothing is built rather than something wrong.

static var _catalog:Dictionary = {}
static var _built:bool = false


static func _ensure_built() -> void:
    if _built:
        return
    _built = true

    _catalog = {
        "mainctrl": {
            "widget_class": CabinSwitch,
            "fixed_fields": {
                "switch_min_position": 0,
                "switch_max_position": 10,
                "command_increase": "main_controller_increase",
                "command_decrease": "main_controller_decrease",
                "state_property": "controller_main_position",
                "action_increase": "main_controller_increase",
                "action_decrease": "main_controller_decrease",
            },
            "config_max_property": "main_controller_position_max",
            "mesh_path_field": "mesh_path",
        },
        # jointctrl (combined main controller + local/dynamic brake handle, e.g. SM42's own
        # nastawnik) - confirmed via input/drivermouseinput.cpp + vehicle/Train.cpp
        # (OnCommand_mastercontrollerincrease): the ORIGINAL engine's mainctrl:-style
        # increase/decrease commands already special-case joint-controller vehicles, releasing
        # the independent brake first when its negative/brake range is active. Our wrapper's
        # main_controller_increase/decrease (TrainController.cpp:482-490) is the plain
        # mover->IncMainCtrl()/DecMainCtrl() version without that special-casing, so this gives
        # real, working throttle control for the power range only - the brake-fusion (negative)
        # range is NOT reproduced (would need a new C++ command mirroring
        # OnCommand_jointcontrollerset's full logic, separate future work) and is intentionally
        # left unbound here rather than silently mismapped.
        "jointctrl": {
            "widget_class": CabinSwitch,
            "fixed_fields": {
                "switch_min_position": 0,
                "switch_max_position": 10,
                "command_increase": "main_controller_increase",
                "command_decrease": "main_controller_decrease",
                "state_property": "controller_main_position",
                "action_increase": "main_controller_increase",
                "action_decrease": "main_controller_decrease",
            },
            "config_max_property": "main_controller_position_max",
            "mesh_path_field": "mesh_path",
        },
        "dirkey": {
            "widget_class": CabinSwitch,
            "fixed_fields": {
                "switch_min_position": -1,
                "switch_max_position": 1,
                "command_increase": "direction_increase",
                "command_decrease": "direction_decrease",
                "state_property": "direction",
                "action_increase": "direction_increase",
                "action_decrease": "direction_decrease",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "brakectrl": {
            "widget_class": CabinKnob,
            "fixed_fields": {
                "value_min": 0.0,
                "value_max": 1.0,
                "command": "brake_level_set",
                "state_property": "brake_controller_position_normalized",
                "action_increase": "brake_level_increase",
                "action_decrease": "brake_level_decrease",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
            # brake_level_set expects a normalized 0..1 level (TrainBrake.cpp converts it back to
            # raw internally), so the widget's own value/command domain has to stay normalized -
            # but MMD's scale is calibrated against the raw handle range (TrainBrake.cpp's
            # fBrakeCtrlPos), so the animation shape needs rescaling by that same raw range or the
            # lever visibly over/under-rotates. See _apply_animation_shape()'s doc comment.
            "animation_range_config_properties": ["brakes_controller_position_min", "brakes_controller_position_max"],
        },
        "security_reset_bt": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": true,
                "command": "security_acknowledge",
                "action": "security_acknowledge",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "releaser_bt": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": true,
                "command": "brake_releaser",
                "action": "brake_release",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        # Confirmed against Train.cpp's own cabin gauge dispatch table (horn_bt:/hornlow_bt:/
        # hornhigh_bt:/whistle_bt: -> ggHornButton/ggHornLowButton/ggHornHighButton/
        # ggWhistleButton) and TrainHorns' own low/high/whistle command+state model (see
        # TrainHorns.hpp) - state_property is the RAW commanded press (unaffected by the
        # emergency-brake override), matching the original's UpdateValue() calls firing straight
        # from the command handler, not the combined "_active" (sound-triggering) state.
        # hornlow_bt:/hornhigh_bt: are the dedicated per-slot buttons (present together on ~80
        # real vehicles). action points at dedicated "horn_low"/"horn_high"/"whistle" InputMap
        # actions (demo/project.godot) - matching TrainHorns' own command naming, not sm42_v1's
        # older horn1/horn2 shim naming (that scene's own action_increase/action_decrease were
        # updated to match).
        #
        # horn_bt: is the single SHARED button used instead on the far more common (~220 real
        # vehicles) case where a vehicle has only one physical horn control. Confirmed real:
        # OnCommand_hornlowactivate's AND OnCommand_hornhighactivate's own null-checks
        # (`ggHornButton == nullptr && ggHornLow/HighButton == nullptr`) both pass as soon as
        # ggHornButton alone exists - so in the original engine ONE horn_bt: button already
        # responds to BOTH low and high (swinging the same gauge to -1.0/+1.0 depending on which
        # was pressed), not low-only. Modeled as a CabinSwitch exactly like SM42's own
        # hand-authored "Horn" node (demo/vehicles/sm42/sm_42_cabin.tscn) - a single bidirectional
        # lever using TrainHorns' "horn" compatibility command (signed: >0 activates low, <0
        # activates high) - rather than CabinButton, which can only carry one command and would
        # leave one of the two keys permanently dead whenever only horn_bt: exists.
        "horn_bt": {
            "widget_class": CabinSwitch,
            "fixed_fields": {
                "switch_min_position": -1,
                "switch_max_position": 1,
                "automatic_reset": true,
                "command_set": "horn",
                "state_property": "horn",
                "action_increase": "horn_low",
                "action_decrease": "horn_high",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "hornlow_bt": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": true,
                "command": "horn_low",
                "state_property": "horn_low_pressed",
                "action": "horn_low",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "hornhigh_bt": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": true,
                "command": "horn_high",
                "state_property": "horn_high_pressed",
                "action": "horn_high",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "whistle_bt": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": true,
                "command": "whistle",
                "state_property": "whistle_pressed",
                "action": "whistle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "main_on_bt": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": true,
                "command": "main_switch",
                "controller_mode": CabinButton.ControllerMode.On,
                "action": "main_switch_on",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "main_off_bt": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": true,
                "command": "main_switch",
                "controller_mode": CabinButton.ControllerMode.Off,
                "action": "main_switch_off",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "fuelpump_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": true,
                "command": "fuel_pump",
                "state_property": "fuel_pump_active",
                "action": "fuel_pump_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "oilpump_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": true,
                "command": "oil_pump",
                "state_property": "oil_pump_active",
                "action": "oil_pump_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "radio_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "radio",
                "state_property": "radio_enabled",
                "action": "radio_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        # Confirmed against the original engine's own source (vehicle/Gauge.cpp:182,
        # `scale *= mul`, called from vehicle/Train.cpp's per-label gauge.Load(...) sites):
        # pressure-family gauge labels (brakepress/brakepressb, pipepress/pipepressb, scndpress,
        # limpipepress, cntrlpress, springbrakepress, epctrlvalue, compressor/compressorb,
        # pantpress, brakes) are loaded with mul=0.1, so MMD's own declared scale must be
        # multiplied by 0.1 before use for these specific labels - every other gauge label
        # (confirmed: tachometer, oilpress) uses the default mul=1.0, i.e. no correction. This is
        # NOT a per-vehicle hardcoded guess - mmd_scale_multiplier is the same fixed correction
        # factor the original engine itself applies for this label, on every vehicle.
        "tachometer": {
            "widget_class": CabinGauge,
            "fixed_fields": {
                "state_property": "speed",
                "max_value": 1.0,
            },
            "config_max_property": "",
            "mesh_path_field": "target_mesh_path",
        },
        # Train.cpp:12334-12347: gauge.AssignFloat(fEngine + 1) / (fEngine + 2), where
        # fEngine[i] = mvControlled->ShowEngineRotation(i) = std::abs(enrot) (Mover.cpp:1998) -
        # the RAW rotations-PER-SECOND value, unmultiplied. Our wrapper's "engine_rpm" is already
        # the human-readable RPM (enrot*60 - confirmed from real state: engine_rpm_count=8.2667,
        # engine_rpm=496.0=8.2667*60), a different domain than what MMD's scale assumes, same
        # class of problem as brakectrl's normalized-vs-raw mismatch. "engine_rpm_count" is the
        # correct binding - it IS enrot, matching the original 1:1. mul=1.0 (default, confirmed -
        # no explicit third Load() argument). The hand-authored sm_42_cabin.tscn wires
        # enrot2m's submodel ("obrot01") to "engine_rpm" (not "_count") with its own hand-picked
        # mesh_rotation, which only works because that scene's rotation value was tuned by hand
        # against the ×60 value, not derived from MMD's scale like this catalog is.
        "enrot1m": {
            "widget_class": CabinGauge,
            "fixed_fields": {
                "state_property": "engine_rpm_count",
                "max_value": 1.0,
            },
            "config_max_property": "",
            "mesh_path_field": "target_mesh_path",
        },
        "enrot2m": {
            "widget_class": CabinGauge,
            "fixed_fields": {
                "state_property": "engine_rpm_count",
                "max_value": 1.0,
            },
            "config_max_property": "",
            "mesh_path_field": "target_mesh_path",
        },
        "brakepress": {
            "widget_class": CabinGauge,
            "fixed_fields": {
                "state_property": "brake_air_pressure",
                "max_value": 1.0,
            },
            "config_max_property": "",
            "mesh_path_field": "target_mesh_path",
            "mmd_scale_multiplier": 0.1,
        },
        "pipepress": {
            "widget_class": CabinGauge,
            "fixed_fields": {
                "state_property": "pipe_pressure",
                "max_value": 1.0,
            },
            "config_max_property": "",
            "mesh_path_field": "target_mesh_path",
            "mmd_scale_multiplier": 0.1,
        },
        "oilpress": {
            "widget_class": CabinGauge,
            "fixed_fields": {
                "state_property": "oil_pump_pressure",
                "max_value": 1.0,
            },
            "config_max_property": "",
            "mesh_path_field": "target_mesh_path",
        },
    }


static func has_label(label:String) -> bool:
    _ensure_built()
    return _catalog.has(label)


static func get_entry(label:String) -> Dictionary:
    _ensure_built()
    return _catalog.get(label, {})
