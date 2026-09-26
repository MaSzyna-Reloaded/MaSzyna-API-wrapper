# TODO

## Architecture rework (#184) - remaining stages

The vehicle becomes an object owned by a server, addressed by RID, with thin `*Node` proxies for
the editor. Each stage is one PR, titled `(#184) <area> - <what>`, and each leaves the game
runnable. Stages 1-3 are done (`RailVehicleServer` owns placement, movement and the step;
`MoverVehicleController` owns the `TMoverParameters`, components reach it through `MoverComponent`).

Design that replaced the withdrawn stage 4 (a global name registry, now deleted):

* **State is a typed property whose getter reads the backend field** (`VehicleBrake::get_pipe_pressure()`
  returns `mover->PipePress`). A component keeps only what the Mover has not got: the brake
  pressure filter, the door interpolation, the wiper positions, a `_prev` for change detection.
* **Config stays the wrapper's**, deliberately - written at (re)configuration, read rarely. It is
  the one intentional duplicate.
* **Dumps are lazy and separate:** `_fill_state_dictionary`/`_fill_config_dictionary` per
  component, composed by `RailVehicleServer::vehicle_dump_state(rid)`. A key the vehicle's variant
  has not got is not written, so `has()` keeps its meaning.

**Stage status, verified against the code on 2026-09-25** (earlier claims in this file were
written when the first part of a stage landed and never corrected - check against the code, not
against this file; the 09-24 list had already gone stale in G by the time it was re-measured):

* **A - done**, except `doc_classes/VehicleState.xml` (50 lines), which still publishes the deleted
  class. `doc_classes/RailVehicleServer.xml` is clean now.
* **B - partial.** **The hottest path misses the one state cache** - see the next block.
  None of the five
  common values (`velocity`, `speed`, `mass_total`, `total_distance`, `direction`) is a property;
  only the first two have a server forwarder. `Dictionary config` has left the controller.
* **C - partial.** Done: the component is an `Object`, fetch/tick split, interface/implementation
  split (21 `MoverVehicle<Domain>` classes, each with a `Vehicle<Domain>` interface of the same
  name, none of which names the Mover), and **`vehicle_component_get(rid, type)` now exists and is
  bound** (`RailVehicleServer.hpp:222`). Missing: `vehicle_component_create` (zero occurrences) -
  components still come from `ClassDBSingleton::instantiate()`/`memnew`;
  `generic_vehicle_component_find` has zero callers; `GenericVehicleComponent` copies
  `_get_component_state` per tick instead of being walked for `PROPERTY_USAGE_SCRIPT_VARIABLE`
  (which appears nowhere); `GenericVehicleComponentNode` finds its vehicle via `get_parent()`; the
  dump mixes nine `prefix/key` namespaces with flat `component_key` names. Typed state names drop
  the prefix (`brake_pipe_pressure` -> `brakes.pipe_pressure`); dump keys keep it.
* **D - one of three.** The controller is an `Object`. Not done: it is still created and owned by
  `VehiclePhysicsNode::_build()` (`:94` creates it, `:114` takes the handle) - it belongs in
  `vehicle_create()`, which would also remove `VehiclePlacement::controller_id`; registering the
  name and the commands still hangs off `attach_to_system()` (2 call sites).
  `RailVehicle3D` creates a second handle of its own - a node that draws a vehicle should own none.
* **E - not started.** Zero of the ~20 proxy nodes; no `VehicleControllerNode`.
* **F - done**, what is left of the area:
  * The node's public API is the `.scn` `dynamic` line only: `data_path` + `file_name` + `skin`
    locate the data; exported stay `train_id`, `initial_velocity`, the occupant,
    `start_track_name`, `start_track_offset`, `start_direction`, `head_display_material`. No cabin,
    sound, pantograph or light property belongs on the node (all MMD).
  * The trailing `destination` of the `dynamic` line is dropped
    (`maszyna_node_dynamic_importer.gd`).
  * `RailVehicle3D` switches processing on (`:64`, `:340`) and never off, so every vehicle ticks
    forever to look at two dirty flags; the setter should turn processing on and the tick off.
* **G - most of it landed.** `@export_node_path("VehicleController")` is gone from **all 14 files**.
  Of the cabin scripts, **20 take a single key** through `CabinSystem.vehicle_state_value()` and
  **9 still take the whole dump**, of which only `cabin_windscreen_wipers.gd` does it per frame.
  Left: `RailVehicle3D` reads the whole dump 4 times (`:541`, `:553` pantograph helpers, `:712`
  roof light, `:1108` wiper positions), `TrainSoundSystem` once, and `CabinSystem`'s whole
  vehicle-facing surface is still keyed on `train_id:String` (24 occurrences) rather than the RID.
  The one-cache change below removes the *cost* of those reads; taking the component removes the
  *coupling*, and both are still wanted.
* **H - not started.** No `UpdatePhase`; the order is hand-written in
  `RailVehicleServer::step_frame()` (`:803`). Check it against
  `TMoverParameters::ComputeMovement`/`Update` and the three ordering bugs on record (#57 line
  breaker, `Mred`, `roof_light_enabled`). `test_vehicle_doors.gd` must exist first - `VehicleDoors`
  ticks and has no test.
* **I - two of four.** `MaszynaMoverPhysicsServer` and `vehicle_get_mover()` are gone, and so is
  `TrainSystem`: vehicles are held and commanded by RID, and `train_id` is only the scenery name in
  `RailVehicleServer`'s name registry, where it may be empty or repeated. Left: `train_id` is still
  written by the physics node rather than having `RailVehicle3D` as its only writer.

**One state cache, and it lives in the vehicle server.** Measured 2026-09-25: the state is cached
in the server only (CabinSystem's own cache went with its move to RIDs, 2026-09-26), and the
hottest path misses it.

* `RailVehicleServer::vehicle_dump_state(rid)` (`:766`) holds the dump **per RID**, keyed on the
  physics step **and** the command serial. This is the right place and it works.
* `VehicleController::get_state()` caches nothing: it composes the whole dictionary from every
  enabled component on every call. The dependency runs server -> controller, so every direct
  reader (`RailVehicle3D` 4x, `TrainSoundSystem`) rebuilds it and never
  touches the cache.

The fix is to turn that dependency round. The body of `get_state()` becomes a private
`_compose_state()` - the one place that builds the dictionary - and `get_state()` asks
`RailVehicleServer::vehicle_dump_state(get_rid())` instead; the controller already knows its RID
(`VehicleController.hpp:274-275,344`), so this needs no part of stage D. The server's miss path
calls `_compose_state()`, so there is no recursion, and a controller with no handle (built by
`FizVehicleBuilder`, never attached) composes its own. `CabinSystem`'s three cache members then go.

**The key stays the step plus the command serial, not the frame.** A step is at least as fine as a
frame, and the second half of the key is there for a recorded reason (`FINDINGS.md`, 2026-09-23): a
command runs synchronously in the middle of a step, so keying on the step alone made the cab act
one keypress late. Moving to a bare frame counter would bring that back.

**The vehicle's name belongs to the vehicle server - done.** `vehicle_set_name()` /
`vehicle_get_name()` / `vehicle_get_rid_by_name()` on `RailVehicleServer`, like
`TrackManager::track_get_rid_by_name()`, are the only name registry; TrainSystem is gone.
`CabinSystem`'s whole vehicle-facing surface (`vehicle_state`, `vehicle_config`,
`vehicle_component`, `vehicle_state_value`, `occupied_cab`) is keyed on `train_id` - flip it to the
RID in one pass.

**The pantograph's power path is simulation living in a node.**
`RailVehicle3D::_update_pantograph_power()` computes contact points, asks `TractionPowerServer`
for the span and writes the voltage into `VehicleElectricEngine` every frame. It belongs in
`RailVehicleServer`'s step (with the remembered span per pantograph); the node keeps
`_apply_pantograph_animation()`. `pantograph_front_offset`/`pantograph_rear_offset` are exported on
the node but are the vehicle's geometry (`TAnimPant::vPos`). Weaker: the wiper *positions* in
`_update_wipers()` are simulation too.

**Live bug:** `RailVehicle3D.cpp:93` offers a node path of type
`"VehicleController,FIZTrainController"` - the first is an `Object` and cannot be picked, the
second exists nowhere.

**A test must not clobber a global setting.** `test_fiz_train_controller` points
`UserSettings.save_maszyna_game_dir()` at its fixture in `before_all`; a crash skips the restore
and the game then starts with `user://gut/fiz_train_controller` and finds no scenery. Pass the
fixture path to what is tested; the same pattern is in `test_dynamic_rail_vehicle_manager` and
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

**`test_dynamic_rail_vehicle_manager` is red:** `registration.controller` is `null` - the sound bank
registers against a vehicle with no controller yet, and only the 4 Hz sweep repairs it, later than
the three frames the test waits. Connecting to `ready` (too late) or `tree_entered` (too early)
does not help; it was believed to register against the template, the packing that stage F removed
- re-check now that F has landed, and fix it in the vehicle building, not the sound system.

**The `.fiz` path has not been run in the game** since the components stopped being nodes - only
in tests.

**Coverage gap before stage C:** nothing tests `GenericVehicleComponent` (none of the 95 scripts);
its renamed GDScript API (`_process_component`, `_get_component_state`, `_get_component_config`,
`get_vehicle_state`, `get_controller`) was checked only by running
`demo/examples/custom_train_part.tscn`/`custom_powered_train_part.tscn`.

Traps for every stage: bump the cache tag with the code whose output is cached
(`FIZ_PARSER_FORMAT_VERSION`, `MaterialManager.CACHE_VERSION`, `E3DModel.FORMAT_VERSION`,
`structure-vN`); run `godot-double --headless --import` before believing "Identifier not
declared"; never pass a bare `[]`/`{}` to a typed collection; add `doc_classes/<Class>.xml` for
every registered C++ class.

### Rail concepts in interfaces named "Vehicle"

`VehicleComponent`/`VehicleController` are generic on purpose (road vehicles), but several
interfaces are rail-only (rail-term count per header): `VehicleBrake` 43 (brake pipe, W/Lu/L,
W/Lu/VI, W/Lu/XR, K valves, FV4a), `VehicleElectricEngine` 34 (pantographs), `VehicleBuffCoupl` 13,
`VehicleWheels` 12 (bogies, pivot spacing, `get_bogie_transform()`), `VehicleSecuritySystem` 2,
`VehicleSpringBrake`/`VehicleElectroPneumaticDynamicBrake` 1-3. Generic and correct:
`VehicleWipers`, `VehicleUniversalController`, `VehicleSpeedControl`, `VehicleHorns`,
`VehicleDoors`, `VehicleHeating`, `VehicleLighting`, `VehicleLoad`. Either rename to `Train*`
(cost: `VehicleWheels` 13 files / 50 mentions, `VehicleBrake` 24 / 283) or split a generic base
from a rail subclass - only worth it once something road-side shares the base. Either way the
interfaces keep naming no backend.

### What still reaches a class by name from C++

Allowed (GDScript hosted by C++, commented at the call site): `Cabin3D::_propagate_train_id()` ->
`set_train_id`; `GenericVehicleComponent` -> `_process_component`, `_get_component_state`,
`_get_component_config`. Not allowed - our own classes still in GDScript, fixed when their base
moves to C++:

| Class | Named accesses | Where |
| --- | --- | --- |
| `E3DModelInstance` | 15 | `is_e3d_loaded` x6, `reload` x2, `get_aabb`, `set_smoke_intensity`, `instancer` x2, `lights_state` x3 |
| `MaszynaTrackCurve` | 10 | `p1`, `c1`, `c2`, `p2`, `roll1`, `roll2` in `TrackManager` and `RailVehicleServer` |
| `RainVolume` | 6 | `velocity_multiplier`, `bound_enabled`, `bound_min`, `bound_max` |
| `MaszynaPlayer` | 1 | `get_camera` |

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

* `pantselect_sw` / `PantsPreset` (choosing which pantographs the master valve raises,
  Train.cpp:3529 change_pantograph_selection, update_pantograph_valves) is not ported.
* `MoverElectricEngineBackend::pantograph()` still opens the master valve itself when a pantograph
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
* The radio volume is state only - the wrapper plays no radio messages.

### Gauge lamps (`<name>_on`)

Only the reverser buttons have their `state_light` so far. Train.cpp:11995-12040 binds a flag to
about forty more gauges (speed control buttons, door permits, door step, ...), each needing a
state key and a catalog `state_light`. The lamps also light without low voltage - TGauge gates
them on it (Gauge.cpp:379).

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
* Occluders are every mesh of a generated cab model, transparent ones included - the original's
  pick pass draws only opaque submodels (`Render_cab(..., Alpha = false)`,
  opengl33renderer.cpp:1208). Hand-authored cabin scenes register no occluders at all.

### DebugWindow

* `debug_hud.tscn` (used by `examples/mover_demo.tscn`) hands the vehicle only to `MoverSwitches`,
  which does not pass it on to its sections - there the sections stay disabled. `game_hud.gd`
  propagates it to every widget.

### Python integration

`PythonScreenServer` runs the original's Python 2 screen scripts, `CabinPythonScreen` draws them on
the cab submodel, `PythonScreenState` maps state onto `TTrain::GetTrainState()` keys. Left out:

* **The runtime is not shipped.** CPython 2.7.18 + Pillow 6.2.2 belong in `ci/docker/linux-sdk`
  (like the export template, FINDINGS 2026-09-24 glibc), installed as `python2.7/` in the game
  dir. The original's `linuxpython64` is a virtualenv without PIL. Windows untested; should use
  the game dir's `python27.dll` and `python64/` (PyInt.cpp:233). The dev copy was built by hand and
  links the system libjpeg/freetype/zlib.
* **Keys with no source yet** - each needs its Mover field published by its component, then one
  line in `PythonScreenState`:
  * controlled vehicle: `pant_compressor` (PantCompFlag), `new_speed` (NewSpeed),
    `speedctrlstandby` (SpeedCtrlUnit.Standby), `scnd_ctrl_actual_pos`, `brake_delay_flag`,
    `brake_op_mode_flag`, `pipelock` (LockPipe), `tractionforce` (Ft), `voltage` (EngineVoltage),
    `im` (Im), `power_drawn`/`power_returned` (EnergyMeter), `lights_compartments`
    (CompartmentLights), `off_from_dimmer` (dimPositions), `main_init` (MainsInitTime), lamps
    beyond the five carried (rearendsignals, auxiliary_*) in `lights_front`/`lights_rear`;
  * train row `eimp_t_*` and the ED share of `dir_brake` (eimic_real, eimv[eimv_Fful], Itot);
  * per car: `eimp_pnN_cp` (CntrlPipePress), `eimp_pnN_rp` (Hamulec->GetBRP()), `eimp_pnN_mass`
    (TotalMass - Mred), `code_N` (last letter of TypeName; `type_name` is not in the config dump),
    `doors_no_N` (iAnimType[ANIM_DOORS]);
  * per powered car: `eimp_cN_fr`..`uhv` (eimv[], Itot, EngineVoltage), `eimp_cN_invno`,
    `eimp_cN_invM_act/error/allow` (InvertersNo, Inverters[] - 38 scripts),
    `diesel_param_N_fill_des`/`clutch_des` (RList[MainCtrlPos]), `clutch_real` (dizel_engage),
    `water_temp`/`engine_temp` (dizel_heat), `retarder_fill` (hydro_R_Fill); powered is told by
    engine type, the original tests eimc[eimc_p_Pmax] > 1;
  * `TDynamicObject::FindPowered()` searches only an EZT/DMU unit - the train type is not in the
    config dump, so the wrapper searches the whole control coupling.
* **Cab keys** (TTrain members): `universal0`..`29` (ggUniversals), `universal3`
  (InstrumentLightActive), `radio_volume`, `distance_counter`, `main_ready` (fHVoltage),
  `lights_train_front`/`rear`.
* **No AI driver / timetable** - `velocity_desired`, `velroad`, `vellimitlast`, `velsignallast`,
  `velsignalnext`, `velnext`, `actualproximitydist`, `train_atpassengerstop`, `train_length`,
  `trainnumber`, every `train_*` key (TTrainParameters::serialize(), mtable.cpp:641), and
  `$timetable=` (dictionary.cpp:36).
* **No test** for `RailVehicleServer.vehicle_get_coupled()` (order from the far end, stop at a
  coupling without the element, a turned vehicle) or `PythonScreenState.compose()`; only checked
  with the E186 in `td_e186.scn`.
* **Commands a script returns are not executed** (two scripts send `lightsset`); map
  `simulation::commandMap` names onto vehicle commands (PyInt.cpp:138-194).
* **Touch input** (`touches`, `screen_touch_list`, Train.cpp:10713) is always empty.
* `pyrylandia` is referenced by an MMD and exists nowhere under `dynamic/`.

### Other

* `VirtualCabin` for cabs without a hi-fi model (`cabNmodel: none` or missing, e.g. su46
  `cab0definition:`) - input and command translation only. The original keeps such a cab
  enterable with the low-poly interior (`Train.cpp:8692`, `DynObj.cpp:1214`). Hook:
  `DynamicTrainCabin` builds an empty cabin with `has_cab_model = false`.
* Keyboard input per control, not per widget: each `CabinButton`/`CabinSwitch`/`CabinKnob` handles
  `action*` itself, so a repeated label (EP07 cab0 has two `cablight_sw:`) toggled itself back.
  The original maps a key to one command (`Cabine[].bLight`, `Train.cpp:10237`). Move key handling
  to `CabinSystem`/`LegacyCabinLogicDelegate` (once per `control_id`), then drop the workaround in
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
  ED/PN brake force split of an induction motor consist, `EqvtPipePress = GetEPP()`, and the
  unpowered-car copy of MainCtrlPos/SpeedCtrl (DynObj.cpp:3272-3276).
* `VehicleElectricEngine::pantograph_first/second_wire_voltage` are written by
  `set_pantograph_wire_voltage()` and read by nothing.
* Wheels turn at half speed: `MoverVehicleWheels::_do_process_component` adds `rad_to_deg(V*dt/D)`,
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

* MMD offsets of non-running sounds are used raw - the `SfxPlayer3D`s are not turned 180 degrees
  like the model (`maszyna_rail_vehicle_3d_instancer.gd:97`), so horns, compressor, brakes etc.
  with `offset:` sit mirrored (x, z). Running sounds convert
  (`MmdSoundBankInstancer._build_running_events()`).
* Missing running sounds: `tractionacmotor:`/`inverter:`/`motorblower:` (`DynObj.cpp:5745-5800`,
  `8012-8080`), `wheelflat:` (`DynObj.cpp:4722`), `derail:` (`DynObj.cpp:5910`), `transmission:`
  (`DynObj.cpp:5822`), cab `huntingnoise:` (`Train.cpp:8284`).
* Wiper sounds (`wiperfrompark:`, `wipertopark:`, `DynObj.cpp:4082-4099`) not played; the arm swing
  direction (`RailVehicle3D::_update_wipers()`, as `TDynamicObject::UpdateWiper()`) not checked in
  game.
* Wheel clatter bump (`AccVert`, `DynObj.cpp:3533`, cab shake only) not ported.
* Open cab window (`Global.CabWindowOpen`): the original plays the consist's outer noise and stops
  the cab running noise (`DynObj.cpp:4638`, `Train.cpp:8274`); no cab window state yet.
* `pitchvariation:` (default 0.975-1.025, `sound.cpp:375`) is parsed, never applied.
* `pantographup:`/`pantographdown:` play at the bank's position for both pantographs; the original
  places them at the pantograph that moved (`DynObj.cpp:3881-3934`, `4007-4036`). The E186 bank
  was not dumped after adding them, nor after `converter:`/`small-compressor:` were wired.
* `brake_release_hiss` (`unbrake`) is the one pneumatic event the brake factory does not build - it
  goes through `TrainSoundSystem._update_triggers()` without `gain` or the `listener_inside`
  correction, so it is louder in the cab than the other hisses.
* `TrainSoundSystem`'s `VOLUME_FACTOR`/`CABIN_UNIT_SIZE_FACTOR` (2.0) were run at 1.0 through a
  `project.godot` override and are not verified by ear at 2.0 (`EXTERIOR_*` are 1.0).
* The gnd-sfx tick is GDScript on a worker (12 ms/frame for 200 players, headless). If it limits,
  move the runtime to a C++ singleton beside `E3DRenderingServer`.
* `SfxGeneratorPlayback.update()` runs on the sfx worker (single producer into the ring buffer);
  revisit if a generator clip ever needs the scene tree.

## Vehicles

* `DynamicRailVehicle3D` builds its `RailVehicle3D` itself (`_rebuild()` in its own `_process`), so
  vehicles appear a frame after the scenery (`SceneryInstancer._wait_for_vehicles()`). Building
  belongs in a `DynamicRailVehicle3DFactory`.
* A distant vehicle's low-poly interior (`OPTIMIZED`, `RailVehicle3D::_update_model_detail()`) has
  no materials to dim and keeps its baked emission regardless of `roof_light_enabled` until back
  within `maszyna/rendering/vehicle_detail_distance`.
* The `structure-vN` tag (`dynamic_rail_vehicle_3d_manager.gd`) is bumped by hand; a
  `MaszynaRailVehicle3DInstancer` change without a bump keeps serving the old structure, and the
  cache survives a checkout.
* Braked standing vehicles never sleep: at `V == 0` `Sign(0) == 1`, so `FTotal = FTrain - FStand`
  keeps `AccS` non-zero (`Mover.cpp:4603`) - same in the original.
* A vehicle with its physics off keeps its last state (fetched only for active vehicles, as the
  original skips `Update()`).
* The rest of `LoadFIZ_Cntrl`'s start modes never reach the Mover: `CompressorStart`,
  `PantCompressorStart`, `MainStart` and `ConverterOverloadWhenMainIsOff` (Mover.cpp:10905-10925)
  are not parsed, and their properties sit on `VehicleElectricEngine`, so a diesel could not
  carry them anyway. `ConverterStart`/`ConverterStartDelay` moved to `VehicleController`; the
  others belong there too. The `converter` command is still an electric engine's only.
* `BrakeValveParams` (the raw `BrakeValve=` string, Mover.cpp:10397) is never set, so
  `TNESt3::SetSize()` builds every ESt distributor as an ESt4: `TRapid` instead of `TRura` and no
  `Podskok` for ESt3, and `AL2`, `PZZ`, `HBG300`, `3d`/`4d` and `-ED` are dropped. That covers
  about 200 FIZ files of the datapack (ESt3, ESt3AL2HBG300, ESt4HBG300-s216, ESt3d_PZZ, ...).

## Rendering

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
  Scenery loading). Wind drift is ported (`E3DRenderingServer::set_wind()`, `0.1 * wind`).
* `MaszynaEnvironmentNode.wind_direction` is a compass bearing, so wind is always horizontal;
  `MaszynaSkyEnvironment.get_wind_direction()` already returns a `Vector3` and `set_wind()` takes
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
  `E3DModelInstance` hides; the original stops beyond `2 * BaseDrawRange * fDistanceFactor`
  (`particles.cpp:452`), the wrapper streams only scenery ones (`maszyna/rendering/smoke_distance`).

### Other

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
* `*_specgloss` shaders other than `parallax_specgloss`/`water_specgloss` ignore the specgloss
  texture (`normalmap_`, `default_`, `reflmap_`, `detail_normalmap_`, `shadowlessnormalmap_`,
  `sunlessnormalmap_`).
* `rain_windscreen.gdshader`: droplets ignore speed and wind (a TODO in the original too); it reads
  the screen texture, so transparent things behind the glass (rain particles) fade under the film.
  Film, large droplets and rivulets are the wrapper's own, tuned by eye (`heavy_rain_start`,
  `film_*`, `rivulet_*`, `refraction_strength`); "down" not checked on a real cab glass.

## Scenery loading

* Air temperature (`MaszynaEnvironmentNode.temperature`) is consumed by nothing; the Mover uses it
  only in `dizel_heat.Te` (`Mover.cpp:8109`) and the vendored one has
  `#define Global_AirTemperature 15.f` - not feedable without touching `src/maszyna/`.
* Other `config` entries dropped (`scenario.time.override/offset/current`, `Globals.cpp:356-385`).
* Include instancing: `skp/skp_trawa.scm` includes `grass.inc` 24078 times, each parsed and baked
  to world space. Idea: classify includes as `instanced` (only `origin`/`rotate` + `triangles`, no
  nested includes; key = path + hash of non-placement params; MultiMesh per chunk/texture/range) or
  `full` (key = path + hash of all params); cache in local space, invalidate by dependency list.
* The subscene cache (`SceneryInstancer.parse_subscene_task()`) is used only by queued parsing;
  `parse_file()` reparses every include.
* `_count_includes()` counts includes of cached subscenes that never run as tasks - the bar jumps.
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
* Nothing gives geometry back: `SceneryChunkRenderingServer.ChunkState.mesh` and
  `E3DRenderingServer`'s model cache never evict. Needs the per-chunk disk cache.
* In the editor streaming follows 3D viewport 0 only (`addons/libmaszyna/editor/scenery_streaming/`).
* The trackbed of switches renders incorrectly.
* `maszyna_node_track_importer.gd` drops every type but `switch`/`normal`: `road` (~16 700),
  `river` (~900), `cross` (72), `turn`, `table`. `road`/`river` need a flat surface path
  (`Track.cpp:1554` on); `cross` is a road intersection with four endpoints, no topology support.
* Lamp head colour (texture, sodium orange) does not match its pool (tinted by
  `maszyna/rendering/scenery_light_tint`). Tinting emission needs a flag in the
  `E3DMaterialResolver` key (like `force_alpha`) - `latarnial_betdziur` shares
  `elektryczne/oprawa` between bulb and housing.
* A lamp shadows its own light (economy mode spokes); `light_set_shadow_caster_mask(~SCENERY_LIGHT_OWNER_LAYER)`
  does not remove them (checked 2026-09-21). Measure whether the clustered renderer ignores the
  mask for spot/omni or the layer bit is not set - a scratchpad project with one box on a second
  layer under a SpotLight3D. Fallbacks: `instance_geometry_set_cast_shadows_setting(..., OFF)` on
  the light-owning model (loses sun shadow) or no shadows in economy mode.
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
  fog, temperature): the menu keeps it (held silent by `MaszynaRuntime.pause()`) and a next scenery
  without an `atmo` section inherits it.
* `MaszynaRuntime.pause()` holds the vehicle step, the weather and the world's sounds only - the
  environment clock, `TractionPowerServer`, `TrackManager` switches and the smoke keep running.

## Semaphores (#296)

`SemaphoreServer` with systems, delegates, sources, kinds and the nodes is in place; a scenery
semaphore's kind is made of the `lights` events aimed at it, and the original's
`MaszynaLegacySemaphoreDelegate` shows one of them when it is handed the event. Left:

* **The isolated sections become the system's sources** once `TrackManager` has them (see
  Scenario events).
* **The logical aspect for trains** - memcell `SetVelocity`/`ShuntVelocity` read through a passive
  `getvalues` (`Driver.cpp:459-470`, `:292-419`) - has no counterpart; a delegate only publishes
  its aspect with `system_publish_event`.
* **`ls_Dark`/`ls_Home` from a `lights` event** (value 3, 24 times in the data set): the semaphore
  API has no light that follows the daylight; the legacy kind factory warns and keeps the light.
* **Semaphore arms** - the `animation` event on a named submodel (`Event.cpp:1569-1735`).
* **Every lit scenery model is a semaphore** (the operator's decision), street lamps with
  `lights 3` included - telling semaphores apart is open.
* **Scenery kinds are made of `lights` events only**: a lit model no event reaches gets the generic
  kind (lights, no aspects). The declared `lights` list is not an aspect, so a scenery semaphore's
  light states on the server read `LIGHT_STATE_OFF` until its first aspect, whatever E3D shows.
* **`SemaphoreAspect.lights` are plain numbers** in the inspector (`LightCommand`), not an enum.

## Scenario events

`ScenarioEventServer` (events, the queue, the simulation time, memory, launchers, track events)
runs the scenery's events; `MaszynaLegacyEventFactory` builds them from the `.scn` data once the
include's server data is built. Built: `updatevalues`, `addvalues`, `copyvalues`, `multiple`,
`lights`, `switch`, `trackvel`, `voltage`, `animation` (rotate, translate, with its
`<model>.<submodel>:done`), `sound`; conditions `memcompare`, `memcompareex`, `probability`,
`trackoccupied`, `trackfree`; a track's `event0/1/2`, `eventall0/1/2` and `<track>:<slot>` events;
isolated sections (`TrackManager.isolated_*`, `isolated`/`area` blocks, `:busy/:free/:inc/:dec`, the
section's own memory); `onstart` and negative-delay events; Shift+0..9 (`keyctrl00-09`), launcher
keys (`ScenarioKeyboard`), HH:MM and radio call launchers (`radiocall1_sw`/`radiocall3_sw`,
Backspace); the "Scenario and Events" HUD window. Checked on `td.scn` with a headless probe (Shift+8
closes both level crossings). Left:

* **Track events, as the original fires them**: the direction filter by the consist's intended
  direction (`eventfilter`, `TrkFoll.cpp:117-121`) - here the actual direction of travel decides;
  the vehicle's one placement point stands for the primary axle; events with a delay <= -1 queued
  on every move along the same track (`TrkFoll.cpp:249-260`); a crewed vehicle is one with a
  `driver_type`, the original's `Mechanik->primary()` is one per consist.
* **Occupancy counts vehicles, not axles**: a vehicle is on the one track its placement point is
  on, where the original counts every axle (`TrkFoll.cpp:88-91`) - a vehicle across a joint
  occupies only one of the two tracks, for isolated sections and `trackoccupied` alike.
* **Isolated sections are not yet a semaphore system's sources** (`SemaphoreServer.system_add_source`).
* **`putvalues`/`getvalues`** send the vehicle only `CabSignal` (`security_cabsignal_trigger`) and
  `Emergency_brake` (`security_radiostop`); the driver's orders (`SetVelocity`, `Wait_for_orders`,
  ... `TController::PutCommand()`, `Driver.cpp:4468-4906`) and the Mover's other commands (`Load=`,
  `UnLoad=`, `BrakeDelay`, ... `Mover.cpp:12187-12720`) are dropped. A passive one (a signal command,
  `Event.cpp:719-751`) runs when its track event fires, where the original's driver acts on it
  while scanning the track ahead (`Driver.cpp:459-640`, `1396+`).
* **Event types without an action**: `whois`
  (`Event.cpp:993-1153`), `logvalues`, `texture` (`:1474-1543`), `friction` (`:2100-2104`).
  `switch` ignores the blade speed and delay (`Event.cpp:1855-1873`); `animation` has no
  `digital` or `.vmd` mode (`Event.cpp:1654-1682`); a `sound` event with a radio channel
  (`simulation::radio_message`) is played as a plain sound.
* **Scenery sounds** use the player's defaults for everything but `max_distance` (the node's
  range); their bank was not dumped nor checked by ear.
* **Memory and the AI**: pushing a memory to the vehicles on its track when it changes
  (`Event.cpp:538-548`), `bCommand`/`CommandCheck` and `:sent` (`MemCell.cpp:52-99`, `196-205`).
* **`departuredelay`** is read and dropped - needs the activator's timetable (`Event.cpp:2412-2425`).
* **Duplicate event names**: the later wins (with a warning); the original joins them as siblings
  and ignores the first (`Event.cpp:2296-2349`).
* **Launchers**: numeric key codes,
  `-10000` (first time in range, `EvLaunch.cpp:182-186`), `traintriggered` (the distance to the
  train, not the camera), a click on a model firing the launcher of its name (`scene.cpp:33-43`).
  Timed launchers are global here; the original polls non-global ones only near the camera.
* **Stary Jawor, eszelon** (headless probe, 2026-09-26): both stations run their logic, the
  shunting signals open (Roztocze Tm18, then Tm19/Tm20 with switches 74-76a once SU46 reaches
  `n176`), the 10:50 launcher fires; it stops where it waits for the AI's eszelon (below).
* **No AI trains**: a scenario whose stages wait for a train the AI drives stops there - e.g.
  `stary_jawor_eszelon` waits for the eszelon to reach Roztocze (`n282:event2`,
  `skp/skp_eszelon_events.ctr`). The track events fire for any moving vehicle with a driver, but no
  such vehicle moves without the AI.
* **Time-of-day launchers** match the clock `MaszynaRuntime` publishes once a second; at a
  simulation speed above 60 a minute can pass between two publishes and the launcher misses it
  (the original checks every frame).
* The `queueevent` console command.
* **Timetables** (`Timetable`, `TimetableEntry`, `MaszynaLegacyTimetableFactory`) are read but not
  used yet - nothing loads a trainset's or a `Timetable:` command's timetable and no driver follows
  one. `maszyna_trainset_importer.gd` takes the first token of `trainset <timetable> <track>
  <offset> <velocity>` for the trainset's name; it is the timetable. Not read: the station
  announcements (`load_sounds()`, `mtable.cpp:644-671`); `is_maintenance`, which the original tests
  on the track count token (`mtable.cpp:535`) and is never true. A timetable file in UTF-8 rather
  than cp1250 keeps mangled Polish letters in its labels (seen in `linia053/scenariusz_os`).
* Events of one include cannot refer to events of another `MaszynaIncludeNode`.
* Proxy nodes for editor-built scenes (`ScenarioEventNode`, `ScenarioMemoryNode`,
  `ScenarioLauncherNode`, the `SemaphoreNode` pattern).

## Tests

* `test_zzz_ep07_main_switch_trip_diagnostic.gd` fails at `9d9bff094` too - the vehicle does not
  accelerate past 2 m/s across 5 notches (it reads the game directory, see below).

* `test_mmd_semantic_catalog.gd` `test_i_radio_indicator_and_powered_omnilight_are_separate` fails
  at `5b5ad32e4` too ("Invalid access to property or key 'light_color' on a base object of type
  'Dictionary'") - not caused by the scenario work, not looked into.

* **No HUD panel test on a non-diesel.** `mover_gauges.gd` broke on an induction motor (it asked
  `VehicleEngine` for `get_rpm()`/`get_oil_pump_pressure()`, which are `VehicleDieselEngine`'s);
  fixtures build a diesel. Needs a panel test per engine kind (diesel, series, induction), for the
  other migrated panels too.
* **A dump key does not name the class owning its getter** - check the declaring header, not the
  fill, when mapping keys to typed reads.
* Tests reading game data (CI has none): `test_zzz_ep07_*` (cab_change, cabin_main_switch,
  main_switch_trip_diagnostic, orientation_regression, pantograph_power_smoke, running_sounds),
  `test_zzz_scenery_scene_smoke.gd` (instantiates `demo_scenery_loading.tscn`),
  `test_zzz_sm42_exterior_model_rotation_regression.gd`, `test_zzz_su46_exterior_lights.gd`,
  `test_zzz_su46_machine_room.gd`, `test_mmd_cabin_instancer.gd` (su45_v2),
  `test_rail_vehicle_rain_exclusion.gd` (sm42_v1). Replace with fixtures in `demo/tests/fixtures/`:
  a cut `.scn` with the track piece and trainset, fabricated vehicles with trimmed `.fiz`/`.mmd`,
  no e3d.
* Tests that switch the game dir with `UserSettings.save_maszyna_game_dir()` write the user's
  `settings.cfg`: `test_dynamic_rail_vehicle_manager.gd`, `test_e3d_lights_state.gd`,
  `test_fiz_train_controller.gd`, `test_maszyna_node_dynamic_importer_direction.gd`,
  `test_material_manager_variants.gd`, `test_nodebank_library_builder.gd` and the game-data tests.
  Needs a non-persistent override.

## Physics performance

* The frame drop with a consist in a scenery is **the scenery's dynamic lights** (operator report),
  not the vehicle step; nothing bounds how many are lit (`FINDINGS.md`, 2026-09-21). Measure the
  count first.
* Compare against the original only on an optimized build (`make compile-profiling`):
  `compile-debug` builds `Mover.cpp` at `-O0`, and `baltyk_skm1` went 31 -> 44 fps from that alone.
* Reference: on `baltyk_skm1.scn` (376 vehicles) the original spends 1.8 ms CPU per frame on
  everything; ours was ~36 ms, with the cost in reaching the same `Mover.cpp` (GDScript crossings,
  per-frame state dictionaries) - the evidence behind #184.
* Multi-core physics: keep the phases of `vehicle_table::update()` (`DynObj.cpp:8181`: locations +
  neighbours, then per iteration forces of all, movement of all), run per island (a coupled consist
  plus vehicles in collision range - `CouplerForce()`/`CollisionDetect()` write the neighbour's
  `V`/`AccS`) on `WorkerThreadPool::add_group_task` with a barrier per phase; no Godot calls on
  workers.

## Linux release built on an old glibc - what is left

* `release-linux-symbols` (`compile-release-symbols`) still builds on the host, so it needs the
  host's glibc and won't start on the machines it should diagnose. Needs the `release-linux`
  container.
* The debug export template (`linux_debug.x86_64`) is still host-built.
