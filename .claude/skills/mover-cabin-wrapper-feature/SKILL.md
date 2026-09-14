---
name: mover-cabin-wrapper-feature
description: Wire a new player-facing Mover control end-to-end in this Godot wrapper - vendored Mover method -> wrapper command -> state/config exposure -> MMD cabin catalog entry (mouse) -> InputMap keybind (keyboard). Use when a cab control (lever, button, knob) does nothing in-game, or when porting a control the wrapper never implemented (e.g. the independent/local brake handle, "localbrake:").
---

# Wiring a new Mover cabin control end-to-end

This wrapper exposes the vendored `TMoverParameters` (`src/maszyna/`, never edited
directly - see [[mover-parity-check]] for tracing *what* the original does) to the
player through several independent layers. A control that "does nothing" is almost
always missing one specific layer, not all of them - find the missing layer instead of
reimplementing the whole stack from scratch. This was confirmed end-to-end while adding
the independent/local brake handle (`localbrake:`), which had **zero** wrapper support
in any layer before.

## The five layers, outside-in

1. **Vendored Mover method** (`src/maszyna/McZapkie/Mover.cpp`/`MOVER.h`) - the ground
   truth. Find it via [[mover-parity-check]]'s Step 2-3 first: what method mutates the
   field (`IncLocalBrakeLevel`/`DecLocalBrakeLevel` for `LocalBrakePosA`), and what real
   command/key triggers it in `~/src/maszyna/Train.cpp` + `command.h` +
   `eu07_input-keyboard.ini`. Never add a new field to the vendored file itself.

2. **Wrapper command** - a method on the relevant `TrainPart` subclass
   (`src/brakes/TrainBrake.cpp`, `src/engines/TrainEngine.cpp`,
   `src/core/TrainController.cpp`, etc.), following the existing sibling pattern
   exactly:
   - Declare in the `.hpp` next to its sibling (e.g. `brake_level_increase()` ->
     `local_brake_increase()`).
   - `ClassDB::bind_method(...)` in `_bind_methods()`.
   - `register_command(...)`/`unregister_command(...)` in `_register_commands()`/
     `_unregister_commands()` (both - forgetting `_unregister_commands()` leaks a
     dangling command entry when the node is freed).
   - Implementation: call the vendored method directly (`mover->IncLocalBrakeLevel(1)`),
     matching this wrapper's existing step-size convention (a single command invocation
     = one discrete notch/step, like `main_controller_increase(step=1)`) rather than
     inventing a new continuous-time API - the original's own continuous key-hold
     behavior is a UI-layer concern (repeat-fire), not something the command needs to
     encode.
   - If the field is already normalized (0..1) in the mover (`LocalBrakePosA`), a
     `_set(double p_level)` variant can assign it directly after `CLAMP` - no need to
     replicate the main brake's raw-Handle-position rescaling dance
     (`brake_level_set`'s `Handle->GetPos(bh_MIN/MAX)` conversion) unless the field
     genuinely uses a different unit.

3. **State exposure** - if a UI widget needs to *read* the control's current position
   (a knob/lever that shows where it is), add it in `_do_fetch_state_from_mover()`,
   right next to the closest existing analogous field for grep-ability
   (`brake_local_position_normalized` next to `brake_controller_position_normalized`).
   Skip this for pure buttons that only ever get toggled, not displayed continuously.

4. **MMD cabin catalog entry** (`addons/libmaszyna/mmd/mmd_semantic_catalog.gd`) - this
   is what makes the *mouse* reach the command. Find the real MMD label first
   (`grep` the vehicle's actual `.mmd` under `~/Games/Maszyna/dynamic/...` - confirmed
   against the original's own `initialize_gauge()`/`initialize_button()` label tables in
   `Train.cpp`, not guessed) and pick the matching `widget_class`:
   - `CabinKnob` for a continuous lever/handle (`command`, `state_property`,
     `action_increase`/`action_decrease`, `value_min`/`value_max`).
   - `CabinSwitch` for a discrete stepped control (`command_increase`/`_decrease`,
     `switch_min_position`/`_max_position`).
   - `CabinButton` for a monostable/bistable button.
   - `CabinIndicator3D` for a passive display-only light/needle with no input at all
     (confirm it's genuinely passive in the original - `TGauge`/`TButton` types in
     Train.cpp are used for BOTH passive displays and draggable/clickable controls, so
     the C++ type alone doesn't tell you which; check whether the original's input
     layer - mouse-drag-on-gauge or a keybind - actually drives it, same check as
     Step 1).
   - A one-line comment citing the exact `Train.cpp` label-table line and the default
     keybind (if any) belongs in the entry, mirroring the existing entries' style - this
     is what lets the next person (or agent) verify the mapping without re-deriving it.

5. **InputMap keybind** (`demo/project.godot`'s `[input]` section) - required for
   *keyboard* to reach the command. `CabinKnob`/`CabinSwitch` widgets already poll their
   own `action_increase`/`action_decrease` action names directly
   (`Input.is_action_pressed(...)` in `cabin_knob.gd`/`cabin_switch.gd`) - so as long as
   the catalog entry's `action_increase`/`action_decrease` strings match a real
   `[input]` action name, no extra node is needed. Add the InputMap action as a raw
   `InputEventKey` block matching the surrounding entries' exact JSON shape (Godot
   doesn't write these by hand elsewhere in this project - copy an existing block and
   change `keycode`/`unicode`). For a numpad key, `keycode = unicode_digit + 4194390`
   holds across every existing numpad-bound action in this file (verified against
   `brake_level_increase`/`_decrease`/`_drive`'s own keycode/unicode pairs) - use that
   instead of guessing Godot's `KEY_KP_*` enum values from memory. Prefer the original
   engine's own default key (`eu07_input-keyboard.ini`) when it doesn't collide with
   this demo's already-diverged scheme (check what's already bound near it first - this
   demo reassigned the *main* brake to a numpad-clock layout (9/3/6/4) that differs from
   vanilla, so don't assume vanilla's key for an unrelated control is still free without
   checking).
   For a control with **no MMD instrument at all** (a pure keyboard driver aid, like
   `brake_level_set_position("drive")`), skip layer 4 and instead add a `CabinCommand`
   node in `dynamic_train_cabin.gd`'s `_build_driver_aid_commands()` (see
   `BrakeLevelSet_Drive` there for the exact pattern:
   `action_name`/`command`/`command_param`/`controller_path`).

## Verifying

Rebuild (`make compile-debug`) after any C++ change, then run the full GUT suite
(`godot --path demo --headless -s addons/gut/gut_cmdln.gd -gdir=res://tests/ -gexit`) -
331+ tests should stay green; a new command with no test coverage is fine to ship
without one unless the operator asks for a test, but never skip the full-suite run
before reporting a fix as done ([[feedback-architecture-rigor]] point 7 - a fix that
"should work" from reading the wiring alone has burned this exact codebase before).
