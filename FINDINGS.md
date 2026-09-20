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

## 2026-09-20 - scenery environment, fog and Skydome

### Huge terrain triangles missing under the camera

* **Symptom:** a terrain triangle kilometres long is missing until the camera gets close to one
  particular spot; trees standing on it are there.
* **Cause:** `SceneryTrianglesBuilder` stored each triangle whole in the 1 km cell of its centroid,
  and `SceneryStreamingServer` streams a chunk in by the distance to that cell
  (`SceneryStreamingServer.cpp:53-56`), capped at `scenery_draw_distance`. Standing on the far part
  of the triangle is standing outside the range of its cell.
* **Fix:** triangles are clipped along the cell grid (Sutherland-Hodgman in XZ, normals and UVs
  interpolated). A cut is always computed from the lower end of the edge, so two triangles sharing
  an edge get the very same vertex and no crack opens.
* **Rule:** whatever is streamed or culled by a cell must not reach outside of it.

### A winter afternoon turning the fog into orange milk

* **Symptom:** same fog settings, fine at 12:15 and an opaque orange wall at 15:13 (20 January,
  50 N). Earlier: the whole winter day tinted orange.
* **Cause:** Skydome blends its day and night values and its sunset colours by the sine of the sun
  elevation with hard-coded windows - full day only above 17.5 degrees, sunset colours up to 23.6.
  A winter sun at mid latitudes peaks at 16-19 degrees, so an afternoon took a quarter of the
  night fog (`night_vol_fog_density` is 24 times the day value) and most of the sunset tint. The
  sky shader carries a copy of both formulas. It only showed once a scenery could set a January
  date (`config movelight`).
* **Fix:** `day_full_elevation` (6 degrees) and `sunset_fade_start/end_elevation` (4 and 10) in
  Skydome, passed to the shader as uniforms and registered as `gnd_skydome/*` settings.
* **Rule:** thresholds on the sun altitude have to be checked against a winter day, not only a
  summer one.

### Two fog layers that did not agree

* The original's fog has no density: it is `1 - exp(-(z / range)^2)` with
  `range = fFogEnd / max(1, Overcast * 2)` (`apply_fog.glsl:16`, `opengl33renderer.cpp:4685`) -
  63% at the range, not a linear ramp complete at `fFogEnd`. A depth fog complete at 1.5 of the
  range with a curve of 1.5 follows it closest.
* Godot's volumetric fog is an extinction per metre over a volume in front of the camera, not an
  opacity at a distance. Scaling its **length** with the fog distance made a far fog fill the view
  with milk; Skydome shortening that length while raising the density made the fog peak at a boost
  of about 0.6 and thin out above it - pulsing against the depth fog. The density has to follow
  the distance inversely, the length stays, and past the peak the density makes up for the
  shortening.
* The volumetric fog stands in front of the sky as much as in front of anything else
  (`volumetric_fog_sky_affect` 1.0) - left out of the sky, fog lit by headlights ends along the
  silhouettes. The depth fog reaching the sky is a different matter: a fog of kilometres is a thin
  layer and must leave the stars alone (`maszyna/rendering/fog_sky_height`); rain fills the air
  all the way up and reaches the sky in full.
* `Environment.fog_aerial_perspective` at 1.0 took the fog colour from a sky radiance as dark as
  the scene at dusk - fully fogged objects stayed dark silhouettes. Off by default.
* The easing-curve editor (`PROPERTY_HINT_EXP_EASING`) reads as "output over input"; on a setting
  that is an exponent over the distance it invites values like 0.01, a wall of fog at the camera.
  The value is floored in code.

### A weather change freezing the game

* **Cause:** `MaterialManager._refresh_managed_material()` wrote every managed material back to the
  disk cache - a resource with its textures embedded, 0.7-3 MB each - on the main thread, for
  materials that mostly have no season or weather variant at all (169 of 805 `.mat` files declare
  a rain one). The cache key knows neither the season nor the weather, and a loaded material gets
  its variant applied anyway.
* **Fix:** no write on a refresh; materials without variants are skipped.

### A new `class_name` unknown to the running game

* Global class names come from `.godot/global_script_class_cache.cfg`, which only the editor's
  file scan updates. Running the game without the editor after adding a script with a `class_name`
  fails with "Identifier not declared" in every script that uses it. `godot-double --headless
  --import` rescans without the GUI.

### Duplicated HUD in the demo scenes

* `TopBar`, `ControlWindows` and the menu code were pasted into both `demo_3d` and
  `demo_scenery_loading`; three windows added later reached only one of them. They are one scene
  now (`demo/hud/game_hud.tscn`); a scene adds a menu entry of its own as a `Button` under
  `MenuActions`.

## 2026-09-20 - material shaders missing from the wrapper

### "Shader is not supported: Default_1 / reflmap"

* **Symptom:** warnings from `MaterialFactory`, the materials fell back to the default one.
* **What proved it:** a count of every `shader:` in the `.mat` files of the game dir against
  `~/src/maszyna/shaders/mat_*.frag` - 26 shaders in the original, 12 mapped here. Missing and in
  use: `reflmap` (69), `detail_parallax_specgloss` (24), `reflmap_specgloss` (17), `default_1` (6),
  `rain_windscreen` (4), `default_detail` (3), `colored` (1).
* Shader names are case insensitive in the data (`shader: Default_1`): the original opens
  `mat_<name>.frag` on a Windows file system (`opengl33renderer.cpp:2018`). Lowercased in the
  parser.
* **Rule:** survey the data before trusting a list of "supported" values, and check the age of
  `~/src/maszyna` against the game dir (`shaders/`): `mat_rain_windscreen.frag` and the whole wiper
  code were missing from a checkout of 2024-08.

### `texture2:` is not always the normal map

* A numbered `textureN:` binds slot N-1 of the *shader* (`material.cpp:76-81`), and the slot
  order differs: `reflmap` = diffuse, reflmap; `default_detail` = diffuse, detailnormalmap;
  `water` = normalmap, dudvmap, diffuse; `detail_parallax_specgloss` has specgloss before
  detailnormalmap. The wrapper aliased `tex2` to `normalmap` for every shader.
* A material **without** `shader:` gets `default_0/1/2` by the number of bound textures
  (`material.cpp:117-134`) and `mat_default_2.frag` is `mat_reflmap.frag`: its second texture -
  also when written as `texture_normalmap:` (`texture_bindings`, `material.cpp:60-65`) - is a
  reflection map read through its alpha. About 2500 of the 8633 shaderless materials bind one
  (`rain: { texture2: asphalt_wet }`); the wrapper bump-mapped them with it. Some of the data puts
  a real normal map there (`glass_black_normal`) - the original reads it as a reflmap too.
* **Fix:** `TextureMap.slots` per shader in `MaterialFactory`, resolved by `_texture_path()`.
* **Rule:** `MaterialManager.CACHE_VERSION` bumped - the disk cache cannot see factory changes.

### `parallax_specgloss` never received its specgloss texture

* `_apply_parallax()` did not set `specgloss_texture` at all (187 materials); the unbound sampler
  read as white. Found while adding `detail_parallax_specgloss`.

### Raindrops on the windscreen black as soot

* **Symptom:** the droplets of `rain_windscreen` showed as black rings from inside the cab.
* **What proved it:** the atlas (`textures/fx/raindrops-atlas.dds`) is a white rim over a black
  interior, and the original does not light it - `dropTex.rgb * dynBright`, with `dynBright` from
  the luminance of the ambient light only (`mat_rain_windscreen.frag`). The port had put the
  droplets into `ALBEDO`, "lit by the scene": inside of a dark cab a white rim lit by nothing is
  black.
* **Fix:** droplets go to `EMISSION`; a Godot fragment shader cannot read the ambient light, so
  the blurred screen luminance behind the glass stands in for it.
* **Rule:** "the engine's lighting will do that" is not a port of an unlit term - check what the
  original multiplies by before moving a colour into `ALBEDO`.

### Wipers: data traps

* Of the four vehicles with a `rain_windscreen` glass only `e186_v2` (and the Vectron cab) also
  has wipers; `ep09_v1` has the glass but no `WiperList:`/`wipers_sw:`, `ep09_v2` has the wipers
  but a plain glass. Test the wiping on `e186_v2`.
* `e186_v2/eu47.fiz` ends its `WiperList:` with `endL` instead of `endwl`. The original never
  closes the list then either, it is `Size=` that bounds the switch (`Train.cpp:2643`). The FIZ
  parser honours `Size=` now.
* The shader clock (`TIME`) cannot be read from a script - it is scaled and rolls over - so a
  moment in time cannot be handed to a shader as the original does (`wiper_timer_out`). Pass the
  time elapsed instead.

### Wiped edge running away from the wiper blade

* **Symptom:** the arms moved right, but the clean band trailed the blade on the way out and ran
  ahead of it on the way back.
* **What proved it:** a probe script that loads the E186 body and cab, puts the blade at 11 phases
  of the sweep, projects it onto the glass and reads `szyby_wipermask` there. The blade stays 3 cm
  off the glass all the way (geometry and rotation sign are right); the mask under it reads 0.22,
  0.41, 0.53, 0.62, 0.71, 0.78, 0.85, 0.90, 0.96, 1.0 - an sRGB curve. Decoded it is 0.04, 0.14,
  0.24, 0.35, 0.46, 0.56, 0.69, 0.79, 0.90, 1.0: the fraction of the arm angle.
* **Cause:** the original declares the mask `sRGB_A`, the port sampled it without `source_color`.
* The arms move eased (`smoothInterpolate`) while the original hands the shader the plain
  position - its own edge is up to a tenth of the sweep off. The wrapper feeds the eased one.
* **Rule:** port the `#texture (name, index, FORMAT)` format of every sampler, also for data
  textures; and measure the data under the moving part before tuning the motion.


### A whole layer of droplets popping in after a wipe

* **Symptom:** the wiped glass stayed clean, then every droplet of the area appeared at once.
* **Cause (read from the shader, not measured):** `GetMixFactor()` of the original picks the
  wiper of a cell only while its factor is below 1. The moment the rain has fully returned
  (1 s in the heaviest rain) `side` falls back to 0, and `side` is a part of the cell's random
  seed - so every droplet of the area is dealt anew in one frame. The large droplets of the
  wrapper's own second layer make it obvious.
* **Fix:** the first wiper the cell belongs to is kept even at factor 1; returning droplets and
  rivulets fade in instead of switching on.

## 2026-09-20 - E186 (dynamic/pkp/e186_v2) not starting up

### Ctrl+J did nothing

* The MMD of E186 has no `cabactivation_sw:`. Keyboard actions are polled by the cab widgets, so a
  cab without the gauge had nobody to take `cab_activation_toggle`. The original runs
  `OnCommand_cabactivationtoggle` regardless of the gauge (`Train.cpp:3077`).
* **Fix:** `LegacyCabinCabActivation`, added by `LegacyCabinLogicDelegate` when the cab has no such
  control - the pattern of `LegacyCabinBattery`.
* **Rule:** every `OnCommand_*` of the original works without its gauge; a control mapped only in
  `MmdSemanticCatalog` is dead in every cab that does not model it.

### Main tank empty within a minute and a half

* **Symptom:** pantograph tank at 0.02 bar, pantographs could not be raised.
* **What proved it:** a headless probe loading `p160dc.fiz` and printing the tanks every 5 s -
  4.4 -> 3.0 bar in 30 s, against 0.03 bar for EP07. `brake_emergency_valve_flow` was 0.34: the
  unacknowledged cab signalling brakes, the emergency valve vents the pipe and the handle keeps
  refilling it from the main tank, which the pantograph tank is connected to (`bPantKurek3`).
* **Cause:** `TrainBrake.cpp` had `EmergencyCutsOffHandle = false; //@TODO`, the FIZ says
  `EmergencyCutsOffHandle=Yes` (`Mover.cpp:10508`, `lock_new` at `Mover.cpp:4534`).
* **Fix:** `TrainBrake.main_pipe_emergency_cuts_off_handle`, read by the FIZ brake parser. After
  it the same probe loses 0.006 bar in 5 s during the emergency braking.
* **Trap:** the first re-measurement showed no change - the parsed FIZ is cached on disk.
  `FIZ_PARSER_FORMAT_VERSION` has to be bumped with every change of a FIZ parser (it was not for
  the `WiperList:` `Size=` change either).

### Pantographs raised but standing still

* The state had both pantographs active with 3400 V, the model did not move. E186 has single-arm
  pantographs with no `ramiegorne2` submodel; the vehicle factory and `RailVehicle3D` demanded all
  five elements. The original skips a missing element (`DynObj.cpp:5414`); the geometry only needs
  the lower arm 1, the upper arm 1 and the slider.

### M, D and R dead in the E186 cab

* Keys are polled by the cab widgets. E186 models `main_sw:` instead of `main_on_bt:`/
  `main_off_bt:`, three `dir*_bt:` buttons instead of `dirkey:`, and has a separate `shp_reset_bt:`
  (`SeparateAcknowledge`, the vigilance button does not reset the cab signalling there) - none of
  them was in the catalog, so nothing took M, D, R, and the emergency braking of the unreset SHP
  could not be cleared at all.
* **Fix:** the labels are mapped (`LegacyCabinMainSwitch` takes `main_sw:`, `LegacyCabinReverser`
  the buttons, new commands `security_cabsignal_acknowledge`, `pantographs_drop_all`), and
  `LegacyCabinUnmodelledControls` registers every catalog control with a key that the cab does
  not model, unless one of its keys is already taken by a modelled control.

## 2026-09-20 - Scenery streaming started from the menu camera

* **Symptom:** after scenery loading reached 100%, the loading screen stayed up for up to 30 s;
  without that wait, terrain around the occupied vehicle was still missing.
* **What proved it:** the player registered its camera in `_ready()` at the demo scene position
  `(30, 3, 615)`, while the camera moved to the selected vehicle only after the scenery and cabin
  had been built. The worker preloaded a whole pass in `HashMap` order and published it only at the
  end, so the main-thread priority queue could not prioritise or cancel that old preload.
* **Cause:** camera priority existed only for published builds. A planning pass had no camera
  revision, conflated queued work with built content, and `passes > 0 && pending_builds == 0` could
  also report completion while the current plan was still preloading.
* **Fix:** loading pauses streaming until the final cab/on-foot camera is known. Plans and queued
  work carry a camera revision and preload is published nearest-first.
* **Follow-up measurement:** waiting for the camera chunk plus its eight neighbours still left over
  1000 nearby builds and delayed the cabin by about 15 s. The required scenery at the camera
  appeared much earlier when the loading screen was disabled.
* **Final startup boundary:** startup waits only for the chunk containing the camera. Its eight
  neighbours and the rest of the draw distance continue streaming after the cabin is shown.
* **Rule:** readiness must describe built content for a specific camera revision; an empty handoff
  queue is not proof that worker-side planning or preload has finished.

### Global transform requested while an E3D node leaves the tree

* **Symptom:** loading printed repeated `!is_inside_tree()` errors from
  `Node3D::get_global_transform()` even after the streaming camera itself was guarded.
* **Cause:** `E3DModelInstance` subscribed to transform notifications for its optimized backend and
  forwarded `global_transform` whenever its RID was valid. Removing or reparenting the node can
  deliver that notification while the RID still exists but the node is already outside the tree.
* **Fix:** transform notifications update the rendering server only while the node is in the tree;
  tree re-entry creates the instance with the current transform as before.
* **Rule:** a valid rendering RID does not imply that its owning `Node3D` currently has a global
  transform; notification handlers must check the node lifecycle separately.

## 2026-09-20 - Skydome clouds behind alpha-blended cabin windows

* **Symptom:** enabling any visible cloud cover in a cabin with alpha-blended windows could push a
  60 FPS frame past its V-Sync budget and drop it to 30 FPS.
* **Cause:** light_angular_distance high cost for PSSM and even for medium filter.
* **Fix:** filter switched to the fastests
* **Follow up:** Give possiblity to disable light_angular_distance in Skybox
