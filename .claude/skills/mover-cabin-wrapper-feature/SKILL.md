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

2. **Wrapper command** - every component is an interface/implementation pair, and the
   backend never appears in the interface (`AGENTS.md`):
   - **Interface** `VehicleX` (`src/brakes/VehicleBrake.hpp`, `src/engines/VehicleEngine.hpp`,
     `src/core/VehicleController.hpp`, ...): declare the command as pure virtual next to its
     sibling (`virtual void local_brake_increase() = 0;`, `VehicleBrake.hpp:262`), bind it with
     `ClassDB::bind_method(...)` in `_bind_methods()`, and add it to both
     `_register_commands()` and `_unregister_commands()` (`VehicleBrake.cpp:369,385`) - forgetting
     the second leaks a dangling command entry when the node is freed.
   - **Implementation** `MoverVehicleX` (`src/brakes/MoverVehicleBrake.cpp`,
     `src/core/MoverVehicleController.cpp`, ...): the `override` takes the Mover with
     `get_mover()` (from `MoverComponent`, `src/mover/MoverComponent.hpp`) and calls the vendored
     method directly (`mover->IncLocalBrakeLevel(1)`, `MoverVehicleBrake.cpp:91`).
   - Keep the existing step-size convention: one command invocation is one notch/step, like
     `main_controller_increase(step=1)`, not a new continuous-time API. The original's
     key-hold behavior is a UI-layer concern (repeat-fire).
   - If the Mover field is already normalized (0..1, `LocalBrakePosA`), a `_set(double)` variant
     assigns it after `CLAMP`. Do not replicate the main brake's `Handle->GetPos(bh_MIN/MAX)`
     rescaling unless the field genuinely uses a different unit.
   - Document the new method in `doc_classes/VehicleX.xml` (C++ API only).

3. **State exposure** - if a UI widget needs to *read* the control's position (a knob/lever
   that shows where it is), it becomes a typed state property. Skip this for pure buttons that
   are only toggled, not displayed continuously.
   - **Interface:** a pure virtual const getter plus a read-only `ADD_PROPERTY`
     (`PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY`), next to the closest analogous one
     (`get_local_position_normalized`, `VehicleBrake.hpp:34`, `VehicleBrake.cpp:290-294`).
   - **Implementation:** the getter reads the Mover field and stores nothing
     (`return mover != nullptr ? mover->LocalBrakePosA : 0.0;`, `MoverVehicleBrake.cpp:266`).
     A getter never changes state - no filters, flags or signals in it (`CODE_STYLE.md`).
   - **Dump key:** publish it in the implementation's `_fill_state_dictionary()` next to its
     sibling (`p_state["brake_local_position_normalized"] = get_local_position_normalized();`,
     `MoverVehicleBrake.cpp:351`). This key, as it appears in
     `RailVehicleServer.vehicle_dump_state(rid)`, is what a catalog entry's `state_property` and
     `CabinState.vehicle_state_value()` read.

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

**Sending commands:** code outside the train composition (player, UI, console) sends
commands through the high-level API, `TrainSystem.send_command(train_id, command, p1, p2)`,
using the train id it already tracks (e.g. `MaszynaPlayer.last_controlled_train_id`) - never
`vehicle.get_controller().send_command(...)`. Direct `VehicleController` access is fine only
where the composition already holds that controller (e.g. `VehicleComponent`s).

**Cabin controls are a separate layer (#94):** `CabinButton`/`CabinSwitch`/`CabinKnob`/
`CabinCommand` never send vehicle commands. They only report manipulations
(`hold`/`release`/`toggle`/`increase`/`decrease`/`set`) with
`CabinSystem.act(train_id, cab, control_id, action, value)`. `control_id` is the MMD label, set by
`MmdCabinInstancer._build_widget()`. What a manipulation does is decided by handlers registered in
`CabinSystem` by `LegacyCabinLogicDelegate` (`addons/libmaszyna/cabin/legacy_cabin/`), which
`DynamicTrainCabin` adds to every MMD-built cabin:
- `forward_commands.gd` wires every remaining control straight to its vehicle command (from the
  control's `command`/`controller_mode`/`command_set`/... set from `MmdSemanticCatalog`);
- controls with their own cab logic from `Train.cpp` (`OnCommand_*` state, timers) get a dedicated
  behaviour file, e.g. `main_switch.gd` (line breaker held for `InitialCtrlDelay`).

A new cab control that only maps to a vehicle command needs no code there, just the catalog entry.
One whose original behaviour lives in `TTrain` gets a new `legacy_cabin/<name>.gd` behaviour that
claims its control ids. Behaviours reach the train only through `CabinState.vehicle_state()` /
`send_vehicle_command()`, never the Mover. Anything they need to read must first be a key of the
vehicle's dump (`_fill_state_dictionary()`, layer 3) - `CabinSystem` reads
`RailVehicleServer.vehicle_dump_state(rid)`.

## Verifying

- After any C++ change, rebuild with `make compile-debug` and check the result.
- Parse-check each changed or added GDScript first:
  `godot-double --headless --path demo --check-only -s res://<path>.gd`.
- Run only the test scripts you wrote or modified, one at a time
  (`-gdir=res://tests/ -gselect=<script name>`), with the output redirected to a file. Never run
  the whole suite (`AGENTS.md`, Checks).
- A new command needs no test of its own unless the operator asks for one. Do not report a fix
  as done from reading the wiring alone, though: confirm that the command reaches the Mover and
  that the dump key changes.
