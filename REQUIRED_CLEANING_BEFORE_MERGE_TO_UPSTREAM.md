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

### Separation of concerns and getters (ALARM)

- [ ] [RC-020](#rc-020) `wire_get_voltage()` changes the power source's state ✔
- [x] [RC-021](#rc-021) Vehicle server calls the scene node
- [x] [RC-022](#rc-022) Drawing node runs pantograph physics and writes simulation inputs
- [ ] [RC-023](#rc-023) Base vehicle layer knows rail
- [ ] [RC-024](#rc-024) Controller and server call each other; `get_state()` builds a cache
- [ ] [RC-025](#rc-025) `vehicle_get_transform()` writes a cache
- [x] [RC-026](#rc-026) Drawing node creates a second vehicle RID
- [ ] [RC-027](#rc-027) Mover member names as public state keys
- [x] [RC-028](#rc-028) `RailVehicleHorns` includes the vendored Mover
- [ ] [RC-029](#rc-029) `Mover*` classes public and instantiated from GDScript
- [ ] [RC-030](#rc-030) `Mover*` components hold state the Mover does not have
- [x] [RC-031](#rc-031) Brake backend keeps a rate only the sound needs
- [ ] [RC-032](#rc-032) Radio component calls up into the vehicle servers
- [ ] [RC-033](#rc-033) `E3DInstanceBackend` and `E3DRenderingServer` include each other
- [x] [RC-034](#rc-034) Driver layer tracks player-controlled vehicles
- [ ] [RC-035](#rc-035) `Cabin3D` keeps a controller path it says it has not got
- [x] [RC-036](#rc-036) `SimulationServer` holds cache, build and language
- [ ] [RC-037](#rc-037) `track_get_endpoints()` fills a cache
- [x] [RC-038](#rc-038) `build_get_number()` reads a file and sets a flag
- [ ] [RC-039](#rc-039) `MaszynaParser::get*()` advance the cursor

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
- [ ] [RC-054](#rc-054) Bogie track samples computed twice per frame
- [x] [RC-055](#rc-055) Pantograph geometry through string-keyed dictionaries per frame
- [ ] [RC-056](#rc-056) `ProjectSettings` read per vehicle every 0.25 s
- [ ] [RC-057](#rc-057) Allocations and boxing in the vehicle server step
- [ ] [RC-058](#rc-058) Track roll read by property name per moved vehicle
- [ ] [RC-059](#rc-059) Unbounded E3D animation loop with allocations
- [ ] [RC-060](#rc-060) Time and light level each walk all E3D instances
- [ ] [RC-061](#rc-061) Smoke emitters looked up and ticked while idle
- [ ] [RC-062](#rc-062) `E3DOptimizedBackend::update()` allocates a map
- [ ] [RC-063](#rc-063) Scenery streaming runs every frame while idle
- [ ] [RC-064](#rc-064) Scenery streaming entry found by linear scan
- [ ] [RC-065](#rc-065) `TractionServer` ticks forever
- [ ] [RC-066](#rc-066) `Cabin3D` processes forever
- [ ] [RC-067](#rc-067) `PlanarMirror3D` sets shader parameters every frame
- [ ] [RC-068](#rc-068) `SimulationServer::get_instance()` looked up per frame
- [ ] [RC-069](#rc-069) Wipers ticked while parked

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
- [ ] [RC-081](#rc-081) `Cabin3D`
- [ ] [RC-082](#rc-082) `TractionServer`
- [ ] [RC-083](#rc-083) `TrackServer`
- [ ] [RC-084](#rc-084) `MoverRailVehicleBuffCoupl`
- [ ] [RC-085](#rc-085) `MoverRailVehicleBrake`
- [ ] [RC-086](#rc-086) `MoverRailVehicleMasterController`
- [ ] [RC-087](#rc-087) Warning signal bitmasks in horns and security system
- [ ] [RC-088](#rc-088) `MoverCircuitUnit` thresholds
- [ ] [RC-089](#rc-089) `MoverRailVehicleWheels`
- [ ] [RC-090](#rc-090) `60.0` rpm conversion repeated in six files
- [ ] [RC-091](#rc-091) Speed control preset count and doors remote control value
- [ ] [RC-092](#rc-092) `E3DNodesBackend` light settings defaults
- [ ] [RC-093](#rc-093) Microsecond conversions and E3D distances
- [ ] [RC-094](#rc-094) `e3d_parser` flags and offsets
- [ ] [RC-095](#rc-095) `MaszynaParser` characters and buffer size
- [ ] [RC-096](#rc-096) Scenery loading and streaming timings
- [ ] [RC-097](#rc-097) Vehicle server, controller and coupler literals
- [ ] [RC-098](#rc-098) FIZ defaults in rail component headers without a source

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

* **Where:** `src/vehicles/rail/RailVehicleLighting.hpp:80`
* **Problem:** `Color(255, 255, 255)` - `Color` takes floats in 0..1, so this is white
  multiplied by 255.
* **Fix:** `Color(1, 1, 1)`.

### RC-011

**Singleton teardown leaks `TractionServer` and breaks the order** ✔

* **Where:** `src/register_types.cpp:581-640` (unregister), from `:642` (free)
* **Rule:** `CODE_STYLE.md` "Singletons C++" (unregister, then `memdelete`, then `nullptr`,
  per singleton)
* **Problem:**
  * `TractionServer` is unregistered and freed only inside
    `if (has_singleton("RailVehicleServer"))` (`:585-594`). Without that server it leaks.
  * `MaszynaMoverVehicleServer`, `RailVehicleServer` and every singleton after them down to
    `UserSettings` are unregistered in one block (`:581-640`) and freed in another (from
    `:642`).
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
  returns (`VehicleController.cpp:379-396`), so a component - and a modder's script through it -
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

### RC-020

**`wire_get_voltage()` changes the power source's state** ✔ ALARM

* **Where:** `src/traction/TractionServer.cpp:537-580`, through `PowerSource::current_get()` at
  `:124-140`; the "Quirk" is at `:108-121`
* **Rule:** a getter never changes state; never work around a mistimed event
* **Problem:**
  * The read adds to `total_admittance`, sets `loaded`, `total_current` and `output_voltage`,
    and resets `fuse_timer`.
  * The voltage therefore depends on how many readers ask per tick. The only caller is the
    vehicle step (`RailVehicleServer.cpp:1787`), once per pantograph.
  * It is a port of `TTraction::VoltageGet` (`Traction.cpp:470`). The loads ask from render
    frames while the sources tick on their own beat, and the "Quirk" keeps the previous load to
    cover that - a workaround for the timing.
* **Decision:**
  * (a) Split the read into a named operation, e.g. `wire_draw_current()` in the vehicle tick,
    and a pure `wire_get_voltage()`.
  * (b) Move the loads' query into the traction tick, so the order is the original's.

### RC-023

**Base vehicle layer knows rail** ALARM

* **Where:** `src/vehicles/base/VehicleController.hpp:15-18`; `VehicleController.cpp:4-5,
  343-349`
* **Problem:**
  * The base controller forward-declares `RailVehicleBrake`, `RailVehicleEngine` and
    `RailVehicleSecuritySystem`, none of which it uses.
  * The `.cpp` includes `RailVehicleEngine.hpp` (apparently unused) and `RailVehicleServer.hpp`,
    and `get_world_transform()` (`:343-349`) calls `RailVehicleServer::vehicle_get_transform()` -
    the base layer reaching the rail one (RC-024, RC-025).
* **Decision:** the base controller loses the rail knowledge; the transform comes from a layer
  the base one may know.

### RC-024

**Controller and server call each other; `get_state()` builds a cache** ALARM

* **Where:**
  * `src/vehicles/rail/RailVehicleController.cpp:12-20` (`get_state()` →
    `VehicleServer::vehicle_dump_state()`), cache written in `VehicleServer.cpp:516-535`
    (fields `VehicleServer.hpp:57-59`)
  * `src/vehicles/base/VehicleController.cpp:344` (`RailVehicleServer::vehicle_get_transform` -
    the base layer reaching the rail one, RC-023)
  * `src/vehicles/rail/RailVehicleWheels.cpp:13` (`get_bogie_transform()` reaches
    `RailVehicleServer`)
* **Rule:** layers do not call each other both ways; a getter never changes state
* **Problem:** the server steps controllers and components, and they call back into it. The
  getter `get_state()`, also the `state` property, writes `state_dump`, `state_dump_serial` and
  `state_dump_valid`. The comment (`VehicleServer.cpp:529-530`) admits the call "would
  recurse".
* **Decision:** one direction of calls; the dump is built in the tick, not in the getter.

### RC-025

**`vehicle_get_transform()` writes a cache** ALARM

* **Where:** `src/vehicles/rail/RailVehicleServer.cpp:1388-1400`; invalidated at `:1062, 1357`;
  fields `RailVehicleServer.hpp:178-179`
* **Rule:** a getter never changes state
* **Problem:** the getter sets `placement->body_transform` and `body_transform_valid`
  (`:1397-1398`). `VehicleController::get_world_transform()` reaches it too.
* **Fix:** compose the transform where the placement changes, in the step.

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
  updated: `addons/libmaszyna/legacy/cabin/python_screen_state.gd:137, 139` and the tests
  (`test_train_electric_induction_engine.gd:152, 178`, `maszyna_startup_test.gd:417`,
  `test_zzz_driver_hints_sn61_v2.gd:32`).

### RC-029

**`Mover*` classes public and instantiated from GDScript** ALARM

* **Where:** `src/register_types.cpp:296-330` (`GDREGISTER_CLASS(MoverRailVehicle*)`,
  `set_controller_implementation` at `:315`); `Mover*.new()` in about 18
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
  * `src/legacy/vehicles/MoverRailVehicleWipers.hpp:22-31`: `wipers` ("the vendored Mover has no
    wipers at all")
  * `MoverRailVehicleWheels.hpp:33-35`: `wheel_angle_*_deg` ("vehicle layer's, not the Mover's")
  * `MoverRailVehicleDoors.hpp:81-82`: `mirror_left_position`
  * `MoverRailVehicleLighting.hpp:55`: `headlights_dimmed`
  * `MoverRailVehicleMasterController.hpp:23-30`: tachometer, `distance_counter`
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

### RC-033

**`E3DInstanceBackend` and `E3DRenderingServer` include each other** ALARM

* **Where:** `src/legacy/e3d/E3DInstanceBackend.cpp:2, 100, 123, 125`;
  `E3DRenderingServer.hpp:2, 37-40`
* **Problem:** the backend uses `E3DRenderingServer::INSTANCE_KIND_DYNAMIC` and
  `TRANSLUCENCY_CUTOUT`, while the server's header includes the backend's, so the two layers
  depend on each other.
* **Fix:** move the enum to a header shared by both, or into the backend.

### RC-035

**`Cabin3D` keeps a controller path it says it has not got** ALARM

* **Where:** `src/cabin/Cabin3D.hpp:45, 98-99` vs the comment at `:78-79`; bound at
  `Cabin3D.cpp:45-51`, accessors `:272-276`; used by
  `addons/libmaszyna/legacy/cabin/maszyna_dynamic_train_cabin.gd:58-62`
* **Problem:** a bound `NodePath controller_path` to a `VehiclePhysicsNode`, although the class
  says "there is deliberately no path to a controller here" and already holds `vehicle_rid`. The
  cab has two ways to reach its vehicle.
* **Fix:** remove the path and use `vehicle_rid` only.

### RC-037

**`track_get_endpoints()` fills a cache**

* **Where:** `src/tracks/TrackServer.cpp:829-832` → `_endpoints()` at `:272-285`
* **Rule:** a getter never changes state
* **Problem:** the getter lazily fills `cached_endpoints` from a `const` method.
* **Fix:** build the cache in `_set_curves` (`:349`), which already clears it (`:356`).

### RC-039

**`MaszynaParser::get*()` advance the cursor**

* **Where:** `src/legacy/parsers/maszyna_parser.cpp:106-124` (`get8`, `get_line`), `:38, 42`
  (`get_tokens`, `get_tokens_until` bound); `maszyna_parser.hpp:49-60`
* **Rule:** a getter never changes state
* **Problem:** `get8`, `get_line`, `get_tokens` and `get_tokens_until` advance `cursor`. They mirror
  `FileAccess.get_8`, but they are `get_*` methods with a side effect.
* **Decision:** rename them to `read_*`, or accept the `FileAccess` idiom and note it.

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

* **Where:** `src/legacy/e3d/E3DRenderingServer.cpp:867-871` (in `_light_create`, `:841`),
  `:1077-1081` (in `_build_instance_smoke_sources`, `:1042`), `:739-744` (in
  `instance_register`, `:732`)
* **Rule:** no `ensure_*` under any name; no wiring in a hot path
* **Problem:** `if (light_stream_owner < 0) { light_stream_owner = streaming->owner_create(...) }`
  - and the same for `smoke_stream_owner` and `stream_owner` - wires callables into
  `SceneryStreamingServer` from code that runs inside the per-frame build.
* **Fix:** create the owners once at server initialisation.

### RC-047

**`build_check_version()` once-guard called "to be sure"**

* **Where:** `src/game_data/GameDataServer.cpp:57-62` (flag `GameDataServer.hpp:29`); callers
  `demo/demo_3d.gd:6`, `demo/demo_scenery_loading.gd:89`, `demo/startup/startup.gd:15`
* **Rule:** no `ensure_*`; never do the same thing twice
* **Problem:** `if (build_version_checked) return false;` - an ensure-style guard that clears
  caches and writes a setting, called from three scenes.
* **Fix:** one call at startup by the owner; the scenes stop calling it.

### RC-048

**Loading queue waits by sleeping and is polled**

* **Where:** `src/utils/WorkerTaskQueue.cpp:61-66` (`is_done()`), `:69-96` (`wait()`, the sleep
  at `:94`); pollers `addons/libmaszyna/scenery/scenery_instancer.gd:527` (loop `:519-530`),
  `addons/libmaszyna/legacy/vehicle/maszyna_vehicle_profile_manager.gd:117`
* **Rule:** never work around a missing event
* **Problem:** `wait()` loops on `OS::delay_usec(100)`, and `is_done()` exists so callers can
  poll it.
* **Fix:** a "done" signal, and a condition variable in `wait()`.

### RC-049

**`area_is_ready()` has no event and is polled per frame**

* **Where:** `src/scenery/SceneryStreamingServer.cpp:605-608` (signals `:62-67`); poller
  `demo/demo_scenery_loading.gd:207` (an `await process_frame` loop in `_build_surroundings`,
  which also polls `RailVehicleRenderingServer.builds_get_pending_count()`)
* **Problem:** there is no "area ready" signal - `streaming_builds_finished` is not about an
  area - so readiness can only be polled.
* **Fix:** emit a signal when the requested area has been built.

### RC-050

**Lazy initialisation re-checked on every call**

* **Where:**
  * `src/legacy/vehicles/MoverRailVehicleWipers.cpp:29-33` (`switch_initialized`, field
    `MoverRailVehicleWipers.hpp:31`, checked on every configuration apply)
  * `src/legacy/cabin/PythonScreenServer.cpp:231-251` (the worker thread is started lazily in
    `screen_create`, `:232, 249-250`; the reload path `:206-210` relies on it)
* **Rule:** no `ensure_*` under any name - state is initialised where it is created
* **Fix:** initialise in the constructor or the server's initialisation.

## Per-frame work

### RC-054

**Bogie track samples computed twice per frame**

* **Where:** `src/vehicles/rail/RailVehicleRenderingServer.cpp:1044-1051`;
  `RailVehicleServer.cpp:1402-1420` (`_compose_body_transform()`)
* **Problem:** `wheels->get_bogie_transform(...)`, called twice, repeats the ±half-spacing
  samples that `_compose_body_transform` has just made, and the wheels component is looked up on
  every placement change of a detailed vehicle.
* **Fix:** the server publishes the bogie transforms together with the body transform.

### RC-056

**`ProjectSettings` read per vehicle every 0.25 s**

* **Where:** `src/vehicles/rail/RailVehicleRenderingServer.cpp:1525-1526`
* **Problem:** `ProjectSettings::get_singleton()->get_setting(DETAIL_DISTANCE_SETTING)` runs per
  vehicle on every slow visit.
* **Fix:** read it once and refresh on `ProjectSettings.settings_changed`, as
  `RailVehicleServer.cpp:42` does.

### RC-057

**Allocations and boxing in the vehicle server step**

* **Where:** `src/legacy/vehicles/MaszynaMoverVehicleServer.cpp:92-103`;
  `src/vehicles/rail/RailVehicleServer.cpp:1657-1661`
* **Problem:** the Mover server clears and refills both controller vectors every frame, with an
  ObjectDB lookup per vehicle, and `track_vehicles` (a map of vectors) is cleared and rebuilt
  every frame.
* **Fix:** keep the structures across frames and update them when a vehicle is added, removed or
  moved to another track.

### RC-058

**Track roll read by property name per moved vehicle**

* **Where:** `src/vehicles/rail/RailVehicleServer.cpp:1483-1484, 1509-1510`
* **Rule:** per-frame lookups; a string call only with a comment saying why
* **Problem:** `curve_data->get("roll1")`/`("roll2")` per moved vehicle per frame, with no
  comment, although `TrackServer` already caches `CurvePoints::roll1/roll2`
  (`TrackServer.hpp:121-128`) - it only does not expose them.
* **Fix:** expose the cached roll from `TrackServer` and read it there.

### RC-059

**Unbounded E3D animation loop with allocations**

* **Where:** `src/legacy/e3d/E3DRenderingServer.cpp:1679, 1683, 1412`
* **Problem:** the loop over `animating_instances` (`:1679`) has no bound; it allocates a
  `PackedStringArray` per instance (`:1683`), and clears and rebuilds `submodel_poses` through
  `_pose_submodels` (`:1412`).
* **Fix:** a per-frame budget, round-robin like `_process_smoke()`, and no per-instance
  allocation.

### RC-060

**Time and light level each walk all E3D instances**

* **Where:** `src/legacy/e3d/E3DRenderingServer.cpp:1902, 1915-1929`; caller
  `maszyna_environment_node.gd:477-478`
* **Problem:** on a change, `environment_set_time()` and `environment_set_light_level()` each call
  `_resolve_all_lights()` (`:1902`), which walks every instance. The environment calls both back
  to back, so a push that changes both costs two full passes.
* **Fix:** one operation that sets both and resolves once.

### RC-061

**Smoke emitters looked up and ticked while idle**

* **Where:** `src/legacy/e3d/E3DRenderingServer.cpp:1263-1279`
* **Problem:** a hash lookup per emitter (`smoke_objects.getptr(smoke_order[...])`, `:1274`),
  and a `Time::get_singleton()` lookup per frame (`:1268`). Invisible or zero-intensity dynamic
  emitters stay in the order and keep being ticked.
* **Fix:** cache the singleton, take idle emitters out of the order, and keep pointers in the
  order.

### RC-062

**`E3DOptimizedBackend::update()` allocates a map**

* **Where:** `src/legacy/e3d/E3DOptimizedBackend.cpp:38, 57`
* **Problem:** `HashMap<E3DSubModel *, bool> overrides` is allocated, and every RID is walked,
  on each call - reached from blink edges in `_process_lights` and from `_resolve_all_lights`.
* **Fix:** keep the map as a member and update only what changed.

### RC-063

**Scenery streaming runs idle with an unbounded loop**

* **Where:** `src/scenery/SceneryStreamingServer.cpp:719-742`
* **Problem:** `_process_streaming()` runs every frame while a camera is set, even when idle,
  and takes the mutex and does ObjectDB and `Time` work every frame (`:723-742`).
* **Fix:** stop when the plan is fulfilled and restart on camera cell change.

### RC-064

**Scenery streaming entry found by linear scan**

* **Where:** `src/scenery/SceneryStreamingServer.cpp:345-359` (`_get_entry()`); callers
  `:831, 973, 989, 1185`
* **Problem:** `_get_entry()` scans the chunk's entries linearly, called inside the per-frame
  budget loops.
* **Fix:** index the entries by id.

### RC-065

**`TractionServer` ticks forever**

* **Where:** `src/traction/TractionServer.cpp:51-52, 65-76`
* **Problem:** the `process_frame` connection is permanent. Every frame it looks up the main
  loop and the root and loops over all power sources, even with no sources or no load.
* **Fix:** connect only while there are sources with a load, and cache the tree.

### RC-066

**`Cabin3D` processes forever**

* **Where:** `src/cabin/Cabin3D.cpp:140, 160-164`
* **Problem:** `set_process(true)` is never switched off. Every 1/50 s step does a
  `VehicleServer::get_instance()` lookup and a `vehicle_component_get()`, even for a cab
  whose vehicle has no diesel engine.
* **Fix:** resolve the engine when `vehicle_rid` changes, and do not process without one.

### RC-067

**`PlanarMirror3D` sets shader parameters every frame**

* **Where:** `src/rendering/PlanarMirror3D.cpp:198, 218, 248-252`
* **Problem:**
  * Out of view, `_set_rendering(false)` calls `set_shader_parameter("reflecting")` every
    frame.
  * In view, it calls `set_keep_aspect_mode()` (a constant) and builds the string-named
    `"mirror_view_projection"` every frame.
* **Fix:** set on change only, with the constant set once and a cached `StringName`.

### RC-068

**`SimulationServer::get_instance()` looked up per frame**

* **Where:** `src/simulation/SimulationServer.hpp:79-81` (`get_instance()`); per tick in
  `src/driver/DriverSystem.cpp:81` (`_process_updates`) and
  `src/scenario/ScenarioEventServer.cpp:281` (`_process_queue`)
* **Problem:** the lookup is a name lookup on `Engine`, done every tick.
* **Fix:** cache the pointer at initialisation.

### RC-069

**Wipers ticked while parked**

* **Where:** `src/legacy/vehicles/MoverRailVehicleWipers.cpp:58-71`
* **Problem:** the per-wiper timers run every tick (`wiper.out_timer += p_delta`), even when
  every wiper is parked and switched off.
* **Fix:** skip, or turn off, while every wiper is parked and switched off.

## Raw pointers in public API

### RC-070

**`E3DRenderingServer::instance_attach_node(Node3D *)`**

* **Where:** `src/legacy/e3d/E3DRenderingServer.hpp:387`, bound at `.cpp:41`
* **Rule:** a public API takes RIDs, Variants, Callables and `ObjectID`s
* **Fix:** `instance_attach_object_instance_id(RID, uint64_t)`, like
  `vehicle_attach_object_instance_id`.

### RC-071

**`SceneryStreamingServer::streaming_set_camera(Camera3D *)`**

* **Where:** `src/scenery/SceneryStreamingServer.hpp:347`, bound at `.cpp:38`
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
  * `src/legacy/vehicles/MoverRailVehicleSecuritySystem.cpp:20, 24`:
    `emit_signal("blinking_changed")`, `("beeping_changed")` - no constant exists, the
    `ADD_SIGNAL` in `RailVehicleSecuritySystem.cpp:31-32` uses literals too
  * `src/utils/UserSettings.cpp:133-184`
  * `src/vehicles/base/VehicleComponent.cpp:132, 159`
* **Fix:** use the constants, and add one where it is missing.

## Magic numbers

All items below break the PROHIBITED rule: "a non-self-evident literal gets a named constant; a
ported value keeps the original's value and a source reference".

### RC-081

**`Cabin3D`**

* **Where:**
  * `src/cabin/Cabin3D.cpp:169-188`: `/ 60.0`, `* 4.0`, `* 1.0625`, `/ 200.0`, `* 100.0`, some
    ported from `DynObj.cpp:8028` without a reference
  * `Cabin3D.hpp:31-32, 54-63`: `SHAKE_STEP`, `SPRING_REST_LENGTH`,
    `shake_spring_stiffness = 125.0` (`DynObj.cpp:2284`) and the other ported defaults, without
    file:line

### RC-082

**`TractionServer`**

* **Where:** `src/traction/TractionServer.cpp`:
  * `:119-135, 547`: `1e-10`, `< 100.0`, `* 1.083`, `10000.0`. These come from
    `TractionPower.h:58`, `TractionPower.cpp:122, 128` and `Traction.cpp:477`, but are neither
    named nor referenced.
  * `:231, 264, 269, 294, 469`: `grow(5.0)`, `last_flags |= 1`, `|= 2` - unnamed bits, although
    `LAST_SPAN_FLAGS` exists
  * `:321-322`: `0.0, 0.2, ..., 1.0, 3, 60.0` repeats the `PowerSource` defaults
    (`TractionPower.h:50-54`) as bare literals

### RC-083

**`TrackServer`**

* **Where:**
  * `src/tracks/TrackServer.hpp:157`: `switch_f_offset1 = -0.05` instead of
    `-SWITCH_OFFSET_DELAY` (`Track.cpp:57`)
  * `.hpp:94, 96, 110, 112, 118, 139`: `SWITCH_OFFSET_DELAY` (`Track.h:71`), `RAIL_HEIGHT`,
    `SWITCH_FULL_DURATION`, `SWITCH_BLADE_RATIO`, `ROLL_FIX_FACTOR` and `width = 1.6`, with no
    source reference

### RC-084

**`MoverRailVehicleBuffCoupl`**

* **Where:** `src/legacy/vehicles/MoverRailVehicleBuffCoupl.cpp:85-104`
* **Problem:** `SpringKC = 50.0 * mass + max_velocity / 0.05`, `4500 * 1000`, `0.55` - only the
  function has a reference (`Mover.cpp:10297`); none of the values are named.

### RC-085

**`MoverRailVehicleBrake`**

* **Where:** `src/legacy/vehicles/MoverRailVehicleBrake.cpp`:
  * `:587`: `CLAMP(..., 0, 4)`
  * `:590`: `* 1000.0`
  * `:612`: `100 * M_PI`
  * `:220, 597, 600`: `< 0.01`, three times
  * `:629`: `5 + 0.001 * (randf_range(0, 10) - randf_range(0, 10))`, no source

### RC-086

**`MoverRailVehicleMasterController`**

* **Where:** `src/legacy/vehicles/MoverRailVehicleMasterController.cpp:56-79`: `11.31`, `1.05`,
  `3.0`, `0.66` - the tachometer values, unnamed and without a source; `3.0` is repeated
  although `max_tachometer = 3.0` is declared.

### RC-087

**Warning signal bitmasks in horns and security system**

* **Where:** `src/legacy/vehicles/MoverRailVehicleHorns.cpp:13-90`;
  `MoverRailVehicleSecuritySystem.cpp:114-123`
* **Problem:** `WarningSignal |= 4`, `TestFlag(..., 4)` and `EmergencyBrakeWarningSignal = 4` -
  the bits 1/2/4 have no names.

### RC-088

**`MoverCircuitUnit` thresholds**

* **Where:** `src/legacy/vehicles/MoverCircuitUnit.cpp:13, 21, 34`
* **Problem:** `BrakePress < 1.0` and `RventRot < 5.0`, with no reference.

### RC-089

**`MoverRailVehicleWheels`**

* **Where:** `src/legacy/vehicles/MoverRailVehicleWheels.cpp:13, 41`
* **Problem:** `const double k = 472.0` has no source reference; `1.0` m is used as a fallback
  diameter.

### RC-090

**`60.0` rpm conversion repeated in six files**

* **Where:** nine times in six files, among them:
  * `src/cabin/Cabin3D.cpp:169-170`
  * `src/legacy/vehicles/MoverDieselEngineUnit.cpp:223, 275, 286`
  * `MoverRailVehicleHeating.cpp:25-26`
* **Problem:** only a local `SECONDS_PER_MINUTE` exists (`RailVehicleRenderingServer.cpp:1426`),
  not shared.
* **Fix:** one shared `SECONDS_PER_MINUTE` constant.

### RC-091

**Speed control preset count and doors remote control value**

* **Where:**
  * `src/legacy/vehicles/MoverRailVehicleSpeedControl.cpp:23`: local `MAX_PRESET_SPEEDS = 10`
    instead of the size of `SpeedCtrlButtons`
  * `MoverRailVehicleDoors.cpp:387`: `remote_control ? 24 : 0`

### RC-092

**`E3DNodesBackend` light settings defaults**

* **Where:** `src/legacy/e3d/E3DNodesBackend.cpp:289, 294-296`
* **Problem:**
  * The literal `"maszyna/vehicles/lights_volumetric_fog_energy", 4.0` (`:289`).
  * The fade values `150.0`, `100.0` and `200.0` are unnamed (`:294-296`).

### RC-093

**Microsecond conversions and E3D distances**

* **Where:**
  * `src/legacy/e3d/E3DRenderingServer.cpp:1755`: `/ 1000000.0`, although `USEC_PER_SECOND` exists
  * `src/tracks/TrackServer.cpp:469`: `/ 1000000.0`
  * `E3DRenderingServer.cpp:940`: `distance * 0.25f`

### RC-094

**`e3d_parser` flags and offsets**

* **Where:** `src/legacy/e3d/e3d_parser.cpp`:
  * `:58`: `flags & 32`
  * `:88` (commented out), `:111, 116, 125`: `- 168`, `/ 256`, `/ 320`, `/ 64`

### RC-095

**`MaszynaParser` characters and buffer size**

* **Where:** `src/legacy/parsers/maszyna_parser.cpp:118`; `[128]` at `maszyna_parser.hpp:32, 34,
  36` and `.cpp:62, 151, 248, 252`
* **Problem:** `c == 10 || c == 13` instead of `'\n'`/`'\r'`, and a `[128]` buffer size repeated
  seven times.

### RC-096

**Scenery loading and streaming timings**

* **Where:** `src/utils/WorkerTaskQueue.cpp:94, 111`: `delay_usec(100)`,
  `get_processor_count() - 2`

### RC-097

**Vehicle server, controller and coupler literals**

* **Where:**
  * `src/vehicles/rail/RailVehicleServer.cpp:1469, 1476`: `0.000001`, `0.999`
  * `src/vehicles/base/VehicleController.hpp:212`: `1e10`
  * `VehicleController.cpp:193`: 1.0 m threshold

### RC-098

**FIZ defaults in rail component headers without a source**

* **Where:**
  * `src/vehicles/rail/RailVehicleElectricEngine.hpp:55-67`
  * `RailVehicleLighting.hpp:75-84`
  * `RailVehicleAIHints.hpp:32` (`1.05`)
* **Problem:** default values copied from the original have no `Mover.cpp`/`MOVER.h`
  reference.
* **Fix:** one reference per block of defaults, pointing at where the original declares them.

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

* **Where:** `src/legacy/vehicles/MoverRailVehicleSecuritySystem.cpp:3` (only `ASSERT_MOVER` is
  used, which comes from `MoverBackend.hpp`); `src/legacy/e3d/E3DModel.hpp:9` (only the `.cpp`
  uses `BIND_PROPERTY`)

### RC-116

**Functional casts instead of `static_cast`**

* **Where:** `src/vehicles/rail/RailVehicleRenderingServer.cpp:945, 1376, 1647`;
  `RailVehicleServer.cpp:332, 1753, 1762-1763`; `src/legacy/e3d/E3DInstanceBackend.cpp:54`;
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
    `RailVehicleBuffCoupl.hpp:19`, `VehicleController.hpp:31-32` (`public:` followed at once by
    `private:`)
  * Public members: `RailVehicleEngine.hpp:102-103` (`_bind_methods`, `motor_param_table`)
  * Protected `_register_commands` overrides made public: `RailVehicleSpeedControl.hpp:38`,
    `RailVehicleHeating.hpp:40`, `RailVehicleSwitches.hpp:81`, `RailVehicleElectricEngine.hpp:79`
  * `VehicleController.hpp:50`: `apply_configuration` is protected but bound publicly
    (`.cpp:43`)
* **Rule:** `CODE_STYLE.md` "Explicit privacy declarations"

### RC-121

**Stale, orphaned and non-English comments**

* **Where:**
  * Polish comments (AGENTS.md asks for English):
    `src/legacy/vehicles/MoverRailVehicleBrake.cpp:572, 630`
  * `MoverRailVehicleBrake.cpp:196`, `MoverRailVehicleLighting.cpp:408`: refer to
    `_do_fetch_state_from_mover()`, which no longer exists
  * orphaned or misplaced doc comments: `src/vehicles/base/VehicleController.hpp:107-112`,
    `VehicleController.cpp:146-147`, `src/vehicles/rail/RailVehicleServer.cpp:2056-2057`

### RC-122

**Commands named `set_*` without a property**

* **Where:**
  * `src/vehicles/rail/RailVehicleSpringBrake.cpp:34-35`: commands
    `set_spring_brake_active/enabled`
  * `RailVehicleElectroPneumaticDynamicBrake.cpp:49`: command `set_ep_brake_force`
  * `RailVehicleHorns.cpp:20-22`: the commands `horn_low`, `horn_high` and `whistle` dispatch to
    methods named `set_*`
* **Rule:** `CODE_STYLE.md` "Godot properties" - `set_<property>` is reserved for property
  setters
* **Fix:** use command verbs (`spring_brake_activate`, ...) for the commands and the methods
  behind them.
