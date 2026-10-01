# TODO

## Architecture rework (#184) - remaining stages

The vehicle becomes an object owned by a server, addressed by RID, with thin `*Node` proxies for
the editor. Each stage is one PR, titled `(#184) <area> - <what>`, and each leaves the game
runnable. Stages 1-3 are done (`RailVehicleServer` owns placement, movement and the step;
`MaszynaMoverVehicleServer` owns the `TMoverParameters`, the controller and components take it through
`attach_implementation()`).

Design that replaced the withdrawn stage 4 (a global name registry, now deleted):

* **State is a typed getter that reads the backend field** (`RailVehicleBrake::get_pipe_pressure()`
  returns `mover->PipePress`), bound but not a property - a property is stored configuration. A component keeps only what the Mover has not got: the brake
  pressure filter, the door interpolation, the wiper positions, a `_prev` for change detection.
* **Config stays the wrapper's**, deliberately - written at (re)configuration, read rarely. It is
  the one intentional duplicate.
* **Dumps are lazy and separate:** `_fill_state_dictionary`/`_fill_config_dictionary` per
  component, composed by `RailVehicleServer::vehicle_dump_state(rid)`. A key the vehicle's variant
  has not got is not written, so `has()` keeps its meaning.

**Stage status, verified against the code on 2026-09-25** (earlier claims in this file were
written when the first part of a stage landed and never corrected - check against the code, not
against this file; the 09-24 list had already gone stale in G by the time it was re-measured):

* **A - done.** `doc_classes/RailVehicleServer.xml` is clean now.
* **B - partial.** **The hottest path misses the one state cache** - see the next block.
  None of the five
  common values (`velocity`, `speed`, `mass_total`, `total_distance`, `direction`) is a property;
  only the first two have a server forwarder. `Dictionary config` has left the controller.
* **C - partial.** Done: the component is an `Object`, fetch/tick split, interface/implementation
  split (`MoverRailVehicle<Domain>` classes, each with a `RailVehicle<Domain>` interface of the same
  name, none of which names the Mover), and **`vehicle_component_get(rid, type)` now exists and is
  bound** (`RailVehicleServer.hpp:222`). Missing: `vehicle_component_create` (zero occurrences) -
  components still come from `ClassDBSingleton::instantiate()`/`memnew`;
  `vehicle_generic_component_find` has zero callers; `GenericVehicleComponentNode` finds its
  vehicle via `get_parent()` - deliberate today ("the vehicle is whatever this node sits under"),
  so turning it round changes how a modder authors a component and wants deciding first;
  `PROPERTY_USAGE_SCRIPT_VARIABLE` appears nowhere, so the script still says what it publishes
  through `_get_component_state` rather than being walked for its own variables; the
  dump mixes nine `prefix/key` namespaces with flat `component_key` names. Typed state names drop
  the prefix (`brake_pipe_pressure` -> `brakes.pipe_pressure`); dump keys keep it.
* **D - one of three.** The controller is a `RefCounted` `Resource`, and the vehicle has one
  handle: `VehiclePhysicsNode` creates it and its controller in `VehicleServer`
  (`controller_configure`, `vehicle_bind_controller`), from its `controller` property or, given
  none, from `_build_controller()` (once, on entering the tree - `MaszynaRailVehiclePhysicsNode`
  asks `FizVehicleBuilder`); the live controller is `VehicleServer.vehicle_get_controller(rid)`.
  `RailVehicle3D` creates no RID, it only takes the node's (`set_vehicle()`). Not done: the
  construction still lives in the node - it belongs in `vehicle_create()`; registering the name and
  the commands still hangs off `attach_to_system()`.

  **What the simulation already does without a node:** stepping keys off the placements, and the
  step skips only a vehicle `RailVehicleServer` does not hold (`vehicle_is_attached()`). So a
  vehicle can be simulated through the servers alone; the node is needed because the *code that
  constructs the controller* lives in it, which is all this stage has left to move.
* **E - not started.** Zero of the ~20 proxy nodes; no `VehicleControllerNode`.
* **F - done**, what is left of the area:
  * The node's public API is the `.scn` `dynamic` line only: `data_path` + `file_name` + `skin`
    locate the data; exported stay `vehicle_id`, `initial_velocity`, the occupant (`driver_type`),
    the load, and from `RailVehicle3D` `start_track_name`, `start_track_offset`, `start_direction`
    (unused in a `TrainSet3D`, which places its vehicles) and `head_display_material`. No cabin,
    sound, pantograph or light property belongs on the node (all MMD).
  * The trailing `destination` of the `dynamic` line is dropped
    (`maszyna_node_dynamic_importer.gd`).
* **G - most of it landed.** `@export_node_path("VehicleController")` is gone from **all 14 files**.
  Of the cabin scripts, **20 take a single key** through `CabinSystem.vehicle_state_value()` and
  **9 still take the whole dump**, of which only `cabin_windscreen_wipers.gd` does it per frame.
  `CabinSystem` is keyed on the RID. **Nothing that draws a vehicle reads the dump**:
  `RailVehicle3D` reads nothing at all (2026-09-30), and `RailVehicleRenderingServer` takes every
  value from the components' typed getters (`RailVehicleLighting`, `RailVehicleWipers`,
  `RailVehicleWheels`, the engines). Left: `TrainSoundSystem`
  reads the whole dump, which is right for it (its triggers are keyed by the MMD's own
  `state_property` names), and `cabin_windscreen_wipers.gd` still reads it per frame.
* **H - not started.** No `UpdatePhase`; the order is hand-written in
  `RailVehicleServer::step_frame()` (`:803`). Check it against
  `TMoverParameters::ComputeMovement`/`Update` and the three ordering bugs on record (#57 line
  breaker, `Mred`, `roof_light_enabled`). `test_vehicle_doors.gd` must exist first - `RailVehicleDoors`
  ticks and has no test.
* **I.** `MaszynaMoverPhysicsServer` and `vehicle_get_mover()` are gone, and so is `TrainSystem`:
  vehicles are held and commanded by RID, and `vehicle_id` is only the scenery name in
  `VehicleServer`'s name registry, where it may be empty or repeated. The item left here -
  `RailVehicle3D` as the name's only writer - is moot since the node became a proxy with no name:
  `VehiclePhysicsNode.vehicle_id` is the one writer.

**One state cache, in the vehicle server - done 2026-09-27.** `RailVehicleController::get_state()`
answers from `RailVehicleServer::vehicle_dump_state(get_rid())`, `VehicleController::compose_state()`
is the one place that builds the dictionary and the cache's miss path is its only caller, and
`get_state()` is the base's contract (`= 0`) so there is no second way of producing the state.

**The key stays the step plus the command serial, not the frame.** A step is at least as fine as a
frame, and the second half of the key is there for a recorded reason (`FINDINGS.md`, 2026-09-23): a
command runs synchronously in the middle of a step, so keying on the step alone made the cab act
one keypress late. Moving to a bare frame counter would bring that back.

**The vehicle's name belongs to the vehicle server - done.** `vehicle_set_name()` /
`vehicle_get_name()` / `vehicle_get_rid_by_name()` on `VehicleServer`, like
`TrackServer::track_get_rid_by_name()`, are the only name registry; TrainSystem is gone.
`CabinSystem`'s vehicle-facing surface is keyed on the RID.

**A test must not clobber a global setting.** `test_fiz_train_controller` points
`UserSettings.save_maszyna_game_dir()` at its fixture in `before_all`; a crash skips the restore
and the game then starts with `user://gut/fiz_train_controller` and finds no scenery. Pass the
fixture path to what is tested; the same pattern is in `test_maszyna_rail_vehicle_3d_manager` and
three more (list under Tests).

**Scenery teardown vs. streaming** - `FINDINGS.md`, 2026-09-22. The worker is drained before a
teardown, but its preload (`e3d_model_manager.gd::load_model`, a full `ResourceLoader.load()`)
still creates renderer resources off the main thread, so a teardown overlapping a running stream
can still race. Remedy: parse on the worker, build on the main thread. Reproduce by clearing
`user://cache/rail_vehicle` and `fiz` and running `test_zzz_ep07_cabin_main_switch`.

**`test_zzz_ep07_cabin_main_switch` is non-deterministic, cause not found** - runs of one build gave
5/5, 4/1 and a core dump in `_free_owned_rids`; the commit before the engine work gave 2/3. It
cannot gate anything. It also loads `scenery/td.scn` from the game dir - needs a fixture scenery.

**`test_zzz_ep07_main_switch_trip_diagnostic` is red** (four assertions: no acceleration past
2 m/s over five notches, the Hasler never sees a speed), verified at `76ebf3d` without the #184
work. Check the occupant (`DriverType`)/`CabActive` first - `test_sm42_startup_sequence` was an
unoccupied cab (`FINDINGS.md`, 2026-09-23).

**`test_maszyna_rail_vehicle_3d_manager` is red:** `registration.controller` is `null` - the sound bank
registers against a vehicle with no controller yet, and only the 4 Hz sweep repairs it, later than
the three frames the test waits. Connecting to `ready` (too late) or `tree_entered` (too early)
does not help; it was believed to register against the template, the packing that stage F removed
- re-check now that F has landed, and fix it in the vehicle building, not the sound system.

**The `.fiz` path has not been run in the game** since the components stopped being nodes - only
in tests.

Traps for every stage: bump the cache tag with the code whose output is cached
(`FIZ_PARSER_FORMAT_VERSION`, `MaterialManager.CACHE_VERSION`, `E3DModel.FORMAT_VERSION`,
`structure-vN`); run `godot-double --headless --import` before believing "Identifier not
declared"; never pass a bare `[]`/`{}` to a typed collection; add `doc_classes/<Class>.xml` for
every registered C++ class.

### Rail concepts in interfaces named "Vehicle"

`VehicleComponent`/`VehicleController` are generic on purpose (road vehicles), but several
interfaces are rail-only (rail-term count per header): `RailVehicleBrake` 43 (brake pipe, W/Lu/L,
W/Lu/VI, W/Lu/XR, K valves, FV4a), `RailVehicleElectricEngine` 34 (pantographs), `RailVehicleBuffCoupl` 13,
`RailVehicleWheels` 12 (bogies, pivot spacing, `get_bogie_transform()`), `RailVehicleSecuritySystem` 2,
`RailVehicleSpringBrake`/`RailVehicleElectroPneumaticDynamicBrake` 1-3. Generic and correct:
`RailVehicleWipers`, `RailVehicleUniversalController`, `RailVehicleSpeedControl`, `RailVehicleHorns`,
`RailVehicleDoors`, `RailVehicleHeating`, `RailVehicleLighting`, `RailVehicleLoad`. Either rename to `Train*`
(cost: `RailVehicleWheels` 13 files / 50 mentions, `RailVehicleBrake` 24 / 283) or split a generic base
from a rail subclass - only worth it once something road-side shares the base. Either way the
interfaces keep naming no backend.

### What still reaches a class by name from C++

Allowed (GDScript hosted by C++, commented at the call site): `Cabin3D::_propagate_vehicle_rid()` ->
`set_vehicle_rid`; `GenericVehicleComponent` -> `_process_component`, `_get_component_state`,
`_get_component_config`. Not allowed - our own classes still in GDScript, fixed when their base
moves to C++:

| Class | Named accesses | Where |
| --- | --- | --- |
| `E3DModelInstance` | 3 | `get_e3d_instance`, `e3d_instance_created` (connect, disconnect) in `RailVehicle3D`, commented |
| `TrackCurve` | 10 | `p1`, `c1`, `c2`, `p2`, `roll1`, `roll2` in `TrackServer` and `RailVehicleServer` |

### Readers still on the state dump (2026-09-30)

The sound system, the AI driver, the player, the external camera and the auto-rewident read
components now (`CODE_STYLE.md`, "A hot path reads a component, never a dump"). Left on the dump:

* `demo/hud/` - `driving_aid.gd`, `vehicle_card.gd`, `vehicle_selector_row.gd`,
  `mover_switches_*.gd`, `knob.gd`, `switch.gd`: HUD widgets refreshed on their own timers; the
  ones that name no value from the data belong on components.
* `TrainSoundSystem._build_brake_events()` and `MaszynaBrakeSfxEventFactory.build_events()` read
  the config dump once per vehicle, at build - not a hot path, but the brake handle positions it
  reads have getters now (`RailVehicleBrake.get_handle_position()`).

### Source layout - the GDScript side (deferred 2026-09-27)

`src/` is split into generic layers and the MaSzyna adapter (`src/legacy/`: `maszyna-mover`
vendored, `vehicles`, `signalling`, `e3d`, `parsers`, `scenery`, `cabin`). In `addons/libmaszyna/`
`legacy/` holds `cabin`, `driver`, `e3d`, `fiz`, `materials`, `mmd`, `scenario`, `scenery`, `sound`
and `vehicle` so far; the rest of the MaSzyna-specific scripts outside it (e.g. `sound/maszyna_*`)
is still to decide and move (preload/`res://` paths and `.tscn`/`.tres` references follow).

## Player and HUD - one owner of the player's vehicle and view

* The HUD keeps copies of the player's state: `DrivingAid.vehicle`, `FollowedVehicleChip.vehicle`,
  `PlayerVehicleChip.vehicle` (set from `PlayerServer`/`PlayerCameraServer` signals).
* **The cab's keys live in the 3D cab's widgets (a view) - SoC breach.** `CabinButton._input`
  (cabin_button.gd:120), `CabinSwitch._input` (cabin_switch.gd:154) and `CabinCommand._input`
  (cabin_command.gd:21) catch their input actions and call `CabinSystem.act()` themselves;
  `CabinLogic.input()` takes only the controls no widget has. So the 3D cab has to stand the whole
  time the player drives (`MaszynaPlayer._show_cabin()` on taking over), also while looking from
  outside - hidden there (2026-09-29), the keys stopped working. The original hides it
  (`vehicle->bDisplayCab = false`, drivermode.cpp:1265). To do: the action -> control binding
  (increase/decrease/toggle/hold, repeat, monostable) goes to the cab logic, built from the same
  MMD/catalog as the widgets (`MmdCabinInstancer`, `LegacyCabinControls`); the player's keys reach
  it through `CabinSystem.act()`; the widgets only draw and take the mouse. Then the 3D cab can
  exist only in the CABIN view (`CabinSystem.vehicle_show_cabin()`/`vehicle_hide_cabin()` are
  ready for it).
* The start vehicle is still looked for every frame until the scenery has it
  (`MaszynaPlayer._find_start_vehicle()`), instead of an event saying the trainset is built.

## Cabins

### Controls whose original handler branches on the kind of switch

The MMD factory gives every widget its `gauge_type`, and the catalog entries with
`shape_from_gauge_type` are ported with their `type()` branches (battery_sw, cabactivation_sw,
fuelpump_sw, oilpump_sw, trainheating_sw, pantalloff_sw, main_sw, pantselected_sw,
pantselectedoff_sw, universal0..9). Not in the cab at all yet, so nothing to branch - each needs a
catalog entry and, where missing, a vehicle command: `compartmentlights_sw` (Train.cpp:
OnCommand_compartmentlights*), `waterpump_sw`, `motorblowersfront_sw`/`rear_sw`/`alloff_sw`,
`epbrake_bt` (ggEPFuseButton, Train.cpp:2350), `doorrightpermit_sw` (Train.cpp:7263),
`dooralloff_sw` (Train.cpp:7657, push_delayed), `compressorlist_sw`, `autosandallow_sw`.

### E186 controls - what is still simplified

* The E186 screen's pantograph page (`traxx_renderer.py`, the game's own script) toggles its
  "odbiornik prądu" 1 / 2 / 1+2 without OP1/OP2 being pressed (seen 2026-09-29, left as it is -
  the script's own selector, not checked against the original).
* `pantselect_sw` / `PantsPreset` (choosing which pantographs the master valve raises,
  Train.cpp:3529 change_pantograph_selection, update_pantograph_valves) is not ported.
* `MoverCurrentCollectorUnit::pantograph()` still opens the master valve itself when a pantograph
  is raised (added in 1c0c044 when no cab could reach the valve). The original opens it only from
  pantselected_sw / pantvalves_sw, so with it a pantograph rises from its own key alone. Remove it
  once every cab has a way to the master valve (pantvalves_sw is not in the catalog either).
* Light presets: `SetLights` is run on a preset change only - the original also runs it on cab
  (de)activation, battery and direction changes (Train.cpp:2924-3137). The model's lamp inventory
  (iInventory) is not known, so a rear end that could show red markers or plates shows the markers
  (DynObj.cpp:7367).
* `headlights_dimmed` is state only - nothing renders a headlight beam to dim.
* Distance counter: the double-press start (FIZ `DCMB`/`DCDPP`, not in the vendored Mover), the
  switch-off after the train's length and its sound (Train.cpp:10153) are not ported.
* The radio plays the scenery's radio messages (`MaszynaDynamicTrainCabin`), non-positional - not yet
  at `m_radiosound`'s own place in the cab.

### Gauge lamps (`<name>_on`)

Only the reverser buttons have their `state_light` so far. Train.cpp:11995-12040 binds a flag to
about forty more gauges (speed control buttons, door permits, door step, ...), each needing a
state key and a catalog `state_light`. The lamps also light without low voltage - TGauge gates
them on it (Gauge.cpp:379).

The door permit lamps (`doorleftpermit_sw`/`doorrightpermit_sw` `_on`, `i-doorpermit_left:`/
`_right:`, Train.cpp:11754, 12033) show `m_doorspermitleft/right`: the permit of the cab's side,
blinking by `DoorsPermitLightBlinking` unless a door of that side in the trainset is open or only
open (`IsAnyDoorOpen`/`IsAnyDoorOnlyOpen` of the driver, Train.cpp:8511-8516). The switches
themselves are `LegacyCabinDoorPermits`.

### Mouse operation (CabinHUDMouseSystem) - not ported from drivermouseinput.cpp

* Absolute slider for the `*set` levers (master controller, train and independent brake):
  `mouse_slider` maps 60% of the window height onto the whole range and puts the cursor at the
  current position (drivermouseinput.cpp:27-155). Here a drag moves a control relative to the
  mouse - in steps, or smoothly with notches for a knob.
* Right button as the control's second binding (decrease), panning only off a control
  (drivermouseinput.cpp:405-414). Here the right button always looks around.
* Varying repeat rate while a button is held, growing with the cursor's distance from where it was
  pressed (drivermouseinput.cpp:437-443, 482-484), and the Shift "fast" variants (:418-428).
* Debug-mode tooltip with the submodel name instead of the caption (drivermode.cpp:377-382).
* Key hints only for the first widget of a label - the others have their actions cleared by the
  instancer (mmd_cabin_instancer.gd:396), so their caption shows no keys.
* **Brake valve drag direction - cause not found.** The grip heuristic
  (`CabinHUDMouseSystem::_increase_signs`) got SM42's valve (`brakectrl: zasadniczy`) left-right
  backwards in game, while a headless check on the same cab model (`6d_kabina`, camera at
  `driver1sitpos`, the knob's own rotation applied) predicted the handle moving the way the drag
  went. `brakectrl` now forces its signs in the catalog (`"mouse_drag_signs"`, down/right brakes).
  Find what differs in game (the vehicle's own MMD - 6d/6d1/6da have opposite `rot` signs - the
  occupied cab, the cab's transform) before trusting the heuristic for other valves, and drop the
  force once it is found.
* Captions are taken when a control is built - a language changed while sitting in a cab shows
  after the cab is entered again.
* Hand-authored cabin scenes register no occluders at all.
* EP07: round buttons get a rectangular outline, like the radio's buttons (reported 2026-09-30).
  Not diagnosed - the game data was not at hand. First dump the button's submodel (mesh AABB,
  faces, material transparency/alpha texture): a quad with a round alpha-tested texture outlines
  as its quad, since the overlay stencil knows nothing of the texture.

### DebugWindow

* `debug_hud.tscn` (used by `examples/mover_demo.tscn`) hands the vehicle only to `MoverSwitches`,
  which does not pass it on to its sections - there the sections stay disabled. `game_hud.gd`
  propagates it to every widget.

### Python integration

`PythonScreenServer` runs the original's Python 2 screen scripts, `CabinPythonScreen` draws them on
the cab submodel, `PythonScreenState` maps state onto `TTrain::GetTrainState()` keys. Left out:

* **Windows runtime untested** - should use the game dir's `python27.dll` and `python64/`
  (PyInt.cpp:233); `make python-runtime` builds only the Linux one.
* **Keys with no source yet**:
  * `off_from_dimmer` (dimPositions[modernDimmerPosition].isOff) - the vendored Mover has no
    dimmer positions; RailVehicleSwitches keeps DimmerList/ModernDimmer only as data;
  * `lights_compartments` (CompartmentLights) - the wrapper never drives the Mover's
    CompartmentLights (`compartmentlights_sw` is not ported, see Cabins); the cab light
    (`CabinSystem.cab_get_light_level()`) is not the compartments;
  * `doors_no_N` (iAnimType[ANIM_DOORS]) - the MMD `animations:` count, held by the model layer,
    not the vehicle's state;
  * lamps beyond the five carried (rearendsignals, auxiliary_*) in `lights_front`/`lights_rear`/
    `lights_train_*`;
  * the powered cars are told by engine type (induction motor, diesels); the original tests
    eimc[eimc_p_Pmax] > 1, and fills `eimp_cN_*` of induction cars and `diesel_param_N_*` in one
    shared count.
* **Cab keys**: `universal10`..`29` are read, but only `universal0`..`9` have a catalog entry and
  a key (`generic_toggle_N`), so the higher ones cannot be operated.
* **No AI driver / timetable** - `velocity_desired`, `velroad`, `vellimitlast`, `velsignallast`,
  `velsignalnext`, `velnext`, `actualproximitydist`, `train_atpassengerstop`, `train_length`,
  `trainnumber`, every `train_*` key (TTrainParameters::serialize(), mtable.cpp:641), and
  `$timetable=` (dictionary.cpp:36).
* **Half a test** for `RailVehicleServer.vehicle_get_coupled()`: the order through a turned vehicle
  is covered (`test_train_set_3d.gd`), the stop at a coupling without the flags asked for is not;
  `PythonScreenState.compose()` is tested only for
  the EIM row (test_train_electric_induction_engine.gd).
* **Commands a script returns are not executed** (two scripts send `lightsset`); map
  `simulation::commandMap` names onto vehicle commands (PyInt.cpp:138-194).
* **Touch input** (`touches`, `screen_touch_list`, Train.cpp:10713) is always empty.
* `pyrylandia` is referenced by an MMD and exists nowhere under `dynamic/`.
* **Another game directory keeps the interpreter** and the runtime it was loaded from (CPython 2.7
  with PIL is not started twice in one process): the worker only moves into the new directory. A
  runtime taken from the old game directory (`python2.7/`, `python64/`) stays until a restart.

### Other

* The `horn_bt` lever (`mmd_semantic_catalog.gd`) holds `horn_low` at +1 and `horn_high` at -1;
  the original animates `ggHornButton` the other way round - -1 for the low tone
  (`Train.cpp:7958`). Check against a cab model before flipping it.
* `VirtualCabin` for cabs without a hi-fi model (`cabNmodel: none` or missing, e.g. su46
  `cab0definition:`) - input and command translation only. The original keeps such a cab
  enterable with the low-poly interior (`Train.cpp:8692`, `DynObj.cpp:1214`). Hook:
  `MaszynaDynamicTrainCabin` builds an empty cabin with `has_cab_model = false`.
* Keyboard input per control, not per widget: each `CabinButton`/`CabinSwitch`/`CabinKnob` handles
  `action*` itself, so a repeated label (EP07 cab0 has two `cablight_sw:`) toggled itself back.
  The original maps a key to one command (`Cabine[].bLight`, `Train.cpp:10237`). Move key handling
  to `CabinSystem`/`LegacyCabinLogic` (once per `control_id`), then drop the workaround in
  `MmdCabinInstancer.build_into()` clearing `action*` on repeated labels.
* Diesel-electric shunt mode on the second controller: with `ShuntModeAllow`/`ShuntMode` the
  original moves `AnPos` by 0.025 per step, clamped 0..1 (`Train.cpp:1190-1197`, `1351-1357`);
  only `IncScndCtrl`/`DecScndCtrl` are ported, `shuntmodepower:` (`Train.cpp:10542`) unmapped.
* Shift+V/Ctrl+V (pantograph compressor) work only through `pantcompressor_sw`/
  `pantcompressorvalve_sw`; the original also allows them in cab 0 of the pantograph unit without
  such a switch (`Train.cpp:2872`, `2915`).
* `CabinSwitch` has no `mesh_rotation_offset`/`mesh_position_offset`, so the MMD offset is dropped
  (`MMD_ANIMATION_UNSUPPORTED`, e.g. SM42 `dirkey: kier rot -0.09 0.01`); the original renders
  `value * scale + offset` (`Gauge.cpp:456`), `CabinButton` already does.
* Rest of TDynamicObject::Update's driver block (DynObj.cpp:3240-3400) not ported: the train-wide
  ED/PN brake force split of an induction motor trainset, `EqvtPipePress = GetEPP()`, and the
  unpowered-car copy of MainCtrlPos/SpeedCtrl (DynObj.cpp:3272-3276).
* Wheels turn at half speed: `MoverRailVehicleWheels::_do_process_component` adds `rad_to_deg(V*dt/D)`,
  the original `114.59155... * V * dt / D` = `rad_to_deg(2*V*dt/D)` (DynObj.cpp:3780-3784 at
  df5a8a8). Waiting for the operator.
* Source citations drifted: many `DynObj.cpp`/`Train.cpp`/`Mover.cpp` line numbers point at an
  older checkout (wipers `DynObj.cpp:4048-4115` is 4129-4201 at df5a8a8, `Train.cpp:2912` is
  3682/3695). Refresh against one named revision.
* EIM `Imaxrpc` and `BRVto` (Mover.cpp:11304-11305) not ported - the vendored Mover lacks them.
* Spring brake: `springbrakerelease` (`Train.cpp:6874`) and the `springbrakepress:` gauge
  (`Train.cpp:12221`) have no cab control or key (`eu07_input-keyboard.ini` binds `none` too).
* Intermittent (2026-09-24): after the first cab entry, num4 (`releaser_bt`) and num6
  (`brake_level_drive`) sometimes do nothing until the handle is moved. A headless probe with real
  key events in every cab of `td.scn` worked every time. `CabinKnob` polls `Input` every frame,
  `CabinButton`/`CabinCommand` react only in `_input` - suspect a lost event or a wrong
  `occupied_cab()`. Next time check the log for `Unknown cabin control: ... (cab N)`: present =
  wrong cab, absent = lost event.
* E186 (`dynamic/pkp/e186_v2`) labels outside `MmdSemanticCatalog`: `pantselected_sw:`
  (`PantsPreset`, `OnCommand_pantographtoggleselected`, `pantographselectnext/previous`,
  `Train.cpp:3405-3549`), `pantfrontoff_sw:`, `pantrearoff_sw:`, `lights_sw:`
  (`lightspresetactivatenext/previous`; `light_position` is `LightsPosNo`, the count),
  `dimheadlights_sw:`, `radiostop_sw:`, `radiovolumenext/prev_sw:`, `universalbrake1_bt:`,
  `doorpermitpreset_sw:`, `distancecounter_sw:`, `universal0-8:`, gauges `brakepressb:`,
  `limpipepress:`, `clock:`, lamps `i-mainpipelock:`, `i-tempomat:`, `i-malfunction:`. Four
  pantographs (`CollectorsNo=4`, `PhysicalLayout=3`); the wrapper animates two.
* `LegacyCabinBattery`, `LegacyCabinCabActivation`, `LegacyCabinManualBrake`, `LegacyCabinWipers`
  only register what `LegacyCabinUnmodelledControls` registers anyway - fold them in.
* Cab activation side effect: `OnCommand_cabactivationenable/disable` also call `SetLights()` when
  `LightsPosNo > 0` (`Train.cpp:2440`, `2463`).
* A tile's placeholder guesses its width (`TileGrid.PLACEHOLDER_STRETCH`) because
  `MaszynaSceneryInfo.Vehicle` has no length and FIZ `Dim=` is parsed nowhere.

## Translations

* The HUD's help (`demo/hud/help.gd`) shows `action.capitalize()` as a msgid, so a new input
  action needs its capitalised name added to `demo/translations/*.po` by hand.
* Units (`km/h`, `bar`, `%d m`, ...) are not msgids.

## Sounds

* **The UI's sounds follow the simulation's speed too.** `TrainSoundSystem` sets
  `AudioServer.playback_speed_scale` to the running speed (up to x4, a tape's effect; set above x8,
  the Cabin and Exterior buses are muted; 2026-09-29) -
  the whole audio's, so the menus' clicks and the music slow down and speed up with the world.
  To do: a speed for the world's sound only - a pitch multiplier on `SfxPlayer`/`SfxPlayer3D` (the
  vendored `gnd-sfx` has none: pitch is per voice, `sfx_playback_runtime.gd:1191`) set on the
  vehicles' and the weather's players, and the global scale left at 1.

* MMD offsets of non-running sounds are used raw - the `SfxPlayer3D`s are not turned 180 degrees
  like the model (`MASZYNA_VEHICLE_FRAME`, `maszyna_rail_vehicle_3d_instancer.gd`), so horns,
  compressor, brakes etc. with `offset:` sit mirrored (x, z). Running sounds convert
  (`MmdSoundBankInstancer._build_running_events()`).
* Missing running sounds: `tractionacmotor:`/`inverter:`/`motorblower:` (`DynObj.cpp:5745-5800`,
  `8012-8080`), `wheelflat:` (`DynObj.cpp:4722`), `derail:` (`DynObj.cpp:5910`), `transmission:`
  (`DynObj.cpp:5822`), cab `huntingnoise:` (`Train.cpp:8284`).
* Wiper sounds (`wiperfrompark:`, `wipertopark:`, `DynObj.cpp:4082-4099`) not played; the arm swing
  direction (`RailVehicleRenderingServer::_pose_wipers()`, as `TDynamicObject::UpdateWiper()`) not
  checked in game.
* Wheel clatter bump (`AccVert`, `DynObj.cpp:3533`, cab shake only) not ported.
* Open cab window (`Global.CabWindowOpen`): the original plays the trainset's outer noise and stops
  the cab running noise (`DynObj.cpp:4638`, `Train.cpp:8274`); no cab window state yet.
* `pitchvariation:` (default 0.975-1.025, `sound.cpp:375`) is parsed, never applied.
* `pantographup:`/`pantographdown:` play at the bank's position for both pantographs; the original
  places them at the pantograph that moved (`DynObj.cpp:3881-3934`, `4007-4036`). The E186 bank
  was not dumped after adding them, nor after `converter:`/`small-compressor:` were wired.
* Brake sounds (`BrakeSoundModel`, 2026-10-01): a loop due while the bank was out of earshot
  starts from its opening bookend when heard again; the original resumes past it
  (`sound.cpp:360-367`). The pressure rates are reset when a bank is silenced; the original keeps
  computing them for every vehicle. `TrainSoundSystem._process()` keeps the remainder of
  `sound_update_elapsed` (`fmod`) and passes the whole `elapsed` on, so a far bank's next update
  counts that remainder twice - the brake rates (`dp/dt`) of far vehicles read a little low.
  Not heard by the operator against the original yet.
* The gnd-sfx tick is GDScript on a worker (12 ms/frame for 200 players, headless). If it limits,
  move the runtime to a C++ singleton beside `E3DRenderingServer`.
* `SfxGeneratorPlayback.update()` runs on the sfx worker (single producer into the ring buffer);
  revisit if a generator clip ever needs the scene tree.
* Scenery sounds (`ScenerySoundServer`, 2026-09-29): a sound played once while out of reach is not
  heard when the camera arrives mid-clip (the original's source keeps playing); a loop restarts
  from its beginning when it comes back into reach. The reach is capped at the draw distance by
  `SceneryStreamingServer`. `sound_create()` sets the player's `max_tracks`, which rebuilds its
  voices - fine at load, not while scenery sounds play. A loop asked for while the sound plays
  once starts only when the camera comes back into reach, where the original's `play_event()`
  starts it as soon as the play ends (`scene.cpp:148-152`).

## Vehicles

* `MaszynaRailVehicle3D` builds itself (`MaszynaRailVehicle3DManager.build_into()` in its own
  `_process`), so vehicles appear a frame after the scenery (`SceneryInstancer._wait_for_vehicles()`
  awaits `vehicle_built`). Building belongs in a `MaszynaRailVehicle3DFactory`.
* A distant vehicle's low-poly interior (`OPTIMIZED`,
  `RailVehicleRenderingServer::_update_detail()`) keeps its baked emission regardless of its cab
  lights until back within `maszyna/vehicles/detail_distance`:
  `E3DRenderingServer.instance_set_emission_energy()` and `instance_set_submodel_emission_energy()`
  reach only the NODES backend.
* The low-poly interior lights only its `cabN` sections (`vehicle_set_cab_light_level()`); its
  compartment and corridor sections (`corridor`/`korytarz`/`compartment`/`przedzial`,
  `DynObj.cpp:2425-2433`) stay unlit - the original lights them from CompartmentLights at its own
  intensity (`DynObj.cpp:1334-1352`), which is not ported.
* `RailVehicleRenderingServer`'s low-poly cab lights have no test: the test models are transform
  submodels without meshes or emissive materials.
* The vehicle's lamps drawn from its lighting (`RailVehicleRenderingServer::_update_lights()`) have no
  test since `test_rail_vehicle_lights.gd` went with `RailVehicle3D.lights`.
* `MaszynaVehicleStructure` and `RailVehicleAppearance` - candidates for a better name.
* `RailVehicleServer.trainset_move(vehicle, distance)` takes a vehicle RID while the other
  `trainset_*` now act on a trainset handle - rename (e.g. `vehicle_move_coupled`).
* A vehicle without a start track no longer moves by its own velocity: the off-track motion went
  with `RailVehicle3D`'s `_process` (the KA car in `demo_3d` stands). Decide whether
  `RailVehicleServer` should move off-track vehicles.
* `RailVehicleLighting::LightEnd {LIGHT_END_FRONT, LIGHT_END_REAR}` duplicates
  `RailVehicleController::CouplerEnd`.
* The 8 m shift of a trainset's first vehicle on a track with `Event0`
  (simulationstateserializer.cpp:1050-1057) is not ported (`RailVehicleServer.trainset_place()`).
* The `structure-vN` tag (`maszyna_rail_vehicle_3d_manager.gd`) is bumped by hand; a
  `MaszynaRailVehicle3DInstancer` change without a bump keeps serving the old structure, and the
  cache survives a checkout.
* Braked standing vehicles never sleep: at `V == 0` `Sign(0) == 1`, so `FTotal = FTrain - FStand`
  keeps `AccS` non-zero (`Mover.cpp:4603`) - same in the original.
* A vehicle with its physics off keeps its last state (fetched only for active vehicles, as the
  original skips `Update()`).
* The rest of `LoadFIZ_Cntrl`'s start modes never reach the Mover: `CompressorStart`,
  `PantCompressorStart`, `MainStart` and `ConverterOverloadWhenMainIsOff` (Mover.cpp:10905-10925)
  are not parsed, and their properties sit on `RailVehicleElectricEngine`, so a diesel could not
  carry them anyway. `ConverterStart`/`ConverterStartDelay` moved to `VehicleController`; the
  others belong there too. The `converter` command is still an electric engine's only.
* `BrakeValveParams` (the raw `BrakeValve=` string, Mover.cpp:10397) is never set, so
  `TNESt3::SetSize()` builds every ESt distributor as an ESt4: `TRapid` instead of `TRura` and no
  `Podskok` for ESt3, and `AL2`, `PZZ`, `HBG300`, `3d`/`4d` and `-ED` are dropped. That covers
  about 200 FIZ files of the datapack (ESt3, ESt3AL2HBG300, ESt4HBG300-s216, ESt3d_PZZ, ...).
* The energy meter (`MoverCurrentCollectorUnit::meter_energy()`, DynObj.cpp:3798-3832) ports the
  original's per-pantograph current (`fPantCurrent`), but the current sent to the wire is still
  `get_current0() / collecting` (`RailVehicleServer::vehicle_collect_current()`) - whether it
  should take the ported one is open. The meter has no test: it needs an electric fixture drawing
  current under a live wire.
* "Edit FIZ" (taking a vehicle out of the tree and back) still logs errors in code the vehicle
  layer rework did not touch: `cabin_python_screen.gd` and `maszyna_dynamic_train_cabin.gd`
  disconnect in `_exit_tree()` what they connected only once (`_ready()`/a guard); and 26
  `!is_inside_tree()` transform reads during the toggle (counted before `RailVehicle3D` became a
  proxy - count again).
* Tests still take the controller (`RailVehicle3D.get_controller()`,
  `VehicleServer.vehicle_get_controller()`) and call it directly; the plan's "tests go by RID"
  (`VehicleServer`/`RailVehicleServer` calls, a helper returning the RID) is not done.
* `driver_type` (which end is manned - headdriver/reardriver) stays on the generic
  `VehicleController`/`VehicleServer`; whether it is a rail value like the type and the load is open.
* `demo/examples/mover_demo.*` and the `custom_*_train_part` examples still assume components
  as nodes (`$SM42/Brake`) and do not run.
* `README.md`, "No simulation time is ever dropped": describes `step_frame()`, owed time and
  `MAX_PHYSICS_ITERATIONS`, none of which exists any more - the step is
  `MaszynaMoverVehicleServer::stepping_advance()` over the frame `SimulationServer` hands on,
  whose cap is `SimulationServer`'s. Rewrite from the code.
* The vehicle selector's "Stop and repair", "Reset position", "Refill main tank" and "Rupture main
  pipe" (vehicleparams.cpp:268-287) are not offered; its cog has the brake release, the emergency
  brake and the trainset moves only.
* The vehicle whose card is open is not marked in the world. Idea: a diamond marker always on
  screen over the vehicle, with its distance from the camera next to it.
* A vehicle is clicked in free camera (`SceneryHUDMouseServer.vehicle_pressed`) only while its
  model is detailed (within `maszyna/vehicles/detail_distance`); its tooltip caption is the
  scenery name taken when the model instance is registered, so a name set later is not shown.

## Rendering

* **GPU 22 ms of a 33 ms frame** (2026-09-29, RX 580, Visual Profiler: Render 3D Scene 21.9 ms GPU,
  2.4 ms CPU; 3849 objects, 1.0 M primitives, 3607 draw calls) - not investigated. Measure on
  `make compile-profiling` and split by pass before any hypothesis.

### Mirror reflections (`PlanarMirror3D`) are smeared

The mirror mechanics work (unfolding, `mirrors_sw`, door permits), but the reflection a mirror glass
shows is a blurred smear, not a mirror image (reported 2026-09-28 on the Impuls 36WEa, `td_impuls`).
Not yet measured: whether the project's TAA (`anti_aliasing/quality/use_taa=true`) smears the
overlay whose picture changes every frame, whether the mirror camera's framing and projection
(`mirror_view_projection`) match what the glass samples, and the texture size actually used in the
game (the headless window is 64x64, so a headless probe cannot tell). Dump the mirror's viewport
texture from a running game and look at it before changing a number. The glass is found by name
(a leaf `zwierciad*`/`*luster*`/`*lustr*`), reflects on the side away from its housing, and is
switched by `maszyna/rendering/real_mirrors`.

### A light's submodels have two managers

`E3DRenderingServer` resolves `lights_state` (modes, override, time of day) and shows `_on`/`_off`;
the cab widgets (`CabinIndicator3D`, `CabinSpotLight3D`) write `Node3D.visible` on the same
submodels. It works only because nothing pushes `lights_state` at a built cab (`FINDINGS.md`,
2026-09-23), and the widgets do nothing under OPTIMIZED. Widgets should ask the model
(`lights_state`) instead.

### Self-illumination of light submodels (reported 2026-09-24: lights shine, `_on` meshes do not)

Checked headlessly: scenery `light_onNN` gets `emission_enabled`, energy 1.0 (`light_common.glsl:164`);
`light_on` meshes carry `fLight` 2.0 (264), 1.0 (39), -1.0 (3) across 248 models. Left:

* **`E3DModelInstance` nodes never light automatically:** `_merge_lights_state()`
  (`e3d_model_instance.gd:214`) pushes `false` for every light via `instance_set_lights_state()`,
  which the server treats as an override winning over the mode. Measured: `latarnial_str`,
  `ls_Dark`, light level 0.05 - `light_on00` hidden. Sceneries unaffected.
* **A `colored` light submodel has no emission** - `get_submodel_material()` returns
  `COLORED_MATERIAL` first (1 of 306, `lampa_parkowa01`'s `light_on00`).
* **`lightcolors` do not tint the submodel** - the original also overrides its diffuse
  (`SetDiffuseOverride`, `AnimModel.cpp:625`).
* **Baked, not per frame:** the original lights while `Global.fLuminance < fLight`
  (`opengl33renderer.cpp:3452`); `material_manager.gd:118` decides once with
  `lights_on_threshold >= 1.0`, so 1.0 glows in daylight and (0, 1) never glows.
* **Unmeasured:** emission 1.0 after AgX (`tonemap_agx_white` 6.19, contrast 1.55). Needs a
  rendered frame of the operator's scenery.

### Smoke emitters

* Vertical decay not ported (`particles.cpp:365-380`): the rise slows with air temperature,
  overcast and vehicle speed; `Global.AirTemperature` is a `#define` in the vendored Mover (see
  Scenery loading). Wind drift is ported (`E3DRenderingServer::environment_set_wind()`, `0.1 * wind`).
* `MaszynaEnvironmentNode.wind_direction` is a compass bearing, so wind is always horizontal;
  `MaszynaSkyEnvironment.get_wind_direction()` already returns a `Vector3` and `environment_set_wind()` takes
  strength and direction separately - only a property that can express a vertical part is missing.
* The culling box follows the emitter, not the plume (`_apply_smoke_placement()`); a fast
  vehicle's trail vanishes when the emitter leaves the screen. The original grows the box over its
  particles (`opengl33particles.cpp:38`, `particles.cpp:284-291`).
* `min_inclination` dropped (no inner cone in `ParticleProcessMaterial`); only `smokesource_st45`
  sets one (10 degrees) of twelve templates.
* Lifetime is per emitter (longest), per particle in the original (initial opacity / fade step,
  `particles.cpp:132`), so faint particles linger.
* The "Modern" flipbook (`demo/vfx/smoke_atlas.png`, `scripts/make_smoke_atlas.py`) is a
  procedural stand-in, set via `maszyna/rendering/smoke_atlas`/`smoke_atlas_frames`; one sequence
  for all particles repeats visibly - needs variants (second atlas axis or random `anim_offset`).
* Smoke is lit by Godot's sun, not the original's flat daylight modulation
  (`opengl33particles.cpp:60-66`); `light_level` unused.
* "Cold engine smokes grey" never ran in the original (`particles.cpp:176` compares instead of
  assigns); would need `dizel_heat.Ts` exposed.
* Vehicle emitters are not switched off when the vehicle is culled, only when its
  E3D instance hides; the original stops beyond `2 * BaseDrawRange * fDistanceFactor`
  (`particles.cpp:452`), the wrapper streams only scenery ones (`maszyna/rendering/smoke_distance`).

### Other

* E3D: VNT1 (packed vertices), vertices with userdata (VNT4+) and TRA1 (double matrices) are not
  read - no file in the game data uses them today (`Model3d.cpp:1977`, `:2135`). 57 scenery and
  31 vehicle submodels are wound CW or mixed in the data itself (e.g. cabs of ET21, ET22, EU05,
  ST44) and render inside out exactly as in the original.
* T3D (`T3DParser`): not ported - `priorityLoadText3D` (a .t3d before an .e3d,
  `Model3d.cpp:1634`), the "banana" root added for a dynamic model under `iConvertModels & 4`
  (`Model3d.cpp:2447`), and a material's own `selfillum` overriding the submodel's
  (`Model3d.cpp:483`). `type: text` is read as a transform, `stars` and `point` are read past
  and not drawn (as from an .e3d). `hotspotpower:` is read into `light_energy`, which
  `E3DModelBuilder` does not pass to the submodel - for an .e3d either.
* `maszyna/lights/reverse_cull_face` is off by default now; street lamps got their own biases
  for it. Vehicle headlamps and cab lights (SpotLight3D nodes with a node's 0.03 / 1.0) were not
  measured - check them for acne at night.
* Skydome needs an option to disable `light_angular_distance`: its PSSM/soft-shadow cost can push a
  60 FPS cabin frame with visible clouds past the V-Sync budget (`FINDINGS.md`, 2026-09-20).
* Normal maps at `normal_scale` 1.0 (`mat_normalmap.frag:46-48`): not checked whether Godot's
  tangents match the original's `f_tbn` (bump direction on models and terrain).
* Overexposure in the demo scenery unmeasured. Candidates, one at a time: `tonemap_mode` in
  `demo_scenery_loading.tscn`, `soft_shadow_filter_quality` 3 -> 1 and the removed
  `directional_shadow/size=8192` (`042b392`), `fog_enabled = false`, `cloudiness` 0.35 -> 0.21
  (`ab75bbe`).
* Unmapped original shaders: `clouds`, `stars`, `invalid` (`textures/sky/stratus.mat`, `stars.mat`,
  `invalid.mat`) and `normalmap_phys` (`textures/pkp/wskazniki/w29.mat`, no shader file either).
* Specgloss types, what is still approximate: `detail_normalmap_specgloss` masks the reflection
  with the normal map's alpha instead of the original's `reflblend`
  (`mat_detail_normalmap_specgloss.frag:68`); `shadowlessnormalmap` binds no normal map at all, so
  its reflection mask is 1.
* `rain_windscreen.gdshader`: droplets ignore speed and wind (a TODO in the original too); it reads
  the screen texture, so transparent things behind the glass (rain particles) fade under the film.
  Film, large droplets and rivulets are the wrapper's own, tuned by eye (`heavy_rain_start`,
  `film_*`, `rivulet_*`, `refraction_strength`); "down" not checked on a real cab glass.

## Scenery loading

* Air temperature (`MaszynaEnvironmentNode.temperature`) is consumed by nothing; the Mover uses it
  only in `dizel_heat.Te` (`Mover.cpp:8109`) and the vendored one has
  `#define Global_AirTemperature 15.f` - not feedable without touching `src/legacy/maszyna-mover/`.
* Other `config` entries dropped (`scenario.time.override/offset/current`, `Globals.cpp:356-385`).
* Include instancing: `skp/skp_trawa.scm` includes `grass.inc` 24078 times, each parsed and baked
  to world space. Idea: classify includes as `instanced` (only `origin`/`rotate` + `triangles`, no
  nested includes; key = path + hash of non-placement params; MultiMesh per chunk/texture/range) or
  `full` (key = path + hash of all params); cache in local space, invalidate by dependency list.
* The subscene cache (`SceneryInstancer.parse_subscene_task()`) is used only by queued parsing;
  `parse_file()` reparses every include.
* Streaming keeps the six `TrackRenderingServer` and two `TractionRenderingServer` instances of
  every piece and drops only meshes; freeing them would save ~16k empty instances in `baltyk`.
* A track streams by the chunk of its first curve point, not its nearest point.
* Tracks and traction have no worker `preload` - built on the main thread in the per-frame budget;
  with ~5 000 pieces at 3 000 m that is thousands of builds after a load.
* The loading screen does not wait for the first pass; waiting for
  `get_statistics()["pending_builds"] == 0` would hide the fill-in.
* Streaming per piece is the wrong granularity: bake a 1 km chunk into one unit (MultiMesh per
  mesh+material, merged triangles, ready track meshes), cache on disk, load on the worker. Switch
  blades (`primary_blade_mesh_instance`/`secondary_blade_mesh_instance`) move and stay outside.
* `MaszynaSceneryChunkRenderingServer` (`legacy/scenery/`, deprecated) is no server: an autoload that
  only owns `MaszynaTrianglesChunkData` streaming for `SceneryStreamingServer` and creates the
  `RenderingServer` instances. Remove it: let `SceneryStreamingServer` stream a triangle chunk
  (mesh + transform) itself, as it does `E3DRenderingServer`'s models.
* Memory left after the lazy loading (`ResourceLazyLoader`, 2026-10-01): every model placement
  keeps a full `E3DInstanceData` for the whole session; the subscene cache
  (`MaszynaCompiledSubscene.triangles`) still holds raw triangles, read whole; a terrain chunk's
  geometry is held as arrays beside its mesh while built.
* Main-thread stalls left in a load: `TractionServer.network_build()` and
  `TrackServer.topology_rebuild()` run once, unbudgeted; `ScenerySoundServer.sound_create()` sets
  `SfxBank.events` once per sound and the vendored setter rebuilds the bank each time (O(n^2)) -
  needs an append in `gnd-sfx`.
* Not measured on a heavy scenery yet: the `[SceneryLoad]` lines (`SceneryLoadMeasurement`) before
  and after, from a parse and from the cache.
* In the editor streaming follows 3D viewport 0 only (`addons/libmaszyna/editor/scenery_streaming/`).
* `maszyna_node_track_importer.gd` drops every type but `switch`/`normal`: `road` (~16 700),
  `river` (~900), `cross` (72), `turn`, `table`. `road`/`river` need a flat surface path
  (`Track.cpp:1554` on); `cross` is a road intersection with four endpoints, no topology support.
* Lamp head colour (texture, sodium orange) does not match its pool (tinted by
  `maszyna/rendering/scenery_light_tint`). Tinting emission needs a flag in the
  `E3DMaterialResolver` key (like `force_alpha`) - `latarnial_betdziur` shares
  `elektryczne/oprawa` between bulb and housing.
* An economy-mode merged light takes the max `energy` of the lights it replaces, not the sum;
  `maszyna/rendering/scenery_light_energy` compensates.
* Scenery light brightness is calibrated by eye (`scenery_light_energy`, `scenery_light_tint`,
  `scenery_light_volumetric_fog_energy`); the tint has no counterpart in the original.
* `latarnial_betdziur` registers a second light `zarowka` (`_on`/`_off` suffix rule,
  `e3d_parser.cpp:592`) that no `lights` list reaches, so the bulb stays off. The original binds
  only `Light_On00..07` (`AnimModel.cpp:303`) - should the suffix rule apply to scenery models?
* The `m_lightopacities` transition (`AnimModel.cpp:542-549`) is not ported - a blinking light
  (`E3DRenderingServer._process_lights()`) snaps on and off; `notransition` is parsed and ignored.
* `Overcast` is folded into the light level (`MaszynaSkyEnvironment.get_light_level()`) instead of
  subtracted at the threshold (`AnimModel.cpp:598`).
* Scenery models have no nodes - not pickable in the editor, don't follow the
  `MaszynaIncludeNode` transform/visibility.
* An `include` with no filename appears in the real data (`maszyna_include_importer.gd` reports it
  with the offset and skips it); source unknown - truncated file or tokenizer misread.
* Unloading a scenery leaves its weather in `MaszynaEnvironmentNode` (the `atmo` precipitation,
  fog, temperature): the menu keeps it (held silent by `SimulationServer.simulation_pause()`) and a next scenery
  without an `atmo` section inherits it.
* `SimulationServer.simulation_pause()` holds the vehicle step, the weather and the world's sounds only - the
  environment clock, `TractionServer`, `TrackServer` switches and the smoke keep running.

## Signalling (#296)

`SignallingServer` with systems, delegates, sources, kinds and the nodes is in place; a scenery
signal head's kind is made of the `lights` events aimed at it, and the original's
`MaszynaLegacySignallingDelegate` shows one of them when it is handed the event. Left:

* **The isolated sections become the system's sources** once `TrackServer` has them (see
  Scenario events).
* **The logical aspect for trains** - memcell `SetVelocity`/`ShuntVelocity` read through a passive
  `getvalues` - is read by the driver's speed table (`MaszynaLegacyDriverRoute`); a signalling
  delegate has only the lights, nothing it shows reaches a train.
* **`ls_Dark`/`ls_Home` from a `lights` event** (value 3, 24 times in the data set): the signal head
  API has no light that follows the daylight; the legacy kind factory warns and keeps the light.
* **Semaphore arms** - the `animation` event on a named submodel (`Event.cpp:1569-1735`).
* **Every lit scenery model is a signal head** (the operator's decision), street lamps with
  `lights 3` included - telling signal heads apart is open.
* **Scenery kinds are made of `lights` events only**: a lit model no event reaches gets the generic
  kind (lights, no aspects). The declared `lights` list is not an aspect, so a scenery signal head's
  light states on the server read `LIGHT_STATE_OFF` until its first aspect, whatever E3D shows.
* **`SignalAspect.lights` are plain numbers** in the inspector (`LightCommand`), not an enum.

## Scenario events

`ScenarioEventServer` (events, the queue, the simulation time, memory, launchers, track events)
runs the scenery's events; `MaszynaLegacyEventFactory` builds them from the `.scn` data once the
include's server data is built. Built: `updatevalues`, `addvalues`, `copyvalues`, `multiple`,
`lights`, `switch`, `trackvel`, `voltage`, `animation` (rotate, translate, with its
`<model>.<submodel>:done`), `sound`; conditions `memcompare`, `memcompareex`, `probability`,
`trackoccupied`, `trackfree`; a track's `event0/1/2`, `eventall0/1/2` and `<track>:<slot>` events;
isolated sections (`TrackServer.isolated_*`, `isolated`/`area` blocks, `:busy/:free/:inc/:dec`, the
section's own memory); `onstart` and negative-delay events; Shift+0..9 (`keyctrl00-09`), launcher
keys (`ScenarioKeyboard`), HH:MM and radio call launchers (`radiocall1_sw`/`radiocall3_sw`,
Backspace); the "Scenario and Events" HUD window. Checked on `td.scn` with a headless probe (Shift+8
closes both level crossings). Left:

* **Track events, as the original fires them**: the direction filter by the trainset's intended
  direction (`eventfilter`, `TrkFoll.cpp:117-121`) - here the actual direction of travel decides;
  the vehicle's one placement point stands for the primary axle; events with a delay <= -1 queued
  on every move along the same track (`TrkFoll.cpp:249-260`); a crewed vehicle is one with a
  `driver_type`, the original's `Mechanik->primary()` is one per trainset.
* **Occupancy counts vehicles, not axles**: a vehicle is on the one track its placement point is
  on, where the original counts every axle (`TrkFoll.cpp:88-91`) - a vehicle across a joint
  occupies only one of the two tracks, for isolated sections and `trackoccupied` alike.
* **Isolated sections are not yet a signalling system's sources** (`SignallingServer.system_add_source`).
* **`putvalues`/`getvalues`**: the passive ones (`SetVelocity`, `ShuntVelocity`, `RoadVelocity`,
  `SectionVelocity`, `OutsideStation`, `PassengerStopPoint:`; a `getvalues` of a memory holding
  `SetVelocity`/`ShuntVelocity`/`SetProximityVelocity` at the start) are never run - the driver
  reads them ahead (`ScenarioEventServer.event_is_passive()`). `CabSignal` stays active, acting on
  the vehicle crossing it, unlike the original where the driver's table does it: a vehicle the
  player drives has no driver here. The Mover's other commands (`Load=`, `UnLoad=`, `BrakeDelay`,
  ... `Mover.cpp:12187-12720`) are dropped.
* **`updatevalues`/`addvalues` of a memory on a track** give its command to the driver of every
  vehicle on that track (`Event.cpp:538-547`) - how Stary Jawor's eszelon is set going. Its `:sent`
  event (`StopCommandSent()`) is not ported.
* **Event types without an action**: `whois`
  (`Event.cpp:993-1153`), `logvalues`, `texture` (`:1474-1543`), `friction` (`:2100-2104`).
  `switch` ignores the blade speed and delay (`Event.cpp:1855-1873`); `animation` has no
  `digital` or `.vmd` mode (`Event.cpp:1654-1682`); a radio message (a `sound` event with a channel)
  tuned in or the radio switched on mid-message is not raised, as `update_sounds_radio()` does -
  it is played only when heard at its start, and at the cab's radio volume of that moment.
  Its transcript likewise shows only when it is heard at its start.
* **Scenery sounds** use the player's defaults for everything but `max_distance` (the node's
  range); an ambient one (range under -1) is on the listener, 0.4 as loud and cut at 1.25 of its
  range - the original fades it out between the range and that, and starts it only within
  2750 m (`audiorenderer.cpp:184-199`, `sound.cpp:364-371`). Not checked by ear.
* **Memory and the AI**: pushing a memory to the vehicles on its track when it changes
  (`Event.cpp:538-548`), `bCommand`/`CommandCheck` and `:sent` (`MemCell.cpp:52-99`, `196-205`).
* **`departuredelay`** takes the activator's own driver's timetable, else the first driver of its
  trainset with one - the original asks for the vehicle's driver only when it is `primary()`
  (`Event.cpp:2431-2435`).
* **Duplicate event names**: the later wins (with a warning); the original joins them as siblings
  and ignores the first (`Event.cpp:2296-2349`).
* **Launchers**: numeric key codes,
  `-10000` (first time in range, `EvLaunch.cpp:182-186`), `traintriggered` (the distance to the
  train, not the camera). Timed launchers are global here; the original polls non-global ones only
  near the camera.
* **A click on a scenery model** (`SceneryHUDMouseServer`) is not hidden by anything in front of
  the model: a lever behind a building is picked through it, where the original's pick buffer
  shows only what is seen. Picking also ignores the Alt picking toggle (`drivermode.cpp:493-500`).
* **Stary Jawor, eszelon** (headless probe, 2026-09-26): both stations run their logic, the
  shunting signals open (Roztocze Tm18, then Tm19/Tm20 with switches 74-76a once SU46 reaches
  `n176`), the 10:50 launcher fires; it stops where it waits for the AI's eszelon (below).
* **No AI trains**: a scenario whose stages wait for a train the AI drives stops there - e.g.
  `stary_jawor_eszelon` waits for the eszelon to reach Roztocze (`n282:event2`,
  `skp/skp_eszelon_events.ctr`). The track events fire for any moving vehicle with a driver, but no
  such vehicle moves without the AI.
* **A jump of the time of day** (the environment's time set, the system time) is not a running
  clock: a time-of-day launcher whose minute is jumped over does not fire, and a timetable
  compares its departures with the clock, so a train is late or early by the jump. The queue and
  the drivers' updates run on the simulation time and do not notice. The original does the same.
* The `queueevent` console command.
* **Timetables** are followed by the drivers (Drivers, part 6). Not read: the station
  announcements (`load_sounds()`, `mtable.cpp:644-671`). The original's quirks around them are in
  `MASZYNA_ORIGINAL_QUIRKS.md`.
* Events of one include cannot refer to events of another `MaszynaIncludeNode`.
* Proxy nodes for editor-built scenes (`ScenarioEventNode`, `ScenarioMemoryNode`,
  `ScenarioLauncherNode`, the `SignalHeadNode` pattern).

## Scenario scripts (Lua)

`ScenarioScriptServer` runs a scenery's `lua <file>` scripts (Lua 5.4, `vendor/lua`, build option
`LIBMASZYNA_LUA`) with the `maszyna.*` modules and the original's `eu07.events` on top of them; the
HUD's View > Lua scripts checks and applies code to the running scenario. Left:

* **`dynobj_putvalues` on a vehicle nobody drives**: the original hands the command to the Mover
  (`MoverParameters->PutCommand`, `lua.cpp:293-294`); here it goes to the driver only, as
  `MaszynaLegacyVehicleCommandAction` does - without a driver it is dropped.
* **Typing in the Lua editor drives the cab**: cab controls read their keys in `_input`
  (`cabin_command.gd`, `cabin_button.gd`, `cabin_switch.gd`), before the GUI, so a key bound to a
  control acts while the editor has the focus - the console has the same problem.

## Drivers (plan, #297)

Agreed 2026-09-26. The AI drives a vehicle the way a player does - through the cab
(`CabinSystem.act()`), never by a path of its own; while the player sits in the cab the AI does not
drive. The original's `TController` is no architectural model: only its vocabulary of orders is
ported, into a delegate.

1. **Done: `Emergency_brake` as a Radio-Stop broadcast** - a scenery `putvalues`/`getvalues`
   `Emergency_brake` is sent from the event's position (`putvalues x y z` plus the include's origin,
   `Event.cpp:709-712`; the memcell's position for `getvalues`) to every vehicle within
   `RADIO_STOP_RANGE`, as `RailVehicleServer.vehicle_emergency_signal_send` does from a vehicle. `CabSignal`
   stays the magnet acting on the activator. Both stay in the event system, not in a driver.
   No test covers the broadcast: receiving it needs a vehicle with a driver, Radio-Stop fitted
   and the radio on.
2. **Done: cab logic without the 3D cab** - `LegacyCabinLogic` (a `CabinLogic`) is the vehicle's,
   attached with `CabinSystem.vehicle_attach_cab_logic()` by `MaszynaDynamicTrainCabin` (player) and
   `SceneryInstancer._build_drivers()` (AI), registered for the occupied cab and moved along when
   the crew changes cabs. The cab's controls come from its MMD (`LegacyCabinControls`), not from the
   widgets. Left widget-side, so an AI caller must pass what a widget would have worked out: the
   knob and switch position limits and spring return, the horn's value. `LegacyCabinControls` parses
   the MMD with no random choices - a cab with random includes may differ from the player's widgets.
   The `brake_level_drive` `CabinCommand` node still carries its `command`/`command_param`, now only
   as the guard of its key (the wiring is `LegacyCabinControls.BRAKE_LEVEL_DRIVE`).
3. **Orders taken; the engine and the turning carried out through the cab.** `DriverSystem` (C++,
   not `DriverServer`: the event action reaches it as a singleton), `DriverDelegate`,
   `MaszynaLegacyAIDriver` (GDScript, acting through `CabinSystem`): a driver for every crewed
   scenery vehicle (`SceneryInstancer._build_drivers()`), the order list and what the orders ask for
   (`get_state()`); Stary Jawor's SU46 gets its orders from the scenario. A driver acts one reaction
   time apart (`DriverSystem.driver_schedule_update()`, `DriverDelegate._update()`).
   `Prepare_engine`, `Release_engine` and `Change_direction` are carried out step by step through
   the cab (`MaszynaLegacyDriverHints`, the original's `driver_hint`s): checked on Stary Jawor's SU46
   - put away, prepared again, turned to its other cab. Left of `PrepareEngine()`/`ReleaseEngine()`
   (`Driver.cpp:2759-3012`): the heating of a diesel (`PrepareHeating()`, the water pump and heater),
   the pantograph air (the compressor, `bPantKurek3`, `PantsValve`) and the speed a pantograph
   counts as up at, the ground and motor overload relay resets, the idle position of SN61's
   controller, `mastercontrollersetreverserunlock`, the motor blowers, the spring brake and the
   doors, releaser and train or independent brake on putting away; the presence of a
   compressor is not asked (readiness waits for the main reservoir only); the brake handle's
   driving position is cued, not checked (the state does not say where it is). `Activation()`'s
   move to another vehicle of the trainset (EN57, ET41) is not ported, nor `ShuntModeAllow`.
   On Stary Jawor sa134-014 and WMB10-819 report their line breaker open after being prepared -
   not looked into. Left besides: `engine_active` lost when the vehicle breaks down while driving
   (`handle_engine()` prepares it again only for driving orders); the trainset's own timetable and
   velocity from the `.scn` (`trainset <timetable> ... <velocity>` -> `OrdersInit`); a push-pull set
   that only turns at `@` (`movePushPull`, `OrdersInit()`); what `OrderCheck()` does to the doors;
   of the lights (`MaszynaLegacyDriverLights`) the model's lamp inventory (`iInventory` - a tail
   without red markers shows them rather than plates), the far end put out on `Disconnect`
   (`Driver.cpp:2510-2525`), the player's vehicle put out on taking over
   (`TakeControl(false)`, `Driver.cpp:5662`) and `Global.AITrainman`'s Pc5 on a player's train;
   `SetSignal`; the station announcements and guard signals of `Timetable:`.
   The first plan read: driver RIDs,
   `driver_attach_delegate`, `driver_send_command(driver, command, values)`;
   `RailVehicleServer.vehicle_attach_driver(vehicle, driver)`, RIDs only. `MaszynaLegacyAIDriver` is
   the original's implementation: it takes the orders (`SetVelocity`, `ShuntVelocity`,
   `Prepare_engine`, `Change_direction`, `Shunt`, `Wait_for_orders`, `Timetable:`...,
   `TController::PutCommand()`, `Driver.cpp:4468-4906`) and carries out those that are a sequence of
   controls (`Prepare_engine`, `Change_direction`) through `CabinSystem.act()`. A driver for every
   `headdriver`/`reardriver` vehicle of the `.scn`. The `putvalues`/`getvalues` action hands the
   orders to the activator's driver. A future scenario kind (Lua...) is another delegate.
4. **Driving** - the track ahead read over the existing topology (`_motion_connection`), the passive
   events' positions and the speed table, speed control through the cab, the timetable followed.
   In `MaszynaLegacyAIDriver` (the delegate), acting through `CabinSystem.act()`; the topology walk
   is a `TrackServer` query, the passive events `ScenarioEventServer`'s, the signals
   `SignallingServer`'s. In order:
   1. Done: what the driver reads of its trainset (`MaszynaLegacyDriverTrainset`,
      `Driver.cpp:6033-6190`): readiness of the brakes (`Ready`, `fReady`, `IsConsistBraked`), the
      gravity along the track (`fAccGravity`) and the trainset's acceleration (`AbsAccS`); the
      vehicle publishes `acceleration`, `brake_force`, `brake_is_braking`/`_holding`/`_cut_off`
      and `engine_idle_rpm_count`. Left: the stretched couplers, doors, light, the relays of the
      other vehicles under control, the individual release of an overcharged vehicle, the parking
      brake of a speed control unit, EP brakes in `IsConsistBraked`, the pipe pressure a brake
      counts as applied at (`BrakePressureActual.PipePressureVal`, taken as 3.9).
   2. Done without a speed table: the speed and acceleration wanted
      (`MaszynaLegacyDriverSpeed`, `pick_optimal_speed()`, `Driver.cpp:7297-7400`) - the
      trainset's top speed, the timetable's, the shunting speed, the speed allowed, the track's,
      waiting told to stop here. Left: the next speed and its distance (with the speed table),
      obstacles ahead, an aggressive driver, EMU/DMU
      thresholds, the cargo trains' and couplers' acceleration limits, the braking test.
   3. Tractive force through the cab for every engine type (`MaszynaLegacyDriverTraction` and one
      subclass per engine, chosen by the driver's vehicle's engine: series motor, diesel-electric,
      induction motor, plain diesel, an EMU's control car; `control_tractive_force()`,
      `IncSpeed()`/`DecSpeed()`, `SpeedSet()`, `control_handles()`, the engine's part of
      `Check/SetTimeControllers()`), with the relays reset, sanding and the anti-slip brake, the
      power off after a Radio-Stop, the cruise control (`SpeedCntrl()`), the stretched couplers and
      the spring brake before adding power. The vehicle publishes what it needs
      (`controller_main_delayed`, `circuit_imin`, `engine_voltage`, `eimic_real`,
      `coupler_stretched`, `radio_stop_active`, `motor_overload_relay_high_threshold`...) and takes
      the commands a player gives (`motor_overload_relay_threshold` and `maxcurrent_sw`,
      `ground_relay_reset`, `antislip`, `speed_control_*`); `ESMVelocity()` is
      `RailVehicleElectricSeriesEngine.get_next_position_velocity()`. The pantographs
      (`MaszynaLegacyDriverPantographs`): the pantograph compressor and its three-way valve while
      preparing, the rear one up on the move. Checked on Stary Jawor (diesel-electric) and on a
      synthetic track with a catenary: EU07 with eight wagons prepares itself, runs up the
      resistors to 28 and the shunt, holds 40 km/h. Also checked there: SN61 (started at its idle
      position, 40 km/h), SA134 (the DMU's universal controller holding its share of power), EU47
      with eight wagons (the EIM controller, the cruise control holding 40.0 km/h). **Not checked
      yet:** EN57 (the control car's class is chosen, but the unit does not prepare - 0 V, reverser
      at 0: `Activation()`'s move to the unit's other vehicles is not ported). Left: the doors closed and the departure signal switched off before adding power
      (`Doors()`, `DepartureSignal` not published); the no-current sections (`fOverhead2`,
      `iOverheadZero`); the shunting mode of a 2Ls150 (`AnPos` in `SpeedSet()`) and of an induction
      motor; SN61's idle position after the reverser (`DirectionForward()`, Driver.cpp:5778); the
      input action for `maxcurrent_sw` (Ctrl+F); the diesels' `Engine:EngineMaxTemperature`
      (the overheat lamp's threshold, `dizel_heat.engine_max_temp`, Mover.cpp:8306 of the
      original - the vendored Mover has no such field); the radio off after a Radio-Stop.
   4. Braking through the cab for every brake system (`MaszynaLegacyDriverBraking`,
      `control_braking_force()`, `IncBrake()`/`DecBrake()`/`LapBrake()`, `Inc/DecBrakeEIM()`,
      `control_releaser()`, the brake part of `Check/SetTimeControllers()`): the individual brake
      (local or manual), the pneumatic one with the braking table, the electro-pneumatic one (its
      operation mode, the handle between EP releasing and braking, or held by time with the EP
      switch), an EMU's own braking, the EIM controllers' braking, the time-controlled handles
      (MHZ_K5P, MHZ_6P, M394, H14K1, St113, H1405 - set at the end of an update, back to holding at
      the next), the universal brake buttons, a DMU's handle following its universal controller.
      The vehicle publishes `brake_operation_mode`, the EP handle positions and
      `brake_handle_ep_time_controlled`, and takes `brake_operation_mode_increase/decrease` and
      `ep_brake`. Left: the braking test (`ForcePNBrake`, `DynamicBrakeTest`), unlocking the pipe
      before the releaser (`control_main_pipe()`), the individual release of an overcharged wagon,
      the manual brake applied on putting away (`manualbrakon`), the weather's friction.
   A player taking a vehicle takes over: `MaszynaPlayer` switches the vehicle's driver off
   (`DriverSystem.vehicle_set_control_active()`); switched off it takes orders and reads its
   trainset, but touches no control. The train left on foot (F4) stays the player's; its driver
   takes the trainset back when the player takes a vehicle of another trainset, or on Shift+Q
   (`ai_driver_enable`), and Q (`ai_driver_disable`) takes it again.
   5. The speed table (`MaszynaLegacyDriverRoute`, `TableTraceRoute()`/`TableUpdate()`/
      `TableUpdateEvent()`): the tracks ahead from `RailVehicleServer.vehicle_trace_route()` (the
      next-track rule now `TrackServer.track_find_next()`, shared with the movement), their
      limits, switches and the end of the line, the passive events of `event1`/`event2`, the
      signals' orders to itself (`SetVelocity`, `ShuntVelocity`) and a memory's command sent once;
      `MaszynaLegacyDriverSpeed` brakes to the next speed within `fMin/MaxProximityDist`. Read
      again on every update rather than kept and moved. Checked on Stary Jawor only standing
      (every train sees its stop); a run past signals waits for the train brake to release (below).
      The vehicles ahead (`scan_obstacles()`, `adjust_desired_speed_for_obstacles()`): the nearest
      vehicle along the route from the trainset's front (`RailVehicleServer.vehicle_find_vehicle()`,
      the gap between the ends at every distance, not the original's centres beyond 100 m); the
      speed class keeps `AccPreferred`, `VelNext` and `ActualProximityDist` narrowed by it, and
      `ReactionTime` 0.1 close to a stop or a vehicle.
      Left: the passenger stop points (part 6), section and road speeds, stopping at an SBL,
      crossings, `BackwardTraceRoute`, the switch branch of an event on a switch, the cargo
      train's distances; of the vehicles ahead: the scan from the rear end while rolling against
      the way it drives (Driver.cpp:6642), a signal beyond a vehicle ahead being that vehicle's
      (`isforsomeoneelse`, Driver.cpp:1566, 1709), the coupler adapters in the gap, and the
      braking point offset (`braking_distance_multiplier()`) in the target speed.
   Checked on Stary Jawor: the eszelon (ST44, 20 wagons), set going by its memory, releases,
   runs to 51 km/h, brakes for a stop signal, takes the next one's 40 and runs on past it.
   The eszelon's lock-up at simulation speed 5 was the couplers stiffened by a long frame
   (`docs/findings-archive.md`, 2026-09-27) - fixed in `RailVehicleServer::stepping_advance()`.
   6. The timetable (`MaszynaLegacyDriverTimetable`, `TableUpdateStopPoint()`): the passenger
      stops of the next station (`PassengerStopPoint:<station>`, cut at `#` as the original's
      parser does) - passed at speed where the train does not stop, else brought forward for the
      train's length and the platform, stopped at, left at the departure time (a goods train at
      once), the odd first number holding it for a clear signal; another station's stop close
      ahead rewinds the timetable to it; `@` turns a push-pull train by its cab (a locomotive
      goes on to its next order, `Disconnect`); the last station ends the
      timetable. The timetable's speed per stretch (`TTVmax`).
      The stop's passengers (`MaszynaLegacyStation.update_load()`, station.cpp:25-88) and the
      train's dispatch (`StationServer`: exchange, wait for the departure, doors closed - in
      place of the original's negative `fStopTime`); the AI's doors at the platform
      (`Doors()`, Driver.cpp:4266-4356) through the vehicle's door commands.
      Left: the departure signal before the doors close (`DepartureSignal`,
      `departuresignalon/off`, Driver.cpp:4305-4320 - not published) and the wait after closing
      (`fActionTime = Random(-3.5, -1.0)`, Driver.cpp:4351); the guard's `moveGuardOpenDoor`; the
      car load weights (`load_weights.txt`) and the visible load of a car
      (`update_load_visibility()`, `update_load_sections()`); the passenger announcements; the
      load unit sent to the Mover as `"tons"` where the original parses `"tonns"`
      (Mover.cpp:4464, `MoverRailVehicleLoad`); of the guard's departure message
      (`tsGuardSignal`) only the radio one is played - the one heard beside the train (`<timetable>.ogg`, Driver.cpp:4466-4472, 6862-6870)
      needs a place for a train's own world sounds, and a .flac one has no loader; the hint to
      tune the radio to a station's channel (`cue_action(radiochannel)`, Driver.cpp:1113), the
      delay flag (`UpdateDelayFlag()`),
      a player's stop left far behind (`AIControllFlag`, Driver.cpp:1190-1200), the
      `VelSignalLast` reset by a stop held at (`eSignNext`). A player who pulls away from a stop
      before its departure time leaves the timetable at that station (it has `arrived` there), so
      the panel counts the delay from the departure while the train drives on, until the next
      station's stop rewinds the timetable.
      The player's timetable panel (`demo/hud/timetable_panel.gd`, F2 / View menu, fed by
      `DriverSystem.driver_get_timetable_state()` and `driver_timetable_changed`) left out:
      the list starting at `StationStart` (driveruipanels.cpp:392) - the panel lists every
      station, passed ones faded; the expanded mode's
      trainset weight and length (driveruipanels.cpp:360-386); coupling or uncoupling does not
      re-resolve which driver of the trainset the panel follows until the next timetable change
      or a change of the player's vehicle.
      The driver's hints to a player (`cue_action()` shown as the original's hint list,
      `driver_hint`, Driver.cpp:2759-2916) are not shown - to be a tooltip-styled HUD panel;
      today `MaszynaLegacyDriverHints.cue()` only acts when the driver drives.
   Braking table (2026-09-27): `CheckVehicles()` ported - the table `fBrake_a0/a1` from the
      vehicles' `BrakeForceR()` (`RailVehicleBrake.get_force_at()`), `fAccThreshold`, the brake
      reaction, the cargo flags, the brake setting per vehicle (`auto_rewident`),
      `BrakeAccFactor()` and `braking_distance_multiplier()`. The eszelon at x20 now stops 7 m
      short of E4 (was 44 m past). The acceleration limit by the couplers' strength, the downhill branch and
      the final `BrakeAccFactor()` check of `adjust_desired_speed_for_current_speed()` are ported
      (2026-09-27). Left: the weather's friction; and why the eszelon
      almost stopped on n226 before E4 and pulled away again (x20, not looked into).
   7. Coupling up and uncoupling (`UpdateConnect()`, `UpdateDisconnect()`,
      `determine_proximity_ranges()`): within 20 m of the vehicle ahead the front vehicle starts
      coupling, and within 2 m the shunter joins the elements the order's coupler number asks for
      - the vehicle's `coupler_connect`, one element an update, as the player's crew does.
      Uncoupling brakes the train, turns the reverser, presses the buffers at 2 km/h with up to
      50 kN, releases the vehicles' brakes (`brake_releaser`) and undoes the coupler
      (`coupler_disconnect`) the counted vehicles away, then turns back and takes the next order.
      The distances kept and the speed margins (`fVelPlus`, `fVelMinus`) are the original's per
      order now; they were the train's everywhere before.
      **Not checked on a scenery** - Stary Jawor has no coupling; linia61 and calkowo do
      (`l61_towarowy1_hn.scm`, `events_tartak.ctr`), too heavy for a headless probe so far.
      Left: the coupler adapter (`couplingadapterattach/remove`); the high voltage and power
      lines of a coupler number (no element a shunter joins); `coupler_connect` joins its elements
      in a fixed order, so one asked for past a skipped one brings the skipped one too; the lights
      after the trainset changed (`CheckVehicles()`); the electro-pneumatic brake's own
      uncoupling position (`bh_EPB`); a coupling a player left half done under another order; the
      margins of modern vehicles and of the weather, and a late train's (`moveLate`).

### To confirm in game (2026-09-29)

* Stary Jawor Osobowy 1: the AI's n323m (SM42-2303) ran past a signal at stop into Roztocze while
  the player shunted in SM42-329. Not reproduced headless with the player idle; the two causes
  found the same day on krzyzowa2 fit it - a far stop read on one update and lost on the next
  (lerpf(), "the driving aid flickered") and signals closing behind the train braking it hard
  ("a signal closing behind the train braked it hard"). Both fixed; to be driven again in game.

### Cab targets (2026-09-27)

* Gauges read the occupied vehicle unless the MMD catalog tags them: the ammeters, voltmeters and
  lamps of the motor car (`Train.cpp` `mvControlled` in `update_gauges`) need `target` in
  `MmdSemanticCatalog`.

### Driving aid (`demo/hud/driving_aid.gd`, 2026-09-28)

* Left out of the original's panel (`driveruipanels.cpp:42-205`): the reverser letter (D/N/R/T),
  the grade, the slipping `!`, the brake cylinder pressure, the load exchange / vehicle ahead line
  and the alerter/SHP line.
* An EIM vehicle shows `MainCtrlPos + ScndCtrlPos` - the original shows `eimic_real` in % plus
  `MainCtrlPos` (`EIMCtrlType` is not exposed), and the integrated brake's `eimic` braking
  (`UniCtrlIntegratedBrakeCtrl`) instead of the brake handle.
* The nearest signal and its lights (asked 2026-09-29, postponed). The driver knows the signal only
  as the memcell its `SetVelocity` is read from (`(p1)_sem_mem`); the lights are another model
  (`(p1)`, `(p1)_sk12`), so names do not tie them. What ties them is the scenery's own `multiple`
  (`(p1)_s1` fires `lights` on the model and `updatevalues` on the memcell together,
  `ss3zcbyw24.inc`) - a memcell → signal head link recorded from that. The lights' colours
  (`Light_OnXX` material or the node's `lightcolors`) have no public getter in
  `E3DRenderingServer`; wanted as coloured lamps.
* `PrepareEngine()`'s readiness (Driver.cpp:2843-2851) is ported without "any compressor enabled"
  and the brake handle position, and compares the main reservoir (`compressor_pressure`) where
  the original reads the feed pipe (`ScndPipePress`).
* `VelLimitLastDist` is ported, `SwitchClearDist` only as far as it extends it; the original's
  `moveSwitchFound`/`moveStopPointFound` in the reset of `VelSignalLast` (Driver.cpp:1043) are not.

## Game data (GameDataServer)

* `MaszynaVehicleProfileManager._ensure_viewport()` is an `ensure_*` API (`AGENTS.md`
  PROHIBITED): the render viewport is to be created where the manager is.
* An owner that is not a scenery's still rebuilds its streamed pieces (`owner_rebuild()`) when the
  scenery that registered them reloads itself in the same reload: the clears are wasted, the
  builds are dropped with the freed pieces.

## Tests

* `test_train_electric_induction_engine.gd` fails 3 tests on a clean `46a51cd2` (checked
  2026-09-29 in a separate worktree): `_powered_up_eim()` never closes the line breaker, so
  `..._does_not_turn_forces_into_nan`, `test_the_state_carries_each_inverter` and
  `test_driven_induction_motor_pulls_once_the_controller_moves` fail. Not looked into.
* `test_e3d_rendering_server.gd` passes 9/9 but the process does not exit (timeout at 60 s),
  seen after godot-cpp was raised to `507ed9d` (2026-09-27); not checked whether it hung before.
* Stary Jawor's eszelon at x10 (headless, 2026-09-27) runs at about 20 km/h wanting 70: the master
  controller jumps 0-7 and `Ft` is 0 half of the time; at x20 the same run reaches 51 km/h. Not
  looked into. At x1 (headless, two runs) it stood once 100 m short of E4 for ~17 s wanting
  70 km/h: not ready while 20 wagons (G) released after braking for E4, the FV4a handle at
  running and the pipe climbing from 4.63 bar; not checked whether the original fills the pipe
  faster there. The operator's stop with the local brake applied in full is not reproduced yet.

* **No regression test for the couplers stiffened by a long frame** (2026-09-27). A snatch of
  `test_vehicle.fiz` wagons does not tell the fixed build from the broken one - the fixture
  wagons stop within 3 s whatever the frame. It needs a free-rolling trainset fixture (or a
  powered one pulling a long train), stepped at 0.017 s and at 0.17 s a frame. Checked so far
  by hand on Stary Jawor only (the eszelon at 0.03 s and 0.17 s: 14.09 and 14.06 m/s at 80 s).

* `test_zzz_ep07_cabin_main_switch.gd` crashes (SIGSEGV) in about half of the runs, at `82cda7a30`
  too: the headless dummy renderer's mesh storage is not thread safe, and the streaming worker
  preloads models while the main thread loads the cab's (`docs/findings-archive.md`, 2026-09-26
  "headless test crashes at teardown"). To decide: serialise model loading (one lock in
  `E3DModelManager.load_model()` - costs the real game a wait on the main thread), or load on the
  worker only with a real renderer.

* **No HUD panel test on a non-diesel.** `mover_gauges.gd` broke on an induction motor (it asked
  `RailVehicleEngine` for `get_rpm()`/`get_oil_pump_pressure()`, which are `RailVehicleDieselEngine`'s);
  fixtures build a diesel. Needs a panel test per engine kind (diesel, series, induction), for the
  other migrated panels too.
* **A dump key does not name the class owning its getter** - check the declaring header, not the
  fill, when mapping keys to typed reads.
* Tests that switch the game dir with `UserSettings.save_maszyna_game_dir()` write the user's
  `settings.cfg`: `test_maszyna_rail_vehicle_3d_manager.gd`, `test_e3d_lights_state.gd`,
  `test_fiz_train_controller.gd`, `test_maszyna_node_dynamic_importer_direction.gd`,
  `test_material_manager_variants.gd`, `test_nodebank_library_builder.gd` and the tests spawning a
  vehicle of `demo/tests/fixtures/dynamic/`.
  Needs a non-persistent override. Every such switch also reloads the game's data
  (`GameDataServer.data_reload()`), in `after_each` too.

## Physics performance

* The frame drop with a trainset in a scenery is **the scenery's dynamic lights** (operator report),
  not the vehicle step; nothing bounds how many are lit (`FINDINGS.md`, 2026-09-21). Measure the
  count first.
* Compare against the original only on an optimized build (`make compile-profiling`):
  `compile-debug` builds `Mover.cpp` at `-O0`, and `baltyk_skm1` went 31 -> 44 fps from that alone.
* Reference: on `baltyk_skm1.scn` (376 vehicles) the original spends 1.8 ms CPU per frame on
  everything; ours was ~36 ms, with the cost in reaching the same `Mover.cpp` (GDScript crossings,
  per-frame state dictionaries) - the evidence behind #184.
* Multi-core physics: keep the phases of `vehicle_table::update()` (`DynObj.cpp:8181`: locations +
  neighbours, then per iteration forces of all, movement of all), run per island (a coupled trainset
  plus vehicles in collision range - `CouplerForce()`/`CollisionDetect()` write the neighbour's
  `V`/`AccS`) on `WorkerThreadPool::add_group_task` with a barrier per phase; no Godot calls on
  workers.

## Linux release built on an old glibc - what is left

* `release-linux-symbols` (`compile-release-symbols`) still builds on the host, so it needs the
  host's glibc and won't start on the machines it should diagnose. Needs the `release-linux`
  container.
* The debug export template (`linux_debug.x86_64`) is built in the SDK by the `godot-engine`
  workflow now; a local install still holds the host-built one until `ci/fetch-godot.sh` replaces it.

## CI

* A pull request from a fork has a read-only token, so after a `GODOT_VERSION` bump it cannot
  publish the engine release; one from this repository has to build it first.
* The Linux library is built in the SDK container without ccache, so every CI run compiles it in
  full (Windows, Android and the tests use ccache on the runner).
* The engine is built without Swappy (Android frame pacing, `install_swappy_android.py`) and without
  AccessKit (`install_accesskit.py`), both of which the official builds carry.
* Android: only `arm64` is built and exported; the `android_x86_64` preset has no template.
