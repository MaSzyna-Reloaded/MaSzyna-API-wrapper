# Findings - archive

The full entries behind the rules in `FINDINGS.md`: the symptom, what proved the cause, the fix,
and the rule. Headings keep their date and title, because comments in the code cite them
(`see FINDINGS.md, 2026-09-23`). Open work belongs in `TODO.md`, not here.

## 2026-09-25 - pantographs raised only with the master valve forced

* **Symptom:** after the cab's pantograph switches were ported as in the original, `P` alone no
  longer raised the E186's pantograph. `MoverElectricEngineBackend::pantograph()` had opened the
  pantographs' master valve on every raise since `1c0c044`, and taking that out broke it.
* **What proved it:** `LoadFIZ_Cntrl` sets the master valve (`PantEPValveStart`) to automatic
  by default and each pantograph's own valve (`PantValveStart`) to manual
  (`Mover.cpp:10927-10946`). The struct default is manual (`MOVER.h:875`). The E186 FIZ declares
  none of these keys. `grep` missed that at first, because the file is cp1250. So in the original
  the master valve opens by itself when there is low voltage.
* **Cause:** the wrapper never ported the five valve keys, so the Mover kept the struct default,
  and the workaround covered the missing default.
* **Fix:** `VehicleElectricEngine` carries the five keys as our own `StartMode`/bools, the FIZ
  parser reads them, `MoverElectricEngineBackend` writes them into the Mover, and the workaround is
  gone. The cab's switches send our `ValveOperation`, which is mapped to `operation_t` only in
  the backend.
* **Also found:** the original reads a legacy sound's files with `,` as a delimiter
  (`audio/sound.cpp:105-111`), so `small-compressor: a.wav,b.wav,c.wav` is begin, main and end,
  not a data error.
* **Rule:** a workaround in a backend call is a sign that a FIZ key is not ported yet. Read the
  key's default in `LoadFIZ_*` before keeping the workaround.

## 2026-09-25 - the E186 cab half built: three data quirks and a missing gauge feature

* **Symptom:** after the E186 controls were added to the catalog, the cab still did not react:
  radiostop_sw, universal*, battery_sw, pantalloff_sw and more had no widget, the light selector
  had no presets, and none of the three reverser lamps ever lit.
* **What proved it:** a headless probe entering the cab on td_e186.scn and listing every widget
  by control id, then MmdCabinInstancer.parse() on p160dc.mmd listing every descriptor - from
  line 210 of base.mmd.inc on, each `label: { ... }` block came out as an instrument called
  `soundinc`.
* **Causes:**
  * `radiocall3_sw { radio_3 ... }` has lost its colon. The original reacts only to labels it
    knows and walks over every other token; the wrapper's parser takes any `x:` token for a label,
    so it took the block's `soundinc:` for one and read every following block from the wrong end.
  * `LightsList:` in p160dc.fiz has no `endL` and runs straight into `WiperList:`. The FIZ builder
    ended an open table only when the next header opened none, so the WiperList header replaced
    the light table without its end_table(), and all twelve presets were lost.
  * TGauge takes `<name>_on` as the lit state of a control, shown instead of it while a flag is set
    (Gauge.cpp:204-210, 386-392; the flags are bound in Train.cpp:11995-12040, the reverser
    buttons to the sign of DirActive). The wrapper had no such thing, so kierunek_*_on stayed
    hidden.
* **Fix:** a block without a label is skipped whole; every FIZ section header ends the table
  before it; the MMD factory builds a CabinIndicator3D on `<name>_on` for a catalog entry with
  `state_light`.
* **Rule:** a parser of the original's data mirrors the original's tolerance, not the format's
  grammar - the data is full of lines only the original's "skip what you do not know" accepts.
* **Rule:** a table section may end at the next header rather than at its end marker; every
  header closes the open table.

## 2026-09-25 - SU46 would not release its train: the converter never started

* **Symptom:** some trains, passenger and freight, could hardly be released. SU46-054 with four
  coaches on `zwierzyniec_osob.scn` stayed braked. The consist refactor (#184) was the first
  suspect.
* **What proved it:** one `get SU46-054` dump. The main reservoir was at 3.40 bar, the brake pipe
  at 3.35, and `compressor_allowed` was false. An FV4a cannot charge the pipe above the main
  reservoir, and the coaches' distributors hold their cylinders until the pipe comes back to
  around 5 bar. The refactor was cleared by comparing every moved coupling and movement function
  with its pre-refactor body.
* **Cause:** SU46 declares `CompressorPower=Converter` and `Cntrl. ConverterStart=Automatic`. For
  that compressor the Mover takes `CompressorAllow = ConverterAllow` (Mover.cpp:3886), and
  `ConverterAllow = Mains` only when the converter start is automatic (Mover.cpp:1885). No parser
  read `ConverterStart`, and the property existed only on `VehicleElectricEngine`, which a
  diesel-electric does not have. So the Mover kept `start_t::manual`, and SU46's cab has no
  converter switch to make up for it. A vehicle starting at velocity 0 begins with its main
  reservoir at `0.55 * MinCP` (Mover.cpp:8932) and relies on the compressor from there.
* **Fix:** `ConverterStart` and `ConverterStartDelay` are `VehicleController` properties, parsed
  with `BatteryStart` and applied next to it, as `LoadFIZ_Cntrl` does (Mover.cpp:10909). After the
  fix the operator's train pulled away.
* **Found on the way:** `BrakeValveParams` is never set, so every ESt distributor is built as an
  ESt4 (TODO.md). The test fixture's `W_Lu_L` valve has no distributor in the Mover (the factory
  falls through to a plain `TBrake`), so the fixture shows pipe pressure but never a cylinder.
* **Rule:** a `Cntrl.` key belongs to the vehicle. A property placed on one engine class silently
  does not exist for the other engine types that read the same key.

## 2026-09-24 - Python cab screens: what the original's scripts actually need

Porting `pyscreen:` meant running the original's own Python 2 scripts. Four things were only
visible that way.

* **A missing key blanks the screen.** `state['hours']` on a missing key raises KeyError, so
  `render()` fails and nothing is drawn. A harness fed `{}` and added each key a script asked for:
  `timetable` needs 2, `etcs_180kmh` 8, and all 145 scripts together read ~250 of
  `GetTrainState()`'s keys. `PythonScreenState` therefore always hands over the original's whole
  key set, with its value types, and fills in only what the wrapper has.
* **The parser ate the labels after a screen.** `pyscreen:` went through the generic instrument
  branch, which reads five value tokens, but a screen has two (legacy form) or a `{...}` block. A
  test now puts an ordinary instrument right after each form of screen.
* **Scripts assume the game dir is the cwd** (`./fonts/`, `from scripts import`), and importing
  writes a `.pyc`. The interpreter `chdir`s to the game dir and runs with `dont_write_bytecode`.
* **A leaked screen with a GDScript lambda callback crashes the exit** ("corrupted size vs.
  prev_size", `~Callable` in `PythonScreenServer`'s destructor at module de-init). Freeing the
  screens and using a method callback exits cleanly, and `CabinPythonScreen` does both.
* **Test trap:** the headless dummy renderer keeps no texture data, so an `ImageTexture` replaced
  by `set_image()` reads back as the 1x1 placeholder. The test checks the size, and the fixture
  script echoes the state it received.
* **Rule:** when porting a data contract consumed by scripts nobody here maintains, run the real
  scripts against it before designing it.

## 2026-09-24 - the E186 line breaker dropped at 17 km/h: the wire was a hundred times too resistive

* **Symptom:** the E186 pulled away and the line breaker opened after ~120 m, at 17 km/h.
* **Proof:** a per-frame probe on `td_e186.scn`. At 250-270 A the pantograph voltage slid
  2267 -> 1889 V over 1 m of travel, and the breaker opened the frame it passed `MinV` (1900 V).
* **Cause:** a scenery gives resistivity in Ohm/km (`traction pwr01 3500 4500 0.01`), and the
  original converts it to Ohm/m (`fResistivity *= 0.001`, Traction.cpp:112, with 0.01 read as the
  default 0.075). `TractionPowerServer` multiplied the raw value by metres. Only a vehicle drawing
  real current shows it.
* **Fix:** `wire_set_params()` takes Ohm/km and stores Ohm/m. The same run now reaches 44 km/h at
  3547 V.
* **Rule:** a value copied from a scenery token carries the unit the original's loader gives it
  right after parsing. Read the lines after `>>`, not just the `>>`.

## 2026-09-24 - every brake handle froze under 1 bar of difference, because of C's abs()

* **Symptom:** the E186 brake pipe stopped at 4.0 bar in running position. The pipe lock
  (`LPOn=3.0 LPOff=4.5`) never released, so the loco had no power.
* **Proof:** the handle fields were all correct and the control reservoir still froze. The formula
  is `CP += 9 * min(abs(LimCP - CP), 0.05) * PR(...) * dt`. A scratch program built with
  `hamulce.cpp`'s includes printed `abs(0.89) = 0.000000`: it had resolved to C's `int abs(int)`.
* **Cause:** the original's `stdafx.h` includes `<stdlib.h>`, which puts the `std::abs` overloads
  in the global namespace. The vendored files have no `stdafx.h`. This hits the control reservoir
  of `MHZ_EN57`, `MHZ_K5P`, `MHZ_6P`, `M394` and `St113`, and the EP step of `TEStEP1` (11 calls
  in `hamulce.cpp`). `FV4a` does not use it.
* **Fix:** CMake force-includes `stdlib.h` into `src/maszyna/*.cpp` (non-MSVC). The vendored files
  stay untouched.
* **Rule:** the vendored engine was written against its own precompiled header. A name that
  resolves differently without it compiles silently (vendored files build with warnings off), so
  check overload-sensitive calls (`abs`, `min`, `max`) when something numeric just stops.

## 2026-09-24 - an induction motor never pulled: nobody turned the controller into power

* **Symptom:** E186 with brakes released, direction set and the controller on T+: `Ft = 0`.
* **Cause:** the integrated controller's setpoint (`CheckEIMIC`, `CheckSpeedCtrl`, `eimic_real`)
  is computed by `TDynamicObject::Update` (DynObj.cpp:3246-3283), which is not vendored.
  `TractionForce` read `eimic_real = 0`.
* **Fix:** `VehicleEngine::_do_process_component` does it for a vehicle with a driver. The ED/PN
  brake split that follows in DynObj.cpp is not ported (TODO.md).
* **Rule:** a Mover method that nothing in `Mover.cpp` calls is called from DynObj.cpp or
  Train.cpp. Grep the original for its callers before assuming the backend drives itself.

## 2026-09-24 - every force of the E186 turned NaN once a direction was set

* **Symptom:** `velocity`, `Ft`, `brake_unit_force` and the wheel angles went `nan` once the
  reverser left neutral with the breaker closed.
* **Cause:** the EIM step divides by `InvertersNo` (`InvertersRatio`, Mover.cpp:5627).
  `LoadFIZ_Engine` gives a powered EIM without `InvNo` one inverter and sizes `Inverters`
  (Mover.cpp:11302). The wrapper did neither, so the step computed 0/0. The same block defaults
  `fcfuH` to `fcfu` (Mover.cpp:11290) and reads `Volt`, `abed`, `edep`, `eimclf`,
  `InvCtrCplFlag` and `Flat`. All of it is ported now.
* **Rule:** porting a `LoadFIZ_*` block means porting what it does after the `extract_value`
  lines too: derived counts, container sizes, fallbacks to another key.

## 2026-09-24 - the E186 line breaker opened the moment it closed

* **Symptom:** with the pantographs up, the main switch would not stay on. `main_switch` returned
  true and was false again three frames later.
* **Cause:** an induction motor opens the breaker above `CollectorParameters.MaxV + 200`
  (Mover.cpp:5706). The original reads FIZ `MaxVoltage` into both
  `EnginePowerSource.MaxVoltage` and `CollectorParameters.MaxV` (Mover.cpp:11622). The wrapper
  wrote only the first, so `MaxV = 0`. Series-motor vehicles use `MaxV` only with `OverVoltProt`,
  which is why they never showed it.
* **Test trap:** the knock-out lives in `TractionForce()`, which runs only with `Power > 0` and
  only for a driven vehicle. Two versions of the test passed with and without the fix. The kept
  test was checked against a build with the fix commented out.
* **Rule:** one FIZ key can feed several Mover fields. Grep every `extract_value(..., "Key", ...)`,
  not the first one found.

## 2026-09-24 - every spring brake started shut off, because the struct defaults were kept

* **Symptom:** on the E186 the spring brake seemed to do nothing, and "Enable" shut it off.
* **Proof:** a probe with e186's `SpringBrake:` values showed `shut_off=true` and `is_ready=false`
  from the first frame. The cylinder filled only through the bypass, because `UpdateSpringBrake`
  takes `MSP = ShuttOff ? 0 : MaxSetPressure` (Mover.cpp:4836).
* **Cause:** `LoadFIZ_SpringBrake` ends with `ShuttOff = false; Activate = false; IsReady = true;`
  (Mover.cpp:11028). The struct defaults (`ShuttOff{true}`, `IsReady{false}`) describe a vehicle
  *without* a spring brake, and the port dropped those lines. Setting them in `if (!Cylinder)`
  cannot work: `CheckLocomotiveParameters()` creates a fallback cylinder first (Mover.cpp:8881).
* **Found on the way:** `set_spring_brake_enabled(true)` called `SpringBrakeShutOff(true)`, and a
  test locked that inversion in. The original swaps `ValveOnArea`/`ValveOffArea`
  (Mover.cpp:11025), which the port read straight. `MTC` defaulted to 0 instead of 127.
  `i-springbrakeactive` showed `Activate` where the original shows `IsActive` (Train.cpp:9195).
* **Rule:** port the whole loader function, including the state it sets at the end. A struct
  default is what a vehicle *without* that section gets.

## 2026-09-24 - the Linux release would not start on most machines, because of the build host's glibc

* **Proof:** `objdump -T <file> | grep -oE 'GLIBC_[0-9.]+' | sort -Vu | tail -1`.
  `libmaszyna.64.so` needed GLIBC_2.43 and the `reloaded` template needed 2.44, both built on
  Manjaro. Ubuntu 24.04 has 2.39, Debian 12 2.36, Ubuntu 22.04 2.35. libstdc++ is static and was
  not the problem.
* **Trap:** the CI image `jezsonic/build-tools:4.7.stable` is Ubuntu 26.04 with glibc 2.43.
  Rebuilding only the library is not enough, because the template needs the newer glibc.
* **Fix:** `make release-linux` builds both in `ci/docker/linux-sdk`, Godot's buildroot SDK
  (`godot-2026.05.x-1`, GCC 15.2, glibc 2.34). The template is built once from `4.7.2-stable`.
* **Rule:** glibc is only forward compatible. Check the highest `GLIBC_` of every shipped binary,
  and build on a sysroot as old as the oldest target distribution.

## 2026-09-24 - switch blades in a scenery never moved, because only a node listened

* **Symptom:** throwing a switch changes the route, but the blades stay put. `TrackSwitch3D` in
  `demo_3d` still animates.
* **Cause:** `TrackManager::_process_switches()` emits `switch_offset_updated`, and the only
  listener was `TrackSwitch3D`. Since `923b293` a scenery builds tracks through
  `TrackRenderingServer`'s RID API with no nodes, so the blades were posed once in
  `_stream_build()` and never again.
* **Fix:** `TrackRenderingServer` subscribes to `switch_offset_updated` itself. The node's copy is
  removed.
* **Rule:** as with the scenery lights (2026-09-21), a feature parked on a node vanishes when an
  instancer without nodes appears. The server that owns the visuals subscribes to the manager.

## 2026-09-24 - what a "load" turns out to be in this engine

Porting `loadcount`/`loadtype` from a `.scn` `dynamic` line.

* **A load is not always cargo.** `AssignLoad(name, amount)` branches on the name.
  `"pantstate"` (Mover.cpp:7649) reads the amount as a bitmask that raises pantographs and picks
  the direction. The load therefore reaches the backend as name + amount together, through that
  one call.
* **A count with no name is not a load.** The name follows only a non-zero count, and the
  original zeroes both when it is missing ("idiotoodporność",
  `simulationstateserializer.cpp:1031`). Reading the next token unconditionally eats `enddynamic`.
* **Passengers are cargo.** The MMD `loads:` block maps cargo to a model
  (`logs: loads/eaos_vrz-99_logs`), and `passengers` is one entry of it. 235 vehicles declare the
  block. The wrapper used to read only `passengers`.
* **A cargo without a model is normal.** The lookup is: the vehicle's own override, then
  `<vehicle type>_<cargo>`, then the cargo name. Finding none is accepted (DynObj.cpp:7195).
  `dynamic/zssk/lgs_v1` has no `loads:` block and uses the third rule.
* **The load's height depends on how full the vehicle is.** `LoadOffset` lerps from `offset_min`
  to 0 (DynObj.cpp:3079). Both inputs are vehicle configuration, so the height is set when the
  configuration lands, on the same event the bogie spacing waits for (2026-09-23).
* **Rule:** before porting a field that looks like data, read what the backend does with its
  *name*.

## 2026-09-24 - a consist ringing like metal, and the original naming the bug in a comment

* **Symptom:** from outside, a moving consist sounds metallic, like comb filtering ("podwójne
  dźwięki").
* **Ruled out:** a probe over the banks found no event registered or played twice (EP07 5/3/30,
  E186 7/4/9).
* **Cause:** every wagon plays the same running-noise loop, all started at the same moment. The
  original names the effect (`audiorenderer.cpp:99`) and starts it at
  `Random(0.0, 80.0) * 0.01` of the sample (DynObj.cpp:6511): a *fraction* of the sample, drawn
  *once per vehicle*.
* **Fix:** `TrainSoundSystem` draws `randf_range(0.0, 0.8)` per bank runtime and starts a looping
  running sound at that fraction. `SfxPlayer.play()` already takes an offset.
* **Not the clatter:** `RunningSoundModel._wheel_clatter()` already phases each axle by position
  (DynObj.cpp:3671-3730). A one-shot per joint does not comb; a shared loop does.
* **Rule:** when many copies of one sound play at once, the defect is phase, not level. Look for a
  start offset before touching a gain.

## 2026-09-24 - the pantograph lost the wire where the original keeps it, in four different ways

* **Symptom:** on `zwierzyniec_tlk` an EP08 loses line voltage a few times per run and trips the
  main switch. After two fixes it still died exactly at the exit of one switch, at any speed.
* **Measured first:** 1335 spans / 2670 ends. 2414 ends have one neighbour, 116 are line ends,
  126 have three (four spans over a switch), and only 2 have a gap over the tolerance. All 1387
  joined pairs differ in height by 0.000 m. The wiring has no holes; the problem is choosing a span.
* **Divergences from the original:**
  * No guide horn. The original accepts a wire up to `fWidthExtra` = 0.381 m beyond the slider and
    counts it as higher (`scene.cpp:105-112`, DynObj.cpp:93).
  * The slider width never came from data. `pantograph_collector_width` sat at 0.5 for every
    vehicle. EP08 has `CSW=1.4`, so the original searches 1.081 m per side.
  * No chain walk. The original steps along `hvNext` (DynObj.cpp:8742). The wrapper re-searched
    and covered the gap with a 0.25 m span-end tolerance.
  * `iLast`, the one that killed it at the switch. A span that ends a section, or whose neighbour
    does (`TTraction::WhereIs()`, Traction.cpp:392), does not follow the chain at all
    (DynObj.cpp:8747). The wrapper followed a first-come `next[]` onto the wrong span.
  * Join tolerance: 0.25 m euclidean against the original's 0.025 m per axis
    (`TTraction::TestPoint`, Traction.cpp:355). At 25 ends that produced three candidates instead
    of one.
  * `parallel` and `section` were parsed and dropped. `section` is not a substation: power reaches
    a span along the wires (`TTraction::PowerSet()`, Traction.cpp:460). `TTraction::VoltageGet` is
    now ported whole.
* **Data trap:** every supply is declared twice under one name, once as `section` and once as a
  substation. The original's name table keeps the last (`Names.h:38`), which is the substation.
* **Rules:**
  * Measure the data before reading the code. Three of four hypotheses died to one pass.
  * A tolerance that papers over data becomes load-bearing once something trusts the structure
    under it.
  * 0 V at a raised pantograph has three causes: no wire in reach, a dead wire, and no contact
    (`PantDiff >= 0.01`, DynObj.cpp:3866). Report them separately, with the track and the offset.

## 2026-09-24 - a parked vehicle jumping, because two writers disagreed about where it stands

* **Symptom:** a stopped vehicle jumps slightly, only on curves and most on one switch. The
  position readout is stable.
* **Cause:** `apply_track_placement()` wrote the body transform twice. The first write came from
  `vehicle_get_transform()` (the track under the centre). The second, on `moved ||
  force_detail_refresh`, came from the chord between the bogie pivots. The two agree on straight
  track only.
* **Fix:** `RailVehicleServer` composes the body from the two pivots, cached against the
  placement. `RailVehicle3D` takes that one answer and only places the bogie nodes.
* **Rule:** one piece of state, one writer. Two writers that both look correct disagree only where
  the geometry shows it.
* **Trap:** a vehicle with no mass integrates to NaN, and NaN never equals itself, so "did it move"
  is true forever.

## 2026-09-24 - a .fiz in the project stopped importing, silently

* **Symptom:** `--import` prints `Error importing 'res://tests/fixtures/test_vehicle.fiz'`, and the
  `.import` gets `valid=false`.
* **Cause:** `.fiz` now produces a `VehicleModel` (a `Resource`), but
  `FIZImportPlugin._get_save_extension()` still said `"scn"`. `ResourceSaver.save()` of a plain
  Resource to `.scn` returns 15 (`ERR_FILE_UNRECOGNIZED`); to `.res` it returns 0.
* **Trap:** Godot reimports on md5, not mtime. Delete the artifact under `.godot/imported/` to
  retest.
* **Rule:** an `EditorImportPlugin`'s save extension changes together with
  `_get_resource_type()`.

## 2026-09-23 - the cab acted one keypress late, because the dump was cached per step

* **Symptom:** a key plays its sound at once, but the operation happens only on the next key.
* **Cause:** `RailVehicleServer::vehicle_dump_state()` cached one Dictionary per physics step on
  the premise that only a step changes state. `TrainSystem::send_command()` changes it
  synchronously, and `CabinSwitch._on_command_received()` -> `_update_state()` read the pre-command
  snapshot.
* **Fix:** the cache is keyed on the step and on a command serial.
  `VehicleController::command_executed()` bumps the serial, updates the state and announces it.
  It is not keyed on `update_state()` alone, because a parked vehicle does not run it.
* **Rules:**
  * A cache keyed on a tick is correct only while the tick is the only writer. Write that premise
    where the cache lives.
  * "One action late" is a read of a snapshot taken before the write. Look for the cache first.

## 2026-09-23 - the release re-parsed every scenery because its game dir was "."

* **Symptom:** in the shipped build every scenery is parsed on each launch. The editor caches fine.
* **Wrong first guess:** the build stamp clearing caches. In fact `clear_cache()` only emits
  `cache_clear_requested`, and the scenery cache does not listen to it.
* **Proof:** running `_is_cache_valid()`'s checks over `user://cache/scenery_compiled/*.res`. The
  editor entries carry absolute `src` and resolve. The release entries carry
  `src=scenery/baltyk/mod/drogi.scm`, with 102 of 102 dependencies missing.
* **Cause:** `get_maszyna_game_dir()` returned `"."` in an export (`UserSettings.cpp:119`, since
  `d3e7d52`), and a relative path given to `FileAccess` resolves against `res://`, which is the
  pack. The parser reaches files by another route, which hid it.
* **Fix:** in a release the game dir is `OS::get_executable_path().get_base_dir()`. The editor and
  the release now share cache keys.
* **Rules:**
  * A path handed to `FileAccess` is absolute, or it is silently a `res://` path.
  * When a cache "does not work", load an entry and run its own validity check before suspecting
    invalidation.

## 2026-09-24 - the shipped library had no symbols, and the crash was in the parser

* **Symptom:** closing the game during loading segfaults on a non-main thread (`#0 0x0`).
* **Cost:** the shipped `.so` was stripped, and two cores were misdiagnosed by stack shape. The
  `-s` comes from godot-cpp's `DEBUG_SYMBOLS` generator expression, propagated through
  `TARGET_LINK_LIBRARIES`, so editing our own `LINK_OPTIONS` changes nothing.
  `make release-linux-symbols` builds `RelWithDebInfo` with `template_release`.
* **Cause (with symbols):** `MaszynaParser::parse_chunk` (`maszyna_parser.cpp:270`) ->
  `Callable::call` -> freed code. The parse runs as a `SceneryLoadingTaskQueue` task, and nothing
  stopped the queue before the scripts went away. Its destructor runs during teardown, too late.
  `callback.is_valid()` does not help.
* **Fix:** `SceneryLoadingTaskQueue::drain()`. `SceneryInstancer` keeps `_active_queues`, and
  `cancel_loading()` drains them, called from `maszyna_include.gd::_exit_tree()`.
* **Regression 1:** draining waits for a multi-second parse, so quitting hung instead.
  `MaszynaParser` now has a static `cancelled` checked by the token loop and by
  `_count_includes`.
* **Regression 2:** `drain()` dropped queued tasks that another task `wait()`s on, which
  deadlocked `wait_to_finish()`. A core from `kill -ABRT` showed threads in
  `SceneryLoadingTaskQueue::wait` (`:90`) from `_run` (`:146`). `wait()`/`is_done()` now give up
  while draining.
* **Diagnosis tip:** `/proc/<pid>/task/*/wchan` needs no privileges, and `kill -ABRT` turns a hang
  into a symbolised core.
* **No test:** a mid-parse teardown test passed with and without the fix (headless parses finish
  too fast), so it was deleted.
* **Rules:**
  * A shipped build keeps its symbol table.
  * A `Callable` across a thread is only as valid as its script. The thread's owner stops it
    before the scripts go.
  * Every worker needs an owner that stops it at teardown. A destructor runs too late.

## 2026-09-24 - the simulation stepped after everything that reads it

* **Symptom:** vehicles judder, worst from the external view, even a single loco.
* **Wrong turns (reverted):** a fixed step, which was worse without interpolation, and the cabin
  shake. `4ec5490` had already decided both: a variable delta as in the original
  (`vehicle_table::update`, DynObj.cpp:8181) and `process_priority = -100`.
* **Cause:** the C++ port stepped from `SceneTree.process_frame`, which is emitted after all
  nodes are processed. It pushed placement onto `RailVehicle3D` afterwards, but other readers
  (`ExternalCamera._process()`) saw the previous frame.
* **Fix:** `RailVehicleStepper` (`process_priority = -100`) calls
  `RailVehicleServer::step_frame()`.
* **Follow-up:** past 0.2 s, `sub_step = delta / MAX_PHYSICS_ITERATIONS` exceeds `PHYSICS_STEP`.
  Time drives events and multiplayer, so `step_frame()` owes the excess to the next frames. Only
  past `maszyna/physics/catch_up_limit` does it take the debt in one logged jump. The debt resets
  when stepping starts.
* **Test trap:** `test_process_movement_with_invalid_controller_reference_is_noop` relied on the
  old order. `controller = null` does not detach, so the test now detaches the controller.
* **Rules:**
  * `process_frame` is the end of a frame. What `_process` reads must be produced before it, and
    `process_priority` is the only ordering inside that phase.
  * When a fix is being reinvented, `git log -S` the moved code and read the original commit
    first.

## 2026-09-23 - a GDScript subclass silently replaced the native _ready()

* **Symptom:** after `Cabin3D` moved to C++, the camera stopped entering the cab, silently.
* **Cause:** C++ `_ready()` emitted `cabin_ready`, but `DynamicTrainCabin` (GDScript) defines
  `_ready()`, which **replaces** a native virtual. `super._ready()` is refused for native
  virtuals.
* **Fix:** `_notification()` with `NOTIFICATION_READY` / `NOTIFICATION_PROCESS`, which reaches
  the native class and the script both.
* **Second half:** the script's override of `set_train_id()` (which rebuilt the interior) is
  bypassed by the typed call from `RailVehicle3D`. A script shadows a native method only for
  `call()` callers. `Cabin3D` now emits `train_id_changed` and the subclass reacts.
* **Rules:**
  * A C++ class under an existing GDScript subclass puts its lifecycle in `_notification()`, never
    in the `_ready()`/`_process()` virtuals.
  * A C++ base does not offer "override this method" unless it is a registered virtual. It
    announces with a signal instead.

## 2026-09-23 - the loco that would not move had nobody in the cab

* **Symptom:** `test_sm42_startup_sequence::test_successful_moving_on` - "Speed should be > 0" -
  was red for weeks since `87d5f8d`.
* **Cause:** `ComputeTotalForce()` (Mover.cpp:4485) keeps physics active only when
  `CabActive != 0 || Vel > 0.0001 || |AccS| > 0.0001 || LastSwitchingTime < 5 || EZT || DMU`. The
  test used `cabin_number = 0`, so `CabActivisation()` never ran (Driver.cpp:2126). After 5 s the
  Mover stopped integrating.
* **Fix:** the test occupies the cab (`cabin_number = 1`).
* **Rules:**
  * A vehicle that is not driven is not simulated. Check `CabActive`/`PhysicActivation` before
    treating "it does not move" as a bug.
  * A test that stays red across many unrelated commits stops being evidence of the commit that
    turned it red.

## 2026-09-23 - a parked vehicle never had its bogies placed, and four guesses before one print

* **Symptom:** both bogies on a curve had the same tangent. A 1 mm nudge fixed it.
* **Cost:** four hypotheses read off the code (two of them real bugs, fixed on the way) before one
  `print` of every guard in `apply_track_placement()` named the branch.
* **Cause:** components added after `VehiclePhysicsNode::_build()`'s `initialize()` configure
  nothing until the next tick dirties the Mover. `MoverVehicleWheels` reads `mover->BDist`, so the
  only placement saw spacing 0, and `moved` stays false while the vehicle stands still.
* **Fix:** `RailVehicle3D` reacts to `mover_config_changed`.
* **Rules:**
  * A per-frame path gated on "moved" never picks up a late value. Recompute config-derived values
    on the config event, not with a retry flag.
  * After two hypotheses read off the code have failed, stop reading and print.

## 2026-09-23 - every vehicle ran with a bogie pivot spacing of zero

* **Cause:** `MoverVehicleWheels::_fill_config_dictionary()` (and
  `MoverVehicleUniversalController`) published keys named after methods, parentheses included:
  `p_config["get_bogie_pivot_spacing()"]`, 11 keys in 2 files. `RailVehicle3D.cpp:1106` asks for
  `bogie_pivot_spacing` and got the default 0.0 for every vehicle.
* **Fix:** the keys carry the value's name.
* **Rules:**
  * A state or config key is data, not a method name. Grep fills for `["get_`.
  * `Dictionary.get(key, default)` hides a typo forever. Test that the key is present.

## 2026-09-23 - the cab's instrument backlight blinking, once per frame, from the transform

* **Symptom:** in the EP07 cab the desk backlight and the ceiling lamp blink when switched on.
* **Proof:** the state dump was stable (`11 11 11`). With the cab entered, `podswietlenie_on` was
  visible one frame in fifteen and `_off` was its complement: one writer at 10 Hz, one every
  frame.
* **Cause:** `E3DRenderingServer::instance_set_transform()` ended in `_update_if_built()`, which
  in `E3DNodesBackend` re-applies `lights_state` to the `_on`/`_off` submodels. `9ca6b9f` made the
  transform notification unconditional (for smoke), so every node-instanced model re-applied its
  lights every frame.
* **Fix:** `E3DInstanceBackend::apply_transform()`. It does nothing for nodes and does
  `instance_set_transform` per RID for optimized instances. This also dropped a per-frame
  re-resolve of the optimized instances. The czuwak/SHP blinker was broken by the same thing.
* **Rule:** a setter applies what it is named after. Routing every `instance_set_*` through "apply
  everything" makes a move overwrite state owned by someone else.
* **Still open:** the light submodels have two managers (TODO.md).

## 2026-09-22 - a teardown abort that is RID allocator corruption, not a double free

* **Symptom:** `test_zzz_ep07_cabin_main_switch` aborts during scenery teardown in about half the
  runs.
* **Instruments:** a print per group in `_free_owned_rids()` found it dying in group 5
  (`E3DRenderingServer.instance_free`, 324 instances). `coredumpctl debug` showed
  `E3DOptimizedBackend::clear` -> RenderingServer -> **SIGABRT**. The errors were "Initializing
  already initialized RID", "Attempting to initialize the wrong RID" and "unimplemented base type
  encountered in renderer scene cull", which means allocator corruption, not a double free.
* **Mechanism:** `_stream_preload()` runs on `SceneryStreamingServer`'s worker and calls the
  GDScript `model_loader` (`e3d_model_manager.gd::load_model`), which runs `load()` and creates
  renderer resources while the main thread frees them. A cold `rail_vehicle`/`fiz` cache
  reproduces it.
* **Fixed on the way, not the cause:** a double free across an await in `_free_owned_rids()`, 14
  unguarded `get_instance()->` calls, and a HashMap iterator held across re-entry in
  `instance_free()`.
* **Half fixed 2026-09-24:** `SceneryStreamingServer::drain()` is called from
  `maszyna_include.gd::_exit_tree()` before `_free_owned_rids()`. The reload path has done this
  since `8d02b43`. The race while a stream is running remains open (TODO.md).
* **Not reproducible headless:** a `--script` run has no autoloads (so no `model_loader`), and the
  dummy renderer creates nothing.
* **Rules:**
  * An abort: read the engine's error lines first. "Already initialized RID" means concurrency;
    "invalid RID" means a double free.
  * Godot's crash dump is not the stack. Use `coredumpctl debug`.
  * Read a worker-thread `Callable` all the way down. `load_model` is `ResourceLoader.load()`.

## 2026-09-22 - "the C++ port made it 4x slower" was a GPU that never woke up

* **Symptom:** `td.scn` ran at ~30 fps instead of ~200 right after `TrackManager`/`SpatialIndex`
  moved to C++.
* **Cause:** the discrete GPU stayed in powersave, so the integrated RX 780M rendered. The GPU
  frame time was over 40 ms from the first measurement.
* **Rules:**
  * Split frame time into CPU and GPU before any hypothesis. If the GPU time moved, check the
    adapter (`--verbose`).
  * Confirm the environment (adapter, power profile, build flags) before blaming the code.

## 2026-09-22 - a config property and a state key of the same name are not the same value

* **Symptom:** `test_train_battery::test_successful_battery_voltage_drop_after_two_seconds` went
  red.
* **Cause:** the state was pointed at the `battery_voltage` config getter. The authored value is
  the nominal voltage (written to `BatteryVoltage` and `NominalBatteryVoltage`), and the backend
  changes `BatteryVoltage` at run time (Mover.cpp:946).
* **Fix:** `get_live_battery_voltage()` for the state. `RPowerCable.SteamPressure`, `PowerTrans`
  and `RAccumulator.RechargeSource` were checked, and the backend never writes them.
* **Rule:** grep the backend for an assignment before publishing a state value through a config
  getter. A value the simulation writes is state.

## 2026-09-22 - an uninitialised pointer that only a property read could reach

* **Symptom:** SIGSEGV in `PackedScene.pack()` (`fiz_train_controller_instancer.gd:218`). The
  bisect gave inconsistent answers.
* **Proof:** `addr2line` on the three `libmaszyna` frames:
  `VehicleDoors::get_locked()` -> `VehicleController::get_mover()` via `MethodBind::bind_call`,
  that is, a property read.
* **Cause:** `VehicleComponent::train_controller_node` had no initialiser. It is set in
  `ENTER_TREE`, and `pack()` read the new typed properties of a never-parented component.
* **Fix:** `= nullptr`.
* **Rules:**
  * Exposed properties make getters reachable before `_ready()`, before `ENTER_TREE`, during
    `pack()` and from the inspector. Everything they touch is valid from the constructor.
  * A raw pointer member gets `= nullptr` at its declaration, always.
  * Run `addr2line -f -C -e <.so> <offsets>` on the extension's hex frames before reading the tail
    of the dump.

## 2026-09-22 - a regex that deleted 588 lines, and the linker that caught it

* **Symptom:** the build succeeded, then Godot refused the extension with
  `undefined symbol: RailVehicleServer::vehicle_move(RID const&, double)`, and every script naming
  a wrapper class failed to parse, which looked like a broken class cache.
* **Cause:** a removal regex with an optional `(    /\*.*?\*/\n)?` prefix under `re.S` matched
  across hundreds of lines and deleted 588 of 856. A missing definition only fails at link time,
  and GDExtension links lazily.
* **Fix:** remove by walking lines from a signature to its matching `    }`. The result was checked
  by diffing `grep -oP "Class::\K\w+"` against `HEAD`.
* **Rules:**
  * No span deletion with a regex whose optional prefix can match across lines. Verify scripted
    source edits structurally (symbol list, line count).
  * `undefined symbol` from a GDExtension means a bound method has no definition. Look for a
    deleted definition first.

## 2026-09-22 - an unguarded singleton dereference only crashes at teardown

* **Symptom:** signal 11 in `_free_owned_rids` -> `TrackManager.track_free` in
  `test_zzz_ep07_cabin_main_switch`.
* **Cause:** `track_free()` emits `tracks_changed`, and `RailVehicle3D`'s handler called
  `TrackManager::get_instance()->...` with no null check after the singletons were unregistered.
  The untyped `->call()` it replaced had only warned.
* **Fix:** every `get_instance()` result in `RailVehicle3D` goes into a local and is checked.
* **Rules:**
  * The typed form removes the string, not the null. The guard becomes more necessary.
  * A signal emitted from a teardown path runs handlers against a half-dismantled world.

## 2026-09-22 - what porting an autoload to C++ actually costs, and the crash it hides

`TrackManager` (1023 lines) and `SpatialIndex` (55) became C++ singletons (#184 stage 2). What
GDExtension cannot carry over:

* Enums flatten: `TrackManager.TrackType.TRACK_NORMAL` -> `TrackManager.TRACK_NORMAL`, 370 call
  sites.
* No inner classes (`EndpointRef` -> `TrackEndpointRef`), and no constructor arguments, so
  `X.new(a, b)` becomes `X.new()` plus assignments.
* No float/RID constants, so they became read-only properties (`TrackManager.rail_height`), and
  `UNDEFINED_TRACK` -> `RID()`.
* `Array[Vector3]` -> `PackedVector3Array`, and GUT will not compare packed arrays with literals.
* **The crash:** `RailVehicle3D::_apply_start_track()` used
  `get_node_or_null("TrackManager")->call(...)`, which returned nullptr once the autoload was gone.
  This is why reaching a singleton by path and calling by name are prohibited.
* **Trap:** the first headless run after a rebuild re-imports and can take minutes. Run `--import`
  alone first.

## 2026-09-22 - reading the vehicle state was changing it, in four places

Found by reading every `_do_fetch_state_from_mover()` in #184 stage 1:

* `TrainController::_consume_coupler_sounds()` cleared `TCoupling::sounds` on the Mover, so the
  first reader ate the events.
* `TrainBrake` advanced a filter with `get_process_delta_time()`, so the fall/rise rates depended
  on how often the state was read.
* `TrainEngine` (`engine_start`/`engine_stop`) and `TrainSecuritySystem`
  (`blinking_changed`/`beeping_changed`) emitted from the fetch, comparing against values from the
  dictionary being filled.
* The `state_dirty` guard (one fetch per tick) hid all four.
* **Fix:** the work moved to `_do_process_mover()` / `_handle_mover_update()`, and the fetches
  only read.
* The twelve coupler counters were sound bookkeeping living in the vehicle. The vehicle now emits
  `coupler_attached`/`coupler_detached` (carrying a `CouplingElement`), and `TrainSoundSystem`
  counts per vehicle RID.
* `power_source` was written by `TrainElectricEngine` and `TrainLighting`, and the last to merge
  (FIZ section order) won. Lighting now publishes `light_power_source`.
* **Rules:** a getter never changes state; state lives in the only layer that needs it (see
  `CODE_STYLE.md`).

## 2026-09-22 - the sound system's per-frame cost was not where the loop was

* **Measured** (300 vehicles, 600 banks, 297 in range): 11.8 ms/frame. `controller.state` took
  5.0, `_update_triggers` 4.8, `_update_brake_sounds` 1.5 and `_ensure_brake_events` 0.23. The
  walk over all banks took 0.35 ms.
* **Cause:** untyped trigger Dictionaries re-read and converted per tick, and
  `BrakeSfxEventFactory` tables plus `_primary_source()` walked per event per tick.
* **Fix:** `Trigger`/`BrakeEvent` records resolved once. The frame went 11.8 -> 2.35 ms, and the
  bulk left is `TrainController.state` (~17 us per controller).
* **Rule:** measure by phase before restructuring. The loop the eye finds was 3% of the cost.
* **Trap:** a `RailVehicle3D` without a track has a NaN transform, `NaN > culling_distance` is
  false, and so it is never culled.

## 2026-09-22 - the sfx playback tick moved off the main thread, and what the numbers showed

* **Measured** (200 players with 4 voices each, main thread): 24.7 ms before, 19.7 ms after
  (wait 12.2, flush+observe 2.2, tick on the worker 12.1).
* **Trap:** headless, the main thread has nothing to overlap with, so the total barely moves. The
  split is the real result.
* `modulate()` re-applied every voice synchronously, and the tick recomputes them anyway (7.6 of
  27.9 ms). It now only stores the values and raises `automation_refresh_pending`.
* **Rules:**
  * Work the tick redoes anyway does not belong in the synchronous API path as well.
  * The scene tree is not thread safe, while global-scope servers are. The worker writes
    `SfxVoiceSlot` values, and only the main thread touches `AudioStreamPlayer(3D)`. The tick is
    posted at `process_frame` and awaited at the start of the next frame, so no lock is needed.
* **Trap:** running against a `git worktree` of another commit leaves
  `global_script_class_cache.cfg` stale. Run `--import`.

## 2026-09-22 - a vehicle of the previous scenery left in the strip when the search found nothing

* **Symptom:** an empty search left the previous scenery's vehicles in the strip.
* **Proof:** the screen run as a scene logged `Invalid type in function 'set_tiles' ... does not
  have the same element type as the expected typed array argument`.
* **Cause:** `%TrainsetGrid.set_tiles([])`. A bare `[]` is refused by an `Array[TileGrid.Tile]`
  parameter declared in another script, and the call never runs. An isolated probe passed because
  the class lived in the calling script.
* **Fix:** a typed `var tiles: Array[TileGrid.Tile] = []` with one exit.
  `set_rows(PackedStringArray(), PackedStringArray())`.
* **Rules:**
  * Never pass a bare `[]`/`{}` to a typed collection parameter.
  * Probe a screen by running it as a scene, not with `--script`, which has no autoloads.

## 2026-09-21 - smoke emitters spawned at the origin of the world

* **Symptom:** sm42, st44 and su45 do not smoke, although `get_smoke_statistics()` reports the
  emitter as built.
* **Cause:** the scene cull overwrites `particles_set_emission_transform()` from the instance
  transform, which was left at identity.
* **Fix:** `instance_set_transform()`, with `particles_set_custom_aabb()` in local space.
* **Rule:** a RenderingServer particle system is placed through its instance. Configure a server
  effect the way the equivalent node does.

### The whole plume cut off in one frame on a notch change

* **Cause:** `particles_set_amount_ratio()` deactivates live particles with index >=
  `amount * ratio`. A ratio of 0 on a notch change killed the whole plume.
* **Fix:** emitting is off. `E3DRenderingServer::process_smoke()` (ticked by `SmokeSourceLibrary`)
  accumulates `spawn_rate * intensity * delta` and calls `particles_emit()`: the original's
  `m_spawncount` (`particles.cpp:157-212`).
* **Opacity, twice:** `dizel_fill` in `color` alpha, and then in `color_initial_ramp`, both reach
  live particles, so the plume stepped when the Mover floored it at 0.05 (Mover.cpp:5508). The
  initial opacity ramp is now written once at build time, and `dizel_fill` is folded into the
  spawn rate (`RailVehicle3D::_update_smoke()`). This is a deliberate divergence from
  `particles.cpp:330`.
* **Rule:** `color`, `color_initial_ramp` and `amount_ratio` all reach particles already in the
  air. Only the emission itself affects just the new ones.

### A state key published by one engine part only

* **Cause:** `diesel_max_rpm` was on `TrainDieselElectricEngine`, so a plain diesel got 0 and never
  smoked.
* **Fix:** it moved to `TrainDieselEngine`, via `TMoverParameters::EngineMaxRPM()` (Mover.cpp:1099),
  which covers both variants.
* **Rule:** put a state key on the part that owns the concept, and look for a Mover accessor that
  covers every subclass.

## 2026-09-20 - regressions after the frame-time optimisation night

37 commits (`8bd5c9a`..`ed5ee09`) judged by frame time only. Nobody checked the cabin, the
lighting or the consist.

### The modelled cabin covered by the low-poly interior, its light always on

* **Proof:** the cached template under `user://cache/rail_vehicle/.../303e-ep-tv_*.res` still had
  `LowPolyInterior instancer`, and its `.hash` matched the current `structure-v11` tag.
* **Cause:** `ebecb4b` set `OPTIMIZED` and `8139f8b` removed it, and neither bumped
  `structure-vN`. OPTIMIZED creates no nodes, so there was no `cabN` to hide and no material to
  dim. "Clear cache" missed `rail_vehicle`, `fiz` and `vehicle_profiles`, and `user://cache`
  survives a checkout, so every bisect step was "bad".
* **Fix:** `structure-v12`. "Clear cache" covers every `ResourceCache` (`VehicleProfileManager`
  became `@tool`). The low-poly interior switches instancer with distance
  (`_update_model_detail()`) and restores `cabN` visibility on `e3d_loaded`.
* **Rule:** code whose output is cached on disk bumps the cache tag in the same commit, and a new
  `ResourceCache` joins "Clear cache". When code has no effect, read the cache file (`strings`,
  mtime, `.hash`) first.

### Coupled wagons drifting apart

* **Cause:** `72d3b33` skipped `update_neighbour(end, null, -1, 0.0)` as a no-op. For a coupled
  end it recomputes `Neighbours[end].distance` from `CouplerDist()` (`TrainController.cpp:423-430`,
  DynObj.cpp:7144-7154, called from DynObj.cpp:8193), which `CouplerForce()` starts from
  (Mover.cpp:4781).
* **Fix:** coupled ends refresh every frame. The saving stays for free ends.
* **Rule:** before caching a call as redundant, open the callee and the original line cited above
  it.

### Blotchy, then black ground

* **Measurements:** *Unshaded* was uniform (so lighting). *Normal Buffer* varied (so the material,
  on flat terrain). `grass_normal.dds` R,G is 0.500 at mip 0 and 0.530-0.532 from mip 2 (DXT).
* **Cause:** `material_factory.gd` set `normal_scale = -5.0` (since `f4138c4`, #74), while the
  original applies the map as is (`mat_normalmap.frag:46-48`). The mip bias became a 24 degree
  tilt. `detail_normalmap.gdshader` (registered in `dc25b6f`) applied it to the combined normal of
  `grass.mat`'s detail map (`param_detail_scale: 0.00125`, `param_detail_height_scale: 0.45`,
  `mat_detail_normalmap.frag:53-59`), which made it 5x stronger and reversed.
* **Not the cause:** the three `WorldEnvironment`s (own worlds), the skydome's clouds shadow, the
  terrain normals.
* **Fix:** the override is removed, `NORMAL_MAP_DEPTH = 1.0`, and the material cache key carries
  `MaterialManager.CACHE_VERSION`.
* **Rule:** a tuning factor with no counterpart in the original scales the data's errors along
  with the data.

### Project setting shown as 0/1/2 instead of a named list

* **Cause:** `add_custom_project_setting()` in `libmaszyna.gd` returned early for an existing
  setting. `project.godot` stores only values, so the hint was lost on every restart.
* **Fix:** the value is set only when missing, and the hint and initial value are always
  registered.

## 2026-09-20 - scenery environment, fog and Skydome

### Huge terrain triangles missing under the camera

* **Cause:** `SceneryTrianglesBuilder` stored a whole triangle in its centroid's 1 km cell, and
  streaming goes by the distance to that cell (`SceneryStreamingServer.cpp:53-56`).
* **Fix:** triangles are clipped along the grid (Sutherland-Hodgman in XZ), with each cut computed
  from the lower edge end so no crack opens.
* **Rule:** whatever is streamed or culled by a cell must not reach outside it.

### A winter afternoon turning the fog into orange milk

* **Cause:** Skydome's hard-coded windows put full day above 17.5 degrees of sun elevation and
  sunset colours up to 23.6. A winter sun at 50 N peaks at 16-19 degrees, so the day took night
  fog (24x density) and a sunset tint. The sky shader copies both.
* **Fix:** `day_full_elevation` (6), `sunset_fade_start/end_elevation` (4, 10) as uniforms and
  `gnd_skydome/*` settings.
* **Rule:** check sun-altitude thresholds against a winter day.

### Two fog layers that did not agree

* The original's fog is `1 - exp(-(z / range)^2)`, with `range = fFogEnd / max(1, Overcast * 2)`
  (`apply_fog.glsl:16`, `opengl33renderer.cpp:4685`). A depth fog complete at 1.5x the range with
  curve 1.5 fits it best.
* Volumetric fog is extinction per metre. Its density follows the distance inversely and the
  length stays fixed. Scaling the length made milk, and Skydome's shortening made it pulse.
* The volumetric fog affects the sky (`volumetric_fog_sky_affect` 1.0). The depth fog reaches the
  sky only up to `maszyna/rendering/fog_sky_height`, and rain reaches it fully.
* `fog_aerial_perspective` 1.0 left fogged objects dark at dusk, so it is off by default.
* The `PROPERTY_HINT_EXP_EASING` editor invites values like 0.01, so the value is floored in code.

### A weather change freezing the game

* **Cause:** `MaterialManager._refresh_managed_material()` wrote every material (0.7-3 MB each)
  to disk on the main thread, even without variants (169 of 805 `.mat` files have a rain one).
* **Fix:** no write on refresh, and materials without variants are skipped.

### A new `class_name` unknown to the running game

* `.godot/global_script_class_cache.cfg` is updated only by the editor's scan. Run
  `godot-double --headless --import`.

### Duplicated HUD in the demo scenes

* `TopBar`, `ControlWindows` and the menu were pasted into `demo_3d` and `demo_scenery_loading`.
  They are one scene now (`demo/hud/game_hud.tscn`), and a scene adds its own `Button` under
  `MenuActions`.

## 2026-09-20 - material shaders missing from the wrapper

### "Shader is not supported: Default_1 / reflmap"

* 26 shaders in the original (`mat_*.frag`) against 12 mapped. Missing and in use: `reflmap` (69),
  `detail_parallax_specgloss` (24), `reflmap_specgloss` (17), `default_1` (6),
  `rain_windscreen` (4), `default_detail` (3), `colored` (1).
* Names are case insensitive (`opengl33renderer.cpp:2018`, a Windows FS), so they are lowercased
  in the parser.
* **Rule:** survey the data before trusting a "supported" list. Check the age of `~/src/maszyna`
  against the game's `shaders/`.

### `texture2:` is not always the normal map

* `textureN:` binds shader slot N-1 (`material.cpp:76-81`), and the slot order differs per shader
  (`reflmap`: diffuse, reflmap; `default_detail`: diffuse, detailnormalmap; `water`: normalmap,
  dudvmap, diffuse; `detail_parallax_specgloss`: specgloss before detailnormalmap).
* Without `shader:`, `default_0/1/2` is chosen by texture count (`material.cpp:117-134`), and
  `mat_default_2.frag` is reflmap. Its second texture, also when written as `texture_normalmap:`
  (`material.cpp:60-65`), is read through alpha. About 2500 of 8633 shaderless materials bind one.
* **Fix:** `TextureMap.slots` per shader, resolved by `_texture_path()`, plus a
  `CACHE_VERSION` bump.

### `parallax_specgloss` never received its specgloss texture

* `_apply_parallax()` never set `specgloss_texture` (187 materials), so it read as white.

### Raindrops on the windscreen black as soot

* The atlas is a white rim over black, and the original does not light it
  (`dropTex.rgb * dynBright`, `mat_rain_windscreen.frag`). The port put it in `ALBEDO`.
* **Fix:** `EMISSION`, with the blurred screen luminance standing in for the ambient.
* **Rule:** check what the original multiplies by before moving an unlit term into `ALBEDO`.

### Wipers: data traps

* Only `e186_v2` (and the Vectron cab) has both the `rain_windscreen` glass and wipers, so test
  wiping there.
* `e186_v2/eu47.fiz` ends `WiperList:` with `endL`. The original bounds it by `Size=`
  (Train.cpp:2643), and so does the parser now.
* A script cannot read the shader `TIME`. Pass the elapsed time, not a moment.

### Wiped edge running away from the wiper blade

* A probe read `szyby_wipermask` under the blade: 0.22 ... 1.0 along an sRGB curve. The original
  declares the mask `sRGB_A`, and the port lacked `source_color`.
* The arms are eased (`smoothInterpolate`), and the wrapper feeds the eased value to the shader.
* **Rule:** port every sampler's `#texture (name, index, FORMAT)`. Measure the data under a moving
  part before tuning the motion.

### A whole layer of droplets popping in after a wipe

* **Cause (read, not measured):** the original's `GetMixFactor()` drops the wiper once the factor
  reaches 1, `side` returns to 0, and `side` seeds the droplets, so they are all re-dealt at once.
* **Fix:** the first wiper is kept at factor 1, and returning droplets fade in.

## 2026-09-20 - E186 (dynamic/pkp/e186_v2) not starting up

### Ctrl+J did nothing

* The E186 MMD has no `cabactivation_sw:`, and keys are polled by cab widgets. The original runs
  `OnCommand_cabactivationtoggle` regardless (Train.cpp:3077).
* **Fix:** `LegacyCabinCabActivation`, added by `LegacyCabinLogicDelegate`, like
  `LegacyCabinBattery`.
* **Rule:** every `OnCommand_*` works without its gauge. A control mapped only in
  `MmdSemanticCatalog` is dead in a cab that does not model it.

### Main tank empty within a minute and a half

* **Proof:** pipe 4.4 -> 3.0 bar in 30 s (EP07: 0.03), and `brake_emergency_valve_flow` 0.34. The
  unacknowledged SHP braked, and the handle refilled the pipe from the main tank (`bPantKurek3`).
* **Cause:** `EmergencyCutsOffHandle = false; //@TODO` in `TrainBrake.cpp`, while the FIZ says
  `Yes` (Mover.cpp:10508, `lock_new` at Mover.cpp:4534).
* **Fix:** `TrainBrake.main_pipe_emergency_cuts_off_handle`.
* **Trap:** the parsed FIZ is cached. Bump `FIZ_PARSER_FORMAT_VERSION` with every FIZ parser
  change.

### Pantographs raised but standing still

* E186 has single-arm pantographs with no `ramiegorne2`. The original skips a missing element
  (DynObj.cpp:5414). Only lower arm 1, upper arm 1 and the slider are needed.

### M, D and R dead in the E186 cab

* E186 models `main_sw:`, three `dir*_bt:` buttons and `shp_reset_bt:` (`SeparateAcknowledge`),
  and none of them was in the catalog.
* **Fix:** the labels are mapped (`LegacyCabinMainSwitch`, `LegacyCabinReverser`), with new
  commands `security_cabsignal_acknowledge` and `pantographs_drop_all`.
  `LegacyCabinUnmodelledControls` registers every catalog control whose key no modelled control
  has taken.

## 2026-09-20 - Scenery streaming started from the menu camera

* **Symptom:** the loading screen stayed up to 30 s after 100%, or the terrain was missing without
  it.
* **Cause:** the camera was registered at the demo position `(30, 3, 615)` before moving to the
  vehicle. A worker pass preloaded in `HashMap` order and published only at the end. Plans had no
  camera revision, and `passes > 0 && pending_builds == 0` could report completion mid-preload.
* **Fix:** streaming pauses until the final camera is known. Plans carry a camera revision, and
  preload is published nearest-first. Startup waits only for the camera's own chunk; waiting for
  its 8 neighbours meant 1000+ builds and ~15 s.
* **Rule:** readiness describes built content for a specific camera revision. An empty handoff
  queue is not proof that planning has finished.

### Global transform requested while an E3D node leaves the tree

* **Cause:** `E3DModelInstance` forwarded `global_transform` on the notification while its RID was
  valid, but the node was already out of the tree.
* **Fix:** it forwards only while the node is in the tree.
* **Rule:** a valid RID does not imply that its `Node3D` has a global transform.

## 2026-09-20 - Skydome clouds behind alpha-blended cabin windows

* **Symptom:** visible cloud cover in a cabin with alpha-blended windows pushes a 60 FPS frame
  past V-Sync to 30 FPS.
* **Cause:** the cost of `light_angular_distance` under PSSM, even with the medium filter.
* **Fix:** the fastest filter. The option to disable it is in TODO.md.

## 2026-09-20 - double slips impassable and painted with the missing-texture checker

* **Symptom:** a train stops dead at a double slip, which renders a fan of `missing_texture.png`
  quads.
* **Proof:** a double slip is four `track switch` nodes `_a/_b/_c/_d` (`TTrack::DoubleSlip()`,
  Track.cpp:2593), not `track cross`, which is a road (Track.cpp:419). Of 4819 switches, 1494
  (31%) have branch ends 0.19-0.21 m apart. `_ENDPOINT_EPSILON` was 0.25 m.
* **Cause:** the switch's own ends merged, and `_merge_endpoint_nodes()` chain-merged all 8 into
  one node. `_get_motion_connection()` returned `null` as ambiguous, `_move_vehicle_state()`
  `break`s silently, and `rebuild_track_stitches()` built the fan. The original uses 2 cm per axis
  (`Equal()`, Track.cpp:2121).
* **Checker:** the `.scn` sentinel `none` was kept as a material name. The original uses a null
  handle (Track.cpp:491) and borrows the trackbed from a neighbour
  (`copy_adjacent_trackbed_material()`, Track.cpp:3326).
* **Fix:** a 2 cm per-axis tolerance with its own hash cell, `none` -> empty, and
  `copy_adjacent_trackbed_material()` ported. Over 26960 endpoints only 2 fell in the 2-25 cm
  band.
* **Rules:**
  * A ported tolerance carries the original's value. A "safer" round number merges geometry the
    data placed deliberately.
  * A movement step that cannot resolve the next track must say so.

## 2026-09-21 - the main brake hiss has no interior/exterior distinction to key off

* **Proof:** only 1 of 1344 `airsound*` declarations sets `placement:`. The rest default to
  `general`, which `TrainSoundSystem._soundproofing()` short-circuits to 1.0.
* **Fix:** `pipe_hiss`, `local_brake_hiss` and `emergency_brake_hiss` get a
  `listener_inside` -> GAIN modulation baked by `BrakeSfxEventFactory`, fed 0/1 from
  `_inside_vehicle()`.
* **Rule:** check what the MMD data declares before modulating a sound with a parameter.

## 2026-09-21 - a +38 dB SfxTrack under the cab hiss, and five rounds of guessing instead of one dump

* **Symptom:** the main brake release hiss is deafening in the cab of every FV4a vehicle, and no
  gain change was audible.
* **Found by:** the Remote tree. `pipe_hiss` (`airsound2`) had `track.volume_db = 38`, while every
  other track was ≤ 0.
* **Cause:** `_signed_flow_automation()` computed
  `maximum_gain = output_scale * (offset + factor * gain_signal_max)` as the curve divisor and
  then also applied it as `volume_db`. For su45: factor 40000, `maximum_gain` 79.98, 38.06 dB.
* **Fix:** `_track_gain()` caps the track at `minf(maximum_gain, 1.0)`, so `maximum_gain` works as
  a divisor only.
* **Rules:**
  * Dump the whole built bank (every event, each clip's `track.volume_db`) before touching a sound
    constant.
  * A gain derived as a normalisation divisor must never also be applied as a gain.

## 2026-09-21 - distant buildings cut out of the fogged sky, whatever the fog distance

* **Proof:** the sky pixels were exactly the fog colour `(149, 113, 95)`, and distant objects came
  out brighter `(170, 133, 113)`. That is only possible with a fog amount above 1.
* **Cause:** `fog_density` was applied twice (scaling `day/night_fog_density` and as
  `storm_fog_intensity`), and Skydome sums the two (`Skydome.gd:1265`, clamp 1.5). Godot does not
  clamp `fog_amount`. Separately, `fog_sky_affect` ignored the density.
* **Fix:** the base density stays Skydome's haze (0.005/0.02), and the storm carries
  `fog_density - base_density`. `sky_affect` is multiplied by the opacity.
* **Rules:**
  * An opacity summed from two sources is summed where it is set. An unclamped value turns a blend
    into an extrapolation.
  * The sky and the geometry are fogged by different parameters (`fog_sky_affect` vs
    `fog_density`), and they agree only when the sky carries the depth fog's opacity.

## 2026-09-21 - the whole scenery unlit, day and night, since the OPTIMIZED instancer

* **Data:** model-node `lights` modes across 975 files: `ls_Dark` 3180, `ls_Off` 1797, `ls_On`
  733, `ls_Home` 607, `ls_Blink` 15. `stary_jawor_noc` alone has 1001 light-bearing models, 1430
  groups and 452 `FREE_SPOTLIGHT`.
* **Cause:**
  * `e3d_parser.cpp` hides `light_on*`, and `lights_state` lived on the node, while scenery models
    are node-less RIDs since `2125898`.
  * The importer dropped `lights`/`lightcolors` (`obj.lights` commented out since `923b293`).
* **Fix:** the light state lives in `E3DRenderingServer`, resolved against the time of day and
  the light level from `MaszynaEnvironmentNode`. Real lights are server RIDs streamed with their
  own range. The `light_onNN`/`light_offNN` pairing is `E3DLightFactory::discover()`.
* **Rule:** state an instancer must honour belongs to the server. A light that is never switched
  on looks exactly like one never implemented.

### Godot's spot cone stops at 90 degrees, the data's does not

* 6 of 871 `FREE_SPOTLIGHT` are wider: `elektryczne/lampa_parkowa01` at 117 degrees,
  `nastawnie/nastawnia_laziska_huta_lh1` at 150, and 4 more. They become omni lights.
* **Rule:** check the data's value range (a histogram) before mapping a parameter one to one.

### A street lamp that lights nothing

* Nine `latarnia*` models have no `FREE_SPOTLIGHT`, and in the original they only drew a glare
  (`opengl33renderer.cpp:4646`). Their halo billboard (`elektryczne/poswiata`) and ground quad
  (`elektryczne/light1|2`) are found by material, never by name.
* The halo carries the lamp colour: mercury `(0.61, 0.59, 1.0)` for `betdziur`, sodium
  `(1.0, 0.66, 0.18)` for `lbc`/`str`, warm `(0.90, 0.84, 0.64)` for `drew`/`hs`.
* **Rule:** read the geometry drawn for a glow before adding a tuning constant.

### The lit patch says how wide the cone is, not where the light ends

* The double-armed `latarniay_*` have two halos (z ±0.76 on `str`, ±0.99 on `betdziur`), and the
  quirk took the first. That affected 26 of 124 lamps in `stary_jawor_noc`.
* The patch is 15.0 m across in all nine models, while its other axis covers the arms
  (15.1-22.0 m), so `max(x, z)` was wrong.
* The patch edge is not the range. Declared street lamps use 40 m (`lampa_parkowa01`, mounted at
  4.9 m) or 80 m (`linia053/lamp-y`, `lamp-5`, `lamp-i`).
* **Rules:**
  * The patch's width is data; its edge is not a falloff radius.
  * A constant identical across a whole family is the one the author meant.

### A RenderingServer light is not a Light3D - it inherits none of the node's defaults

* **Symptom:** shadow acne bands radiating from the lamp.
* **Cause:** `spot_light_create()` starts with the server's defaults: reverse cull face on, and
  biases other than the node's (spot 0.03 / omni 0.1, normal bias 1.0). The project setting
  `lights_shadow_reverse_cull_face=false` was read but never written into the light.
* **Fix:** every shadow parameter is set explicitly after creation.
* **Rule:** porting from a node to a RID carries nothing over. Set each parameter the node's
  constructor sets.

## 2026-09-21 - "the release runs old GDScript" - the release was never unpacked into the game dir

* **Proof:** the zip's `.so` matched `demo/bin/...` and the pck's `build_number.txt` was fresh. The
  game dir's `reloaded` had mtime 23:11 and md5 `5aa4a837...` against a fresh `4fa78d7a...`, and
  the repo root held the unpacked files.
* **Cause:** `upgrade-*.sh` ran `cd <repo> && ... && unzip -o <zip>`, which unpacked into the repo.
* **Fix:** `make -C "$REPO"` and `unzip -o "$REPO/bin/linux/<zip>" -d "$GAME"`.
* **Rules:**
  * When a build "has no effect", prove the binary being run is the one built (mtime, md5).
  * A one-liner that `cd`s and uses a relative destination has two working directories. Name the
    destination absolutely.

## 2026-09-21 - no fog in the exported release, perfect fog in the editor

* **Symptom:** no fog at 80 m and a white-out at 4160 m in the release. The editor is fine.
* **Ruled out:** a stale binary. The symlinked addons (`gnd_skydome`, `gnd_weather`, `gnd_sfx`,
  `libmaszyna`) are exported with their content.
* **Instrument traps:** `grep -c` on a binary counts lines. `FileAccess.file_exists()` on a `.gd`
  in a pck is false (`.gdc` + `.gd.remap`), so inspect the pck with `load_resource_pack()` +
  `DirAccess`.
* **Cause:** `Script.get_property_default_value()` returns null for every property of a GDScript
  compiled into a pck. The control was `maszyna_model_data.gd`. `SkydomeSettings.get_value()` falls
  back to it, and the `gnd_skydome/*` settings do not exist in a release, so every value became 0.
* **Fix:** `GndSkydomeMaszynaEnvironment._skydome_value()` falls back to the live Skydome node's
  value, cached on first read because five call sites write it back scaled.
* **Rules:**
  * A default that exists only in the script's source does not survive export. Fall back to a
    live object.
  * A setting registered by an EditorPlugin does not exist in an export unless it is in
    `project.godot`, and values equal to the default are exactly the ones kept out.
  * Test the invariant, not the intermediate (density × length, not density).
