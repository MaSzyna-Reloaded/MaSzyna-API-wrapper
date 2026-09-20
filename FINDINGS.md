# Findings

Root causes that took a measurement to find. Each entry: the symptom, what proved the cause, the
fix, and the rule it leaves behind. Open work belongs in `TODO.md`, not here.

## 2026-09-20 - regressions after the frame-time optimisation night

37 commits in about 18 hours (`8bd5c9a`..`ed5ee09`), most of them optimisations judged by frame
time only. Nobody looked at the cabin, the lighting or the consist afterwards.

### The modelled cabin covered by the low-poly interior, its light always on

* **Symptom:** inside the cab only the low-poly interior is visible; its lamp submodel glows and
  ignores the switch; Remote shows `LowPolyInterior` with the `OPTIMIZED` instancer. The log says
  the cab was built (`DynamicTrainCabin: built cab 1 ... 61 generated children`).
* **Proof:** the cached template `user://cache/rail_vehicle/dynamic/pkp/303e_v1/303e-ep-tv_452de...res`
  carried `LowPolyInterior instancer` in its name table, and its `.hash` equalled the hash of the
  current `structure-v11` tag - so the fixed instancer code never ran.
* **Cause:** `ebecb4b` set `OPTIMIZED` on the low-poly interior and `8139f8b` removed it again;
  neither bumped the `structure-vN` tag in `dynamic_rail_vehicle_3d_manager.gd`. `OPTIMIZED`
  creates no nodes, so `RailVehicle3D::_update_low_poly_cabs_visibility()` had no `cabN` to hide
  and `_on_low_poly_cabin_e3d_loaded()` collected no materials to dim.
* **Why it was hard:** "Clear cache" cleared only `e3d`, `materials` and `scenery`, never
  `rail_vehicle`, `fiz` or `vehicle_profiles`. `user://cache` also survives a checkout, so every
  `git bisect` step was "bad", including the assumed-good endpoint, and old revisions kept writing
  `OPTIMIZED` templates under the same hash. Windows and Linux differed because each has its own
  `user://`.
* **Not the cause:** the `TrainController` state optimisations (#56/#57) - the occupied cab comes
  from the cabin node's `cab_number`, not from the controller state.
* **Fix:** tag -> `structure-v12`; "Clear cache" covers every `ResourceCache`
  (`VehicleProfileManager` became `@tool` so the editor dock can call it); the low-poly interior
  switches instancer with the distance together with the exterior
  (`RailVehicle3D::_update_model_detail()`), and its `e3d_loaded` restores the `cabN` visibility
  and the dimmed materials.
* **Rule:** a change to code whose output is cached on disk bumps that cache's tag in the same
  commit, and every new `ResourceCache` is wired into "Clear cache". When a code change has no
  effect in game, read the cache file (`strings`, mtime, `.hash`) before forming a hypothesis.

### Coupled wagons drifting apart

* **Symptom:** gaps of 10-25 m between coupled vehicles of a moving consist.
* **Cause:** `72d3b33` wrapped `controller.update_neighbour(end, null, -1, 0.0)` in a
  "already cleared" flag, reading the `null` as a no-op. For a coupled end that call is not a
  clear: it recomputes `Neighbours[end].distance` from `CouplerDist()`
  (`TrainController.cpp:423-430`), exactly as the original does on every update
  (`DynObj.cpp:7144-7154`, called from `vehicle_table::update()`, `DynObj.cpp:8193`), and
  `CouplerForce()` starts from that distance on every step (`Mover.cpp:4781`). Frozen at its
  first value, the coupler stretched without any force building up.
* **Fix:** coupled ends refresh the distance every frame again; the saving stays for free ends,
  where the call really is a clear (`rail_vehicle_physics_server.gd`, `_update_neighbours()`).
* **Rule:** before dropping or caching a call as redundant, open the callee - including the C++
  side - and the original-engine line cited in the comment above it. The reference was there.

### Blotchy, then black ground

* **Symptom:** large dark and bright patches on flat terrain, gone when the camera is close;
  present with the sun's shadows off.
* **Measurements, in order:** Debug Draw *Unshaded* - uniform, so lighting, not geometry or
  textures. *Normal Buffer* - normals vary at the scale of the patches. `td.scn` - the terrain is
  flat (`y = 0`, normals `0 1 0`), so the variation comes from the material. Sun
  `rotation.x = -25.6` - the sun position is right. Per-mip block colours of
  `normals/grass_normal.dds` - R,G are 0.500 at mip 0 but 0.530-0.532 from mip 2 down (0.5 is not
  representable in DXT).
* **Cause:** `material_factory.gd` set `normal_scale = -5.0` on every normal-mapped material
  (since `f4138c4`, #74); the original applies the normal map as it is
  (`mat_normalmap.frag:46-48`).
  * The mip bias of 0.032 is a 5 degree tilt at scale 1 and 24 degrees at scale -5. With the sun
    26 degrees above the horizon that leaves distant flat grass unlit, while mip 0 close to the
    camera looks right.
  * `grass.mat` uses `detail_normalmap` with `param_detail_scale: 0.00125` - a second normal map
    tiled over hundreds of metres, meant to be added at `param_detail_height_scale: 0.45`
    (`mat_detail_normalmap.frag:53-59`). `detail_normalmap.gdshader` applied `normal_scale` to
    the combined normal, so the detail came out five times too strong and reversed: the patches.
    That shader was only registered in `dc25b6f` (09-18); before, `grass.mat` fell back to the
    default shader, which is why the patches were new.
* **Not the cause:** the three `WorldEnvironment`s (two in `vehicle_viewer.tscn`, one in
  `VehicleProfileManager`) live in `SubViewport`s with `own_world_3d = true`; the skydome's
  `clouds_shadow_*` only drive the sun's shadow opacity and penumbra; the terrain normals path
  (`MaszynaTrianglesImporter.cpp`, `SceneryTrianglesBuilder.cpp`) is unchanged since 09-14.
* **Fix:** the `-5.0` override removed, `normal_scale` stays at the 1.0 of the material types;
  `detail_normalmap.gdshader` keeps `normal_scale` off the detail map and uses
  `NORMAL_MAP_DEPTH = 1.0` like `parallax.gdshader`; the material cache key carries
  `MaterialManager.CACHE_VERSION`, which it lacked entirely.
* **Rule:** a tuning factor with no counterpart in the original is a liability - it scales the
  data's errors along with the data.

### Project setting shown as 0/1/2 instead of a named list

* **Cause:** `add_custom_project_setting()` in `libmaszyna.gd` returned early when the setting
  already existed. `project.godot` stores only values; the hint from `add_property_info()` is lost
  with every editor restart, so any `maszyna/*` setting saved there lost its hint and initial
  value. `shadow_cabin_mode=1` was written by hand in `873b3eb`, `shadow_mode` was not.
* **Fix:** the value is set only when missing; the hint and the initial value are registered
  always. Side effect: the editor now drops lines equal to the default on the next save.
