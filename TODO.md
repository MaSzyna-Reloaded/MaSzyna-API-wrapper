# TODO

## Architecture rework (#184) - remaining stages

The vehicle becomes an object owned by a server, addressed by RID, with thin `*Node` proxies for
the editor. Each stage is one PR, titled `(#184) <area> - <what>`, and each leaves the game
runnable.

**Done: stages 1-3.** The prohibitions in `AGENTS.md`/`CODE_STYLE.md`; the four read side effects;
the coupler counters leaving the vehicle; the `power_source` collision; `TrackManager` and
`SpatialIndex` in C++; and the three servers - `BaseVehiclePhysicsServer` states what simulating a
vehicle means in RIDs, `MaszynaMoverPhysicsServer` is the only class that knows `TMoverParameters`
and owns every Mover, `RailVehicleServer` owns track placement, movement, switch crossing, the
neighbour scan, the transforms and the step.

**Stage 4 was designed wrong and has been withdrawn.** It put the vehicle's state behind a global
name registry with integer ids, per-class local indexes and a `switch` over declaration order -
a from-scratch reimplementation of Godot's own property system, written that way only because
`VehicleController` and `VehicleComponent` are `Node`s and a node is not a place for vehicle
state. The registry, the ids and `VehicleState` are gone. What replaces them:

* **State is a typed property whose getter reads the backend field.** The values already exist in
  `TMoverParameters`; copying them once per tick into a Dictionary, a state object or a component
  field is a second copy to keep in sync. `VehicleBrake::get_pipe_pressure()` returns
  `mover->PipePress` and stores nothing. A component keeps only what the Mover has not got: the
  brake pressure filter, the door interpolation, the wiper positions, a `_prev` for change
  detection.
* **Config goes the other way, deliberately.** The Mover's own configuration is a mess and the
  wrapper's properties and enums are better, so config stays the wrapper's - written at
  (re)configuration, read rarely. It is the one duplicate in this design, and it is intentional.
* **Dumps are lazy and separate.** `_fill_state_dictionary(Dictionary)` and
  `_fill_config_dictionary(Dictionary)` per component; `get_state()`/`get_config()` only call the
  overridden one; `RailVehicleServer::vehicle_dump_state(rid)` composes the vehicle's. A key the
  vehicle's variant has not got is simply not written, so `has()` keeps meaning what it meant.
  Nothing is built until somebody asks.

**Stage A is done:** the registry, the ids, `VehicleState` and `test_vehicle_state_properties.gd`
are deleted; all 21 components and the controller's own 32 keys answer through
`_fill_state_dictionary`; components announce themselves to the controller
(`register_component`) instead of being searched for; `roof_light_changed` reads
`VehicleLighting::get_roof_light_enabled()`, the first typed state property, instead of a registry
lookup.

**Stages B and C are done.** `_fill_state_dictionary`/`_fill_config_dictionary` with
`vehicle_dump_state(rid)`/`vehicle_dump_config(rid)` (the dump is composed once per physics step
and cached until the next one); the controller's common properties; and `VehicleComponent` is an
`Object` owned by the vehicle, not a node - `attach(controller)`/`detach()`/`process(delta)`
replace the notifications, `get_component(TYPE)` replaces walking the tree, and
`GenericVehicleComponentNode` is the modder's authoring point.

**The FIZ half of stage F is done.** A `.fiz` parses into a `VehicleModel` +
`VehicleComponentModel`s (typed C++ `Resource`s, modelled on `E3DModel`/`E3DSubModel`) rather than
into a packed node tree; `FizVehicleBuilder` is the factory with the `ResourceCache`, and
`FizVehiclePhysicsNode` only names a data dir and a filename and asks the factory for the model,
the way `MaterialManager` and `E3DModelManager` are asked. `VehicleModel::capture()` walks
`get_property_list()` for `PROPERTY_USAGE_STORAGE`, so a component's authored configuration
serialises without a line of per-component code.

**Remaining stages** (the full plan, with per-stage verification, is in the session plan file):

* ~~**B - dumps and the vehicle's common properties.** `_fill_config_dictionary` beside the state
  one; `vehicle_dump_config(rid)`; `velocity`, `speed`, `mass_total`, `total_distance`,
  `direction` as typed properties of `VehicleController` with `vehicle_velocity_get(rid)`
  forwarding to them; `Dictionary config` leaves the controller for the components that parse it.~~
* ~~**C - `VehicleComponent` stops being a `Node`.** An `Object` owned by the server: no
  `_notification`, no `_process`, the controller handed to it at creation instead of being found
  by walking `get_parent()`. `_do_process_mover` becomes `_process_state(delta)`. Every
  component's state becomes typed properties and the flat-dictionary prefixes are cut
  (`brake_pipe_pressure` -> `brakes.pipe_pressure`); the dump keys keep the prefix, because the
  dump is one flat Dictionary for the whole vehicle. `vehicle_component_create` /
  `vehicle_component_get(rid, TYPE)` / `generic_vehicle_component_find(rid, tag)`, with
  `vehicle_component_get` returning the typed object the way
  `PhysicsServer3D::body_get_direct_state(RID)` does. The interface/implementation split
  (`Vehicle<Domain>` / `Mover<Interface>`) is its own commit at the start. `GenericVehicleComponent`
  gets its dump for free from `get_property_list()` + `PROPERTY_USAGE_SCRIPT_VARIABLE`.~~
* ~~**D - `VehicleController` stops being a `Node`.**~~ Done. It is an `Object` the vehicle owns:
  `attach_to_system()` registers the vehicle and its commands before any component attaches (a
  command of a train the system does not know yet is refused), `initialize()` then starts the
  Mover, and `RailVehicleServer`'s `process_frame` tick drives it. `RailVehicle3D` adopts the
  vehicle's handle instead of creating a second one, binds to it in `_enter_tree()` and does not
  process until `vehicle_changed` says there is a vehicle. The controller's `dirty` flag is gone
  with it: configuration is written by the named `apply_configuration()` - the vehicle's own and
  every component's, in registration order - and `add_component()` applies and announces a
  component added to a vehicle that is already running.
* **E - proxy nodes.** `VehicleControllerNode` plus one `<Interface>Node` per component, each
  thin: `@export`s for the editor, forward to the server object, no logic.
* ~~**F - `DynamicRailVehicle3D` stops fabricating nodes.**~~ Done. The `rail_vehicle` cache
  holds a `VehicleStructure` - what the `.mmd` says a vehicle is built from: the exterior,
  low-poly and passengers model filenames, the resolved skin slots, the wiper prefix, `jointcabs`
  and the cab scene - and `build_from_structure()` assembles the nodes per vehicle instead of
  `PackedScene.pack()`/`instantiate()`. `read_structure()` is the expensive half and the only
  thing cached; `initialize_instance()` is what a vehicle gets for itself (sound pools, and the
  animation bindings, which are paths into its own submodel tree). Tag bumped to `structure-v18`;
  `FIZ_PARSER_FORMAT_VERSION` deliberately not touched, since no FIZ parser changed. The cab
  stays a `PackedScene`, because a cab genuinely is a tree of widgets.
  What is left of this area:
  * **The node's public API is the `.scn` `dynamic` line and nothing else.** `data_path` +
    `file_name` + `skin` locate the data, and the `.fiz`/`.mmd`/`.e3d` behind them say what the
    vehicle *is* - measured for the cabs: `cab1model`/`cab2model`, `cabXdefinition` and
    `jointcabs` are all MMD, so no cabin, sound, pantograph or light property belongs on the node.
    What stays exported is what says which *instance* this is and where it stands:
    `train_id`, `initial_velocity`, the occupant, `start_track_name`, `start_track_offset`,
    `start_direction`, and `head_display_material` as the project's own asset slot.
  * ~~`cabin_number:int` is neither a number nor a cab.~~ Done: `VehicleController.DriverType`
    (`DRIVER_NOBODY`/`DRIVER_HEAD`/`DRIVER_REAR`), named after the `drivertype` a `dynamic`
    declares, carried by the node, the physics node and the controller; the backend's own +1/-1/0
    stays behind `get_occupied_cab()`.
  * ~~`loadcount`/`loadtype` are parsed and thrown away.~~ Done: they reach the vehicle as
    `load_name`/`load_amount` and `TMoverParameters::AssignLoad()` takes both at once. The
    trailing `destination` of the same line is still dropped.
  * `_process` runs in every vehicle of the scenery forever to look at two dirty flags. The
    `_dirty`/`_process` pattern stays; the setter turns processing on and the tick turns it off.
* **The pantograph's power path is simulation, and it lives in a node.**
  `RailVehicle3D::_update_pantograph_power()` takes the vehicle's transform, works out each
  collector's contact point, asks `TractionPowerServer` which span is overhead, reads its voltage
  and writes it into `VehicleElectricEngine` - every frame, from a `Node3D` whose job is to draw
  the vehicle. Nothing there needs a node: `RailVehicleServer` already owns the placement and
  `vehicle_get_transform(rid)`, so the whole path belongs in its step, beside the movement and the
  neighbour scan, with the remembered span per pantograph kept there too. What stays in the node
  is what genuinely draws: `_apply_pantograph_animation()` on the arm submodels.
  The collector offsets (`pantograph_front_offset`/`pantograph_rear_offset`) are exported on the
  node today because the instancer reads them off the model; they are the vehicle's own geometry
  (the original keeps them in `TAnimPant::vPos`) and have to reach the vehicle for this to move.
  The same question applies, more weakly, to `_update_wipers()` and `_update_smoke()` - those
  consume state to drive submodels, which is drawing, but the wiper *positions* are simulation.
* **G - consumer migration, and the cabin goes through CabinSystem.** Cabin elements stop knowing
  about vehicles at all: they talk to `CabinSystem`, and it holds the vehicle **RID** and takes
  what it needs from the servers (`vehicle_component_get(rid, TYPE)` for live values,
  `vehicle_dump_state(rid)` for a whole-vehicle read). The write side already works this way -
  controls report manipulations through `CabinSystem.act()` and handlers translate them into
  vehicle commands - so this is the read side catching up.
  * `@export_node_path("VehicleController")` disappears from all 14 files that carry it rather
    than changing type. It is not an authored setting: `cabin_3d.gd::_propagate_train_controller`
    writes it into every widget at run time, so each one keeps a copy of what the cabin root
    already knows, and four of them re-resolve it to a node **every frame**
    (`base_cabin_tool_3d.gd`, which is the base of every cabin tool, plus the two cabin lights and
    `train_sound_3d.gd`). The six `demo/hud/` files never resolve it at all.
  * `cabin_state.gd` stops keying on `train_id` and reading `TrainSystem.get_train_state()`.
  * The sound system, the HUD and the 8 call sites in `RailVehicle3D` take the component once and
    read typed properties.
  * Afterwards nothing may call `vehicle_dump_state()` per frame - it is composed once per physics
    step, which is enough for a cab reading it from dozens of widgets, but it is still a whole
    Dictionary.
* **H - update phases.** An `UpdatePhase` enum ordering both passes, checked against
  `TMoverParameters::ComputeMovement`/`Update` and against the three ordering bugs on record
  (#57 line breaker, `Mred`, `roof_light_enabled`). `test_vehicle_doors.gd` has to exist first:
  `VehicleDoors` is one of the six components that really tick and has no test at all.
* **I - `train_id` and removing the shims.** `train_id` moves to `RailVehicle3D` as its only
  writer (two vehicles with an empty one collide today - `dynamic_rail_vehicle_3d.gd:45-49`);
  `TrainSystem` keeps `train_id -> RID`; `vehicle_get_mover()` and the borrowed Mover pointer go.

**A test must not clobber a global setting.** `test_fiz_train_controller` points
`UserSettings.save_maszyna_game_dir()` at its own fixture in `before_all` and restores it in
`after_all`. A crash mid-test (one happened on 2026-09-22) skips the restore, so the *game* then
starts with its game directory set to `user://gut/fiz_train_controller` and finds no scenery. The
fixture path should be passed to what is under test instead of being written into the user's
settings; the same pattern is in `test_dynamic_rail_vehicle_manager` and three more.

**Scenery teardown aborts when streaming is busy** - `FINDINGS.md`, 2026-09-22. The streaming
worker runs `e3d_model_manager.gd::load_model`, which is a full `ResourceLoader.load()`, so
renderer resources are created off the main thread while a teardown frees them. Reproduce by
clearing `user://cache/rail_vehicle` and `fiz` and running
`test_zzz_ep07_cabin_main_switch` - it aborts on roughly half the cold runs. The fix is a choice:
parse on the worker and build on the main thread, or drain the worker before freeing anything.

**`test_zzz_ep07_cabin_main_switch` is non-deterministic and the cause is not found.** Runs of one
build have given 5/5, 4/1 and a teardown core dump in `_free_owned_rids`. A clean build of the
commit before the engine work gave 2/3 with no crash, so it is unstable on both sides. One real
hazard on that path was fixed - the RID list was cleared only after the whole loop, so a budgeted
teardown that awaited a frame and then left the tree freed the same RIDs twice - and the crash grew
rarer but did not go away. Until the rest is found this test cannot gate anything.

**A third test red before this work**, alongside the two already recorded:
`test_zzz_ep07_main_switch_trip_diagnostic` fails four assertions - the vehicle does not accelerate
past 2 m/s across five controller notches and the Hasler never sees a speed. Verified at `76ebf3d`
with the engine work stashed, so it is not from the #184 rework. Same family as
`test_sm42_startup_sequence` was - and that one turned out to be a vehicle with nobody in the cab
(see `FINDINGS.md`, 2026-09-23), so check `cabin_number`/`CabActive` here before anything else.

**Tests that read the game directory** fail whenever it is not mounted, which is exactly what
`AGENTS.md` forbids them to depend on: `test_zzz_ep07_cabin_main_switch` loads
`scenery/td.scn` through `user://gut/fiz_train_controller`. Needs a fixture scenery instead.

**Coverage gap, to close before stage C:** nothing tests `GenericVehicleComponent` at all - not one
of the 95 test scripts instantiates one, and its only proof is the two example scenes
(`demo/examples/custom_train_part.tscn`, `custom_powered_train_part.tscn`). Its GDScript API was
renamed in stage B (`_process_train_part` -> `_process_component`, `_get_train_part_state` ->
`_get_component_state`, `_get_train_part_config` -> `_get_component_config`, `get_train_state` ->
`get_vehicle_state`, `get_train_controller_node` -> `get_controller`) and verified only by running
the example scene and seeing no `Invalid call` error. The modder-facing gateway deserves a test of
its own before it is moved onto the server.

The 30 fps on `td.scn` that this branch was split over was **not** a regression: the discrete GPU
had not woken from powersave and the simulator was running on the integrated RX 780M. Recorded in
`FINDINGS.md`; nothing here is outstanding because of it.

Traps that apply to every stage: bump the cache tag in the same commit as the code whose output is
cached (`FIZ_PARSER_FORMAT_VERSION`, `MaterialManager.CACHE_VERSION`, `E3DModel.FORMAT_VERSION`,
`structure-vN`); run `godot-double --headless --import` before believing an "Identifier not
declared" after adding a class; never pass a bare `[]`/`{}` to a typed collection parameter; add a
`doc_classes/<Class>.xml` for every registered C++ class.

## Cabins

* `VirtualCabin` for cabs without a hi-fi model (MMD `cabNmodel: none` or missing, e.g. su46
  `cab0definition:`) - built only when the cab exists, purely for input actions and command
  translation (no geometry, no MMD instrument widgets). The original keeps such a cab enterable
  and shows the low-poly interior instead (`Train.cpp:8692`, `DynObj.cpp:1214`). Hook point:
  `DynamicTrainCabin` currently builds an empty cabin with `has_cab_model = false`.
* Cabin keyboard input per control, not per widget - today every `CabinButton`/`CabinSwitch`/
  `CabinKnob` handles its own `action*` in `_input`/`_process`, so a label repeated in one cab
  (EP07 cab0 has two `cablight_sw:`) got the key once per widget and toggled itself back. The
  original maps a key to one command changing one state (`Cabine[].bLight`, `Train.cpp:10237`),
  the gauges only display it. Move key handling to `CabinSystem`/`LegacyCabinLogicDelegate`
  (once per `control_id`), widgets only display; then drop the workaround in
  `MmdCabinInstancer.build_into()` clearing `action*` on repeated labels.
* Diesel-electric shunt mode on the second controller - `second_controller_increase/decrease`
  only port the regular mode (`IncScndCtrl`/`DecScndCtrl`). With `ShuntModeAllow` and `ShuntMode`
  the original moves the shunt power `AnPos` by 0.025 per step instead, clamped to 0..1
  (`Train.cpp:1190-1197`, `1351-1357`); the `shuntmodepower:` gauge (`Train.cpp:10542`) is
  unmapped too.
* Pantograph auxiliary compressor keys without a cab switch - Shift+V/Ctrl+V reach the commands
  only through the `pantcompressor_sw`/`pantcompressorvalve_sw` widgets. The original also allows
  them in the machine room (cab 0) of the pantograph unit when the MMD has no such switch
  (`Train.cpp:2872`, `2915`).
* `CabinSwitch` has no `mesh_rotation_offset`/`mesh_position_offset`, so the MMD offset of a
  switch is dropped (`MMD_ANIMATION_UNSUPPORTED`), e.g. SM42 `dirkey: kier rot -0.09 0.01`. The
  original renders `value * scale + offset` (`Gauge.cpp:456`); `CabinButton` already does.
* Spring brake, what is left after the parity pass (2026-09-24): `springbrakerelease`
  (`Train.cpp:6874`, the emergency release rod) and the `springbrakepress:` gauge
  (`Train.cpp:12221`) have no cab control and no key, the game's `eu07_input-keyboard.ini` binds
  the release to `none` as well.
* Intermittent, not reproduced (2026-09-24): after the first entry into a cab, the releaser
  (num4, `releaser_bt`) and the drive shortcut (num6, `brake_level_drive`) sometimes do nothing
  until the brake handle is moved once (num3/num9). A headless probe entering every cab of
  `td.scn` in turn with real key events (EP07-424, 111aw, two bdhpumn) worked every time: handle
  -2 -> 0, releaser active, both controls registered in `CabinSystem`. The one difference found:
  the handle (`CabinKnob`) polls `Input` every frame, while `CabinButton`/`CabinCommand` react
  only to events in `_input` - so suspect the event not reaching the cab, or reaching it with the
  wrong `occupied_cab()`. Next time it happens, check the log for
  `Unknown cabin control: ... (cab N)` - present means a wrong cab, absent means a lost event.

## Sounds

* MMD sound offsets of the existing (non-running) sounds are used raw - the `SfxPlayer3D`s are
  not turned by 180 degrees like the exterior model (`maszyna_rail_vehicle_3d_instancer.gd:97`),
  so horns, compressor, brake sounds etc. with an `offset:` sit mirrored (x and z). The running
  sounds already convert their positions (`MmdSoundBankInstancer._build_running_events()`).
* Running sounds still missing: `tractionacmotor:`/`inverter:`/`motorblower:` (inverter vehicles,
  `DynObj.cpp:5745-5800`, `8012-8080`), `wheelflat:` (`DynObj.cpp:4722`), `derail:`
  (`DynObj.cpp:5910`), `transmission:` (`DynObj.cpp:5822`), cab `huntingnoise:`
  (`Train.cpp:8284`).
* Wheel clatter bump - each clatter click nudges the vehicle vertically (`AccVert`,
  `DynObj.cpp:3533`), cab shake only; not ported.
* Open cab window (`Global.CabWindowOpen`) - the original then plays the outer noise of the own
  consist and stops the cab running noise (`DynObj.cpp:4638`, `Train.cpp:8274`); the wrapper has
  no cab window state yet.
* Random pitch variation per sound source (`pitchvariation:`, default 0.975-1.025,
  `sound.cpp:375`) is parsed but not applied to any sound.
* The gnd-sfx playback tick is GDScript on a worker thread (12 ms per frame for 200 players in
  the headless benchmark). If that becomes the limit, the runtime is a candidate for a C++
  singleton next to `E3DRenderingServer`, with the nodes staying proxies as they are now.
* `SfxGeneratorPlayback.update()` now runs on the sfx worker thread (single producer into the
  `AudioStreamGeneratorPlayback` ring buffer). No wrapper code uses generator clips; revisit if
  an implementation ever needs the scene tree.

## Vehicles

* **Traction: `hvParallel` (bieznia wspolna) is not ported.** A `traction` node may name a
  parallel span (`parallel <name>`); `maszyna_node_traction_importer.gd` reads it into
  `MaszynaTractionData.parallel` and nothing carries it to `TractionPowerServer`. The original
  puts such spans in a ring and, while the pantograph is on one of them, always searches the area
  instead of following the chain, because the wire actually overhead may be a sibling it cannot
  reach along `hvNext` (Traction.cpp:838-852, DynObj.cpp:8753). `zwierzyniec_tlk` declares none,
  which is why the junction fix works there; a scenery that declares them will pick the wrong
  span. `iLast` - the original forcing the same search on the last and second-to-last span of a
  section - is not ported either.
* **A vehicle's load reaches the backend, but has no visual side.** `loadcount`/`loadtype` of a
  `dynamic` now become `load_name`/`load_amount` and are handed to
  `TMoverParameters::AssignLoad()`, which is also how a scenery starts a locomotive with raised
  pantographs (`pantstate`, `Mover.cpp:7647`). What is still missing is the cargo a load is drawn
  as - the MMD's own `loads:` block, which is where the passenger model already comes from - and
  the trailing `destination` of the `dynamic` line, which is still dropped.
* `DynamicRailVehicle3D` builds its `RailVehicle3D` itself (`_rebuild()` ->
  `DynamicRailVehicle3DManager.load()` in its own `_process`), so vehicles are instanced a frame
  after the scenery is attached (`SceneryInstancer._wait_for_vehicles()` waits for them). The
  building belongs in a separate `DynamicRailVehicle3DFactory`.
* A distant vehicle's low-poly interior (`OPTIMIZED` instancer, `RailVehicle3D::_update_model_detail()`)
  has no materials to dim, so it keeps the emission baked into the model regardless of
  `roof_light_enabled`; it is dimmed again once the vehicle is back within
  `maszyna/rendering/vehicle_detail_distance`.
* The vehicle template cache tag (`structure-vN`, `dynamic_rail_vehicle_3d_manager.gd`) is bumped
  by hand; a change to `MaszynaRailVehicle3DInstancer` without a bump keeps serving the old
  structure, and the cache survives a checkout (it made a `git bisect` report "bad" everywhere).

## Rendering

### A light's submodels have two managers

`E3DRenderingServer` owns which of a light's `_on`/`_off` submodels is shown - it resolves
`lights_state` out of the declared modes, the manual override and the time of day, and the backends
apply it. The cab's MMD widgets (`CabinIndicator3D`, `CabinSpotLight3D`) switch the very same
submodels by writing `Node3D.visible` on them directly. That is the same exclusive state held by
two owners, and it only looks correct because nothing pushes `lights_state` at a cab model after it
is built (see `FINDINGS.md`, 2026-09-23, where a per-frame push made them fight). It also means the
widgets do nothing whatsoever under the OPTIMIZED instancer, which has no nodes to write to. The
widgets should ask the model to switch the light (`lights_state`) instead of poking its nodes.

### Smoke emitters

* The vertical decay of a particle is not ported (`particles.cpp:365-380`): the original slows a
  particle's rise by the air temperature and, for a vehicle, by the overcast and the vehicle's own
  speed, so a plume flattens out instead of rising forever. `Global.AirTemperature` is a `#define`
  in the vendored Mover and cannot be fed from outside (see the air temperature entry under
  "Scenery loading"). The wind drift itself is ported - `E3DRenderingServer::set_wind()` turns it
  into the emitter's particle gravity, `0.1 * wind` being what the original's
  `0.1 * age * wind` per step integrates to.
* `MaszynaEnvironmentNode.wind_direction` is a compass bearing in degrees, so the wind is always
  horizontal. `MaszynaSkyEnvironment.get_wind_direction()` already returns a `Vector3` and
  `E3DRenderingServer::set_wind()` takes strength and direction separately, so a vertical
  component needs no API change - only a property that can express one.
* The culling box of an emitter follows the emitter, not the plume
  (`E3DRenderingServer::_apply_smoke_placement()`), because a `RenderingServer` particle system is
  culled as a whole. A fast vehicle leaves its trail far outside that box, so the whole plume
  disappears when the emitter itself goes off screen. The original culls per source too
  (`opengl33particles.cpp:38`), but against the box its own particles span
  (`smoke_source::update()` grows it, `particles.cpp:284-291`).
* `min_inclination` of a template is dropped: `ParticleProcessMaterial` has one `spread` around
  the emission direction and no inner cone. Only `smokesource_st45` declares a non-zero one (10
  degrees) out of the twelve templates.
* A particle's lifetime is per emitter in Godot and per particle in the original, where it is the
  particle's own random initial opacity divided by the fade step (`particles.cpp:132`). The
  wrapper takes the longest of them and fades every particle linearly over it, so a particle that
  started faint stays faintly visible longer than it should.
* The "Modern" generator mode's flipbook (`demo/vfx/smoke_atlas.png`) is generated procedurally by
  `scripts/make_smoke_atlas.py` - a fBm puff that expands, erodes and thins over sixteen frames.
  It is a stand-in for real authored or simulated smoke; replacing it needs no code, only the
  `maszyna/rendering/smoke_atlas` and `smoke_atlas_frames` settings. The flipbook is also the same
  sixteen frames for every particle, so a dense plume repeats visibly - the usual fix is several
  variants picked per particle, which needs a second atlas axis or a random `anim_offset`.
* Smoke is lit by Godot's own sun instead of the flat daylight modulation the original applies
  (`opengl33particles.cpp:60-66`), and `E3DRenderingServer`'s `light_level` is not used for it.
* The "cold engine smokes grey" rule of the original never ran - `particles.cpp:176` compares
  where it meant to assign - and is not ported. It needs `dizel_heat.Ts`, which no `TrainPart`
  exposes yet.
* Emitters of a vehicle are not switched off when the vehicle is culled, only when its
  `E3DModelInstance` is hidden; the original stops spawning beyond
  `2 * BaseDrawRange * fDistanceFactor` for every source (`particles.cpp:452`), while the wrapper
  streams only the scenery ones by `maszyna/rendering/smoke_distance`.

* Normal maps are applied at `normal_scale` 1.0 like the original (`mat_normalmap.frag:46-48`);
  the `-5.0` that `material_factory.gd` used to set made bumps five times stronger and reversed.
  Not checked against the original yet: whether Godot's generated tangents match the original's
  `f_tbn`, i.e. whether the bumps now face the right way on models and on terrain.
* Overexposure in the demo scenery is not measured yet. Candidates, one at a time:
  `tonemap_mode` of `MaszynaEnvironmentNode` in `demo_scenery_loading.tscn`,
  `directional_shadow/soft_shadow_filter_quality` 3 -> 1 and the removed
  `directional_shadow/size=8192` (`042b392`), `fog_enabled = false` and `cloudiness`
  0.35 -> 0.21 (`ab75bbe`).

## Scenery loading

* Air temperature (`config scenario.weather.temperature` -> `MaszynaEnvironmentNode.temperature`)
  is consumed by nothing. The Mover uses it only in the diesel engine heat model
  (`dizel_heat.Te`, original `Mover.cpp:8109`), and the vendored Mover has it as
  `#define Global_AirTemperature 15.f`, assigned on every step - it cannot be fed from outside
  without touching `src/maszyna/`. Adhesion does not depend on it (`Adhesive(RunningTrack.friction)`).
* Other scenery `config` entries are dropped (`scenario.time.override/offset/current` shift the
  timetables, `Globals.cpp:356-385`).
* Include cache / instancing - e.g. `skp/skp_trawa.scm` includes `grass.inc` 24078 times, each
  one parsed again and baked into world-space triangle chunks. Idea: the include importer
  classifies each included file in the context (`path => mode, placement params`): `instanced`
  (only `origin`/`rotate` + `triangles`, no nested includes - key = path + hash of the non-placement
  params, per-occurrence `Transform3D`, rendered as MultiMesh per chunk/texture/range) or `full`
  (whole `.scm` piece - key = path + hash of all params, reusable across sceneries). Results must be
  cached in local space (importers currently bake context origin/rotate into the data); invalidate
  by the dependency list like the compiled scenery cache.
* Subscene cache (`SceneryInstancer.parse_subscene_task()`) is used only by queued parsing -
  in-place `SceneryInstancer.parse_file()` (no queue) parses every include again.
* Parse progress counts includes inside subscenes loaded from cache (`_count_includes()`), which
  are never run as tasks - the parse bar jumps at the end when subscenes come from cache.
* Scenery streaming (`SceneryStreamingServer`) keeps the six `TrackRenderingServer` instances and
  the two `TractionRenderingServer` instances of every piece allocated and only drops their
  meshes; only the meshes are rebuilt when a piece comes back into range. Freeing the instances
  too would save the per-frame cost of ~16k empty instances in a scenery like `baltyk`.
* Scenery streaming places a track in the chunk of its first curve point, so a track longer than
  a chunk streams in by its start, not by its nearest point.
* Scenery streaming has no `preload` for tracks and traction: their meshes are still built on the
  main thread within the per-frame budget. Building them on the worker thread (like the E3D models
  are loaded) would shorten the fill-in after a load.
* The loading screen does not wait for the first streaming pass, so the world still fills in after
  a load (seconds, at the catch-up budget). Waiting for `get_statistics()["pending_builds"]` to
  reach 0 before the loading screen fades would move that behind the spinner.
* Filling a scenery in is bounded by the main thread: only E3DRenderingServer has a `preload`, so
  track, traction and chunk meshes are all built inside the per-frame budget. With ~5 000 pieces
  in range at a 3 000 m draw distance that is thousands of builds after every load.
* Streaming is per piece, which is the wrong granularity: a 1 km chunk should be baked into one
  unit (MultiMesh per mesh+material for models, merged meshes for triangles, ready track meshes),
  cached on disk and loaded by the worker - build would then be a handful of `RenderingServer`
  instances instead of thousands, and the cache would make dropping a chunk's geometry from RAM
  possible. Switch blades (`primary_blade_mesh_instance`/`secondary_blade_mesh_instance`) move, so
  they stay outside the baked geometry or need their own access to it.
* Nothing gives geometry back: `SceneryChunkRenderingServer.ChunkState.mesh` holds every merged
  terrain mesh for the whole session and `E3DRenderingServer`'s model cache never evicts, so
  clearing a piece frees its `RenderingServer` instance but not its mesh. Needs the per-chunk disk
  cache above to be fixable.
* Scenery loaded in the editor is streamed around the camera of 3D viewport 0 only
  (`addons/libmaszyna/editor/scenery_streaming/`); switching to another viewport does not follow.
* Switch state changes are not visualised (broken for several commits, unrelated to streaming) and
  the trackbed of switches renders incorrectly.
* `maszyna_node_track_importer.gd` still drops every track type but `switch`/`normal`, so `road`
  (~16 700 nodes per data set), `river` (~900), `cross` (72), `turn` and `table` never appear in a
  scenery. The node is now discarded cleanly through `endtrack` instead of desyncing the parser,
  but nothing is built. `road`/`river` need a flat surface path with no rail profile
  (`Track.cpp:1554` onwards); `cross` is a road intersection with four endpoints and no common
  point, which `TrackManager` has an enum value for but no topology or geometry support.
* The lit submodel of a lamp keeps the colour its texture carries (sodium orange) while the light
  it casts is tinted towards white by `maszyna/rendering/scenery_light_tint`, so the glowing head
  and its pool do not match. Tinting the emission too means a material variant for `light_on*`
  submodels: `E3DMaterialResolver` memoises one material per name and shares it across thousands
  of placements, and in `elektryczne/latarnial_betdziur` the bulb and the lamp housing use the
  same `elektryczne/oprawa` material - so it needs a flag in the resolver key, as `force_alpha`
  already has, not a tint on the shared material.
* A lamp still shadows its own light: in economy mode the single light in the middle throws the
  arms and the pole across the pool as long dark spokes, and
  `light_set_shadow_caster_mask(~SCENERY_LIGHT_OWNER_LAYER)` does **not** remove them - checked in
  game on 2026-09-21. Either Godot's clustered renderer ignores that mask for spot and omni lights
  (it honours it for directional), or the layer bit is not reaching the instances; measure which
  before changing anything, with a scratchpad project that puts one box on a second layer under a
  SpotLight3D and reads the rendered pixels. Fallbacks if the mask is a dead end:
  `instance_geometry_set_cast_shadows_setting(..., OFF)` on the light-owning model (which also
  loses its shadow from the sun) or no shadows in economy mode, where the spokes are an artefact
  of the merge - with one light per arm the neighbours filled each other's shadows in.
* An economy-mode merged light takes `energy` as the maximum of the lights it replaces, not their
  sum, so a five-armed lamp is as bright as one arm; `maszyna/rendering/scenery_light_energy`
  carries the difference.
* Scenery light brightness is calibrated by eye so far, through
  `maszyna/rendering/scenery_light_energy`, `scenery_light_tint` and
  `scenery_light_volumetric_fog_energy`. The tint exists because a lamp colour used raw
  (`(1.0, 0.66, 0.18)` for sodium) throws away most of the light's luminance; there is no
  counterpart for it in the original, which never lit the scene with these lamps at all.
* `elektryczne/latarnial_betdziur` registers a second light named `zarowka` (the E3D parser pairs
  `zarowka_on`/`zarowka_off` by the `_on`/`_off` suffix rule, `e3d_parser.cpp:592`). A scenery
  node's `lights` list only ever reaches light `00`, so nothing declares a mode for it and the
  bulb inside the lamp housing stays on its "off" submodel. The original binds lights by the
  `Light_On00..07` name alone (`AnimModel.cpp:303`) and has no such second light - check whether
  the suffix rule should apply to scenery models at all.
* `ls_Blink` (`E3DRenderingServer::LIGHT_MODE_BLINK`) follows `ls_Dark` instead of blinking, and
  the smooth on/off transition of `m_lightopacities` (`AnimModel.cpp:500-548`) is not ported -
  both need a per-frame timer, while the time of day is pushed once a second. `lights 2` is used
  15 times in the whole data set and `notransition` never, so neither is worth a timer yet.
* `Overcast` is folded into the light level by `MaszynaSkyEnvironment.get_light_level()` rather
  than subtracted at the threshold as the original does (`AnimModel.cpp:598`).
* Scenery models have no nodes, so they can't be picked/selected in the editor and don't follow
  the `MaszynaIncludeNode` transform/visibility (world-space, like tracks and traction).
* An `include` with no filename shows up while parsing the real data dir
  (`maszyna_include_importer.gd` now reports it with the parser offset and skips it, instead of
  trying to open the scenery directory). The source is unknown - no asset declares a
  parameterised include path, so it is either a truncated file or a tokenizer misread.

* `brake_release_hiss` (the `unbrake` label) is the one pneumatic brake event the brake factory
  does not build - it still goes through `TrainSoundSystem._update_triggers()` with an
  `MmdSoundEventBuilder` event, which is fed neither `gain` nor the `listener_inside` correction
  the other hiss events now carry. It is therefore louder in the cab, relative to them.
* The brake volume/unit-size factors are no longer Project Settings at all - they are
  `TrainSoundSystem`'s own `VOLUME_FACTOR`/`EXTERIOR_VOLUME_FACTOR`/`CABIN_UNIT_SIZE_FACTOR`/
  `EXTERIOR_UNIT_SIZE_FACTOR` constants, carrying what used to be the registered defaults
  (2.0/1.0/2.0/1.0). The demo had been running with a `project.godot` override of 1.0 for the
  first and third, so those two constants have not been verified by ear at 2.0.

## Tests

* Remove simulator game data from tests - CI has no game dir. Tests loading real sceneries or
  vehicles (`td.scn`, `demo_scenery_loading.tscn`, `dynamic/pkp/...`): `test_zzz_ep07_*`
  (cab_change, cabin_main_switch, main_switch_trip_diagnostic, orientation_regression,
  pantograph_power_smoke, running_sounds), `test_zzz_scenery_scene_smoke.gd` (instantiates
  `demo_scenery_loading.tscn` - a demo scene has no place in tests),
  `test_zzz_sm42_exterior_model_rotation_regression.gd`, `test_zzz_su46_exterior_lights.gd`,
  `test_zzz_su46_machine_room.gd`, `test_mmd_cabin_instancer.gd` (su45_v2),
  `test_rail_vehicle_rain_exclusion.gd` (sm42_v1). Replace them with fixtures under
  `demo/tests/fixtures/`: a cut `.scn` with just the track piece and trainset where the problem
  shows, and fabricated vehicles (`RailVehicle3D`, a cabin with only the controls under test,
  `TrainController` with a trimmed `.fiz`/`.mmd`, no e3d) - copied and cut from what the data-dir
  scenery parses into.
* `demo/tests/fixtures/test_vehicle.fiz` no longer imports - `godot-double --headless --path demo
  --import` prints `Error importing 'res://tests/fixtures/test_vehicle.fiz'` and rewrites its
  `.import` with `valid=false`. `FizImportPlugin._get_resource_type()` returns `Resource` since
  `b0affd6` while the committed `.import` still says `PackedScene`, so either
  `FizVehicleBuilder.build_model_at()` returns null on the fixture or the save fails. Everything
  resting on that fixture is dead until it is fixed.
* Tests switch the game dir with `UserSettings.save_maszyna_game_dir()`, which writes the user's
  `settings.cfg` (a failed/killed test leaves it pointing at a `user://gut/...` fixture dir):
  `test_dynamic_rail_vehicle_manager.gd`, `test_e3d_lights_state.gd`,
  `test_fiz_train_controller.gd`, `test_maszyna_node_dynamic_importer_direction.gd`,
  `test_material_manager_variants.gd`, `test_nodebank_library_builder.gd` and the game-data tests
  above. Needs a non-persistent game dir override for tests.

## Physics performance

* The frame-rate drop with a consist in a scenery is **the scenery's dynamic lights**, reported by
  the operator - not the vehicle step, which is where this section spent its measurements. The
  lights became real spot/omni RIDs streamed per instance with `FINDINGS.md`, 2026-09-21; nothing
  bounds how many of them are lit at once. Measure the count before changing anything.


* Those measurements were taken on a `make compile-debug` build, where the vendored `Mover.cpp` is
  compiled at `-O0`. Rebuilding the same code with optimizations (`make compile-profiling`) took
  `baltyk_skm1` from 31 to 44 fps - more than every code change of that session put together. Any
  comparison against the original, which is a release build, has to be made this way.
* Measured on `baltyk_skm1.scn` (376 vehicles), against the original running the same scenario on
  the same machine: the original spends **1.8 ms of CPU per frame** on everything - AI drivers,
  physics of every consist, events - while our frame is ~36 ms. The physics code is the same
  vendored `Mover.cpp` on both sides, so the difference is not the simulation but how it is
  reached: ~4 000 GDScript<->C++ crossings per frame in the step loop, and ~23 000 dictionary
  operations per frame building the vehicle state (31 keys per vehicle, each a String built from a
  literal, plus the same again for every TrainPart and once more in `TrainSystem`). This is the
  concrete evidence for the architecture rework of #184 - `TrainController`/`TrainPart` carry their
  state through `Dictionary` and node signals instead of being data a loop walks over. #57 (state
  proxy) removes the copying but keeps a crossing per read, which is why #184 calls it the wrong
  direction.
* What the frame looked like after this session's fixes (editor profiler, Time: Self):
  `Script Functions` 24.8 ms, of which `_process` self 15.2 ms is the Mover calls themselves;
  everything else in GDScript is below 1.2 ms per entry. Scenery streaming, audio, Godot physics,
  collision pairs, SDFGI and the renderer were each ruled out by measurement (renderer: 3.9 ms CPU
  / 13.4 ms GPU).
* `RailVehiclePhysicsServer` step in C++ - the per-vehicle GDScript loop (track sampling,
  neighbour scan, movement) is ~6.5 ms per physics tick for 149 vehicles on
  `zwierzyniec_ed72.scn`; the Mover math itself is cheap. Needs a C++ snapshot of the track data.
* Multi-core physics after the C++ step - keep the phases of `vehicle_table::update()`
  (`DynObj.cpp:8181`: locations + neighbours, then per iteration forces of all, movement of all),
  run them per island (a consist coupled by couplers plus vehicles within collision range -
  `CouplerForce()`/`CollisionDetect()` write the neighbour's `V`/`AccS`, so single vehicles are
  not independent) on `WorkerThreadPool::add_group_task` with a barrier between phases; no Godot
  calls on the workers - state, signals and positions gathered on the main thread per tick.
* Braked standing vehicles never switch their physics off: at `V == 0` `Sign(0) == 1`, so
  `FTotal = FTrain - FStand` keeps `AccS` non-zero (`Mover.cpp:4603`, `ComputeTotalForce()`
  activity test) - same in the original, only unbraked vehicles sleep.
* Cab activation side effect not ported: `OnCommand_cabactivationenable/disable` also call
  `SetLights()` when `LightsPosNo > 0` (`Train.cpp:2440`, `2463`).
* A vehicle with switched off physics keeps its last `TrainController.state` (fetched only for
  active vehicles, like the original skips `Update()`).
* Material shaders of the original left unmapped: `clouds`, `stars`, `invalid` (engine internals,
  `textures/sky/stratus.mat`, `stars.mat`, `invalid.mat`) and `normalmap_phys`
  (`textures/pkp/wskazniki/w29.mat`; the shader file does not exist in the game dir either).
* Wiper sounds (`wiperfrompark:`, `wipertopark:` of the MMD, `DynObj.cpp:4082-4099`) are not
  played. The direction the wiper arms swing (`RailVehicle3D::_update_wipers()`, rotation about Y
  as `TDynamicObject::UpdateWiper()`) was not checked against the original in game.
* The droplets of `rain_windscreen.gdshader` ignore vehicle speed and wind (a TODO in the
  original shader as well).
* `*_specgloss` material shaders other than `parallax_specgloss`/`water_specgloss` do not sample
  the specgloss texture (`normalmap_`, `default_`, `reflmap_`, `detail_normalmap_`,
  `shadowlessnormalmap_`, `sunlessnormalmap_`): approximated by their plain counterpart.
* `rain_windscreen.gdshader` reads the screen texture (droplet lenses, water film): transparent
  things behind the glass - the rain particles first of all - are not in it and fade out where
  the film covers the glass. The film, the large droplets and the rivulets are this wrapper's own,
  tuned by eye in a test scene (`heavy_rain_start`, `film_*`, `rivulet_*`, `refraction_strength`
  of the material type); "down" follows the gravity of the original droplets (smaller v) and was
  not checked on a real cab glass.
* E186 (`dynamic/pkp/e186_v2`) cab labels still outside of `MmdSemanticCatalog`:
  `pantselected_sw:` with the `PantsPreset` selection (`OnCommand_pantographtoggleselected`,
  `pantographselectnext/previous`, `Train.cpp:3405-3549`), `pantfrontoff_sw:`, `pantrearoff_sw:`,
  `lights_sw:` (`lightspresetactivatenext/previous`; the `light_position` state is `LightsPosNo`,
  the count, not the position), `dimheadlights_sw:`, `radiostop_sw:`, `radiovolumenext/prev_sw:`,
  `universalbrake1_bt:`, `doorpermitpreset_sw:`, `distancecounter_sw:`, `universal0-8:`, the gauges
  `brakepressb:`, `limpipepress:`, `clock:`, the lamps `i-mainpipelock:`, `i-tempomat:`,
  `i-malfunction:` and the `pyscreen:` displays. The model has four pantographs
  (`CollectorsNo=4`, `PhysicalLayout=3`), the wrapper animates the first two.
* `LegacyCabinBattery`, `LegacyCabinCabActivation`, `LegacyCabinManualBrake` and
  `LegacyCabinWipers` only register what `LegacyCabinUnmodelledControls` would register from the
  catalog anyway (their keys already go through it) - they can be folded into it.
* The placeholder of a tile whose side view is still rendering guesses its width from the
  silhouette's own shape (`TileGrid.PLACEHOLDER_STRETCH`), because nothing the selector reads
  knows how long a vehicle is: `MaszynaSceneryInfo.Vehicle` carries only the train id, the data
  path, the skin and the file name, and the FIZ `Dim=` is parsed nowhere. With the length the tile
  could come up at its final width and stop jumping when the profile arrives.

### Left behind by the VehiclePhysicsNode commit

* **`test_dynamic_rail_vehicle_manager`** is red, and what it reports was measured rather than
  guessed at: `registration.controller` is a plain `null`, so the sound bank never captured a
  controller at all, while `vehicle.get_controller()` returns a valid one at assert time. It was
  a **freed** object before `VehicleController::release()` preserved the vehicle's identity
  across a rebuild - that part is fixed. What remains is that the bank registers against a
  vehicle that has no controller yet and only the 4 Hz sweep repairs it, later than the three
  idle frames the test waits. Connecting the repair to the vehicle's `ready` does not help
  (the vehicle is already ready by then) and connecting it to `tree_entered` fires too early,
  so the bank is most likely registered against the template rather than the instance - which
  is exactly the packing-and-instancing that stage F removes. Fix it there, not in the sound
  system.
* **`test_zzz_ep07_main_switch_trip_diagnostic`** is red. Both follow the vehicle-building path that stage F is about to replace, so they are
  rewritten there rather than patched now - but the second one describes a vehicle that will not
  accelerate, which is exactly what `test_sm42_startup_sequence` turned out to be: an unoccupied
  cab, so no physics.
* **The `.fiz` path has not been run in the game**, only in tests. Nothing has driven a vehicle
  end to end since the components stopped being nodes.

### RailVehicle3D runs before it has a vehicle

Half done. The node now binds its `VehiclePhysicsNode` in `_enter_tree()` and does not process
until `vehicle_changed` says there is a vehicle, so nothing is placed against a vehicle that is
not there. What is left: it still creates a handle of its own in `_enter_tree()` and adopts the
vehicle's later, rather than never creating one - a node that draws a vehicle should not own a
handle at all.

### Rail concepts living in interfaces named "Vehicle"

`VehicleComponent`/`VehicleController` are generic on purpose - the same servers are meant to
carry road vehicles. Several component interfaces below them are not generic at all, and their
names say otherwise. Counted by rail-specific vocabulary in each header:

| Interface | rail terms | what they are |
| --- | --- | --- |
| `VehicleBrake` | 43 | the brake pipe, the W/Lu/L, W/Lu/VI, W/Lu/XR and K valves, FV4a handles |
| `VehicleElectricEngine` | 34 | pantographs, traction circuit |
| `VehicleBuffCoupl` | 13 | buffers, screw coupler |
| `VehicleWheels` | 12 | bogies, pivot spacing, `get_bogie_transform()`, minimum curve radius |
| `VehicleSecuritySystem` | 2 | SHP, vigilance device |
| `VehicleSpringBrake`, `VehicleElectroPneumaticDynamicBrake` | 1-3 | rail brakes |

Genuinely generic and correctly named: `VehicleWipers`, `VehicleUniversalController`,
`VehicleSpeedControl`, `VehicleHorns`, `VehicleDoors`, `VehicleHeating`, `VehicleLighting`,
`VehicleLoad`.

A car has wheels and no bogies, brakes and no brake pipe. Two ways out - rename the rail ones to
`Train*` (with their `Mover*` implementations), or keep the generic name and put the rail parts
in a subclass. The second only pays once something road-side actually shares the generic half,
and nothing does today. Measured cost of the rename, should it be taken: `VehicleWheels` 13 files
/ 50 mentions, `VehicleBrake` 24 files / 283 mentions.

What already holds and must stay either way: these interfaces name no backend at all, and the
`Mover*` implementation is the only class touching `TMoverParameters`.

### The controller was never split into interface and implementation

The convention the components follow - `Vehicle<Domain>` names no backend, `Mover<Interface>` is
the only class that touches `TMoverParameters` - was never applied to `VehicleController`. It
holds `TMoverParameters *mover`, `initialize_mover()`, `initialize_mover_state()` and
`get_mover()` in the class that is supposed to be the interface. The method names say the backend
out loud, which is exactly what the rule forbids, and renaming them alone would be churn undone
by the split.

The controller does not even need the pointer: it already holds `RID physics_rid`, the vehicle's
handle in `MaszynaMoverPhysicsServer`. The raw `TMoverParameters *` beside it is a cache, kept
because every component reaches for it every frame - which is how a borrowed pointer to a
structure another layer owns ended up crossing the boundary.

What the split looks like, mirroring the components: `VehicleController` keeps the vehicle's
state, configuration and operations and names no backend; a `MoverVehicleController` owns the
Mover handle, creates it, configures it and ticks it. `get_mover()` disappears from the interface,
which is what today's components reach through - so this and the entry below are one piece of
work, not two.

**Measured scale, so nobody starts this thinking it is a field move:** `get_mover()` has **316
call sites across 27 files**, **32 component methods take `TMoverParameters *` in their
signature**, and `VehicleController` itself dereferences `mover->` **103 times**.

### A non-Mover component still cannot exist - one layer left

Could the vehicle take a `CarBrakes` today? The component model itself is ready:
`COMPONENT_BRAKES` names a kind rather than a class, `add_component()` takes any
`VehicleComponent *`, `VehicleComponentModel.implementation` is a class name ClassDB
instantiates, and the component base and every interface now name no backend at all - its tick
and configuration are `_do_process_component(delta)` and `_apply_configuration()`, and Mover
access lives in `src/mover/MoverBackend.hpp`.

What is left is the controller: **`VehicleController::initialize_mover()` always creates a
Mover** and `ERR_FAIL_NULL`s on it, so there is no vehicle without one. That is the same piece of
work as the split above - `initialize()` has to ask a backend factory for the vehicle's
simulation instead of naming `MaszynaMoverPhysicsServer`.

So "the same servers carry road vehicles" is a statement of intent, not a fact. The shape that
would make it one: the component's tick and configuration take no backend type at all - the
component reaches its own backend through its implementation, the way `MoverVehicleBrake` already
does internally - and `initialize()` asks a backend factory for the vehicle's simulation instead
of naming `MaszynaMoverPhysicsServer`.

### What still reaches a class by name from C++

Counted after `Cabin3D` moved to C++. Two of these are the exception `CODE_STYLE.md` allows, the
rest are not.

**Allowed, and commented at the call site** - a GDScript class the C++ node merely hosts:

* `Cabin3D::_propagate_train_id()` calls `set_train_id` on the cab's elements.
* `GenericVehicleComponent` calls `_process_component`, `_get_component_state` and
  `_get_component_config` on the modder's script - the class cannot be known at build time, which
  is what that class exists for.

**Not allowed - our own classes that are simply still GDScript.** Each is the same situation
`Cabin3D` was in, and each stops being an exception when its base moves to C++:

| Class | Named accesses from C++ | Where |
| --- | --- | --- |
| `E3DModelInstance` | 15 | `is_e3d_loaded` x6, `reload` x2, `get_aabb`, `set_smoke_intensity`, `instancer` x2, `lights_state` x3 |
| `MaszynaTrackCurve` | 10 | `p1`, `c1`, `c2`, `p2`, `roll1`, `roll2` in `TrackManager` and `RailVehicleServer` - on the track path |
| `RainVolume` | 6 | `velocity_multiplier`, `bound_enabled`, `bound_min`, `bound_max` |
| `MaszynaPlayer` | 1 | `get_camera` |

~~**A rule broken outright**: `RailVehicle3D` reached `TractionPowerServer` through
`_singleton("TractionPowerServer")`.~~ Done - the server is a C++ singleton with a typed
`get_instance()`, the `_singleton()` helper is gone with its last caller, and the power sources
tick off `SceneTree`'s `process_frame` instead of an autoload's `_process`.

### The traction network's star branch is unreachable

`TractionPowerServer::wire_get_voltage()` returns a wire's **nominal** voltage whenever that wire
is not powered directly, and `power_source` is set only on directly powered wires
(`_resolve_power_sources()`). The whole two-source branch below it - `power_near`, the two
`resistance` values, the `r0g`/`r1g` split of TTraction::VoltageGet() - therefore never runs. It
was already unreachable in the GDScript this was ported from; the port kept the behaviour rather
than the dead code, and says so at that early return.

So `_connect_wires()` and `_propagate_resistance()` build a network nothing reads. Either the
early return is wrong (a wire fed through the network should take the computed voltage, which is
what the original does) or the network is not needed - worth settling before anyone tunes
resistivity and finds it changes nothing.

## Linux release built on an old glibc - what is left

* `release-linux-symbols` (`compile-release-symbols`) still builds on the host, so a build with
  symbols asks for the host's glibc again and does not start on the machines whose crashes it is
  meant to diagnose. It needs the same container as `release-linux`.
* The debug export template (`linux_debug.x86_64`) is still the host-built one.
