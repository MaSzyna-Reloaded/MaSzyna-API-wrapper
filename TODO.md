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

## Scenery loading

* Include cache / instancing - e.g. `skp/skp_trawa.scm` includes `grass.inc` 24078 times, each
  one parsed again and baked into world-space triangle chunks. Idea: the include importer
  classifies each included file in the context (`path => mode, placement params`): `instanced`
  (only `origin`/`rotate` + `triangles`, no nested includes - key = path + hash of the non-placement
  params, per-occurrence `Transform3D`, rendered as MultiMesh per chunk/texture/range) or `full`
  (whole `.scm` piece - key = path + hash of all params, reusable across sceneries). Results must be
  cached in local space (importers currently bake context origin/rotate into the data); invalidate
  by the dependency list like the compiled scenery cache.

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
