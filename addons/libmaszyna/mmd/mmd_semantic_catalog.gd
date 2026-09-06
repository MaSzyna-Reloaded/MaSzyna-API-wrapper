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
        # Confirmed against demo/vehicles/sm42/sm_42_cabin.tscn's own hand-authored wiring
        # (command/state_property/action copied verbatim) and now backed generically by
        # TrainLighting::devices_light()/roof_light() (TrainLighting.cpp) instead of a
        # per-vehicle script - state is power-gated the same way there (24V/110V availability).
        "instrumentlight_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "devices_light",
                "state_property": "devices_light_enabled",
                "action": "devices_light_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "cablight_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "roof_light",
                "state_property": "roof_light_enabled",
                "action": "cabin_light_toggle",
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
        # radiochannel_sw: real vehicles carry different physical radio hardware - some (e.g.
        # "Koliber" units) only have separate next/prev channel buttons
        # (radiochannelnext_sw:/radiochannelprev_sw: below), others (e.g. "Radmor" units) have an
        # actual turnable multi-position selector knob under this label - a real interactive
        # control, not just a passive readout, so it needs the same CabinSwitch shape as mainctrl
        # (mouse-turnable + command_increase/decrease), not CabinGauge (display-only, no input).
        # switch_min/max_position match radio_channel_min/max's own real default range
        # (TrainController.hpp - 1..10, the same range the original engine hardcodes universally
        # in OnCommand_radiochannelset). value_offset=1: confirmed real in-game - channel 1
        # (switch_position=1) was rendering the knob one full step past its physical rest
        # position, and decrease could never visually return to rest, because the knob's own
        # first notch corresponds to switch_position=1, not 0 (channel 0 isn't a valid radio
        # channel at all) - the same "state domain doesn't start where the mesh's rest position
        # is" mismatch as enrot/brakectrl, just an integer shift instead of a scale/unit one.
        "radiochannel_sw": {
            "widget_class": CabinSwitch,
            "fixed_fields": {
                "switch_min_position": 1,
                "switch_max_position": 10,
                "value_offset": 1,
                "command_increase": "radio_channel_increase",
                "command_decrease": "radio_channel_decrease",
                "state_property": "radio_channel",
                "action_increase": "radio_channel_increase",
                "action_decrease": "radio_channel_decrease",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
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
        # The E3D indicator follows the plain Radio flag, matching vehicle/Train.cpp:9160.
        # The separate OmniLight follows radio_powered and copies SM42's hand-authored
        # RadioPowerLed parameters; unlike the indicator mesh, its glow requires supply power.
        "i-radio": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {
                "state_property": "radio_enabled",
            },
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
            "light_widget_class": CabinOmniLight3D,
            "light_fixed_fields": {
                "state_property": "radio_powered",
                "light_color": Color(0.0, 0.738281, 0.121986, 1.0),
                "light_energy": 0.007,
                "light_energy_on": 0.05,
                "light_energy_off": 0.0,
                "omni_range": 0.1,
            },
        },
        # i-* labels are indicator meshes in MaSzyna: the widget only switches the matching
        # <submodel>_on/<submodel>_off pair. The optional light widget below is a separate Godot
        # lighting effect anchored at the same submodel when that position exists.
        # Numeric parameters come from SM42's hand-authored reference cabin.
        "i-cablight": {
            "widget_class": CabinIndicator3D,
            "fixed_fields": {
                "state_property": "roof_light_enabled",
            },
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
            "light_widget_class": CabinSpotLight3D,
            "flip_upward_spotlight": true,
            "light_fixed_fields": {
                "state_property": "roof_light_enabled",
                "light_enabled": true,
                "light_color": Color(0.960938, 0.881759, 0.75824, 1.0),
                "light_energy_on": 0.411,
                "light_energy_off": 0.0,
                "light_volumetric_fog_energy": 16.235,
                "light_size": 1.0,
                "light_specular": 5.297,
                "spot_range": 2.785,
                "spot_attenuation": 1.44,
                "spot_angle": 63.62,
                "spot_angle_attenuation": 1.27456,
            },
        },
        "i-instrumentlight": {
            "widget_class": CabinIndicator3D,
            "fixed_fields": {
                "state_property": "devices_light_enabled",
            },
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
            "light_widget_class": CabinOmniLight3D,
            "light_fixed_fields": {
                "state_property": "devices_light_enabled",
                "light_color": Color(0.808594, 0.518647, 0.06633, 1.0),
                "light_energy_on": 0.002,
                "light_energy_off": 0.0,
                "light_indirect_energy": 0.525,
                "light_size": 0.078,
                "omni_range": 0.564628,
                "omni_attenuation": 1.41,
            },
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
        # Confirmed against vehicle/Train.cpp:5267-5316 (OnCommand_headlighttoggleleft/enableleft
        # etc.) and TrainLighting::light_switch()'s own doc comment (TrainLighting.hpp) - these ten
        # MMD switch labels are cab-relative: upperlight_sw:/leftlight_sw:/rightlight_sw:/
        # leftend_sw:/rightend_sw: (no "rear" prefix) toggle whichever physical end is the
        # CURRENTLY ACTIVE cab's own front, so their own switch position mirrors TrainLighting's
        # "active_..." state; rearupperlight_sw:/etc. toggle the opposite end, mirroring
        # "opposite_...". command_param is light_switch()'s own p_light argument - the MMD label's
        # suffix after stripping "light"/"_sw" (confirmed real examples from that doc comment:
        # "upper", "left", "leftend", "rearupper", "rearleftend"), NOT the full MMD label. action
        # reuses the headlight_*_toggle/redmarker_*_toggle InputMap actions (demo/project.godot) -
        # same one action per physical switch as headlight_left_toggle etc. already use for the
        # keyboard-only fallback.
        "upperlight_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "light_switch",
                "command_param": "upper",
                "state_property": "lights/active_headlight_upper_enabled",
                "action": "headlight_upper_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "leftlight_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "light_switch",
                "command_param": "left",
                "state_property": "lights/active_headlight_left_enabled",
                "action": "headlight_left_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "rightlight_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "light_switch",
                "command_param": "right",
                "state_property": "lights/active_headlight_right_enabled",
                "action": "headlight_right_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "leftend_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "light_switch",
                "command_param": "leftend",
                "state_property": "lights/active_redmarker_left_enabled",
                "action": "redmarker_left_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "rightend_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "light_switch",
                "command_param": "rightend",
                "state_property": "lights/active_redmarker_right_enabled",
                "action": "redmarker_right_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "rearupperlight_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "light_switch",
                "command_param": "rearupper",
                "state_property": "lights/opposite_headlight_upper_enabled",
                "action": "headlight_rear_upper_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "rearleftlight_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "light_switch",
                "command_param": "rearleft",
                "state_property": "lights/opposite_headlight_left_enabled",
                "action": "headlight_rear_left_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "rearrightlight_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "light_switch",
                "command_param": "rearright",
                "state_property": "lights/opposite_headlight_right_enabled",
                "action": "headlight_rear_right_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "rearleftend_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "light_switch",
                "command_param": "rearleftend",
                "state_property": "lights/opposite_redmarker_left_enabled",
                "action": "redmarker_rear_left_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        "rearrightend_sw": {
            "widget_class": CabinButton,
            "fixed_fields": {
                "monostable": false,
                "command": "light_switch",
                "command_param": "rearrightend",
                "state_property": "lights/opposite_redmarker_right_enabled",
                "action": "redmarker_rear_right_toggle",
            },
            "config_max_property": "",
            "mesh_path_field": "mesh_path",
        },
        # Confirmed against vehicle/Train.cpp:9199-9208 - these read the mover's own already-
        # resolved per-end light bitmask (MOVER.h's iLights[front]/iLights[rear], tested against
        # the `light::` bit flags) directly, not the raw selector position - iLights is the
        # final, live "which bulbs are actually lit" result (already accounts for
        # light_power/selector position/wiring), added to TrainLighting's own state
        # (lights/front_headlight_upper_enabled etc., TrainLighting.cpp) specifically for these
        # labels.
        # "upper"=headlight_upper, "left/right light"=headlight_left/right (white),
        # "left/right end"=redmarker_left/right (red tail/end-of-train markers) - confirmed via
        # the same bit flags MOVER.h itself defines. No blink_time - these are steady on/off,
        # unlike the alerter/SHP indicators. light_enabled stays at its default (false) - same
        # "no real per-vehicle lamp reference data" reasoning as i-radio/i-security_cabsignal.
        "i-upperlight": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {"state_property": "lights/front_headlight_upper_enabled"},
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
        },
        "i-leftlight": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {"state_property": "lights/front_headlight_left_enabled"},
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
        },
        "i-rightlight": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {"state_property": "lights/front_headlight_right_enabled"},
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
        },
        "i-leftend": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {"state_property": "lights/front_redmarker_left_enabled"},
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
        },
        "i-rightend": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {"state_property": "lights/front_redmarker_right_enabled"},
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
        },
        "i-rearupperlight": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {"state_property": "lights/rear_headlight_upper_enabled"},
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
        },
        "i-rearleftlight": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {"state_property": "lights/rear_headlight_left_enabled"},
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
        },
        "i-rearrightlight": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {"state_property": "lights/rear_headlight_right_enabled"},
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
        },
        "i-rearleftend": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {"state_property": "lights/rear_redmarker_left_enabled"},
            "config_max_property": "",
            "mesh_path_field": "",
            "position_at_submodel": true,
        },
        "i-rearrightend": {
            "widget_class": CabinSpotLight3D,
            "fixed_fields": {"state_property": "lights/rear_redmarker_right_enabled"},
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
