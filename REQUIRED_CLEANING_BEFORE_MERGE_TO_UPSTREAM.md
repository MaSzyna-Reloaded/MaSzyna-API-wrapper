# Required cleaning before merge to upstream

Review of `src/` against `AGENTS.md` and `CODE_STYLE.md`, done 2026-09-29 on
`dev/integrate-track-rendering` (`bd2247f9`). Scope: every C++ file in `src/` except the vendored
`src/legacy/maszyna-mover` and the generated `src/gen` - 320 files, ~42k lines.

Legend:

* `✔` - verified by hand in the code; the rest was verified by reading the surrounding code
  during the review, line numbers as of `bd2247f9`
* `ALARM` - a separation-of-concerns breach; per `AGENTS.md` the direction is decided by the
  operator before any code changes
* Numbers `RC-NNN` are permanent - an item that turns out to be invalid is ticked and marked
  so, never renumbered or reused

`clang-format --dry-run` passes on all 320 files. `clang-tidy` (`make style-check`) was not run.

## Index

### Junk

- [ ] [RC-001](#rc-001) Stale SCons `.os` object files and directories left in `src/`

### Correctness

- [ ] [RC-002](#rc-002) `BrakeMethod` reaches the Mover unmapped ✔
- [ ] [RC-003](#rc-003) Diesel backend forces test power source ✔
- [ ] [RC-004](#rc-004) Configuration applied two to three times per pass ✔
- [ ] [RC-005](#rc-005) Cargo list duplicated by repeated configuration
- [ ] [RC-006](#rc-006) Radio call commands never unregistered ✔
- [x] [RC-007](#rc-007) `e3d_loaded` connected again on every dirty frame ✔
- [ ] [RC-008](#rc-008) Headlight colour 255 times overbright ✔
- [ ] [RC-009](#rc-009) Main switch voltage defaults derived from zero
- [ ] [RC-010](#rc-010) Mover controller commands dereference a null backend
- [ ] [RC-011](#rc-011) Singleton teardown leaks `TractionServer` and breaks the order ✔
- [ ] [RC-012](#rc-012) `MaszynaTranslationServer` dereferences singletons unchecked
- [ ] [RC-013](#rc-013) Missing and mistyped Godot bindings
- [ ] [RC-014](#rc-014) State keys published before the simulation is ready
- [ ] [RC-015](#rc-015) Generic component command result discarded
- [ ] [RC-016](#rc-016) Cab control drag signs lost without a mesh
- [ ] [RC-017](#rc-017) `PlanarMirror3D` default outside its own range
- [ ] [RC-018](#rc-018) `get_cache_dir` bound with a default for a missing argument
- [ ] [RC-019](#rc-019) Mover fields written by two components

### Separation of concerns and getters (ALARM)

- [ ] [RC-020](#rc-020) `wire_get_voltage()` changes the power source's state ✔
- [x] [RC-021](#rc-021) Vehicle server calls the scene node
- [x] [RC-022](#rc-022) Drawing node runs pantograph physics and writes simulation inputs
- [ ] [RC-023](#rc-023) Base vehicle layer knows rail and lighting
- [ ] [RC-024](#rc-024) Controller and server call each other; `get_state()` builds a cache
- [ ] [RC-025](#rc-025) `vehicle_get_transform()` writes a cache
- [x] [RC-026](#rc-026) Drawing node creates a second vehicle RID
- [ ] [RC-027](#rc-027) Mover member names as public state keys
- [ ] [RC-028](#rc-028) `RailVehicleHorns` includes the vendored Mover
- [ ] [RC-029](#rc-029) `Mover*` classes public and instantiated from GDScript
- [ ] [RC-030](#rc-030) `Mover*` components hold state the Mover does not have
- [ ] [RC-031](#rc-031) Brake backend keeps a rate only the sound needs
- [ ] [RC-032](#rc-032) Radio component calls up into the vehicle server
- [ ] [RC-033](#rc-033) `E3DInstanceBackend` and `E3DRenderingServer` include each other
- [ ] [RC-034](#rc-034) Driver layer tracks player-controlled vehicles
- [ ] [RC-035](#rc-035) `Cabin3D` keeps a controller path it says it has not got
- [ ] [RC-036](#rc-036) `SimulationServer` holds cache, build and language
- [ ] [RC-037](#rc-037) `track_get_endpoints()` fills a cache
- [ ] [RC-038](#rc-038) `build_get_number()` reads a file and sets a flag
- [ ] [RC-039](#rc-039) `MaszynaParser::get*()` advance the cursor

### Missing events and wiring

- [x] [RC-040](#rc-040) Cabin shown after counting frames
- [x] [RC-041](#rc-041) `pending_start_track_retry` retry flag
- [x] [RC-042](#rc-042) `force_detail_refresh` "try again next tick" flag
- [x] [RC-043](#rc-043) `RailVehicle3D` wires nodes inside `_process`
- [x] [RC-044](#rc-044) Animation bindings resolved twice after reload
- [ ] [RC-045](#rc-045) Component enable and power source land a tick late through flags
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
- [ ] [RC-063](#rc-063) Scenery streaming runs idle with an unbounded loop
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
- [ ] [RC-086](#rc-086) `MoverRailVehicleController`
- [ ] [RC-087](#rc-087) Warning signal bitmasks in horns and security system
- [ ] [RC-088](#rc-088) `MoverCircuitUnit` thresholds
- [ ] [RC-089](#rc-089) `MoverRailVehicleWheels`
- [ ] [RC-090](#rc-090) `/ 60.0` rpm conversion repeated in six files
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

## Junk

### RC-001

**Stale SCons `.os` object files and directories left in `src/`** ✔

* **Where:**
  * `src/maszyna/McZapkie/{Mover,hamulce,Oerlikon_ESt,friction}.os`
  * `src/maszyna/utilities.os`
  * 45 `*.os` in total in `src/`
  * directories holding nothing but `.os`: `src/brakes`, `core`, `doors`, `engines`,
    `lighting`, `load`, `parsers`, `resources`, `systems`, `godot-ecs`, `maszyna`
  * empty `src/debug`
* **Problem:** these are object files from the old SCons build (2025-11-24/25). That build no
  longer exists - there is no `SConstruct` and the build is cmake. The Mover sources moved to
  `src/legacy/maszyna-mover` (`7c683ba3`). The files are ignored (`.gitignore:8 *.os`), so
  they are invisible in `git status`, but the directories mimic a source layout that is gone.
* **Fix:** `find src -name '*.os' -delete`, then remove the empty directories. No tracked file
  is touched.

## Correctness

### RC-002

**`BrakeMethod` reaches the Mover unmapped** ✔

* **Where:** `src/legacy/vehicles/MoverRailVehicleBrake.cpp:586-588`; the map is at
  `MoverRailVehicleBrake.hpp:50-56`
* **Rule:** correctness; "never do the same thing twice"
* **Problem:**
  * `lookup != brake_method_map.find(...)` compares a default-constructed iterator, which is
    undefined behaviour.
  * The next line overwrites the result with `BrakeMethod = get_brake_method()` anyway.
  * The Mover therefore receives the raw enum 0..9 instead of the mapped values
    1, 2, 9, 10, 11, 12, 14, 16, 17, 137. **No** brake method is mapped correctly, FR513
    included: the enum gives 4, the map says 11.
* **Fix:** a single `find()`, send the mapped value, and warn on a miss.

### RC-003

**Diesel backend forces test power source** ✔

* **Where:** `src/legacy/vehicles/MoverDieselEngineUnit.cpp` (was
  `MoverDieselEngineBackend.cpp:94-96`)
* **Rule:** correctness, magic value
* **Problem:** `// FIXME: test data` sets
  `p_mover->EnginePowerSource.SourceType = TPowerSource::Accumulator` for every diesel and
  diesel-electric vehicle, regardless of its FIZ.
* **Fix:** take the source type from the configuration (as the electric backend does), or
  remove the line after checking what the original sets (`Mover.cpp`, `LoadFIZ_PowerParamsDecode`).

### RC-004

**Configuration applied two to three times per pass** ✔

* **Where:**
  * `src/vehicles/base/VehicleController.cpp:148-155`
  * `src/vehicles/base/VehicleComponent.cpp:72-73`
  * `src/legacy/vehicles/MoverRailVehicleController.cpp:82-91, 481-483`
* **Rule:** "never do the same thing twice to be safe"; one road to one effect
* **Problem:**
  * `apply_configuration()` calls every `component->apply_config()` directly, then emits
    `simulation_configured`.
  * Every component is also connected to that signal with `apply_config`, so it runs a second
    time. The comment above the function says the signal "is not how the components are
    reached" - but it is.
  * On top of that, the Mover controller applies the configuration "a second time because
    `CheckLocomotiveParameters()` resets some parameters", although `apply_config()` already
    runs `CheckLocomotiveParameters` and `initialize_mover_state`. At startup each of these
    runs three times, and `simulation_configured`/`config_changed` are emitted twice.
* **Fix:** components are reached only by the direct loop, and the signal connection is
  removed. The order problem with `CheckLocomotiveParameters` is solved once, where it happens.
* **Partly 2026-09-30:** one more pass is gone - `MaszynaRailVehiclePhysicsNode` configured every
  vehicle a second time through a deferred `_reload`; the controller is now built once, on
  entering the tree (`VehiclePhysicsNode._build_controller()`). The three paths above are
  unchanged.

### RC-005

**Cargo list duplicated by repeated configuration**

* **Where:** `src/legacy/vehicles/MoverRailVehicleLoad.cpp:23`
* **Rule:** correctness (a consequence of RC-004)
* **Problem:** `p_mover->LoadAttributes.emplace_back(...)` never clears the list first, so
  every accepted cargo appears once per configuration pass - at least twice at startup.
* **Fix:** clear `LoadAttributes` at the start of `_apply_configuration`, in addition to RC-004.

### RC-006

**Radio call commands never unregistered** ✔

* **Where:** `src/vehicles/rail/RailVehicleRadio.cpp:102-103` (register) vs `:106-115`
  (unregister)
* **Problem:** `radio_call1` and `radio_call3` are registered but not unregistered. After a
  rebuild or a disable they point at a freed component, and registering again fails with
  "Command is already registered".
* **Fix:** unregister both, with the same bound callables.

### RC-007

**`e3d_loaded` connected again on every dirty frame** ✔

* **Where:** `src/vehicles/rail/RailVehicle3D.cpp:574-580`; also `_ready()` at `:454-458`
* **Rule:** no wiring in a hot path; one `connect`, one `disconnect`
* **Problem:** `_process_dirty()` connects `head_display_e3d`'s `e3d_loaded` every time it runs,
  with no disconnect. `_ready()` already connects every `E3DModelInstance` child to the same
  callable. The result is "already connected" errors, and the wiring happens in `_process`.
* **Fix:** wire once, where `head_display_e3d_path` is resolved on entering the tree, and
  disconnect on change or exit.
* **Done 2026-09-30:** `RailVehicle3D` is a proxy with no per-frame work; the head display is
  `RailVehicleRenderingServer.vehicle_set_head_display_material()`, applied when the model is built.

### RC-008

**Headlight colour 255 times overbright** ✔

* **Where:** `src/vehicles/rail/RailVehicleLighting.hpp:85`
* **Problem:** `Color(255, 255, 255)` - `Color` takes floats in 0..1, so this is white
  multiplied by 255.
* **Fix:** `Color(1, 1, 1)`.

### RC-009

**Main switch voltage defaults derived from zero**

* **Where:** `src/vehicles/rail/RailVehicleElectricEngine.hpp:125-132`
* **Rule:** correctness; magic numbers
* **Problem:** the default is
  `MAKE_MEMBER_GS(float, power_current_collector_min_main_switch_voltage, 0.5f * power_current_collector_max_voltage)`,
  computed at construction while `max_voltage` is still 0, so it is always 0 and never follows
  the authored value. The 0.5 and 0.6 ratios have no source reference.
* **Fix:** derive the value where `max_voltage` is set, or at apply time, with named ratios
  referenced to the original.

### RC-010

**Mover controller commands dereference a null backend**

* **Where:** `src/legacy/vehicles/MoverRailVehicleController.cpp:656-721` - `battery`,
  `converter`, `cab_*`, `*_controller_*`, `direction_*`
* **Problem:** these methods are bound and registered as commands, but call `mover->...` with no
  null check. Every getter of the class does check, and `is_simulation_ready()` exists because
  `mover` can be null. A command sent before the configuration lands crashes.
* **Fix:** the same guard the getters use - one condition per method.

### RC-011

**Singleton teardown leaks `TractionServer` and breaks the order** ✔

* **Where:** `src/register_types.cpp:473-482, 512-551`
* **Rule:** `CODE_STYLE.md` "Singletons C++" (unregister, then `memdelete`, then `nullptr`,
  per singleton)
* **Problem:**
  * `TractionServer` is unregistered and freed only inside
    `if (has_singleton("RailVehicleServer"))`. Without that server it leaks.
  * `RailVehicleServer`, `TrackServer`, `SimulationServer`, `E3DRenderingServer`,
    `SceneryStreamingServer`, `GameLog`, `E3DParser` and `UserSettings` are unregistered in one
    block and freed in another.
* **Fix:** one independent block per singleton, in reverse registration order.

### RC-012

**`MaszynaTranslationServer` dereferences singletons unchecked**

* **Where:** `src/utils/MaszynaTranslationServer.cpp:40, 54, 57, 74`
* **Rule:** `CODE_STYLE.md` "Singletons C++" - `get_instance()` does not guarantee a pointer
* **Problem:** `UserSettings::get_instance()->get_maszyna_game_dir()` and
  `SimulationServer::get_instance()->get_language()` are called without a null check.
* **Fix:** check each pointer once.

### RC-013

**Missing and mistyped Godot bindings**

* **Where:**
  * `src/vehicles/rail/RailVehicleBrake.cpp:223-231`: `BRAKE_METHOD_FR510` is not bound.
  * `RailVehicleBrake.hpp:241` + `.cpp:66`: `main_pipe_minimum_unblocking_handle_position` is an
    `int` with default `-3.0`, bound as `FLOAT`.
  * `RailVehicleDoors.cpp:71-74`: `VOLTAGE_AUTO` is not bound, and the `side` member is never
    bound.
* **Fix:** bind the missing constants and the member, and make the type consistent.

### RC-014

**State keys published before the simulation is ready**

* **Where:**
  * `src/vehicles/rail/RailVehicleElectricEngine.cpp:604-615`
  * `RailVehicleElectricInductionEngine.cpp:44`
  * `RailVehicleDieselElectricEngine.cpp:6-12`
  * `RailVehicleRadio.cpp:87-90`
* **Problem:** these components write state keys before, or without, the
  `is_simulation_ready()` check. The contract in `VehicleComponent.hpp:42-43` says nothing is
  published until then.
* **Fix:** move the writes behind the check.

### RC-015

**Generic component command result discarded**

* **Where:** `src/vehicles/base/GenericVehicleComponentNode.cpp:74-75`
* **Problem:** `component->send_command(...); return Variant();` - `send_command` returns
  `void`, so a modder's script never learns whether its command was accepted (#43).
* **Fix:** propagate the result, or remove the return value from the bound signature.

### RC-016

**Cab control drag signs lost without a mesh**

* **Where:** `src/cabin/CabinHUDMouseSystem.cpp:101`
* **Problem:** `controls[rid].drag_signs = p_drag_signs;` sits inside `if (mesh != nullptr)`,
  so the caller's drag signs are dropped when the mesh is not resolved.
* **Fix:** store them outside the mesh branch.

### RC-017

**`PlanarMirror3D` default outside its own range**

* **Where:** `src/rendering/PlanarMirror3D.hpp:25` vs `.cpp:66`
* **Problem:** the default `resolution_scale = 2.0` is outside the inspector range
  `"0.05,1,0.05"`.
* **Fix:** make the default and the range agree.

### RC-018

**`get_cache_dir` bound with a default for a missing argument**

* **Where:** `src/cache/ResourceCache.cpp:24`
* **Problem:** `get_cache_dir` takes no arguments but is bound with `DEFVAL("")`.
* **Fix:** remove the `DEFVAL`.

### RC-019

**Mover fields written by two components**

* **Where:**
  * `src/legacy/vehicles/MoverRailVehicleWheels.cpp:43-44` (`Mred`,
    `// FIXME: THIS IS MODIFICATION OF OTHER SECTION`), also written by the controller at
    `MoverRailVehicleController.cpp:449`
  * `MoverRailVehicleDieselElectricEngine.cpp:84,87` (`dizel_RevolutionsDecreaseRate`,
    `ShuntModeAllow`), already set by `MoverDieselEngineUnit.cpp`
    (was `MoverDieselEngineBackend.cpp:115,128`)
  * `MoverRailVehicleUniversalController.cpp:43` (`MainCtrlPos`, a runtime field written during
    configuration), also written by `initialize_mover_state` at `MoverRailVehicleController.cpp:64`
* **Rule:** one writer per field
* **Problem:** the result depends on the order the components are applied in.
* **Fix:** one owner per field.

## Separation of concerns and getters (ALARM)

### RC-020

**`wire_get_voltage()` changes the power source's state** ✔ ALARM

* **Where:** `src/traction/TractionServer.cpp:537`, through `PowerSource::current_get()` at
  `:123-140`; the "Quirk" is at `:108-115`
* **Rule:** a getter never changes state; never work around a mistimed event
* **Problem:**
  * The read adds to `total_admittance`, sets `loaded`, `total_current` and `output_voltage`,
    and resets `fuse_timer`.
  * The voltage therefore depends on how many readers ask per tick. The caller is
    `RailVehicleServer::vehicle_collect_current()`, once per pantograph.
  * It is a port of `TTraction::VoltageGet` (`Traction.cpp:470`). The loads ask from render
    frames while the sources tick on their own beat, and the "Quirk" keeps the previous load to
    cover that - a workaround for the timing.
* **Decision:**
  * (a) Split the read into a named operation, e.g. `wire_draw_current()` in the vehicle tick,
    and a pure `wire_get_voltage()`.
  * (b) Move the loads' query into the traction tick, so the order is the original's.

### RC-021

**Vehicle server calls the scene node** ALARM

* **Where:** `src/vehicles/rail/RailVehicleServer.cpp:1225-1228`, include at `:11`,
  `rail_vehicle_id` at `.hpp:110-112`
* **Problem:** the server (lower layer) includes `RailVehicle3D.hpp`, keeps the node's id and
  calls `rail_vehicle->apply_track_placement()` from its tick.
* **Decision:** the server publishes the placement (a signal or a pull by the node), and the node
  applies it.
* **Done 2026-09-30:** `RailVehicleServer` includes no node and keeps no node id; it emits
  `vehicle_placed` (`vehicle_set_track()`) and `vehicle_placement_changed`
  (`vehicle_report_placement()`, which replaced `vehicle_apply_placement()`), and
  `RailVehicleRenderingServer` moves the node the vehicle is drawn at.
  `vehicle_attach_rail_vehicle()`/`vehicle_get_rail_vehicle()` are gone.

### RC-022

**Drawing node runs pantograph physics and writes simulation inputs** ALARM

* **Where:** `src/vehicles/rail/RailVehicle3D.cpp:1479-1487` (`set_pantograph_wire_voltage`),
  `:382-383` (`set_power_current_collector_first_position`), `:1523-1593` (raise physics),
  `:1598-1642` (wire search)
* **Problem:** the rendering node computes simulation inputs and writes them into the electric
  engine; the FIXME(#184) comments admit it.
* **Decision:** move the pantograph model into the vehicle layer (component or server), and keep
  the node for drawing only.
* **Done:** `RailVehicleServer` keeps each vehicle's pantographs (`Pantograph` in its placement):
  where they stand, the arms, how far they are raised, the span each is on; its step raises them,
  follows the wire and feeds the voltage. `RailVehicle3D` measures the model's arms, hands them
  over (`vehicle_set_pantograph_geometry()`) and draws `vehicle_get_pantograph_raise()`. The
  collector position lives there alone (`vehicle_get_pantograph_position()`); the electric
  engine's copy is gone. A model rebuilt - the player's cab entered, a detail switch - no longer
  lowered the pantograph (FINDINGS.md 2026-09-29).

### RC-023

**Base vehicle layer knows rail and lighting** ALARM

* **Where:** `src/vehicles/base/VehicleController.hpp:14-18, 209`; includes at
  `VehicleController.cpp:3-5`
* **Problem:** the base controller forward-declares rail classes and holds
  `RailVehicleLighting *lighting`, `prev_roof_light_enabled` and the `roof_light_changed` signal,
  all of which exist only for the low-poly interior's glow (relayed as
  `VehicleServer.vehicle_roof_light_changed` to `RailVehicleRenderingServer`, formerly
  `RailVehicle3D`).
* **Decision:** the lighting component announces its own change; the base controller loses the
  rail knowledge.
* **Done 2026-09-30 (lighting):** the cab light and the instrument light are the cab's
  (`LegacyCabinCabLights`, `CabinState`), not the vehicle's: `lighting`, `prev_roof_light_enabled`,
  `roof_light_changed` and `VehicleServer.vehicle_roof_light_changed` are gone, and the low-poly
  cabs are lit by `CabinSystem.cab_set_light_level()`. The rail forward declarations remain.

### RC-024

**Controller and server call each other; `get_state()` builds a cache** ALARM

* **Where:**
  * `src/vehicles/rail/RailVehicleController.cpp` (`get_state()` →
    `VehicleServer::vehicle_dump_state()`), cache written in `VehicleServer::vehicle_dump_state()`
  * `src/vehicles/base/VehicleController.cpp` (`VehicleServer::vehicle_set_name`,
    `RailVehicleServer::vehicle_get_transform` - the base layer reaching the rail one, RC-023)
  * `VehicleComponent.cpp:219`, `RailVehicleWheels.cpp:18`
* **Rule:** layers do not call each other both ways; a getter never changes state
* **Problem:** the server steps controllers and components, and they call back into it. The
  getter `get_state()`, also the `state` property, writes `state_dump`, `state_dump_serial` and
  `state_dump_valid`. The comment admits the call "would recurse".
* **Decision:** one direction of calls; the dump is built in the tick, not in the getter.

### RC-025

**`vehicle_get_transform()` writes a cache** ALARM

* **Where:** `src/vehicles/rail/RailVehicleServer.cpp:753-754`
* **Rule:** a getter never changes state
* **Problem:** the getter sets `placement->body_transform` and `body_transform_valid`.
  `VehicleController::get_world_transform()` reaches it too.
* **Fix:** compose the transform where the placement changes, in the step.

### RC-026

**Drawing node creates a second vehicle RID** ALARM

* **Where:** `src/vehicles/rail/RailVehicle3D.cpp:438`, swap at `:402-414`, re-attach at
  `:420-422`
* **Rule:** one owner; one road to one effect
* **Problem:** `RailVehicle3D` creates its own RID with `server->vehicle_create()`, then swaps
  it for the one from `VehiclePhysicsNode`. That produces the `rid_owned` dual-ownership model
  and re-attaches a controller the server already has.
* **Decision:** the RID is created only by `VehiclePhysicsNode`/the server, and the drawing node
  only receives it.
* **Done 2026-09-30:** `VehiclePhysicsNode` creates the vehicle and its controller in
  `VehicleServer` (`controller_configure`, `vehicle_bind_controller`); `RailVehicle3D` creates no
  RID, takes the node's when it is built and only attaches it to `RailVehicleServer`;
  `vehicle_attach_controller` is gone.

### RC-027

**Mover member names as public state keys** ALARM

* **Where:**
  * `src/vehicles/rail/RailVehicleEngine.cpp:400-403`: `"Mm"`, `"Mw"`, `"Fw"`, `"Ft"`
  * `RailVehicleElectricEngine.cpp:609`: `"Im"`
  * `RailVehicleDieselElectricEngine.cpp:6`: `"Im"`
* **Rule:** the backend never appears in a public interface
* **Problem:** these keys are Mover member names in the public state dump, while every other key
  is descriptive `snake_case`.
* **Fix:** descriptive names such as `motor_torque` and `motor_current`, with their readers in
  GDScript updated.

### RC-028

**`RailVehicleHorns` includes the vendored Mover**

* **Where:** `src/vehicles/rail/RailVehicleHorns.cpp:2`
* **Problem:** `#include "legacy/maszyna-mover/utilities.h"` in an interface class, and nothing
  in the file uses it.
* **Fix:** remove the include.

### RC-029

**`Mover*` classes public and instantiated from GDScript** ALARM

* **Where:** `src/register_types.cpp:240-306` (`GDREGISTER_CLASS(MoverRailVehicle*)`);
  `fiz_vehicle_builder.gd:163-208` (`MoverRailVehicleController.new()`)
* **Rule:** the backend never appears in a public interface
* **Problem:** the backend's name is part of the extension's API. `set_controller_implementation`
  exists precisely to hide it, but the FIZ importer bypasses it.
* **Decision:** register them as internal/abstract and create them through the implementation
  factory; or accept it for the FIZ importer and write that down.

### RC-030

**`Mover*` components hold state the Mover does not have** ALARM

* **Where:**
  * `src/legacy/vehicles/MoverRailVehicleWipers.hpp:14-23`: `wipers` ("the vendored Mover has no
    wipers at all")
  * `MoverRailVehicleWheels.hpp:25-27`: `wheel_angle_*_deg` ("vehicle layer's, not the Mover's")
  * `MoverRailVehicleDoors.hpp:62-63`: `mirror_left_position`
  * `MoverRailVehicleLighting.hpp`: `headlights_dimmed` (the cab lights went to the cab layer)
  * `MoverRailVehicleController.hpp:27-40`: tachometer, `distance_counter`
* **Rule:** "if this layer were replaced wholesale, would the field go with it?"
* **Problem:** replacing the backend would lose vehicle state.
* **Decision:** move the fields to the `RailVehicle*` interface components (as `TODO.md`'s #184
  design describes for the wiper positions).

### RC-031

**Brake backend keeps a rate only the sound needs** ALARM

* **Where:** `src/legacy/vehicles/MoverRailVehicleBrake.cpp:260-276`
* **Problem:** `local_brake_pressure_change_rate` is filtered (`0.9`/`0.1`) in the brake backend,
  and the comment says it exists for `maszyna_brake_sfx_event_factory.gd`'s hiss automation.
* **Decision:** the sound layer derives the rate from the pressure it already receives.

### RC-032

**Radio component calls up into the vehicle server** ALARM

* **Where:** `src/legacy/vehicles/MoverRailVehicleRadio.cpp:36-40`
* **Problem:** `RailVehicleServer::get_instance()->vehicle_emergency_signal_send(controller->get_rid())` - a
  component (lower layer) calls the server that owns vehicles.
* **Decision:** the component emits an event, and the server, or whoever cares, reacts.

### RC-033

**`E3DInstanceBackend` and `E3DRenderingServer` include each other** ALARM

* **Where:** `src/legacy/e3d/E3DInstanceBackend.cpp:2, 100`
* **Problem:** the backend uses `E3DRenderingServer::INSTANCE_KIND_DYNAMIC`, so the two layers
  depend on each other.
* **Fix:** move the enum to a header shared by both, or into the backend.

### RC-034

**Driver layer tracks player-controlled vehicles** ALARM

* **Where:** `src/driver/DriverSystem.hpp:52, 94-103`
* **Problem:** `HashSet<RID> player_controlled_vehicles` and `vehicle_is_driven()` ("its driver
  or a player") mean the driver layer stores and answers for player occupancy - the
  `PlayerServer` layer above it.
* **Decision:** player occupancy is owned by `PlayerServer`; whoever needs "driven by anyone"
  asks both.

### RC-035

**`Cabin3D` keeps a controller path it says it has not got** ALARM

* **Where:** `src/cabin/Cabin3D.hpp:43, 93-94` vs the comment at `:74`
* **Problem:** a bound `NodePath controller_path` to a `VehiclePhysicsNode`, although the class
  says "there is deliberately no path to a controller here" and already holds `vehicle_rid`. The
  cab has two ways to reach its vehicle.
* **Fix:** remove the path and use `vehicle_rid` only.

### RC-036

**`SimulationServer` holds cache, build and language** ALARM

* **Where:** `src/simulation/SimulationServer.hpp:87-88, 129, 167-168`; `.cpp:198, 236`
* **Problem:** `cache_clear()`, `build_get_number()`, `build_check_version()` and
  `set_language()`/`get_language()` sit on the simulation clock server. Cache invalidation,
  build stamps and UI language are not simulation state; the language belongs with
  `MaszynaTranslationServer`, which only relays it.
* **Decision:** which server owns cache and build, and move the language to translation.
* **Done 2026-09-30 (cache and build):** `GameDataServer` owns them (`cache_clear()`,
  `cache_clear_requested`, `build_get_number()`, `build_check_version()`), with the game's data
  reload. Left: the language still sits on `SimulationServer`.

### RC-037

**`track_get_endpoints()` fills a cache**

* **Where:** `src/tracks/TrackServer.cpp:836` → `_endpoints()` at `:256-268`
* **Rule:** a getter never changes state
* **Problem:** the getter lazily fills `cached_endpoints` from a `const` method.
* **Fix:** build the cache in `_set_curves`, which already clears it.

### RC-038

**`build_get_number()` reads a file and sets a flag**

* **Where:** `src/simulation/SimulationServer.cpp:242-247`
* **Rule:** a getter never changes state
* **Problem:** the non-const getter reads the file on first call and writes
  `build_number_read = true`.
* **Fix:** read the file once at initialisation.

### RC-039

**`MaszynaParser::get*()` advance the cursor**

* **Where:** `src/legacy/parsers/maszyna_parser.cpp:164-182, 305`
* **Rule:** a getter never changes state
* **Problem:** `get8`, `get_line` and `get_tokens` advance `cursor`. They mirror
  `FileAccess.get_8`, but they are `get_*` methods with a side effect.
* **Decision:** rename them to `read_*`, or accept the `FileAccess` idiom and note it.

## Missing events and wiring

### RC-040

**Cabin shown after counting frames**

* **Where:** `src/vehicles/rail/RailVehicle3D.cpp:205-212`
* **Rule:** never work around a mistimed event; no wiring in a hot path; no string callables
* **Problem:** `cabin_show_frames = 2` and a `CONNECT_ONE_SHOT` on `process_frame` with
  `Callable(this, "_show_cabin_after_frames")`, re-subscribed every frame until the count runs
  out.
* **Fix:** find what the cabin waits for and show it on that event.
* **Done 2026-09-30:** the cabin waited for nothing - it is built within `add_child()` and the
  camera is moved into it in the same frame; `show_cabin()` shows it at once, the counter is gone.

### RC-041

**`pending_start_track_retry` retry flag**

* **Where:** `src/vehicles/rail/RailVehicle3D.hpp:125`; `.cpp:620-622, 857-860, 1337`
* **Rule:** never work around a missing event
* **Problem:** the flag is retried from both `_process_dirty` and
  `_on_track_server_tracks_changed`, and gates `apply_track_placement`.
* **Fix:** place the vehicle on the event that says the tracks have landed, and only there.
* **Done 2026-09-30:** the flag and its retries are gone. `RailVehicle3D` places the vehicle on
  whichever comes last - its vehicle (`set_vehicle()`) or `TrackServer.tracks_changed`
  (`_place_on_start_track()`); a vehicle in a `TrainSet3D` is placed by
  `RailVehicleServer.trainset_place()`, which waits for `VehicleServer.vehicle_configured`.

### RC-042

**`force_detail_refresh` "try again next tick" flag**

* **Where:** `src/vehicles/rail/RailVehicle3D.cpp:838, 1280, 295, 1355-1358`
* **Rule:** `CODE_STYLE.md` "Never work around a missing event" - this exact flag is its
  example
* **Problem:** set on `screen_entered`, on a detail switch and on a config change, then consumed
  on a later tick.
* **Fix:** refresh the detail in the handler of each event.
* **Done 2026-09-30:** the flag is gone with `RailVehicle3D`'s tick. `RailVehicleRenderingServer`
  poses a vehicle on `vehicle_placement_changed`, rebinds its parts on the model's
  `instance_built`, and checks the detail distance in its bounded slow round.

### RC-043

**`RailVehicle3D` wires nodes inside `_process`**

* **Where:** `src/vehicles/rail/RailVehicle3D.cpp:508-514, 574-623`
* **Rule:** wiring is not per-frame work, not even behind `_dirty`
* **Problem:** `if (dirty) { _process_dirty(); }` does `get_node_or_null`, connect/disconnect and
  `_bind_vehicle_node()`; `_cache_animation_bindings()` re-resolves every node path there too.
* **Fix:** resolve and connect in `_enter_tree`/`_ready`, and on the setter's change when the
  node is in the tree.
* **Done 2026-09-30:** `RailVehicle3D` has no `_process`; it connects its scene parts on
  `NOTIFICATION_ENTER_TREE` and when a part path changes in the tree.

### RC-044

**Animation bindings resolved twice after reload**

* **Where:** `src/vehicles/rail/RailVehicle3D.cpp:1279`, `:697-702`
* **Problem:** `reload()` emits `e3d_loaded` synchronously (`e3d_model_instance.gd:168`), and
  `_on_model_node_e3d_loaded` caches the bindings. `animation_bindings_dirty = true` then makes
  them resolve again next frame.
* **Fix:** remove the dirty flag set after `reload()`.
* **Done 2026-09-30:** the bindings are `RailVehicleRenderingServer`'s, resolved once per built
  model (`_bind_parts()` on `instance_built`); there is no dirty flag.

### RC-045

**Component enable and power source land a tick late through flags**

* **Where:** `src/vehicles/base/VehicleComponent.cpp:140-165, 201-204`;
  `RailVehicleElectricEngine.cpp:754-757`
* **Rule:** one road to one effect; no deferral
* **Problem:** `set_enabled` sets `enabled_changed` and `dirty`, consumed by `process()` on the
  next tick, so commands and config land a tick late. `mark_dirty` is bound, which gives a second
  road to `apply_config`. `set_power_source` does the same.
* **Fix:** apply at once in the setter's owner operation, and remove the bound `mark_dirty`.

### RC-046

**Lazy `owner_create` inside the streaming build**

* **Where:** `src/legacy/e3d/E3DRenderingServer.cpp:493-498, 621-625, 830-834`
* **Rule:** no `ensure_*` under any name; no wiring in a hot path
* **Problem:** `if (light_stream_owner < 0) { light_stream_owner = streaming->owner_create(...) }`
  wires callables into `SceneryStreamingServer` from `_light_create` and
  `_build_instance_smoke_sources`, which run inside the per-frame build.
* **Fix:** create the owners once at server initialisation.

### RC-047

**`build_check_version()` once-guard called "to be sure"**

* **Where:** `src/simulation/SimulationServer.cpp:254-259`; callers `demo_3d.gd:6`,
  `demo_scenery_loading.gd:34`, `startup.gd:15`
* **Rule:** no `ensure_*`; never do the same thing twice
* **Problem:** `if (build_version_checked) return false;` - an ensure-style guard that clears
  caches and writes a setting, called from three scenes.
* **Fix:** one call at startup by the owner; the scenes stop calling it.

### RC-048

**Loading queue waits by sleeping and is polled**

* **Where:** `src/scenery/SceneryLoadingTaskQueue.cpp:64-66, 97`; pollers
  `scenery_instancer.gd:450, 607, 612`
* **Rule:** never work around a missing event
* **Problem:** `wait()` loops on `OS::delay_usec(100)`, and `is_done()` exists so callers can
  poll it.
* **Fix:** a "done" signal, and a condition variable in `wait()`.

### RC-049

**`area_is_ready()` has no event and is polled per frame**

* **Where:** `src/scenery/SceneryStreamingServer.cpp:300`; poller
  `demo_scenery_loading.gd:94`
* **Problem:** there is no "area ready" signal, so readiness can only be polled.
* **Fix:** emit a signal when the requested area has been built.

### RC-050

**Lazy initialisation re-checked on every call**

* **Where:**
  * `src/legacy/vehicles/MoverRailVehicleWipers.cpp:29-33` (`switch_initialized`, checked on
    every configuration apply)
  * `src/legacy/cabin/PythonScreenServer.cpp:302-319` (the worker thread is started lazily in
    `screen_create`)
* **Rule:** no `ensure_*` under any name - state is initialised where it is created
* **Fix:** initialise in the constructor or the server's initialisation.

## Per-frame work

### RC-051

**Every `RailVehicle3D` processes every frame**

* **Where:** `src/vehicles/rail/RailVehicle3D.cpp:72, 310, 527, 641-666`
* **Rule:** nothing while idle; no lookups or allocations per frame
* **Problem:** `set_process(true)` is never switched off. Every vehicle, off-screen too, runs
  `_sync_lights_from_controller` each frame: a `keys()` allocation, nested string compares and
  `model_node->call("is_e3d_loaded")`. It also calls `Engine::get_singleton()` each frame.
* **Fix:** drive the lights by the lighting change event (see RC-023), and turn processing off
  while nothing moves.
* **Done 2026-09-30:** `RailVehicle3D` does no per-frame work. `RailVehicleRenderingServer` does
  the drawing on `process_frame`, bounded round-robin (`MAX_SLOW_UPDATES_PER_FRAME` vehicles for
  detail, lamps and smoke, `MAX_DETAILED_UPDATES_PER_FRAME` detailed ones for pantographs, wipers
  and mirrors), and connects only while it draws a vehicle. Still per slow visit: the component
  lookups by type and the detail-distance setting (RC-056).

### RC-052

**Whole config dictionary built per frame for the wiper angle**

* **Where:** `src/vehicles/rail/RailVehicle3D.cpp:1168, 1173`
* **Problem:** `controller->get_config().get("wipers_angle", 0.0)` builds the whole config
  dictionary per frame while the wipers move, although `RailVehicleWipers::get_angle()` exists.
  A `PackedFloat64Array` is also copied per frame.
* **Fix:** read the typed getter of the cached component.
* **Done 2026-09-30:** `RailVehicleRenderingServer::_pose_wipers()` reads
  `RailVehicleWipers::get_angle()`/`get_sweep_positions()` and poses only when the positions
  changed (the positions array is still copied per visit).

### RC-053

**Coupler lookups and string building per frame**

* **Where:** `src/vehicles/rail/RailVehicle3D.cpp:1045-1047, 1068-1072, 1119, 1135-1136`
* **Problem:** `_coupler()` searches the components linearly, `get_pneumatic_layout` builds
  strings and calls `Dictionary::has`, and `_pneumatic_variant` looks up the server by name and
  the neighbour in ObjectDB - every frame.
* **Fix:** cache the coupler and the layout when the coupling changes.
* **Done 2026-09-30:** the couplers are drawn by `RailVehicleRenderingServer::_update_couplers()`
  on `vehicle_trainset_changed`/`vehicle_coupler_attached`/`detached` and on a built model, not
  per frame.

### RC-054

**Bogie track samples computed twice per frame**

* **Where:** `src/vehicles/rail/RailVehicleRenderingServer.cpp` (`_pose_running_gear()`, was
  `RailVehicle3D.cpp:1234-1237, 1376-1385`); `RailVehicleServer::_compose_body_transform()`
* **Problem:** `wheels->get_bogie_transform(...)` repeats the ±half-spacing samples that
  `_compose_body_transform` has just made, and the wheels component is looked up on every
  placement change of a detailed vehicle.
* **Fix:** the server publishes the bogie transforms together with the body transform.

### RC-055

**Pantograph geometry through string-keyed dictionaries per frame**

* **Where:** `src/vehicles/rail/RailVehicle3D.cpp:1501, 1563-1589, 1601`
* **Problem:** the geometry is read and written through string keys
  (`double(p_geometry["pant_wys"])`), `pantograph_wire_cache` entries are copied,
  `TractionServer::get_instance()` is looked up per pantograph, and `_pantograph_frame()` is
  rebuilt per arm.
* **Fix:** a typed struct cached at configuration (and RC-022).
* **Done:** with RC-022 - `RailVehicleServer::Pantograph`, one span per pantograph (the node
  searched twice at the same point), the frame from the placement once a step.

### RC-056

**`ProjectSettings` read per vehicle every 0.25 s**

* **Where:** `src/vehicles/rail/RailVehicleRenderingServer.cpp` (`_update_detail()`, was
  `RailVehicle3D.cpp:1261`)
* **Problem:** `ProjectSettings::get_singleton()->get_setting("maszyna/vehicles/detail_distance")`
  runs per vehicle on every slow visit.
* **Fix:** read it once and refresh on `ProjectSettings.settings_changed`.

### RC-057

**Allocations and boxing in the vehicle server step**

* **Where:** `src/vehicles/rail/RailVehicleServer.cpp:1092, 1108-1173, 1111, 1226`
* **Problem:** `track_vehicles` (a map of vectors) is cleared and rebuilt every frame,
  `stepped_controllers` is a `TypedArray` re-cast in five loops, and there is an ObjectDB lookup
  per vehicle.
* **Fix:** keep the structures across frames and update them on placement change, with a
  native vector of controllers.

### RC-058

**Track roll read by property name per moved vehicle**

* **Where:** `src/vehicles/rail/RailVehicleServer.cpp:837-838, 863-864`
* **Rule:** per-frame lookups; a string call only with a comment saying why
* **Problem:** `curve_data->get("roll1")` per moved vehicle per frame. It is allowed because
  `TrackCurve` is GDScript, but there is no comment.
* **Fix:** copy the roll into the track data in `TrackServer` when the curve is set, and add
  the comment if the lookup stays.

### RC-059

**Unbounded E3D animation loop with allocations**

* **Where:** `src/legacy/e3d/E3DRenderingServer.cpp:1182-1235`
* **Problem:** the loop over `animating_instances` has no bound; it allocates a
  `PackedStringArray` per instance, looks up `Engine`, the main loop and the root per frame
  (`get_root()->get_process_delta_time()`), and clears and rebuilds `submodel_poses` through
  `_pose_submodels`.
* **Fix:** a per-frame budget, round-robin like `_process_smoke()`, and cached lookups.

### RC-060

**Time and light level each walk all E3D instances**

* **Where:** `src/legacy/e3d/E3DRenderingServer.cpp:1406-1441`; caller
  `maszyna_environment_node.gd:452-453`
* **Problem:** `environment_set_time()` and `environment_set_light_level()` each call `_resolve_all_lights()`,
  which walks every instance and allocates a `Dictionary` per instance. The environment calls
  both back to back, so every push costs two full passes.
* **Fix:** one operation that sets both and resolves once, with no per-instance dictionary.

### RC-061

**Smoke emitters looked up and ticked while idle**

* **Where:** `src/legacy/e3d/E3DRenderingServer.cpp:1016-1032, 1255-1277`
* **Problem:** a hash lookup per emitter (`smoke_objects.getptr(smoke_order[...])`), and
  `RenderingServer`/`Time` singleton lookups per frame. Invisible or zero-intensity dynamic
  emitters keep being ticked.
* **Fix:** cache the singletons, take idle emitters out of the order, and keep pointers in the
  order.

### RC-062

**`E3DOptimizedBackend::update()` allocates a map**

* **Where:** `src/legacy/e3d/E3DOptimizedBackend.cpp:37`
* **Problem:** `HashMap<E3DSubModel *, bool> overrides` is allocated, and every RID is walked,
  on each call - reached from blink edges in `_process_lights` and from `_resolve_all_lights`.
* **Fix:** keep the map as a member and update only what changed.

### RC-063

**Scenery streaming runs idle with an unbounded loop**

* **Where:** `src/scenery/SceneryStreamingServer.cpp:384-409, 428`
* **Problem:** `_process_streaming()` runs every frame while a camera is set, even when idle.
  `_apply_plan()` calls `_is_area_ready_locked(1)`, which loops over every entry of the 3×3
  nearby chunks, and adds ObjectDB, `Time` and mutex work.
* **Fix:** stop when the plan is fulfilled and restart on camera cell change; keep a ready
  counter instead of the loop.

### RC-064

**Scenery streaming entry found by linear scan**

* **Where:** `src/scenery/SceneryStreamingServer.cpp:446, 468, 478`
* **Problem:** `_get_entry()` scans the chunk's entries linearly, up to twice per pending build,
  inside the per-frame budget loop.
* **Fix:** index the entries by id.

### RC-065

**`TractionServer` ticks forever**

* **Where:** `src/traction/TractionServer.cpp:52, 69-76`
* **Problem:** the `process_frame` connection is permanent. Every frame it looks up the main
  loop and the root and loops over all power sources, even with no sources or no load.
* **Fix:** connect only while there are sources with a load, and cache the tree.

### RC-066

**`Cabin3D` processes forever**

* **Where:** `src/cabin/Cabin3D.cpp:126, 131, 145`
* **Problem:** `set_process(true)` is never switched off. Every 1/50 s step does a
  `RailVehicleServer::get_instance()` lookup and a `vehicle_component_get()`, even for a cab
  whose vehicle has no diesel engine.
* **Fix:** resolve the engine when `vehicle_rid` changes, and do not process without one.

### RC-067

**`PlanarMirror3D` sets shader parameters every frame**

* **Where:** `src/rendering/PlanarMirror3D.cpp:159, 166, 173, 188, 209-211`
* **Problem:**
  * Out of view, `_set_rendering(false)` calls `set_shader_parameter("reflecting")` every
    frame.
  * In view, it calls `set_keep_aspect_mode()` (a constant) and builds the string-named
    `"mirror_view_projection"` every frame.
* **Fix:** set on change only, with the constant set once and a cached `StringName`.

### RC-068

**`SimulationServer::get_instance()` looked up per frame**

* **Where:** `src/driver/DriverSystem.cpp:88`; `src/scenario/ScenarioEventServer.cpp:256`
  (`SimulationClock.cpp` removed 2026-09-30 - the clock ticks inside `SimulationServer`)
* **Problem:** the lookup is a name lookup on `Engine`, done every frame or every slice.
* **Fix:** cache the pointer at initialisation.

### RC-069

**Wipers ticked while parked**

* **Where:** `src/legacy/vehicles/MoverRailVehicleWipers.cpp:51-104`
* **Problem:** the loop runs every tick (`wiper.out_timer += p_delta`), even when every wiper is
  parked and switched off.
* **Fix:** skip, or turn off, while every wiper is parked and switched off.

## Raw pointers in public API

### RC-070

**`E3DRenderingServer::instance_attach_node(Node3D *)`**

* **Where:** `src/legacy/e3d/E3DRenderingServer.hpp:327`
* **Rule:** a public API takes RIDs, Variants, Callables and `ObjectID`s
* **Fix:** `instance_attach_object_instance_id(RID, uint64_t)`, like
  `vehicle_attach_object_instance_id`.

### RC-071

**`SceneryStreamingServer::streaming_set_camera(Camera3D *)`**

* **Where:** `src/scenery/SceneryStreamingServer.hpp:187`, `.cpp:19`
* **Problem:** a bound server method with a raw pointer; the two HUD mouse servers take an
  `ObjectID` for the same thing.
* **Fix:** take a `uint64_t` `ObjectID`, like the HUD mouse servers (and RC-112 for the name).

### RC-072

**`RailVehicleServer::vehicle_component_get()` returns a pointer**

* **Where:** `src/vehicles/rail/RailVehicleServer.hpp:296`
* **Fix:** return an `ObjectID`/`Object` Variant publicly, and keep a pointer accessor private
  for native callers.
* **Done 2026-09-30:** returns `Ref<VehicleComponent>` - components are `RefCounted` now.

### RC-073

**`SignalHeadNode::set_model(Node *)` / `get_model()`**

* **Where:** `src/signalling/SignalHeadNode.hpp:57-58`
* **Problem:** bound with raw pointers, although the class stores an `ObjectID` inside.
* **Fix:** take and return an `ObjectID`.

### RC-074

**`MaszynaTrianglesImporter::import_triangles(MaszynaParser *)`**

* **Where:** `src/legacy/scenery/MaszynaTrianglesImporter.hpp:15`
* **Problem:** a bound static method with a raw pointer to a `RefCounted`.
* **Fix:** `Ref<MaszynaParser>`.

### RC-075

**Vehicle layer bound methods taking and returning pointers**

* **Where (left):** none.
* **Done 2026-09-30:** `update_neighbour` and `couple` take a `Ref<RailVehicleController>` and
  typed `CouplerEnd`s (the "nothing found" case is `clear_neighbour(end)`); `get_cabin` left
  `RailVehicle3D` for `CabinSystem.vehicle_get_cabin(vehicle_rid)`.
* **Done 2026-09-30:** `VehicleController` and `VehicleComponent` are `RefCounted`; `add_component`,
  `get_component`, `get_controller` (`RailVehicle3D`, `VehicleComponent`,
  `GenericVehicleComponentNode`, `VehiclePhysicsNode`), `get_coupled_controller` and
  `GenericVehicleComponentNode::get_component` take and return `Ref<>`. A raw `T*` of a
  `RefCounted` returned to GDScript took a reference away and freed the vehicle (FINDINGS.md
  2026-09-30).
* **Done 2026-09-30:** `VehicleModel`, `VehicleComponentModel` and their `capture`/`apply`
  (`Object *`) are gone - the controller and its components are `Resource`s and the description
  of a vehicle themselves.
* **Fix:** RIDs (the vehicle), `ObjectID`s, `Ref<>`, or `Variant`.

### RC-076

**`E3DSubModel::set_parent(E3DSubModel *)`**

* **Where:** `src/legacy/e3d/E3DSubModel.hpp:100`
* **Problem:** a public C++ method with a raw pointer. It is not bound, so this is low priority.
* **Fix:** make it private or a friend of the parser, or take an index.

### RC-123

**Scene-tree nodes holding pointers to objects they do not own**

* **Rule:** a node living in the scene tree keeps no pointer to another object - a node as an
  `ObjectID`, the vehicle by its RID (FINDINGS.md 2026-09-30, "Edit FIZ" aborted the editor)
* **Where (left):** `src/rendering/PlanarMirror3D.cpp`: `glass` (`MeshInstance3D *`, its parent);
  the rest of the codebase not yet swept
* **Done 2026-09-30:** `RailVehicle3D` (model, bogies, wheels, pantographs, wipers, mirrors,
  cabin, load, head display; no components at all - read from `RailVehicleServer` by RID),
  `GenericVehicleComponentNode` (`Ref<GenericVehicleComponent>`), `VehiclePhysicsNode`
  (`Ref<VehicleController>`).

## Calls by name

### RC-077

**GDScript `E3DModelInstance` calls without the required comment** ✔

* **Where:** `src/vehicles/rail/RailVehicle3D.cpp:615, 626, 642, 765, 800, 803, 1272, 1276, 1293,
  1298, 1333`
* **Rule:** a string call only where the class cannot be known at build time, **with a comment
  saying so**
* **Problem:** `model_node->call("is_e3d_loaded" | "reload" | "set_smoke_intensity" | "get_aabb")`
  is allowed, since `E3DModelInstance` is GDScript, but only `:416, 600, 667` carry the comment.
* **Fix:** one comment at the `model_node` declaration, or at each call. Cache `StringName`s.
* **Done 2026-09-30:** `RailVehicle3D` names `E3DModelInstance` three times
  (`get_e3d_instance`, and `e3d_instance_created` connected and disconnected), each with the
  comment; the rest went with the node's drawing code.

### RC-078

**Commands registered with `Callable(this, "name")`**

* **Where:** about 150 sites - every `_register_commands`/`_unregister_commands`:
  * `src/vehicles/rail/RailVehicle{Controller,Engine,ElectricEngine,DieselEngine,Brake,Lighting,Doors,SecuritySystem,Radio,SpeedControl,SpringBrake,ElectroPneumaticDynamicBrake,Horns,Heating,Switches,Wipers}.cpp`
  * `src/legacy/vehicles/MoverRailVehicle*Engine.cpp:66-73` and the other `Mover*`
  * `VehicleComponent.cpp:73, 91`
  * (`RailVehicle3D.cpp`'s went with its rewrite, 2026-09-30)
* **Rule:** call a method, do not name it
* **Problem:** the class is known, so a rename or typo silently yields a callable to nothing.
* **Fix:** `callable_mp(this, &Class::method)`.

### RC-079

**Signal names as literals despite constants**

* **Where:**
  * ~~`src/vehicles/rail/RailVehicle3D.cpp:391, 397`: `connect("roof_light_changed", ...)`~~ -
    gone 2026-09-30; `VehicleServer` connects with the constant
  * `src/legacy/vehicles/MoverRailVehicleSecuritySystem.cpp:20, 24`:
    `emit_signal("blinking_changed")`
* **Fix:** use the constants, and add one where it is missing.

## Magic numbers

All items below break the PROHIBITED rule: "a non-self-evident literal gets a named constant; a
ported value keeps the original's value and a source reference".

### RC-080

**`RailVehicle3D`**

* **Where:** `src/vehicles/rail/RailVehicle3D.cpp`:
  * `:518`: `0.25` s interval
  * `:73`: `resize(4)` with slots 0-3 used by convention at `:1472-1473, 1527-1533`
  * `:904, 915`: arm count `5`
  * `:938`: `HEIGHT = 0.07`, no source
  * `:1138`: `* 5`
  * `:749, 751`: cab count `3`
  * `:1312-1323`: smoke `4.0`, `0.01`, `60.0`, `0.005`, `0.02`
  * `:1551-1592`: pantograph `2.45`, `3.45`, `0.015`, `0.001`, `0.55`, `0.4`, `0.15`, `0.01`,
    with no per-value reference
* **Done 2026-09-30:** the code left the node. The pantograph values are named constants of
  `RailVehicleServer` (`PANTOGRAPH_*`), the smoke ones of
  `RailVehicleRenderingServer::_update_smoke()` (`particles.cpp:188-205`); the cab, wiper and
  pantograph element counts and the detail interval and budgets are named in
  `RailVehicleRenderingServer`.

### RC-081

**`Cabin3D`**

* **Where:**
  * `src/cabin/Cabin3D.cpp:160, 165, 177-179`: `/ 60.0`, `* 4.0`, `* 1.0625`, `/ 200.0`, `* 100.0`.
    `:178-179` are ported from `DynObj.cpp:8028` without a reference.
  * `:225`: `cab_window_open ? 3 : ...`
  * `Cabin3D.hpp:29-30, 49-58`: `SHAKE_STEP`, `SPRING_REST_LENGTH`, `shake_spring_stiffness = 125.0`
    (`DynObj.cpp:2284`) and the other ported defaults, without file:line

### RC-082

**`TractionServer`**

* **Where:** `src/traction/TractionServer.cpp`:
  * `:119, 121, 126, 135, 547`: `1e-10`, `< 100.0`, `* 1.083`, `10000.0`. These come from
    `TractionPower.h:58`, `TractionPower.cpp:122, 128` and `Traction.cpp:477`, but are neither
    named nor referenced.
  * `:231, 264, 269, 294`: `grow(5.0)`, `last_flags |= 1`, `|= 2` - unnamed bits, although
    `LAST_SPAN_FLAGS` exists
  * `:321`: `0.0, 0.2, ..., 1.0, 3, 60.0` repeats the `PowerSource` defaults
    (`TractionPower.h:50-54`) as bare literals

### RC-083

**`TrackServer`**

* **Where:**
  * `src/tracks/TrackServer.hpp:155`: `switch_f_offset1 = -0.05` instead of
    `-SWITCH_OFFSET_DELAY` (`Track.cpp:57`)
  * `.hpp:92, 94, 108, 110, 116, 137`: `SWITCH_OFFSET_DELAY` (`Track.h:71`), `RAIL_HEIGHT`,
    `SWITCH_FULL_DURATION`, `SWITCH_BLADE_RATIO`, `ROLL_FIX_FACTOR` and `width = 1.6`, with no
    source reference
  * `.cpp:34`: `10.0` repeats the header's default

### RC-084

**`MoverRailVehicleBuffCoupl`**

* **Where:** `src/legacy/vehicles/MoverRailVehicleBuffCoupl.cpp:80-99`
* **Problem:** `SpringKC = 50.0 * mass + max_velocity / 0.05`, `4500 * 1000`, `0.55` - only the
  function has a reference (`Mover.cpp:10297`); none of the values are named.

### RC-085

**`MoverRailVehicleBrake`**

* **Where:** `src/legacy/vehicles/MoverRailVehicleBrake.cpp`:
  * `:553`: `CLAMP(..., 0, 4)`
  * `:556`: `* 1000.0`
  * `:578`: `100 * M_PI`
  * `:231, 563, 566`: `< 0.01`, three times
  * `:596-597`: `5 + 0.001 * (randf_range(0, 10) - randf_range(0, 10))`, no source

### RC-086

**`MoverRailVehicleController`**

* **Where:** `src/legacy/vehicles/MoverRailVehicleController.cpp`:
  * `:205`: `CategoryFlag == 2 ? 50 : 100`
  * `:361-384`: `11.31`, `1.05`, `3.0`, `0.66`. `max_tachometer = 3.0` is declared, yet `3.0`
    is repeated at `:377, 381`.
  * `:423`: `index < 6` instead of the array size

### RC-087

**Warning signal bitmasks in horns and security system**

* **Where:** `src/legacy/vehicles/MoverRailVehicleHorns.cpp:17-107`;
  `MoverRailVehicleSecuritySystem.cpp:110-119`
* **Problem:** `WarningSignal |= 4` and `EmergencyBrakeWarningSignal = 4` - the bits 1/2/4 have
  no names.

### RC-088

**`MoverCircuitUnit` thresholds**

* **Where:** `src/legacy/vehicles/MoverCircuitUnit.cpp` (was
  `MoverElectricEngineBackend.cpp:192, 200, 213`)
* **Problem:** `BrakePress < 1.0` and `RventRot < 5.0`, with no reference.

### RC-089

**`MoverRailVehicleWheels`**

* **Where:** `src/legacy/vehicles/MoverRailVehicleWheels.cpp:13, 41`
* **Problem:** `const double k = 472.0` has no source reference; `1.0` m is used as a fallback
  diameter.

### RC-090

**`/ 60.0` rpm conversion repeated in six files**

* **Where:**
  * `src/legacy/vehicles/MoverDriveUnit.cpp` (was `MoverEngineBackend.cpp:57`)
  * `MoverDieselEngineUnit.cpp` (was `MoverDieselEngineBackend.cpp:166, 218, 231`)
  * `MoverRailVehicleElectricSeriesEngine.cpp:82`
  * `MoverRailVehicleHeating.cpp:25-26`
  * `src/cabin/Cabin3D.cpp:160`
* **Fix:** one `SECONDS_PER_MINUTE` constant.

### RC-091

**Speed control preset count and doors remote control value**

* **Where:**
  * `src/legacy/vehicles/MoverRailVehicleSpeedControl.cpp:20`: `MAX_PRESET_SPEEDS = 10`
    instead of the size of `SpeedCtrlButtons`
  * `MoverRailVehicleDoors.cpp:334`: `remote_control ? 24 : 0`

### RC-092

**`E3DNodesBackend` light settings defaults**

* **Where:** `src/legacy/e3d/E3DNodesBackend.cpp:246-253`
* **Problem:**
  * The literal `"maszyna/vehicles/lights_volumetric_fog_energy", 4.0`.
  * `"maszyna/lights/reverse_cull_face"` duplicates
    `E3DRenderingServer::LIGHTS_SHADOW_REVERSE_CULL_FACE_SETTING`.
  * The fade values `150.0`, `100.0` and `200.0` are unnamed.

### RC-093

**Microsecond conversions and E3D distances**

* **Where:**
  * `src/legacy/e3d/E3DRenderingServer.cpp:1259`: `/ 1000000.0`, although `USEC_PER_SECOND` exists
  * `src/tracks/TrackServer.cpp:469`: `/ 1000000.0`
  * `E3DRenderingServer.cpp:693`: `distance * 0.25f`

### RC-094

**`e3d_parser` flags and offsets**

* **Where:** `src/legacy/e3d/e3d_parser.cpp`:
  * `:146, 169, 174, 183`: `- 168`, `/ 256`, `/ 320`, `/ 64`
  * `:114` and `:430`: `flags & 32` and `(1 << 5)` - the same flag written two ways
  * `:144`: `flags & 0b000001` labelled "transparent", which contradicts bit 32

### RC-095

**`MaszynaParser` characters and buffer size**

* **Where:** `src/legacy/parsers/maszyna_parser.cpp:176`
* **Problem:** `c == 10 || c == 13` instead of `'\n'`/`'\r'`, and a `[128]` buffer size repeated
  six times.

### RC-096

**Scenery loading and streaming timings**

* **Where:**
  * `src/scenery/SceneryLoadingTaskQueue.cpp:97, 108`: `delay_usec(100)`,
    `get_processor_count() - 2`
  * `src/scenery/SceneryStreamingServer.cpp:433`: `>= 1000`

### RC-097

**Vehicle server, controller and coupler literals**

* **Where:**
  * `src/vehicles/rail/RailVehicleServer.cpp:823, 830`: `0.000001`, `0.999`
  * `src/vehicles/base/VehicleController.hpp:211`: `1e10`
  * `VehicleController.cpp:207`: 1.0 m threshold
  * ~~`src/vehicles/rail/RailVehicleBuffCoupl.hpp:56`: `power_coupling, 128`~~ - done
    2026-09-30, `RailVehicleController::COUPLING_FLAG_PERMANENT`

### RC-098

**FIZ defaults in rail component headers without a source**

* **Where:**
  * `src/vehicles/rail/RailVehicleBrake.hpp:221-231, 255-260`
  * `RailVehicleDieselEngine.hpp:52-127`
  * `RailVehicleElectricEngine.hpp:149-156`
  * `RailVehicleLighting.hpp:80-89`
  * `RailVehicleSpeedControl.hpp:47-61`
  * `RailVehicleDoors.hpp:110, 118-119`
  * `RailVehicleElectricSeriesEngine.hpp:45-49`
  * `RailVehicleAIHints.hpp:32`
* **Problem:** default values copied from the original have no `Mover.cpp`/`MOVER.h`
  reference.
* **Fix:** one reference per block of defaults, pointing at where the original declares them.

## DRY / KISS

### RC-099

**`_rename()` duplicated in three servers**

* **Where:** `src/signalling/SignallingServer.cpp:158` and
  `src/scenario/ScenarioEventServer.cpp:438` (identical); `src/tracks/TrackServer.cpp:787`
  (`isolated_set_name`, a third variant)
* **Fix:** one shared helper in `utils`.
* **Done:** `names_rename()` in `src/utils/Names.hpp` (not `utils.hpp`: the vendored `hamulce.h`
  includes that one), used by both servers and by `TrackServer::isolated_set_name`/`isolated_free`,
  which now warn on a duplicate too.

### RC-100

**Clock-hold processing code duplicated**

* **Where:** `src/driver/DriverSystem.cpp:62-83`; `src/scenario/ScenarioEventServer.cpp:227-249`
* **Problem:** identical `_refresh_processing`/`_set_processing`, and `_refresh_processing` is a
  one-line forwarding wrapper.
* **Fix:** one implementation; drop the wrapper.
* **Done:** `SimulationServer::clock_subscribe()`/`clock_unsubscribe()` hold the clock and connect
  `simulation_advanced` in one operation; `DriverSystem` and `ScenarioEventServer` use it and lost
  `_refresh_processing`. `RailVehicleServer::_refresh_stepping` stays: it computes a two-part
  condition for two callers, it is not a forwarder.

### RC-101

**Electric traction forwarders copied into three engines**

* **Where:** `src/legacy/vehicles/MoverRailVehicleElectricSeriesEngine.cpp:28-74`,
  `…ElectricInductionEngine.cpp:30-93`, `…DieselElectricEngine.cpp:28-74`
* **Problem:** nine `traction.*` forwarders and identical `_register_commands`/
  `_unregister_commands` bodies, written three times.
* **Fix:** move them into the shared traction part once.
* **Done:** Engines are composed of units - plain C++ interfaces in `src/vehicles/rail/` with Mover
  implementations: `RailVehicleDriveUnit`, `RailVehicleDieselEngineUnit`,
  `RailVehicleTractionMotorsUnit`, `RailVehicleCurrentCollectorUnit`, `RailVehicleCircuitUnit`. The
  `*Backend` layer and `VehicleElectricTraction` are gone. The traction motors' Godot face lives in
  `RailVehicleElectricEngine` and `RailVehicleDieselElectricEngine` (two copies, forced by single
  GDCLASS inheritance - down from three in the Mover classes).

### RC-102

**Several public roads to one effect**

* **Where:**
  * `src/vehicles/rail/RailVehicleDoors.hpp:78-83`,
    `src/legacy/vehicles/MoverRailVehicleDoors.cpp:243-262`: `permit_left/right_doors`,
    `operate_left/right_doors` next to `permit_doors(side)`/`operate_doors(side)`
  * `RailVehicleHorns.hpp:52-60`: `set_horn_low/high` next to `set_horn`
  * `RailVehicleBrake.hpp:290-291`: `brake_level_set_position` and
    `brake_level_set_position_str`
  * `src/legacy/e3d/E3DRenderingServer.cpp:1477-1491`: `environment_set_wind`, `environment_set_wind_strength`,
    `environment_set_wind_direction`
* **Rule:** PROHIBITED - "a second road to the same effect is deleted"
* **Fix:** keep one operation per effect; bind the commands to it with the argument bound.
* **Done:** Doors: `permit_doors`/`operate_doors(state, side)`, the commands bind the side. Horns:
  `set_horn` and the `horn` command removed; the `horn_bt` lever sends `horn_low`/`horn_high`
  through the new `position_commands` switch wiring (`forward_commands.gd`). Brake: one
  `brake_level_set_position(String)`, the unused enum overload and `BrakeHandlePosition` removed.
  Wind: only `environment_set_wind`, its helper and the two stored halves removed.

### RC-103

**Camera mode written past `camera_set_mode()`**

* **Where:** `src/player/PlayerCameraServer.cpp:172`
* **Problem:** `camera_show_vehicle()` writes `mode = CAMERA_MODE_FREE` directly, skipping
  `camera_set_mode()` and its checks.
* **Fix:** call `camera_set_mode()`.
* **Done:** `camera_placed` is emitted first, then `camera_set_mode(FREE)`.

### RC-104

**`TrackServer::set_is_topology_changed()` second writer**

* **Where:** `src/tracks/TrackServer.cpp:1508`
* **Problem:** a public writer of the flag next to `_mark_topology_changed()` and
  `topology_rebuild()`.
* **Fix:** remove the public setter.
* **Done:** Removed; `is_topology_changed` is read-only, the test calls `topology_rebuild()`.

### RC-105

**Dead code**

* **Where:**
  * `src/scenery/SceneryStreamingServer.hpp:32-36`: `HYSTERESIS_FACTOR` is unused, although the
    comment describes it
  * `src/legacy/parsers/maszyna_parser.cpp:42`: `get_stops()` is never called
  * `E3DParser::get_instance()` is never called
  * `SubModelData::transparent` is written, never read
  * `brake_method_map` is unused (RC-002)
  * `RailVehicleController.cpp:684-689`: the bound stub `change_track` only warns
  * `RailVehicleElectricEngine.hpp:186-187`: `pantograph_*_wire_voltage` are written, never read
  * `VehicleComponent.cpp:127`: an unused `p_callback` parameter
  * `src/utils/utils.cpp:14`: a "Placeholder" template defined only in the `.cpp`
  * `e3d_parser.cpp:52-60`: the unreachable `else` in `_calculate_normals`
* **Done:** Done, except `brake_method_map`: it is not dead but RC-002's unfixed bug, left to
  RC-002. `p_callback` dropped from `unregister_command` everywhere.

### RC-106

**Private helpers with a single call site**

* **Where:**
  * `Cabin3D::_engine_revolutions` (`src/cabin/Cabin3D.cpp:144`)
  * `CabinHUDMouseSystem::_grip`, `_hit_part`, `_increase_signs` (`CabinHUDMouseSystem.cpp:100,
    180, 296`)
  * `SceneryLoadingTaskQueue::_start_workers`, `_run_next` (`:19, 112`)
  * `TrackServer`: `_set_curves`, `_update_length`, `_update_switch_endpoint_metadata`,
    `_curve_common_endpoint_index`, `_connect_all_tracks`, `_rebuild_graph_ids`,
    `_add_track_topology`, `_merge_endpoint_nodes`
  * all six `network_build()` steps in `TractionServer`
  * `UserSettings::_setup_defaults`
  * `LuaVehicleModule.cpp:20` `check_vehicle`
  * `e3d_parser.cpp`: `_read_matrix` (`:324`), `_calculate_normals` (`:41`)
  * `E3DRenderingServer`: `_register_lights`, `_register_smoke_sources`
  * `MoverRailVehicleBrake.cpp:279, 288`: `_controller_position_normalized`, `_force_ratio`
  * `ResourceCache::_initialize()`
* **Rule:** "a private function with one call site is not a helper"
* **Fix:** inline, except where splitting really keeps a long algorithm readable - then the
  operator decides.
* **Done:** Inlined: `_engine_revolutions`, `_grip`, `_hit_part`, `_start_workers`, `_run_next`,
  `_update_length`, `_curve_common_endpoint_index`, `_update_switch_endpoint_metadata`,
  `_add_track_topology`, `_setup_defaults`, `_read_matrix`, `_register_smoke_sources` (it is
  `E3DParser`'s, not `E3DRenderingServer`'s), `_controller_position_normalized`, `_force_ratio`,
  `ResourceCache::_initialize`. Kept by operator decision as steps of long algorithms:
  `_increase_signs`, `_set_curves`, `_connect_all_tracks`, `_rebuild_graph_ids`,
  `_merge_endpoint_nodes`, the `network_build()` steps, `_register_lights`, `_calculate_normals`.
  Struck: `LuaVehicleModule` `check_vehicle` has nine call sites.

### RC-107

**Forwarding wrappers**

* **Where:**
  * `src/vehicles/base/VehicleController.cpp:365-369`, `VehicleComponent.cpp:217-221`:
    `broadcast_command`
  * `GenericVehicleComponentNode.cpp:61-106`
  * `RailVehicleEngine.cpp:389-391`: `get_type` → `get_engine_type`
  * `RailVehicleLoad.cpp:18-24`: overrides that only call the base
  * `src/cache/ResourceCache.cpp:164`: `emit_cache_cleared_signal()`
  * `SceneryStreamingServer.cpp:277`: `_get_pending_builds_locked()`
  * `RailVehicle3D.cpp:145`: `_process` bound as `process_manually`
* **Fix:** call the target directly.
* **Done:** Done, except `GenericVehicleComponentNode`: by operator decision it keeps the shortcuts
  its scripts use (`register_command`, `unregister_command`, `get_controller`, `log_debug`,
  `log_warning`); the unused ones are gone. `get_engine_type` is now the public virtual
  `get_type()`. The `cache_cleared` signal was never registered and nothing listened - removed with
  its emitter.

### RC-108

**Same work done twice**

* **Where:**
  * `VehiclePhysicsNode.cpp:117`: `set_vehicle_rid` repeats what `vehicle_attach_controller`
    already did (`RailVehicleServer.cpp:425`)
  * `RailVehicleElectricEngine.cpp:659-664`: `if (has_power_cable())` twice in a row
  * `src/utils/UserSettings.cpp:77, 87`: `_apply_defaults()` twice on a successful load
  * `e3d_parser.cpp:528, 534`: bound checks the `while` at `:530` already does
  * `MoverRailVehicleLighting.cpp:454-455`: re-derives `active_end`, duplicating
    `_active_end()`
* **Rule:** "never do the same thing twice to be safe"
* **Done:** Done. `UserSettings`: `ConfigFile::load()` does not clear, so only the call after a
  successful load was redundant.

### RC-109

**Duplicated declarations**

* **Where:**
  * `RailVehicleController.hpp:78-86`: `StartMode` duplicates `RailVehicleEngine::StartMode`
  * `RailVehicleDieselElectricEngine.hpp:11-22`: re-declares the electric traction interface;
    `wwlist` is duplicated with the induction engine
  * `SignalHeadNode.hpp:25` and `SignalHeadKind.hpp:18`: `DEFAULT_BLINK_TIME = 0.5` twice
  * `E3DNodesBackend.cpp` vs `E3DRenderingServer`: the reverse cull face setting name (RC-092)
* **Done:** `RailVehicleEngine::StartMode` removed, `RailVehicleController::StartMode` used (the
  include cycle the comment claimed did not exist). Traction interface: see RC-101. `wwlist` stays
  declared in DieselElectric and Induction - unrelated Godot branches. Reverse cull face setting
  name: `E3DLightFactory::LIGHTS_SHADOW_REVERSE_CULL_FACE_SETTING`.

### RC-110

**Config keys nobody reads**

* **Where:** `src/legacy/vehicles/MoverDieselEngineBackend.cpp:227`
  (`p_config["engine_shake_enabled"] = true`); `MoverRailVehicleBuffCoupl.cpp:148-161`
  (`p_config.set("get_buffer_stiffness_k()", ...)` - getter-call strings used as keys)
* **Problem:** there is no reader in GDScript or C++.
* **Fix:** remove them.

## Naming
* **Done:** Removed, with the two test asserts that checked `engine_shake_enabled`.

### RC-111

**New radio signals not `<subject>_<what>_changed`**

* **Where:** `src/vehicles/rail/RailVehicleServer.cpp:29-30`: `vehicle_radio_called`,
  `vehicle_radio_stop_received`
* **Rule:** server signal naming - these are newer than the rule (2026-09-28)
* **Decision:** these are events, not changes of state. Either the rule gets an event form, or
  the signals are renamed.
* **Done:** the rule gained an event form, `<subject>_<past-tense verb>`, so
  `vehicle_radio_called` stays. Radio-Stop is the emergency alarm (the original's
  `radiostopsend`, `scene.h:273`), so its API no longer says `radio_stop`:
  `vehicle_emergency_signal_received`, `vehicle_emergency_signal_send()`, `emergency_signal_send()`.

### RC-112

**Older server methods and signals named otherwise**

* **Where:**
  * `SceneryStreamingServer`: `set_camera`, `drain`, `get_draw_distance`,
    `get_camera_position`, `has_camera`, `is_area_ready`, `get_streamed_count`,
    `get_statistics`, `set/is_streaming_enabled`
  * `E3DRenderingServer`: `get_light_statistics`, `get_smoke_statistics`, `set_current_time`,
    `set/get_animation_speed`, `set_light_level`, `set_wind*`, `set_material_resolver`,
    `set_model_loader`, `set_smoke_source_resolver`
  * `CabinHUDMouseSystem`/`SceneryHUDMouseServer`: `set_camera`, `set_active`, `input`,
    `get_hovered_*`
  * `TrackServer`: `get_switch_max_offset`, `get_switch_offset_delay`, `get_rail_height`,
    `set/get_is_topology_changed`
  * `SimulationServer`: `clear_cache`, `get_build_number`, `check_build_version`,
    `set/get_language`, `advance`, `set/get_time_of_day`, `get_simulation_time`, light and
    temperature accessors
  * `RailVehicleServer`: `get_vehicles`, `get_vehicles_in_rect`, `broadcast_command`, `step`,
    `set/is_stepping_enabled`, `radio_stop`, `generic_vehicle_component_find`
  * `DriverSystem::get_drivers`, `ScenarioEventServer::get_launchers`,
    `ScenarioScriptServer::get_contexts`, `MaszynaTranslationServer::load_translation`,
    `get_languages`
  * signals: `switching_started`, `switching_finished`, `switch_offset_updated`,
    `topology_rebuilt`, `camera_placed`, `paused`, `unpaused`, `simulation_advanced`,
    `cache_clear_requested`, `vehicle_moved`, `vehicle_command_received`, `vehicle_freed`,
    `vehicle_heading_to_track_start/end`, `vehicle_stopped_on_track`, `instance_freed`,
    `instance_built`, `submodel_animation_finished`, `screen_rendered`
* **Rule:** `CODE_STYLE.md` says an older method "is left as it is unless the operator asks"
* **Decision:** whether the upstream merge is the moment to rename them.
* **Done:** all of them are renamed to `<subject>_<action>` - e.g. `vehicle_get_rids`,
  `stepping_advance`, `streaming_set_camera`, `area_is_ready`, `environment_set_time`,
  `mouse_input`, `control_get_hovered`, `cache_clear`, `build_check_version`,
  `simulation_pause`, `simulation_get_time`, `driver_get_rids`, `translation_load` - and so are
  the signals without a subject (`simulation_paused`, `switch_movement_started`,
  `switch_offset_changed`, `instance_submodel_animation_finished`). Kept:
  * the signals that already read `<subject>_<event>` under the new event form
  * the accessors of the servers' Godot properties (`time_of_day`, `simulation_speed`,
    `light_level`, `air_temperature`, `language`, `switch_max_offset`, `switch_offset_delay`,
    `rail_height`, `is_topology_changed`), which the property rule names `set_`/`get_`

### RC-113

**`consist` instead of `trainset`**

* **Where:** `src/player/PlayerCameraServer.hpp:29-30` (`CAMERA_FOLLOW_VIEW_CONSIST_FRONT/REAR`);
  `src/scripting/lua/LuaCameraModule.cpp:10` (`"consist_front"`)
* **Rule:** names come from the vocabulary of the data - `.scn` says `trainset`
* **Fix:** `CAMERA_FOLLOW_VIEW_TRAINSET_FRONT/REAR` and `"trainset_front"`.
* **Done:** every own `consist` is now `trainset`:
  * the camera enum and the Lua view names
  * the `trainset_changed` signal
  * `ExternalCamera3D.View`, `MAX_TRAINSET_VEHICLES` and the sound system's listener trainset
  * the test `test_trainset_coupling_brakes.gd`
  * the comments and the docs

  The original's names stay: the command `consist_releaser` (`consistreleaser`,
  `input/command.cpp:393`), `IsConsistBraked` and `find_nearest_consist_vehicle`.

## Cosmetic

### RC-114

**Deprecated `MAKE_MEMBER_GS*` macros**

* **Where:** 524 uses. The largest are `src/vehicles/rail/RailVehicleBrake.hpp` (65),
  `RailVehicleDieselEngine.hpp` (58), `RailVehicleElectricEngine.hpp` (41) and
  `src/legacy/e3d/E3DSubModel.hpp` (29). Also every rail component header, every `*ListItem`/
  `*Item` resource, `VehicleController.hpp:151-175` and `RailVehicleController.hpp:190-204`.
* **Rule:** `CODE_STYLE.md` "MAKE_* macros ... are deprecated"
* **Problem:** besides being deprecated, the macros leave the member fields public.
* **Fix:** explicit declarations, file by file; then delete `src/macros.hpp`.

### RC-115

**`macros.hpp` included where unused**

* **Where:** `src/legacy/vehicles/MoverRailVehicleSecuritySystem.cpp:3` (nothing used);
  `src/legacy/e3d/E3DModel.hpp:9` (only the `.cpp` needs it)

### RC-116

**Functional casts instead of `static_cast`**

* **Where:** `src/vehicles/rail/RailVehicleRenderingServer.cpp` (`_update_lights()` `bool(...)`,
  `_update_load()` `double(...)`); `RailVehicleServer.cpp` (`_track_position_text()`,
  `vehicle_collect_current()`: `double(...)`, `int(...)`);
  `src/legacy/e3d/E3DInstanceBackend.cpp:54`. (`RailVehicle3D.cpp`'s went with its rewrite,
  2026-09-30.)
* **Rule:** `CODE_STYLE.md` "Conversions"

### RC-117

**State keys with slashes**

* **Where:** `MoverRailVehicleElectroPneumaticDynamicBrake.cpp:39-44` (`dcemued/…`),
  `MoverRailVehicleSpeedControl.cpp:120-124` (`speed_control/…`),
  `MoverRailVehicleSpringBrake.cpp:90-94` (`spring_brake/…`),
  `MoverRailVehicleLighting.cpp:218-237` (`lights/…`)
* **Rule:** by analogy with "property names ... without slashes"
* **Decision:** whether state keys follow the property rule; renaming them touches the GDScript
  readers.

### RC-118

**Headers not self-contained**

* **Where:**
  * `src/legacy/vehicles/MoverRailVehicleBrake.hpp:50-115` (`std::unordered_map`, `std::string`)
  * `MoverRailVehicleDoors.hpp:64` (`std::map`)
  * `MoverRailVehicleWipers.hpp:21` (`std::vector`)
  * `src/logging/GameLog.hpp:12-25` (macros use `UtilityFunctions` without the include)
  * ~~`src/vehicles/rail/RailVehicle3D.hpp:94` (`RailVehicleEngine` is not declared)~~ - gone
    with the rewrite, 2026-09-30
* **Rule:** `CODE_STYLE.md` "A header is self-contained"

### RC-119

**`using namespace godot;` in a header**

* **Where:** `src/register_types.h:6`

### RC-120

**Privacy sections**

* **Where:**
  * Empty or dangling sections: `RailVehicleEngine.hpp:119-120, 140-141`,
    `RailVehicleDieselEngine.hpp:133-134`, `RailVehicleElectricSeriesEngine.hpp:34-35`,
    `RailVehicleBuffCoupl.hpp:19-20`, `RailVehicleBrake.hpp:274-276`
  * Public data members: `RailVehicleEngine.hpp:96` (`motor_param_table`),
    `RailVehicleElectricEngine.hpp:112, 186-187`
  * Protected `_register_commands` overrides made public: `RailVehicleSpeedControl.hpp:36`,
    `RailVehicleHeating.hpp:40`, `RailVehicleSwitches.hpp:48`, `RailVehicleElectricEngine.hpp:205`
  * `VehicleController.hpp:54`: `apply_configuration` is protected but bound publicly
    (`.cpp:58`)
* **Rule:** `CODE_STYLE.md` "Explicit privacy declarations"

### RC-121

**Stale, orphaned and non-English comments**

* **Where:**
  * `src/utils/UserSettings.cpp:85-86`: Polish comments (AGENTS.md asks for English)
  * `MoverRailVehicleBrake.cpp:207, 263`, `MoverRailVehicleLighting.cpp:437`: refer to
    `_do_fetch_state_from_mover()`, which no longer exists
  * orphaned or misplaced doc comments: `VehicleController.hpp:105-109`,
    `VehicleController.cpp:158-161, 243-247`, `RailVehicleServer.cpp:977-985, 1004-1007`
    (line numbers of `bd2247f9`; `RailVehicle3D.cpp:256-264` went with its rewrite)
  * `VehicleController.cpp:143-147`: the comment contradicts the code (RC-004)

### RC-122

**Commands named `set_*` without a property**

* **Where:** `RailVehicleHorns` (`set_horn_low/high`, `set_whistle`, `set_horn`),
  `RailVehicleSpringBrake` (`set_spring_brake_active/enabled`), the EP brake's
  `set_ep_brake_force`
* **Rule:** `CODE_STYLE.md` "Godot properties" - `set_<property>` is reserved for property
  setters
* **Fix:** use command verbs (`horn_low`, `spring_brake_activate`, ...), together with RC-102.
