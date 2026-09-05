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
        # The original engine's own approach for "i-*:" indicator lights (Train.cpp's TButton) is
        # to show/hide a matching "<submodel>_on"/"<submodel>_off" mesh pair - not reproduced here.
        # Instead this reuses CabinSpotLight3D (already a generic, reusable addon widget - not
        # SM42-specific), positioned at the "czuwak" submodel MMD actually names (see
        # _position_at_submodel()) instead of SM42's own 3 hand-placed "CzuwakOmni" lights (their
        # exact 3D offsets are that specific cab's own hand-tuned art, not derivable from MMD - one
        # light at the submodel's own transform is the closest generic equivalent). Numeric light
        # parameters (color/energy/range/angle/specular/volumetric fog) are copied from
        # CzuwakOmni1 - light_projector (a demo-specific texture asset,
        # res://vehicles/sm42/czuwak_projector.png) is deliberately NOT copied: an
        # addons/libmaszyna/ catalog can't depend on demo/ content.
        #
        # SM42's OWN CzuwakOmni1/2/3 have no state_property at all - the actual flashing there
        # comes from a separate CabinBlinker node ("Czuwak", cabin_blinker.gd) with its own
        # internal Timer, driving a `blink` signal a cabin-script handler uses to toggle those
        # lights externally. blink_time (below) ports that same Timer-based flash directly into
        # CabinSpotLight3D itself instead, so this stays one widget per MMD label.
        # Confirmed against TrainController.cpp:46/265,429 - exact command+state pair already
        # proven in production via SM42's own hand-authored Battery node.
        "battery_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "battery",
                "state_property": "battery_enabled",
                "action": "battery_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        # Confirmed against TrainElectricEngine.cpp:160,172,322 - converter()/converter_enabled.
        "converter_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "converter",
                "state_property": "converter_enabled",
                "action": "converter_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        # Confirmed against TrainElectricEngine.cpp:159,170,323 - compressor()/compressor_enabled -
        # the switch label (vehicle/Train.cpp:11875, "compressor_sw:" -> ggCompressorButton), not
        # to be confused with "compressor:"/"compressorb:" (the pressure GAUGE, still genuinely
        # missing - no raw pressure value exists in the wrapper, only the enabled/allowed booleans).
        "compressor_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "compressor",
                "state_property": "compressor_enabled",
                "action": "compressor_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        # CORRECTED: radiochannel_sw: is NOT the switch that changes the channel - confirmed
        # against vehicle/Train.cpp:11894 ({"radiochannel_sw:", ggRadioChannelSelector}) and
        # :11370 (ggRadioChannelSelector.PutValue((RadioChannel())-1), called every frame from the
        # gauge-update pass, not from a command handler) - it's a passive rotary POSITION
        # indicator ("pokrętło"/knob), the same TGauge shape as tachometer/enrot, not a switch.
        # The actual increase/decrease controls are the two SEPARATE momentary buttons below
        # (radiochannelnext_sw:/radiochannelprev_sw:, vehicle/Train.cpp:11895-11896). state_property
        # is the wrapper's own "radio_channel" (1-based, TrainController.cpp:436) - one step off
        # from what the original's own gauge is actually fed (RadioChannel()-1, 0-based), so the
        # knob's rest position will be rotated by one channel-step's worth of MMD scale versus a
        # pixel-perfect port; same class of small domain mismatch as enrot/brakectrl, not a guess.
        "radiochannel_sw": {
            "widget_class": CabinGauge,
            "fixed_fields": {
                "state_property": "radio_channel",
                "max_value": 1.0,
            },
            "config_max_property": "",
            "mesh_path_field": "target_mesh_path",
        },
        # Momentary buttons (vehicle/Train.cpp:8103-8135: ggRadioChannelNext/Previous.UpdateValue
        # on press/release, exactly like ggHornButton) - controller_mode=On (not the CabinButton
        # default OnOff) so the command fires exactly ONCE per press, not once on press AND once on
        # release: radio_channel_increase/decrease take an int step, not a persistent on/off state,
        # so a second call on release would double-step the channel. Sending `true` as p1 relies
        # on the same `p_step > 0 ? p_step : 1` guard as CabinSwitch's zero-arg call (both convert
        # to step=1) - confirmed, not a guess, since main_on_bt/main_off_bt already prove
        # ControllerMode.On/Off's single-shot-on-press behavior. No state_property: neither button
        # has a wrapper-tracked "is pressed" readback, same as releaser_bt/security_reset_bt.
        "radiochannelnext_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": true,
                "command": "radio_channel_increase",
                "controller_mode": CabinButton.ControllerMode.On,
                "action": "radio_channel_increase",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "radiochannelprev_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": true,
                "command": "radio_channel_decrease",
                "controller_mode": CabinButton.ControllerMode.On,
                "action": "radio_channel_decrease",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        # Confirmed against TrainElectricEngine.cpp:313-318 - pantograph(PantographSelector,bool)
        # takes the selector as its FIRST argument (TrainSystem.cpp:203-209 maps send_command's p1
        # to the Callable's first arg, p2 to the second) - command_param supplies the fixed
        # PANTOGRAPH_FIRST selector, pushed supplies the enabled bool as p2. state_property
        # confirmed against TrainElectricEngine.cpp:201 (`Pantographs[0].is_active`) -
        # MOVER.h:154's `end { front = 0, rear = 1 }` confirms index 0 really is the front
        # pantograph, matching PANTOGRAPH_FIRST's own front mapping (TrainElectricEngine.cpp:316).
        "pantfront_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "pantograph",
                "command_param": TrainElectricEngine.PANTOGRAPH_FIRST,
                "state_property": "current_collector/pantograph_first_active",
                "action": "pantograph_front_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        # Confirmed against TrainController.cpp:422 - internal_state["total_distance"] =
        # p_mover->DistCounter, the exact same field the original engine's own distcounter: gauge
        # binds (vehicle/Train.cpp:12370-12374, gauge.AssignDouble(&mvControlled->DistCounter)) -
        # loaded with no explicit mul argument there, so max_value=1.0 (the same "no correction"
        # convention as tachometer/enrot) is correct, not a guess.
        "distcounter": {
            "widget_class": CabinGauge,
            "fixed_fields": {
                "state_property": "total_distance",
                "max_value": 1.0,
            },
            "config_max_property": "",
            "mesh_path_field": "target_mesh_path",
        },
        # Confirmed against TrainController.cpp:443 - internal_state["current1"] =
        # p_mover->ShowCurrent(1), matching the original engine's own hvcurrent1: gauge
        # (vehicle/Train.cpp:12137-12142, gauge.AssignFloat(fHCurrent + 1)) in its default path
        # (vehicle/Train.cpp:8638-8641, fHCurrent[1] = mvControlled->ShowCurrent(1) - a plain,
        # unmultiplied passthrough). The one case NOT reproduced: when the vehicle is a
        # multi-unit EZT with ShowNextCurrent toggled on, the original engine instead shows
        # mvSecond's (the other physical unit's) ShowCurrent(1)*1.05 - a driver-facing "peek at
        # next unit's ammeter" feature this wrapper has no command/state for at all, out of scope
        # here.
        "hvcurrent1": {
            "widget_class": CabinGauge,
            "fixed_fields": {
                "state_property": "current1",
                "max_value": 1.0,
            },
            "config_max_property": "",
            "mesh_path_field": "target_mesh_path",
        },
        # Confirmed against vehicle/Train.cpp:9160 (btLampkaRadio.Turn(mvOccupied->Radio)) - the
        # indicator condition is the plain Radio flag itself, NOT gated by Power24v/110v
        # availability (unlike e.g. the radio-call/radio-stop commands, which do check power) -
        # so state_property is "radio_enabled" (TrainController.cpp:433), not "radio_powered".
        # SM42's own hand-authored cabin has no real per-vehicle radio lamp to copy
        # light_color/spot_range/etc. from (only its czuwak (alerter) lamps have real numeric
        # data), so light_enabled is left at CabinSpotLight3D's own default (false) - the light
        # itself stays off; only the on_target/off_target submodel pair and the click sound (if
        # soundinc:/sounddec: are present) are driven from state.
        "i-radio": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {
                "state_property": "radio_enabled",
            },
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
        },
        # Confirmed against Mover.cpp:183-188 (is_cabsignal_blinking(): `return power &&
        # cabsignal_active` - same static "alert active" shape as is_blinking(), not a real-time
        # oscillating value) and TrainSecuritySystem.cpp:64 (p_state["cabsignal_blinking"]).
        # Same reasoning as i-radio above: SM42's own hand-authored cabin has no dedicated SHP
        # indicator light to copy real numeric params from (only a "czuw_shp" SecurityAcknowledge
        # BUTTON mesh, not a light), so light_enabled stays at its default (false); on_target/
        # off_target submodel toggling and the click sound still work.
        "i-security_cabsignal": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {
                "state_property": "cabsignal_blinking",
                "blink_time": 0.2,
            },
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
        },
        "i-security_aware": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {
                "state_property": "blinking",
                # "blinking" (TrainSecuritySystem::is_blinking(), Mover.cpp) is a STATIC "alert
                # active" flag, not a real-time oscillating value - blink_time (matching
                # CabinBlinker's own default, cabin_blinker.gd) is what actually makes the light
                # flash instead of just turning steadily on.
                "blink_time": 0.2,
                # the ONE catalog entry with real reference light data to copy (SM42's own
                # CzuwakOmni1) - every other indicator label defaults to light_enabled=false.
                "light_enabled": true,
                "light_color": Color(0.960938, 0.506832, 0.349091, 1.0),
                "light_energy_on": 0.2,
                "light_energy_off": 0.0,
                "light_size": 0.696,
                "light_specular": 2.014,
                "light_volumetric_fog_energy": 16.0,
                "shadow_enabled": true,
                "spot_range": 2.129,
                "spot_attenuation": 1.98,
                "spot_angle": 69.32,
            },
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
        },
    }


static func has_label(label:String) -> bool:
    _ensure_built()
    return _catalog.has(label)


static func get_entry(label:String) -> Dictionary:
    _ensure_built()
    return _catalog.get(label, {})
