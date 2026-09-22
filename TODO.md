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

**Remaining stages** (the full plan, with per-stage verification, is in the session plan file):

* **B - dumps and the vehicle's common properties.** `_fill_config_dictionary` beside the state
  one; `vehicle_dump_config(rid)`; `velocity`, `speed`, `mass_total`, `total_distance`,
  `direction` as typed properties of `VehicleController` with `vehicle_velocity_get(rid)`
  forwarding to them; `Dictionary config` leaves the controller for the components that parse it.
* **C - `VehicleComponent` stops being a `Node`.** An `Object` owned by the server: no
  `_notification`, no `_process`, the controller handed to it at creation instead of being found
  by walking `get_parent()`. `_do_process_mover` becomes `_process_state(delta)`. Every
  component's state becomes typed properties and the flat-dictionary prefixes are cut
  (`brake_pipe_pressure` -> `brakes.pipe_pressure`); the dump keys keep the prefix, because the
  dump is one flat Dictionary for the whole vehicle. `vehicle_component_create` /
  `vehicle_component_get(rid, TYPE)` / `generic_vehicle_component_find(rid, tag)`, with
  `vehicle_component_get` returning the typed object the way
  `PhysicsServer3D::body_get_direct_state(RID)` does. The interface/implementation split
  (`Vehicle<Domain>` / `Mover<Interface>`) is its own commit at the start. `GenericVehicleComponent`
  gets its dump for free from `get_property_list()` + `PROPERTY_USAGE_SCRIPT_VARIABLE`.
* **D - `VehicleController` stops being a `Node`.** `initialize_mover()` moves into
  `vehicle_create()`, so the Mover no longer waits for `_ready()`; registration in `TrainSystem`
  stops hanging off `ENTER_TREE`.
* **E - proxy nodes.** `VehicleControllerNode` plus one `<Interface>Node` per component, each
  thin: `@export`s for the editor, forward to the server object, no logic.
* **F - the vehicle is built by the server, not by fabricating nodes.**
  `fiz_train_controller_instancer.gd` calls `vehicle_component_create` instead of `add_child` +
  `PackedScene.pack()`, and `DynamicRailVehicle3D` stops fabricating nodes -
  `dynamic_rail_vehicle_3d_manager.gd` packs model + FIZ controller + cabin + sound bank into a
  `PackedScene` today and instantiates copies of it. The cache holds a vehicle configuration, not
  a node tree. Bump `structure-vN` and `FIZ_PARSER_FORMAT_VERSION` in that same commit.
* **G - consumer migration.** The sound system, the 13 cabin widgets, the HUD, `cabin_state.gd`
  and the 8 call sites in `RailVehicle3D` take the component once and read typed properties.
  Afterwards nothing may call `vehicle_dump_state()` per frame.
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
* `test_dynamic_rail_vehicle_manager.gd` fails - the bank's `registration.controller` is null
  while `vehicle.get_controller()` already returns one, so the sound bank registered before the
  vehicle had resolved its controller and the 4 Hz sweep has not caught up within the three idle
  frames the test waits. Confirmed pre-existing at `87d5f8d`, before any of the #184 work.
* `test_sm42_startup_sequence.gd::test_successful_moving_on` fails - "Speed should be > 0" at
  line 69, the vehicle never starts moving after the startup sequence. Confirmed pre-existing on
  a clean tree (stash the work, rebuild, run: it fails the same way), so it is not a regression of
  the #184 work - but it is a red test nobody is looking at, and it is the only test covering that
  the startup sequence ends in motion.
* Tests switch the game dir with `UserSettings.save_maszyna_game_dir()`, which writes the user's
  `settings.cfg` (a failed/killed test leaves it pointing at a `user://gut/...` fixture dir):
  `test_dynamic_rail_vehicle_manager.gd`, `test_e3d_lights_state.gd`,
  `test_fiz_train_controller.gd`, `test_maszyna_node_dynamic_importer_direction.gd`,
  `test_material_manager_variants.gd`, `test_nodebank_library_builder.gd` and the game-data tests
  above. Needs a non-persistent game dir override for tests.

## Physics performance

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
