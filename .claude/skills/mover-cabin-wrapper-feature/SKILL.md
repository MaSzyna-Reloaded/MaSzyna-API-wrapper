---
name: mover-cabin-wrapper-feature
description: Wire a new player-facing Mover control end-to-end in this Godot wrapper - vendored Mover method -> wrapper command -> state/config exposure -> MMD cabin catalog entry (mouse) -> InputMap keybind (keyboard). Use when a cab control (lever, button, knob) does nothing in-game, when it cannot be clicked, dragged or outlined with the mouse (CabinHUDMouseSystem), when its caption or position sounds are wrong, or when porting a control the wrapper never implemented (e.g. the independent/local brake handle, "localbrake:").
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

**Where the logic of a `Train.cpp` handler goes.** Split it by what it touches:
- whatever changes the vehicle (a Mover call, a counter the vehicle keeps: `OperatePantographsValve`,
  `FuelPumpSwitchOff`, the distance counter, Radio-Stop, the light presets) goes into the
  matching `Mover*` component as a command - the cab only chooses which one to send;
- whatever is about the physical control (its position, what a press or a release means for this
  kind of switch) stays in the cab's `legacy_cabin/` behaviour.

**The kind of switch (`TGaugeType`).** An MMD instrument may say `type: push|impulse|return|
delayed|pushtoggle|toggle`; without one it is a toggle (`Gauge.h:89`). The MMD factory maps it
(`MmdCabinInstancer.BUTTON_TYPES`, as `Gauge.cpp:243` reads it) onto
`BaseCabinTool3D.button_type` of every widget - data, no logic. Some original handlers branch on it
(`ggX.type()`; `rg -o 'gg\w+\.type\(\)' Train.cpp` lists them): for those the catalog entry
carries `"shape_from_button_type": true` with a comment naming the handler line, and the factory
then makes a push spring back (`monostable`) and show no state at rest (the original returns it to
neutral on release). The behaviour owning the control gets its type from
`LegacyCabinLogicDelegate._button_type(control_id)` when it is created, and ports the handler's
branches on it - e.g. `pump.gd` (a push pump runs while held, a two-state one flips and sets
`*SwitchOff`), `pantograph_selected.gd` (`ENABLE_ON`/`NONE` vs `ENABLE`/`DISABLE`),
`main_switch.gd` (only an impulse switch reacts to its release). A control the cab does not model
is a toggle, exactly like an undefined gauge in the original.

Every other control keeps the fixed `monostable` of its entry: most original handlers act on press
and release whatever the gauge (`sand_bt`, `security_reset_bt`), and flipping them by type would
break them. The fixed `monostable` is also what the keyboard path of an unmodelled control
(`unmodelled_controls.gd`) uses.

How the factory builds such a control: a `CabinButton` shows `offset + mesh_rotation * value`, and
`value_rest` is where a spring-back switch returns to (0.5 for a three-position lever). A catalog
`state_light` (`{state_property, lit_condition}`) builds a `CabinIndicator3D` on the `<name>_on`
submodel, which is how TGauge shows a lit control (`Gauge.cpp:204-210`). A submodel named `none` is
not wired to any mesh.

Keep the original's operation enums out of the interface. For example, the pantograph valves take
our `VehicleElectricEngine.ValveOperation`, and only `MoverElectricEngineBackend` maps it to
`operation_t`. Their start mode comes from the FIZ `Cntrl.` keys, with the defaults of
`LoadFIZ_Cntrl`.

A control of the cab alone, with no vehicle behind it (`universal0..9`, `generictoggle`,
`Train.cpp:6720`), is a catalog entry without a `command`: `forward_commands.gd` then only keeps
its position in `CabinState`, where `python_screen_state.gd` reads it.

## Mouse operation (`CabinHUDMouseSystem`)

The mouse reaches a control through the C++ singleton `CabinHUDMouseSystem`
(`src/cabin/CabinHUDMouseSystem.*`, the original's `drivermouseinput.cpp` + `drivermode.cpp:352`
tooltip). A new widget gets it for free if it follows the pattern; a control that "cannot be
clicked" is usually missing one of these:

- **Registration.** Each widget calls `BaseCabinTool3D._set_mouse_control(mesh, actions, press,
  release, increase, decrease, step_rotation, step_position)` at the one place it resolves `_mesh`
  (`_process_dirty`), and the base frees it on exit. The operations are the widget's own public
  named operations (`press()`/`release()`/`increase()`/`decrease()`/`toggle()`), which the keyboard
  path calls too - never a second copy of the logic for the mouse. An empty `increase` makes the
  control click-only. `step_rotation`/`step_position` are how far one increase moves the mesh
  (`mesh_rotation`/`mesh_position` for a switch, a per-step fraction for a knob).
- **What is hit.** Triangles (`Mesh.get_faces()`), not the bounding box - SM42's `jointctrl`
  (`nastawnik`) is one long submodel with a wheel at each side, and its box swallowed the whole
  desk. A control is its mesh **and every mesh under it**: SM42's brake valve `zasadniczy` has the
  handle `raczkaKranu` and the knob `glowka` as child submodels that rotate with it. Dump a cab's
  E3D tree with `E3DParser.parse(FileAccess)` and `E3DSubModel.resource_name`/`.submodels` to see it.
- **Occlusion.** `DynamicTrainCabin` registers every mesh of its `CabModel` as an occluder once the
  model is loaded (`e3d_loaded`): the desk hides the shaft under it, like the original's pick buffer
  (`opengl33renderer.cpp:1188-1214`). Hand-authored cabin scenes register none (`TODO.md`).
- **Near misses.** Without an exact hit, the control whose middle is within
  `PICK_TOLERANCE_PIXELS` wins if a ray through its middle really reaches it - small desk toggles.
- **Drag direction.** A drag works along one mouse axis, up-down or left-right, chosen by its first
  movement (`DRAG_AXIS_LOCK_PIXELS`) and kept until the button is let go - SM42's valve handle
  swings first down, then to the right, and either gesture works it. Which way an increase goes on
  each axis is taken once, when the control is grabbed, from its grip, not from where the cursor
  landed: a turning control's grip is its point farthest from the axis (SM42's valve `zasadniczy`
  is grabbed by its head next to the axis, where the two sides move opposite ways - the hand thinks
  of the handle `raczkaKranu`); about a nearly horizontal axis (a wheel) it is the top of the
  turning circle (SM42's `jointctrl` wheel pushed forward below its hub turned backwards). Where
  that heuristic gets a control wrong, the catalog entry forces the signs with
  `"mouse_drag_signs": Vector2(x, y)` (x: +1 right, y: +1 down increases) and a quirk comment -
  `brakectrl` does (down/right brakes, as the original's slider: up releases). Right button stays
  the camera's.
- **Outline** is Godot's stencil outline (`STENCIL_MODE_OUTLINE`) put into `material_overlay` -
  never onto the mesh's own material, the cab's materials are shared. Small controls
  (`SMALL_CONTROL_SIZE`) get a heavier ring and a faint tint, large ones a thin ring only.
- **Captions** come from `MmdCabControlCaptions` (port of `locale::label_cab_control`,
  `translation.cpp:174-347`, keyed by MMD label without the colon) through
  `MaszynaLocale.gettext()`, which reads `<game_dir>/lang/<MaszynaRuntime.language>.po`. There is
  no `en.po` - English is the msgids themselves. Key hints come from the widget's InputMap actions;
  captions are taken when the cab is built.
- **State** under the caption is the widget's own (`_set_mouse_state()` -> `control_set_state`):
  on/off, pressed/released, a position number, or a position's name from the widget's
  `position_names` (position -> msgid). The catalog entry fills it: `"position_names"` in
  `fixed_fields` for fixed names (`dirkey`, `horn_bt`), `"position_names_config"` for positions the
  vehicle's config places (`brakectrl`: the handle type's `bh_*` positions, exposed as
  `brakes_controller_position_*`). Every label goes through `MaszynaLocale.gettext()`, which looks in
  the game's `lang/<language>.po` first, then in the wrapper's own `addons/libmaszyna/translations/`
  (Godot translations, one `.po` per game language plus `template.po`, registered in
  `project.godot`). A new state word gets a msgid in all of them.

**Positions of a knob (brake valve).** `CabinKnob.position_min/max` is the raw range its normalized
value spans - set by the instancer from the entry's `animation_range_config_properties` (for
`brakectrl`: `Handle->GetPos(bh_MIN/MAX)`, whole positions = the BCPN rows). MMD `soundN:`/`sound-N:`
go to `sound_positions` and play only when the knob comes to stand on that whole position
(`TGauge::UpdateValue`, `Gauge.cpp:302-343`); otherwise `soundinc:`/`sounddec:`. A knob takes the
mouse continuously (`drag(travel)` passed as the `drag` Callable, not increase/decrease steps): it
moves smoothly, stops in the notch of each whole position and leaves it only when jerked - the pull
against the notch must reach `DETENT_BREAKAWAY_PIXELS` before it relaxes (`DETENT_RELAX_TIME`). This
is our mouse's, not the original's (its mouse sets the handle directly); the keyboard stays
continuous, as FV4a is in the original (`Train.cpp:1960`).

## Verifying

- After any C++ change, rebuild with `make compile-debug` and check the result.
- Parse-check each changed or added GDScript first:
  `godot-double --headless --path demo --check-only -s res://<path>.gd`.
- Run only the test scripts you wrote or modified, one at a time
  (`-gdir=res://tests/ -gselect=<script name>`), with the output redirected to a file. Never run
  the whole suite (`AGENTS.md`, Checks).
- A throwaway headless probe (`-s probe.gd` in the scratchpad) that adds scenes must do it in
  `_initialize()`, not `_init()` - before the tree runs, `_ready()` has not happened yet.
- A headless `--import` or test run rewrites `demo/hud/mover_switches_general.tscn` (adds
  `unique_id`s); revert it with `git checkout` - it is not part of the change.
- A new command needs no test of its own unless the operator asks for one. Do not report a fix
  as done from reading the wiring alone, though: confirm that the command reaches the Mover and
  that the dump key changes.
