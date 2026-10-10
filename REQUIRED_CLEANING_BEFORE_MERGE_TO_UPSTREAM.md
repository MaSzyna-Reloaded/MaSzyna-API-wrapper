# Required cleaning before merge to upstream

Review of `src/` against `AGENTS.md` and `CODE_STYLE.md`, done 2026-09-29 on
`dev/integrate-track-rendering` (`bd2247f9`). Scope: every C++ file in `src/` except the vendored
`src/legacy/maszyna-mover` and the generated `src/gen` - 320 files, ~42k lines.

Re-verified item by item against the code on 2026-10-09 (`61d035eb8`): fixed items ticked and
their entries removed, the open entries cut down to what is still open, line numbers as of that
commit.

Legend:

* `✔` - verified by hand in the code; the rest was verified by reading the surrounding code
* `ALARM` - a separation-of-concerns breach; per `AGENTS.md` the direction is decided by the
  operator before any code changes
* Numbers `RC-NNN` are permanent - never renumbered or reused. A resolved item is ticked in the
  index and its entry below is deleted, **in the commit that resolves it**; an item that turns out
  to be invalid the same way, marked "(invalid)" in the index
* Checked before every commit (`AGENTS.md`, "Before every commit"): the staged diff ticks what it
  resolves, corrects the lines of what it touches, and adds no new instance of an open item

## Index

### Junk

- [x] [RC-001](#rc-001) Stale SCons `.os` object files and directories left in `src/`

### Correctness

- [x] [RC-002](#rc-002) `BrakeMethod` reaches the Mover unmapped ✔
- [x] [RC-003](#rc-003) Diesel backend forces test power source ✔
- [x] [RC-004](#rc-004) Configuration applied up to five times per vehicle ✔
- [x] [RC-005](#rc-005) Cargo list duplicated by repeated configuration
- [x] [RC-006](#rc-006) Radio call commands never unregistered ✔
- [x] [RC-007](#rc-007) `e3d_loaded` connected again on every dirty frame ✔
- [ ] [RC-008](#rc-008) Headlight colour 255 times overbright ✔
- [x] [RC-009](#rc-009) Main switch voltage defaults derived from zero
- [x] [RC-010](#rc-010) Mover controller commands dereference a null backend
- [ ] [RC-011](#rc-011) Singleton teardown leaks `TractionServer` and breaks the order ✔
- [ ] [RC-012](#rc-012) `MaszynaTranslationServer` dereferences `UserSettings` unchecked
- [ ] [RC-013](#rc-013) `RailVehicleDoors::VOLTAGE_AUTO` not bound
- [x] [RC-014](#rc-014) State keys published before the simulation is ready
- [ ] [RC-015](#rc-015) `VehicleComponent::send_command()` discards the command result
- [x] [RC-016](#rc-016) Cab control drag signs lost without a mesh
- [ ] [RC-017](#rc-017) `PlanarMirror3D` default outside its own range
- [ ] [RC-018](#rc-018) `get_cache_dir` bound with a default for a missing argument
- [x] [RC-019](#rc-019) Mover fields written by two components
- [ ] [RC-124](#rc-124) Bare coupler built from `max_velocity` instead of `Ftmax`
- [ ] [RC-125](#rc-125) `Cabin3D` shake: jolt limit and the random jolt
- [ ] [RC-126](#rc-126) `TrackServer` track width and switch blade speed
- [ ] [RC-127](#rc-127) Doors `VOLTAGE_112` never powered
- [ ] [RC-128](#rc-128) Lighting defaults off the original
- [ ] [RC-129](#rc-129) Tachometer without `MaxTachoSpeed` and the slow jump
- [ ] [RC-130](#rc-130) Brake control pipe pressure and the `NBpA` clamp

### Separation of concerns and getters (ALARM)

- [x] [RC-020](#rc-020) `wire_get_voltage()` changes the power source's state ✔
- [x] [RC-021](#rc-021) Vehicle server calls the scene node
- [x] [RC-022](#rc-022) Drawing node runs pantograph physics and writes simulation inputs
- [x] [RC-023](#rc-023) Base vehicle layer knows rail
- [x] [RC-024](#rc-024) Controller and server call each other; `get_state()` builds a cache
- [x] [RC-025](#rc-025) `vehicle_get_transform()` writes a cache
- [x] [RC-026](#rc-026) Drawing node creates a second vehicle RID
- [ ] [RC-027](#rc-027) Mover member names as public state keys
- [x] [RC-028](#rc-028) `RailVehicleHorns` includes the vendored Mover
- [ ] [RC-029](#rc-029) `Mover*` classes public and instantiated from GDScript
- [ ] [RC-030](#rc-030) `Mover*` components hold state the Mover does not have
- [x] [RC-031](#rc-031) Brake backend keeps a rate only the sound needs
- [ ] [RC-032](#rc-032) Radio component calls up into the vehicle servers
- [x] [RC-033](#rc-033) `E3DInstanceBackend` and `E3DRenderingServer` include each other
- [x] [RC-034](#rc-034) Driver layer tracks player-controlled vehicles
- [ ] [RC-035](#rc-035) `Cabin3D` keeps a controller path it says it has not got
- [x] [RC-036](#rc-036) `SimulationServer` holds cache, build and language
- [ ] [RC-037](#rc-037) `track_get_endpoints()` fills a cache
- [x] [RC-038](#rc-038) `build_get_number()` reads a file and sets a flag
- [ ] [RC-039](#rc-039) `MaszynaParser::get*()` advance the cursor
- [x] [RC-131](#rc-131) `RailVehicleHorns::get_combined_signal()` returns the Mover's bits
- [x] [RC-132](#rc-132) `MoverTypes.hpp` - shared Mover translations without an owner

### Missing events and wiring

- [x] [RC-040](#rc-040) Cabin shown after counting frames
- [x] [RC-041](#rc-041) `pending_start_track_retry` retry flag
- [x] [RC-042](#rc-042) `force_detail_refresh` "try again next tick" flag
- [x] [RC-043](#rc-043) `RailVehicle3D` wires nodes inside `_process`
- [x] [RC-044](#rc-044) Animation bindings resolved twice after reload
- [ ] [RC-045](#rc-045) Component enable lands a tick late through flags
- [ ] [RC-046](#rc-046) Lazy `owner_create` inside the streaming build
- [ ] [RC-047](#rc-047) `build_check_version()` once-guard called "to be sure"
- [ ] [RC-048](#rc-048) Loading queue waits by sleeping and is polled
- [ ] [RC-049](#rc-049) `area_is_ready()` has no event and is polled per frame
- [ ] [RC-050](#rc-050) Lazy initialisation re-checked on every call

### Per-frame work

- [x] [RC-051](#rc-051) Every `RailVehicle3D` processes every frame
- [x] [RC-052](#rc-052) Whole config dictionary built per frame for the wiper angle
- [x] [RC-053](#rc-053) Coupler lookups and string building per frame
- [x] [RC-054](#rc-054) Bogie track samples computed twice per frame
- [x] [RC-055](#rc-055) Pantograph geometry through string-keyed dictionaries per frame
- [x] [RC-056](#rc-056) `ProjectSettings` read per vehicle every 0.25 s
- [x] [RC-057](#rc-057) Allocations and boxing in the vehicle server step
- [x] [RC-058](#rc-058) Track roll read by property name per moved vehicle
- [x] [RC-059](#rc-059) Unbounded E3D animation loop with allocations
- [x] [RC-060](#rc-060) Time and light level each walk all E3D instances
- [x] [RC-061](#rc-061) Smoke emitters looked up and ticked while idle
- [ ] [RC-062](#rc-062) `E3DOptimizedBackend::update()` allocates a map
- [x] [RC-063](#rc-063) Scenery streaming runs every frame while idle
- [x] [RC-064](#rc-064) Scenery streaming entry found by linear scan
- [x] [RC-065](#rc-065) `TractionServer` ticks forever
- [x] [RC-066](#rc-066) `Cabin3D` processes forever
- [ ] [RC-067](#rc-067) `PlanarMirror3D` sets shader parameters every frame
- [x] [RC-068](#rc-068) `SimulationServer::get_instance()` looked up per frame
- [x] [RC-069](#rc-069) Wipers ticked while parked

### Raw pointers in public API

- [ ] [RC-070](#rc-070) `E3DRenderingServer::instance_attach_node(Node3D *)`
- [ ] [RC-071](#rc-071) `SceneryStreamingServer::streaming_set_camera(Camera3D *)`
- [x] [RC-072](#rc-072) `RailVehicleServer::vehicle_component_get()` returns a pointer
- [ ] [RC-073](#rc-073) `SignalHeadNode::set_model(Node *)` / `get_model()`
- [ ] [RC-074](#rc-074) `MaszynaTrianglesImporter::import_triangles(MaszynaParser *)`
- [x] [RC-075](#rc-075) Vehicle layer bound methods taking and returning pointers
- [ ] [RC-076](#rc-076) `E3DSubModel::set_parent(E3DSubModel *)`
- [ ] [RC-123](#rc-123) Scene-tree nodes holding pointers to objects they do not own

### Calls by name

- [x] [RC-077](#rc-077) GDScript `E3DModelInstance` calls without the required comment ✔
- [ ] [RC-078](#rc-078) Commands registered with `Callable(this, "name")`
- [ ] [RC-079](#rc-079) Signal names as literals despite constants

### Magic numbers

- [x] [RC-080](#rc-080) `RailVehicle3D`
- [x] [RC-081](#rc-081) `Cabin3D`
- [x] [RC-082](#rc-082) `TractionServer`
- [x] [RC-083](#rc-083) `TrackServer`
- [x] [RC-084](#rc-084) `MoverRailVehicleBuffCoupl`
- [x] [RC-085](#rc-085) `MoverRailVehicleBrake`
- [x] [RC-086](#rc-086) `MoverRailVehicleMasterController`
- [x] [RC-087](#rc-087) Warning signal bitmasks in horns and security system
- [x] [RC-088](#rc-088) `MoverCircuitUnit` thresholds
- [x] [RC-089](#rc-089) `MoverRailVehicleWheels`
- [x] [RC-090](#rc-090) `60.0` rpm conversion repeated in six files
- [x] [RC-091](#rc-091) Speed control preset count and doors remote control value
- [x] [RC-092](#rc-092) `E3DNodesBackend` light settings defaults
- [x] [RC-093](#rc-093) Microsecond conversions and E3D distances
- [x] [RC-094](#rc-094) `e3d_parser` flags and offsets
- [x] [RC-095](#rc-095) `MaszynaParser` characters and buffer size
- [x] [RC-096](#rc-096) Scenery loading and streaming timings
- [x] [RC-097](#rc-097) Vehicle server, controller and coupler literals
- [x] [RC-098](#rc-098) FIZ defaults in rail component headers without a source

### DRY / KISS

- [x] [RC-099](#rc-099) `_rename()` duplicated in three servers
- [x] [RC-100](#rc-100) Clock-hold processing code duplicated
- [x] [RC-101](#rc-101) Electric traction forwarders copied into three engines
- [x] [RC-102](#rc-102) Several public roads to one effect
- [x] [RC-103](#rc-103) Camera mode written past `camera_set_mode()`
- [x] [RC-104](#rc-104) `TrackServer::set_is_topology_changed()` second writer
- [x] [RC-105](#rc-105) Dead code
- [x] [RC-106](#rc-106) Private helpers with a single call site
- [x] [RC-107](#rc-107) Forwarding wrappers
- [x] [RC-108](#rc-108) Same work done twice
- [x] [RC-109](#rc-109) Duplicated declarations
- [x] [RC-110](#rc-110) Config keys nobody reads

### Naming

- [x] [RC-111](#rc-111) New radio signals not `<subject>_<what>_changed`
- [x] [RC-112](#rc-112) Older server methods and signals named otherwise
- [x] [RC-113](#rc-113) `consist` instead of `trainset`

### Cosmetic

- [ ] [RC-114](#rc-114) Deprecated `MAKE_MEMBER_GS*` macros
- [ ] [RC-115](#rc-115) `macros.hpp` included where unused
- [ ] [RC-116](#rc-116) Functional casts instead of `static_cast`
- [ ] [RC-117](#rc-117) State keys with slashes
- [ ] [RC-118](#rc-118) Headers not self-contained
- [ ] [RC-119](#rc-119) `using namespace godot;` in a header
- [ ] [RC-120](#rc-120) Privacy sections
- [ ] [RC-121](#rc-121) Stale, orphaned and non-English comments
- [ ] [RC-122](#rc-122) Commands named `set_*` without a property

---

## Correctness

### RC-008

**Headlight colour 255 times overbright** ✔

* **Where:** `src/vehicles/rail/RailVehicleLighting.hpp:83`
* **Problem:** `Color(255, 255, 255)` - `Color` takes floats in 0..1, so this is white
  multiplied by 255.
* **Fix:** `Color(1, 1, 1)`.

### RC-011

**Singleton teardown leaks `TractionServer` and breaks the order** ✔

* **Where:** `src/register_types.cpp:586-645` (unregister), from `:647` (free)
* **Rule:** `CODE_STYLE.md` "Singletons C++" (unregister, then `memdelete`, then `nullptr`,
  per singleton)
* **Problem:**
  * `TractionServer` is unregistered and freed only inside
    `if (has_singleton("RailVehicleServer"))` (`:590-599`). Without that server it leaks.
  * `MaszynaMoverVehicleServer`, `RailVehicleServer` and every singleton after them down to
    `UserSettings` are unregistered in one block (`:586-645`) and freed in another (from
    `:647`).
* **Fix:** one independent block per singleton, in reverse registration order.

### RC-012

**`MaszynaTranslationServer` dereferences `UserSettings` unchecked**

* **Where:** `src/utils/MaszynaTranslationServer.cpp:25, 48, 65, 78`
* **Rule:** `CODE_STYLE.md` "Singletons C++" - `get_instance()` does not guarantee a pointer
* **Problem:** `UserSettings::get_instance()` is dereferenced without a null check, while
  `GameDataServer` in the same file is checked (`:20-21`).
* **Fix:** check the pointer once, as for `GameDataServer`.

### RC-013

**`RailVehicleDoors::VOLTAGE_AUTO` not bound**

* **Where:** `src/vehicles/rail/RailVehicleDoors.hpp:74` vs `.cpp:75-78`
* **Problem:** the enum value `VOLTAGE_AUTO` is declared but not bound with the other voltage
  constants, so it is missing from Godot.
* **Fix:** bind it.

### RC-015

**`VehicleComponent::send_command()` discards the command result**

* **Where:** `src/vehicles/base/VehicleComponent.cpp:21-23` (bound), `:211-215` (body)
* **Problem:** it returns `void` and drops the `Variant` that `VehicleController::send_command()`
  returns (`VehicleController.cpp:377-394`), so a component - and a modder's script through it -
  never learns whether its command was accepted (#43).
* **Fix:** return the controller's result.

### RC-017

**`PlanarMirror3D` default outside its own range**

* **Where:** `src/rendering/PlanarMirror3D.hpp:25` vs `.cpp:69`
* **Problem:** the default `resolution_scale = 2.0` is outside the inspector range
  `"0.05,1,0.05"`.
* **Fix:** make the default and the range agree.

### RC-018

**`get_cache_dir` bound with a default for a missing argument**

* **Where:** `src/cache/ResourceCache.cpp:23`
* **Problem:** `get_cache_dir` takes no arguments but is bound with `DEFVAL("")`.
* **Fix:** remove the `DEFVAL`.

### RC-027

**Mover member names as public state keys** ALARM

* **Where:**
  * `src/vehicles/rail/RailVehicleEngine.cpp:225-228`: `"Mm"`, `"Mw"`, `"Fw"`, `"Ft"`
  * `RailVehicleElectricEngine.cpp:121`: `"Im"`
  * `RailVehicleDieselElectricEngine.cpp:9`: `"Im"`
* **Rule:** the backend never appears in a public interface
* **Problem:** these keys are Mover member names in the public state dump, while every other key
  is descriptive `snake_case`.
* **Fix:** descriptive names such as `motor_torque` and `motor_current`, with their readers
  updated: `addons/libmaszyna/legacy/cabin/python_screen_state.gd:133, 135` and the tests
  (`test_train_electric_induction_engine.gd:152, 178`, `maszyna_startup_test.gd:417`,
  `test_zzz_driver_hints_sn61_v2.gd:32`).

### RC-029

**`Mover*` classes public and instantiated from GDScript** ALARM

* **Where:** `src/register_types.cpp:298-332` (`GDREGISTER_CLASS(MoverRailVehicle*)`,
  `set_controller_implementation` at `:317`); `Mover*.new()` in about 18
  `addons/libmaszyna/legacy/fiz/fiz_train_*_parser.gd` and in `fiz_vehicle_builder.gd:170-171`
  (horns, radio)
* **Rule:** the backend never appears in a public interface
* **Problem:** the backend's name is part of the extension's API. `set_controller_implementation`
  exists precisely to hide it, and the controller is created through it, but the FIZ importer
  still instantiates the `Mover*` components by name.
* **Decision:** register them as internal/abstract and create them through the implementation
  factory; or accept it for the FIZ importer and write that down.

### RC-030

**`Mover*` components hold state the Mover does not have** ALARM

* **Where:**
  * `src/legacy/vehicles/MoverRailVehicleWipers.hpp:22-35`: `wipers` ("the vendored Mover has no
    wipers at all")
  * `MoverRailVehicleWheels.hpp:33-35`: `wheel_angle_*_deg` ("vehicle layer's, not the Mover's")
  * `MoverRailVehicleDoors.hpp:81-82`: `mirror_left_position`
  * `MoverRailVehicleLighting.hpp:55`: `headlights_dimmed`
  * `MoverRailVehicleMasterController.hpp:34-41`: tachometer, `distance_counter`
  * `MoverRailVehicleController.hpp:28`: `fitted_adapter_models`
* **Rule:** "if this layer were replaced wholesale, would the field go with it?"
* **Problem:** replacing the backend would lose vehicle state.
* **Decision:** move the fields to the `RailVehicle*` interface components (as `TODO.md`'s #184
  design describes for the wiper positions).

### RC-032

**Radio component calls up into the vehicle server** ALARM

* **Where:** `src/legacy/vehicles/MoverRailVehicleRadio.cpp:40-47, 51-59, 67`
* **Problem:** a component (lower layer) calls the servers that own vehicles: `radio_stop` calls
  `vehicle_emergency_signal_send()` (`:40-47`), `radio_call` calls `vehicle_radio_call()`
  (`:51-59`), and `:67` asks `VehicleServer::vehicle_has_person_role()`.
* **Decision:** the component emits events, and the server, or whoever cares, reacts; what it
  needs to know of the occupancy is handed down by the owner.

### RC-035

**`Cabin3D` keeps a controller path it says it has not got** ALARM

* **Where:** `src/cabin/Cabin3D.hpp:58, 115-116` vs the comment at `:95-96`; bound at
  `Cabin3D.cpp:46-52`, accessors `:312-316`; used by
  `addons/libmaszyna/legacy/cabin/maszyna_dynamic_train_cabin.gd:56-60`
* **Problem:** a bound `NodePath controller_path` to a `VehiclePhysicsNode`, although the class
  says "there is deliberately no path to a controller here" and already holds `vehicle_rid`. The
  cab has two ways to reach its vehicle.
* **Fix:** remove the path and use `vehicle_rid` only.

### RC-037

**`track_get_endpoints()` fills a cache**

* **Where:** `src/tracks/TrackServer.cpp:832-835` → `_endpoints()` at `:275-288`
* **Rule:** a getter never changes state
* **Problem:** the getter lazily fills `cached_endpoints` from a `const` method.
* **Fix:** build the cache in `_set_curves` (`:352`), which already clears it (`:359`).

### RC-039

**`MaszynaParser::get*()` advance the cursor**

* **Where:** `src/legacy/parsers/maszyna_parser.cpp:104-122` (`get8`, `get_line`), `:36, 40`
  (`get_tokens`, `get_tokens_until` bound); `maszyna_parser.hpp:52-63`
* **Rule:** a getter never changes state
* **Problem:** `get8`, `get_line`, `get_tokens` and `get_tokens_until` advance `cursor`. They mirror
  `FileAccess.get_8`, but they are `get_*` methods with a side effect.
* **Decision:** rename them to `read_*`, or accept the `FileAccess` idiom and note it.

### RC-124

**Bare coupler built from `max_velocity` instead of `Ftmax`** ✔

* **Where:** `src/legacy/vehicles/MoverRailVehicleBuffCoupl.cpp:72, 108-115`
* **Problem:** a Bare coupler's `SpringKC`, `FmaxC`, `SpringKB` and `FmaxB` add the vehicle's
  `max_velocity` (Vmax) where `LoadFIZ_BuffCoupl` adds `Ftmax`, the maximum tractive force
  (`Mover.cpp:10666-10671`).
* **Fix:** take `Ftmax`; check that it is set before the coupler applies its configuration.

### RC-125

**`Cabin3D` shake: jolt limit and the random jolt** ✔

* **Where:** `src/cabin/Cabin3D.hpp:72`; `Cabin3D.cpp:224`
* **Problem:** `shake_jolt_limit` defaults to 0.15, the original's `BaseShake.jolt_limit` to 2.0
  (`DynObj.h:839`). The extra random shake at speed (`DynObj.cpp:8113-8123`), added to the
  spring force before the attenuation, is not ported.
* **Fix:** the original's default and the missing branch.

### RC-126

**`TrackServer` track width and switch blade speed** ✔

* **Where:** `src/tracks/TrackServer.hpp:111, 122`; `TrackServer.cpp:1218`
* **Problem:** `DEFAULT_TRACK_WIDTH` is 1.6, the original's `fTrackWidth` 1.435 (`Track.h:205`).
  The blade speed is derived from `SWITCH_FULL_DURATION` (2 s over the full travel); the original
  moves it at a fixed `fOffsetSpeed = 0.1` (`Track.h:67`, `Track.cpp:1944`), which matches only
  while the offset delay is the default.
* **Fix:** the original's width, and `fOffsetSpeed` in place of the duration.

### RC-127

**Doors `VOLTAGE_112` never powered** ✔

* **Where:** `src/legacy/vehicles/MoverRailVehicleDoors.hpp:84`;
  `src/vehicles/rail/RailVehicleDoors.hpp:78`, `.cpp:22, 78`;
  `addons/libmaszyna/legacy/fiz/fiz_train_doors_parser.gd:83`
* **Problem:** the voltage maps to 112 V, but the Mover powers doors only at 0, 24 or 110 V
  (`Mover.cpp:8768`), so doors set to it never move.
* **Fix:** `VOLTAGE_110` = 110 V, and the FIZ parser's value with it.

### RC-128

**Lighting defaults off the original** ✔

* **Where:** `src/vehicles/rail/RailVehicleLighting.hpp:66, 77-78, 83-88`
* **Problem:**
  * `lights_default_selector_position` defaults to 0, the original's `LightsDefPos` to 1
    (`MOVER.h:1701`), and the selector starts there (`Mover.cpp:11925-11926`).
  * `light_alternative_max_voltage` 24.0 and `light_alternative_capacity` 495.0 have no source;
    the original's `TPowerParameters` starts at 0 (`MOVER.h:1032`).
  * The head light colour, the normal and high-beam multipliers and `instrument_type` are bound
    but read by nothing.
* **Fix:** the original's defaults; the unread properties wired or removed.

### RC-129

**Tachometer without `MaxTachoSpeed` and the slow jump** ✔

* **Where:** `src/legacy/vehicles/MoverRailVehicleMasterController.cpp:56-71`
* **Problem:** the needle's limit is always `Vmax * 1.05`; the original takes the FIZ's
  `MaxTachoSpeed` when it is set (`Train.cpp:8584-8587`, `Mover.cpp:10815`). Below 5 km/h the
  original's needle swings to a random 0-4 (`Train.cpp:8594-8597`); the port has one rule above
  1 km/h.
* **Fix:** port both.

### RC-130

**Brake control pipe pressure and the `NBpA` clamp** ✔

* **Where:** `src/legacy/vehicles/MoverRailVehicleBrake.cpp:597, 641-646`
* **Problem:** the original reads `HiPP` into `CntrlPipePress` right after the random start
  value and sets `HighPipePress` from it (`Mover.cpp:10469-10471`); the port keeps the random
  value and sets `HighPipePress` apart. `NBpA` is clamped to 0..4, which `LoadFIZ_Brake` does not
  do (`Mover.cpp:10401`).
* **Fix:** the original's order; the clamp dropped or kept with a reason.

## Missing events and wiring

### RC-045

**Component enable lands a tick late through flags**

* **Where:** `src/vehicles/base/VehicleComponent.cpp:201-205` (`set_enabled`), `:139-164`
  (`process()`), `:11, 135-137` (`mark_dirty`, bound)
* **Rule:** one road to one effect; no deferral
* **Problem:** `set_enabled` sets `enabled_changed` and `dirty`, consumed by `process()` on the
  next tick, so commands and config land a tick late. `mark_dirty` is bound, which gives a
  second road to `apply_config`.
* **Fix:** apply at once in the setter's owner operation, and remove the bound `mark_dirty`.

### RC-046

**Lazy `owner_create` inside the streaming build**

* **Where:** `src/legacy/e3d/E3DRenderingServer.cpp:873-877` (in `_light_create`, `:847`),
  `:1082-1086` (in `_build_instance_smoke_sources`, `:1049`), `:744-749` (in
  `instance_register`, `:737`)
* **Rule:** no `ensure_*` under any name; no wiring in a hot path
* **Problem:** `if (light_stream_owner < 0) { light_stream_owner = streaming->owner_create(...) }`
  - and the same for `smoke_stream_owner` and `stream_owner` - wires callables into
  `SceneryStreamingServer` from code that runs inside the per-frame build.
* **Fix:** create the owners once at server initialisation.

### RC-047

**`build_check_version()` once-guard called "to be sure"**

* **Where:** `src/game_data/GameDataServer.cpp:57-62` (flag `GameDataServer.hpp:29`); callers
  `demo/demo_3d.gd:9`; in the game (`MaSzyna-Reloaded/maszyna-reloaded`) `game.gd:89`,
  `startup/startup.gd:15`
* **Rule:** no `ensure_*`; never do the same thing twice
* **Problem:** `if (build_version_checked) return false;` - an ensure-style guard that clears
  caches and writes a setting, called from three scenes.
* **Fix:** one call at startup by the owner; the scenes stop calling it.

### RC-048

**Loading queue waits by sleeping and is polled**

* **Where:** `src/utils/WorkerTaskQueue.cpp:61-66` (`is_done()`), `:69-96` (`wait()`, the sleep
  at `:94`); pollers `addons/libmaszyna/scenery/scenery_instancer.gd:530` (loop `:524-533`),
  `addons/libmaszyna/legacy/vehicle/maszyna_vehicle_profile_manager.gd:117`
* **Rule:** never work around a missing event
* **Problem:** `wait()` loops on `OS::delay_usec(100)`, and `is_done()` exists so callers can
  poll it.
* **Fix:** a "done" signal, and a condition variable in `wait()`.

### RC-049

**`area_is_ready()` has no event and is polled per frame**

* **Where:** `src/scenery/SceneryStreamingServer.cpp:606-609` (signals `:64-69`); poller
  `demo/demo_scenery_loading.gd:207` (an `await process_frame` loop in `_build_surroundings`,
  which also polls `RailVehicleRenderingServer.builds_get_pending_count()`)
* **Problem:** there is no "area ready" signal - `streaming_builds_finished` is not about an
  area - so readiness can only be polled.
* **Fix:** emit a signal when the requested area has been built.

### RC-050

**Lazy initialisation re-checked on every call**

* **Where:**
  * `src/legacy/vehicles/MoverRailVehicleWipers.cpp:29-33` (`switch_initialized`, field
    `MoverRailVehicleWipers.hpp:37`, checked on every configuration apply)
  * `src/legacy/cabin/PythonScreenServer.cpp:225-259` (the worker thread and the Python host
    are started lazily in `screen_create`, `:226, 256-258`; the reload path `:200-205` relies on it)
* **Rule:** no `ensure_*` under any name - state is initialised where it is created
* **Fix:** initialise in the constructor or the server's initialisation.

## Per-frame work

### RC-062

**`E3DOptimizedBackend::update()` allocates a map**

* **Where:** `src/legacy/e3d/E3DOptimizedBackend.cpp:38, 57`
* **Problem:** `HashMap<E3DSubModel *, bool> overrides` is allocated, and every RID is walked,
  on each call - reached from blink edges in `_process_lights` and from `_resolve_all_lights`.
* **Fix:** keep the map as a member and update only what changed.

### RC-067

**`PlanarMirror3D` sets shader parameters every frame**

* **Where:** `src/rendering/PlanarMirror3D.cpp:198, 218, 248-252`
* **Problem:**
  * Out of view, `_set_rendering(false)` calls `set_shader_parameter("reflecting")` every
    frame.
  * In view, it calls `set_keep_aspect_mode()` (a constant) and builds the string-named
    `"mirror_view_projection"` every frame.
* **Fix:** set on change only, with the constant set once and a cached `StringName`.

## Raw pointers in public API

### RC-070

**`E3DRenderingServer::instance_attach_node(Node3D *)`**

* **Where:** `src/legacy/e3d/E3DRenderingServer.hpp:390`, bound at `.cpp:42`
* **Rule:** a public API takes RIDs, Variants, Callables and `ObjectID`s
* **Fix:** `instance_attach_object_instance_id(RID, uint64_t)`, like
  `vehicle_attach_object_instance_id`.

### RC-071

**`SceneryStreamingServer::streaming_set_camera(Camera3D *)`**

* **Where:** `src/scenery/SceneryStreamingServer.hpp:363`, bound at `.cpp:40`
* **Problem:** a bound server method with a raw pointer; the two HUD mouse servers take an
  `ObjectID` for the same thing.
* **Fix:** take a `uint64_t` `ObjectID`, like the HUD mouse servers.

### RC-073

**`SignalHeadNode::set_model(Node *)` / `get_model()`**

* **Where:** `src/signalling/SignalHeadNode.hpp:54-55`, bound at `.cpp:22-23`
* **Problem:** bound with raw pointers, although the class stores an `ObjectID` inside.
* **Fix:** take and return an `ObjectID`.

### RC-074

**`MaszynaTrianglesImporter::import_triangles(MaszynaParser *)`**

* **Where:** `src/legacy/scenery/MaszynaTrianglesImporter.hpp:17-19`
* **Problem:** a bound static method with a raw pointer to a `RefCounted`.
* **Fix:** `Ref<MaszynaParser>`.

### RC-076

**`E3DSubModel::set_parent(E3DSubModel *)`**

* **Where:** `src/legacy/e3d/E3DSubModel.hpp:103`
* **Problem:** a public C++ method with a raw pointer. It is not bound, so this is low priority.
* **Fix:** make it private or a friend of the parser, or take an index.

### RC-123

**Scene-tree nodes holding pointers to objects they do not own**

* **Rule:** a node living in the scene tree keeps no pointer to another object - a node as an
  `ObjectID`, the vehicle by its RID (FINDINGS.md 2026-09-30, "Edit FIZ" aborted the editor)
* **Where:**
  * `src/rendering/PlanarMirror3D.hpp:26`: `glass` (`MeshInstance3D *`, its parent)
  * `src/vehicles/base/GenericVehicleComponent.hpp:21`: `Node *script_owner`, held by a
    `RefCounted` component - the same dangle from the other side
  * the rest of the codebase not yet swept

## Calls by name

### RC-078

**Commands registered with `Callable(this, "name")`**

* **Where:** 122 sites in 20 files - every `_register_commands`, the largest
  `src/vehicles/rail/RailVehicleController.cpp` (20), `RailVehicleBrake.cpp` (19) and
  `RailVehicleDoors.cpp` (13)
* **Rule:** call a method, do not name it
* **Problem:** the class is known, so a rename or typo silently yields a callable to nothing.
* **Fix:** `callable_mp(this, &Class::method)`.

### RC-079

**Signal names as literals despite constants**

* **Where:**
  * `src/legacy/vehicles/MoverRailVehicleSecuritySystem.cpp:21, 25`:
    `emit_signal("blinking_changed")`, `("beeping_changed")` - no constant exists, the
    `ADD_SIGNAL` in `RailVehicleSecuritySystem.cpp:31-32` uses literals too
  * `src/utils/UserSettings.cpp:133-184`
  * `src/vehicles/base/VehicleComponent.cpp:132, 159`
* **Fix:** use the constants, and add one where it is missing.

## Cosmetic

### RC-114

**Deprecated `MAKE_MEMBER_GS*` macros**

* **Where:** 540 uses in 40 headers. The largest are `src/vehicles/rail/RailVehicleDieselEngine.hpp`
  (80), `RailVehicleBrake.hpp` (65), `src/legacy/e3d/E3DSubModel.hpp` (30) and
  `RailVehicleDoors.hpp` (28). Also every rail component header, every `*ListItem`/`*Item`
  resource, `VehicleController.hpp` and `RailVehicleController.hpp`.
* **Rule:** `CODE_STYLE.md` "MAKE_* macros ... are deprecated"
* **Problem:** besides being deprecated, the macros leave the member fields public.
* **Fix:** explicit declarations, file by file; then delete `src/macros.hpp`.

### RC-115

**`macros.hpp` included where unused**

* **Where:** `src/legacy/vehicles/MoverRailVehicleSecuritySystem.cpp:4` (only `ASSERT_MOVER` is
  used, which comes from `MoverBackend.hpp`); `src/legacy/e3d/E3DModel.hpp:9` (only the `.cpp`
  uses `BIND_PROPERTY`)

### RC-116

**Functional casts instead of `static_cast`**

* **Where:** `src/vehicles/rail/RailVehicleRenderingServer.cpp:954, 1386, 1654`;
  `RailVehicleServer.cpp:349, 1818, 1827-1828`; `src/legacy/e3d/E3DInstanceBackend.cpp:54`;
  `src/legacy/e3d/E3DNodesBackend.cpp:193`
* **Rule:** `CODE_STYLE.md` "Conversions"

### RC-117

**State keys with slashes**

* **Where:** about 73 keys in 6 files: `MoverRailVehicleElectroPneumaticDynamicBrake.cpp`
  (`dcemued/…`, 6), `MoverRailVehicleSpeedControl.cpp` (`speed_control/…`, 6),
  `MoverRailVehicleSpringBrake.cpp` (`spring_brake/…`, 5), `MoverRailVehicleLighting.cpp`
  (`lights/…`, 20), `src/vehicles/rail/RailVehicleEnginePowerSource.cpp` (30, mostly
  `current_collector/…`), `RailVehicleElectricEngine.cpp:130-135` (`indicators/…`)
* **Rule:** by analogy with "property names ... without slashes"
* **Decision:** whether state keys follow the property rule; renaming them touches the GDScript
  readers.

### RC-118

**Headers not self-contained**

* **Where:**
  * `src/legacy/vehicles/MoverRailVehicleBrake.hpp`, `MoverRailVehicleDoors.hpp`,
    `MoverRailVehicleWipers.hpp` get `<map>`, `<vector>` and `<string>` only through `MOVER.h:13-17`
  * `src/logging/GameLog.hpp:15-24` (macros use `UtilityFunctions` without the include)
* **Rule:** `CODE_STYLE.md` "A header is self-contained"

### RC-119

**`using namespace godot;` in a header**

* **Where:** `src/register_types.h:6`

### RC-120

**Privacy sections**

* **Where:**
  * Empty or dangling sections: `RailVehicleEngine.hpp:159`, `RailVehicleDieselEngine.hpp:202`,
    `RailVehicleBuffCoupl.hpp:19`, `VehicleController.hpp:28-29` (`public:` followed at once by
    `private:`)
  * Public members: `RailVehicleEngine.hpp:102-103` (`_bind_methods`, `motor_param_table`)
  * Protected `_register_commands` overrides made public: `RailVehicleSpeedControl.hpp:38`,
    `RailVehicleHeating.hpp:40`, `RailVehicleSwitches.hpp:81`, `RailVehicleElectricEngine.hpp:79`
  * `VehicleController.hpp:50`: `apply_configuration` is protected but bound publicly
    (`.cpp:41`)
* **Rule:** `CODE_STYLE.md` "Explicit privacy declarations"

### RC-121

**Stale, orphaned and non-English comments**

* **Where:**
  * Polish comments (AGENTS.md asks for English):
    `src/legacy/vehicles/MoverRailVehicleBrake.cpp:592, 655`
  * `MoverRailVehicleBrake.cpp:215`, `MoverRailVehicleLighting.cpp:410`: refer to
    `_do_fetch_state_from_mover()`, which no longer exists
  * orphaned or misplaced doc comments: `src/vehicles/base/VehicleController.hpp:107-112`,
    `VehicleController.cpp:144-145`, `src/vehicles/rail/RailVehicleServer.cpp:2130-2131`

### RC-122

**Commands named `set_*` without a property**

* **Where:**
  * `src/vehicles/rail/RailVehicleSpringBrake.cpp:34-35`: commands
    `set_spring_brake_active/enabled`
  * `RailVehicleElectroPneumaticDynamicBrake.cpp:49`: command `set_ep_brake_force`
  * `RailVehicleHorns.cpp:19-21`: the commands `horn_low`, `horn_high` and `whistle` dispatch to
    methods named `set_*`
* **Rule:** `CODE_STYLE.md` "Godot properties" - `set_<property>` is reserved for property
  setters
* **Fix:** use command verbs (`spring_brake_activate`, ...) for the commands and the methods
  behind them.
