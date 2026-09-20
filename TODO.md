# TODO

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
* Scenery models are `E3DRenderingServer` RIDs with the `OPTIMIZED` instancer, which does not
  render `SUBMODEL_FREE_SPOTLIGHT` submodels (no light RIDs) - the NODES instancer creates
  `SpotLight3D`s for them. Scenery node `lights`/`lightcolors` are still ignored by
  `maszyna_node_model_importer.gd`.
* Scenery models have no nodes, so they can't be picked/selected in the editor and don't follow
  the `MaszynaIncludeNode` transform/visibility (world-space, like tracks and traction).
* An `include` with no filename shows up while parsing the real data dir
  (`maszyna_include_importer.gd` now reports it with the parser offset and skips it, instead of
  trying to open the scenery directory). The source is unknown - no asset declares a
  parameterised include path, so it is either a truncated file or a tokenizer misread.

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
* Wiper simulation: nothing drives the `maszyna_wiper_pos`, `maszyna_wiper_timer_out` and
  `maszyna_wiper_timer_return` shader globals of `rain_windscreen.gdshader` - `TrainWipers` only
  stores the FIZ `WiperList:` and the vendored Mover has no `wiperSwitchPos`. The glass shows
  droplets everywhere; the wiping itself is ported in the shader. To port: the wiper movement
  (`DynObj.cpp:4048-4115`, `dWiperPos`/`wiperDirection`), the switch (`Train.cpp:2643`), the blade
  animation (`DynObj.cpp:726-730`) and the feed of the globals for the occupied cab
  (`opengl33renderer.cpp:755-789`). The droplets also ignore vehicle speed and wind (a TODO in the
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
