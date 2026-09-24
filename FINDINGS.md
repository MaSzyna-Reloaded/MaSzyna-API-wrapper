# Findings

Root causes that took a measurement to find. Each entry: the symptom, what proved the cause, the
fix, and the rule it leaves behind. Open work belongs in `TODO.md`, not here.

## 2026-09-24 - every force of the E186 turned NaN once a direction was set

* **Symptom:** `get` showed `velocity`, `Ft`, `brake_unit_force` and the wheel angles as `nan`
  as soon as the reverser left neutral with the line breaker closed; the breaker then dropped.
* **What proved it:** a headless probe on `p160dc.fiz` printing the forces every half second -
  `Ft`, `Mm`, `Im` went `nan` in the first step after `direction_increase`.
* **Cause:** the EIM traction step divides by `InvertersNo` (`InvertersRatio`, Mover.cpp:5627).
  `LoadFIZ_Engine` gives a powered EIM without `InvNo` one inverter and sizes `Inverters`
  (Mover.cpp:11302); the wrapper did neither, so 0/0. The same block also defaulted `fcfuH` to
  `fcfu` (Mover.cpp:11290) and read `Volt`, `abed`, `edep`, `eimclf`, `InvCtrCplFlag`, `Flat` -
  none of which reached the Mover; all are ported now.
* **Rule:** porting a `LoadFIZ_*` block means porting what it does after the `extract_value`
  lines too - derived counts, container sizes, fallbacks to another key.

## 2026-09-24 - the E186 line breaker opened the moment it closed

* **Symptom:** on `td_e186.scn`, with the pantographs up, the main switch (line breaker) could not
  be switched on.
* **What proved it:** a headless probe building `p160dc.fiz` with the cab occupied and 3000 V fed
  to both pantographs: `main_switch_closable` went true, the `main_switch` command returned true,
  and three frames later `main_switch_enabled` was false again.
* **Cause:** an induction motor opens the breaker when the line voltage is above
  `CollectorParameters.MaxV + 200` (`Mover.cpp:5706`). The original reads FIZ `MaxVoltage` into
  both `EnginePowerSource.MaxVoltage` and `CollectorParameters.MaxV` (`Mover.cpp:11622`); the
  wrapper wrote only the first, so `MaxV` stayed 0 and any voltage above 200 V knocked it out.
  Series motor vehicles (EP07, EU07) never showed it - their relay treats `MaxV` only with
  `OverVoltProt`, which they do not set.
* **Trap in the regression test:** the knock-out sits in `TractionForce()`, which the Mover runs
  only with `Power > 0`, and only for a driven vehicle. The first two versions of the test passed
  with and without the fix; it was checked against a build with the line commented out before
  being kept.
* **Rule:** one FIZ key can feed several Mover fields. Porting a key means grepping every
  `extract_value(..., "Key", ...)` for it, not the first one found.

## 2026-09-24 - every spring brake started shut off, because the struct defaults were kept

* **Symptom:** on `td_e186.scn` it was unclear whether the spring brake did anything at all, and
  the debug window's "Enable" button shut it off.
* **What proved it:** a headless probe building a vehicle with e186's `SpringBrake:` values and
  printing `spring_brake/*` once a second: `shut_off=true` and `is_ready=false` from the first
  frame. The cylinder was filled only through the pneumatic bypass (`BP`), never to `MaxSP`,
  because `UpdateSpringBrake` takes `MSP = ShuttOff ? 0 : MaxSetPressure` (`Mover.cpp:4836`).
  With the train brake released the spring brakes the vehicle whatever the driver does.
* **Cause:** the original's `LoadFIZ_SpringBrake` ends with `ShuttOff = false; Activate = false;
  IsReady = true;` (`Mover.cpp:11028`), and the struct defaults (`ShuttOff{true}`,
  `IsReady{false}`) describe a vehicle *without* a spring brake. The port copied the parameters
  and dropped those three lines. Putting them in the `if (!Cylinder)` branch did not help:
  `CheckLocomotiveParameters()` creates a fallback cylinder (`Mover.cpp:8881`) before any
  component is configured, so that branch never runs.
* **Found on the way:** `set_spring_brake_enabled(true)` called `SpringBrakeShutOff(true)`, and
  the test locked that inverted meaning in; the valve areas were read straight while the original
  reads FIZ `ValveOnArea` into `ValveOffArea` and vice versa (`Mover.cpp:11025`); and
  `MTC` defaulted to 0 instead of the original's 127, so a FIZ without it never passed the
  command along the consist. `i-springbrakeactive` showed the switch (`Activate`) where the
  original shows the spring braking (`IsActive`, `Train.cpp:9195`).
* **Rule:** porting a FIZ section means porting the whole loader function, including the state it
  sets at the end - not only the `extract_value` lines. A struct default is what a vehicle
  without that section gets.

## 2026-09-24 - the Linux release would not start on most machines, because of the build host's glibc

* **Symptom:** the Linux zip starts on the machine it was built on and fails on "many PCs".
* **What proved it:** `objdump -T <file> | grep -oE 'GLIBC_[0-9.]+' | sort -Vu | tail -1` on both
  files of the zip. `libmaszyna.64.so` asked for **GLIBC_2.43** (`sqrtf`, `atan2f`, `acosf`, and
  2.38 for `__isoc23_strtol`), and `reloaded` - the double precision export template built on the
  same Manjaro host - for **GLIBC_2.44**. Ubuntu 24.04 has 2.39, Debian 12 2.36, Ubuntu 22.04 2.35.
  libstdc++ was never the problem: it is linked statically, and `NEEDED` lists only libc, libm and
  the loader.
* **Trap:** the repository's CI image (`jezsonic/build-tools:4.7.stable`) is Ubuntu 26.04 with
  glibc 2.43, so moving the build into "the docker we have" changes nothing. And rebuilding only the
  library is not enough - the template binary asks for the newer glibc of the two.
* **Fix:** `make release-linux` builds both in `ci/docker/linux-sdk`, Godot's own buildroot SDK -
  the one Godot 4.7's official builds use (`godot-2026.05.x-1`, GCC 15.2, glibc 2.34). The template
  is built once from the `4.7.2-stable` sources and installed where the export looks it up.
* **Rule:** glibc is only forward compatible. What a release needs is decided by the build host,
  not by the code - check the highest `GLIBC_` version of every shipped binary, and build on a
  sysroot at least as old as the oldest distribution meant to run it.

## 2026-09-24 - switch blades in a scenery never moved, because only a node listened

* **Symptom:** throwing a switch in a loaded scenery changes the route - the train takes the other
  branch - but the blades stay where they were built. `TrackSwitch3D` nodes in `demo_3d` still
  animate, so it read as "it used to work".
* **Cause:** `TrackManager::_process_switches()` steps the blade offset and emits
  `switch_offset_updated` every frame of the move, and the only listener turning that into blade
  movement was `TrackSwitch3D._on_track_manager_switch_offset_updated()`. Since `923b293` a
  scenery builds its tracks through `TrackRenderingServer`'s RID API with no nodes at all, so the
  blades got their pose once, in `_stream_build()`, and nothing ever moved them again.
* **Fix:** `TrackRenderingServer` subscribes to `switch_offset_updated` itself and moves the
  blades of whichever render track belongs to the switch; the node's copy is removed.
* **Rule:** the same shape as the scenery lights (2026-09-21): a feature parked on a node vanishes
  silently the moment an instancer without nodes appears. The server that owns the visuals
  subscribes to the manager's events itself.

## 2026-09-24 - a consist ringing like metal, and the original naming the bug in a comment

* **Symptom:** from outside, a moving consist sounds metallic - a ringing colour that no single
  vehicle has. Reported as "podwójne dźwięki", which is exactly what comb filtering sounds like.
* **Ruled out first, cheaply:** the built banks. A headless probe walked the `SfxPlayer3D`s of
  three vehicles and printed every event name per player - EP07 5/3/30, E186 7/4/9, no name twice
  in a bank and none across two banks of one vehicle. So nothing was registered or played twice,
  which is where the search would otherwise have gone.
* **Cause:** every wagon of a consist plays the *same* running-noise recording, and they all start
  it at the same moment - when their bank is registered. Identical loops a few metres apart comb
  against each other, and the effect grows with the number of wagons.
* **The original names it** at the one place it works around it: "potentially adjust starting point
  of the last buffer (to reduce chance of reverb effect with multiple, looping copies playing)"
  (`audiorenderer.cpp:99`). Its answer is `m_outernoise.start( Random( 0.0, 80.0 ) * 0.01 )`
  (`DynObj.cpp:6511`) - a start offset drawn once per vehicle when the model is loaded.
* **Two details worth keeping:** the offset is a **fraction of the sample**, not a time - the
  original multiplies it by the buffer size - so a short recording is shifted as much as a long
  one; and it is drawn **once per vehicle**, not per playback, so a vehicle keeps its own phase.
* **Fix:** `TrainSoundSystem` draws `randf_range(0.0, 0.8)` per bank runtime and starts a looping
  running sound at that fraction of its own clip. `gnd-sfx` needed no change at all -
  `SfxPlayer.play()` already takes an offset, which is worth knowing before extending it.
* **Not the clatter.** The wheel clatter was already right: `RunningSoundModel._wheel_clatter()`
  phases each axle by its own position along the vehicle and the rail joint spacing, which is the
  original's own model (`DynObj.cpp:3671-3730`, each axle's `distance` seeded from `axle.offset`).
  A one-shot per rail joint does not comb; a shared loop does.
* **Rule:** when many copies of one sound play at once, the defect is phase, not level. Look for a
  start offset before touching a gain, and check whether the player already has one.

## 2026-09-24 - the pantograph lost the wire where the original keeps it, in four different ways

* **Symptom:** driving an EP08 on `zwierzyniec_tlk`, the line voltage drops a few times per run and
  trips the main switch. The operator's own words: "w oryginale tak nie było". Later, after two of
  the four causes were fixed: it still dies **exactly at the exit of one switch, at any speed**.
* **Measured before anything was read.** Every traction span of the scenery (1335 spans, 2670
  ends): 2414 ends have exactly one neighbour, 116 are genuine line ends, **126 have three** -
  four spans meeting over a switch - and exactly **two** ends anywhere have a gap wider than the
  joining tolerance. So the wiring is not holed; choosing among the spans is the whole problem.
  A second pass killed the next two hypotheses just as cheaply: all 1387 joined pairs differ in
  height by **0.000 m**, and every span around the reported spot is dead flat.
* **Four separate divergences from the original, each enough on its own:**
  * **No guide horn.** The original accepts a wire up to `fWidthExtra` = 0.381 m outside the
    slider and counts it as geometrically higher (`scene.cpp:105-112`, `DynObj.cpp:93`). Its
    comment describes this exact failure: "problem jest, gdy nowy drut jest wyżej, wtedy pantograf
    odłącza się od starego, a na podniesienie do nowego potrzebuje czasu". The wrapper had one
    width and dropped anything beyond it.
  * **The slider width never came from the data.** `RailVehicle3D::pantograph_collector_width`
    sat at its header default of 0.5 for every vehicle in the game; the only assignment in the
    repository was in a test. An EP08 declares `CSW=1.4`, so the original searches 0.7 + 0.381 =
    1.081 m to each side and the wrapper searched 0.5 - less than half.
  * **No chain.** Running off the end of a span is not a loss of contact: the original steps along
    `hvNext` until the point falls inside a span (`DynObj.cpp:8742`). The wrapper dropped the wire
    and re-searched the area, which papered over it with a 0.25 m tolerance at each span's end.
  * **`iLast` - and this is the one that killed it at the switch.** The original marks a span that
    ends a section, or whose neighbour does (`TTraction::WhereIs()`, `Traction.cpp:392`), and for
    such a span it **does not follow the chain at all** - "dla ostatniego i przedostatniego przęsła
    wymuszamy szukanie innego; nie to, że nie ma, ale trzeba sprawdzić inne" (`DynObj.cpp:8747`).
    A switch exit is exactly that: the branch ends there. The wrapper followed `next[]`, which at a
    three-way meeting had been chosen first-come-first-served, onto a span the switch does not
    continue into.
* **A fifth, found by the same reading:** the joining tolerance was 0.25 m as a euclidean radius
  where the original uses **0.025 m per axis** (`TTraction::TestPoint`, `Traction.cpp:355`). At 25
  span ends of this scenery that made us see three candidate neighbours where the original sees
  one. It was harmless while nothing walked the chain, and stopped being harmless the moment
  something did.
* **`parallel` and `section` were parsed and thrown away.** Both are read by the importers into
  their data holders and neither reached the server: a span sharing a running (`hvParallel`) must
  not trust the chain either, and a supply declared `section` is *not* a substation - it names the
  part of the network a span belongs to, and the power has to reach it along the wires
  (`TTraction::PowerSet()`, `Traction.cpp:460`). `TTraction::VoltageGet` was ported whole with
  them, including a star branch that this wrapper had written down as unreachable.
* **A trap in that data, worth knowing before reading a scenery's wiring:** every supply is
  declared **twice** under one name, once `section` and once as a substation. The original's name
  table keeps the **last** one (`Names.h:38`), and in these files the substation is last - so both
  engines end up treating `pwr01`/`pwr17` as substations, and the section half changes nothing
  here. A port that had picked the first declaration would have unpowered half the scenery.
* **Rule:** measure the data before reading the code. Three of the first four hypotheses - gaps
  between spans, a height step at a junction, an arm lagging a rising wire - each died to a single
  pass over the scenery's own numbers, and each would have cost an afternoon in the debugger.
* **Rule:** a tolerance that "papers over" data is load-bearing the moment something else starts
  trusting the structure underneath it. The 0.25 m span-end tolerance and the 0.25 m join radius
  were both harmless until the chain walk was ported, and then they chose the wrong wire.
* **Rule:** a raised pantograph reads 0 V in three different ways - no wire in reach, a wire
  carrying nothing, and a wire it is not touching (`PantDiff >= 0.01`, `DynObj.cpp:3866`) - and
  from the cab all three look identical. Report them separately, with the track and the offset,
  or every diagnosis starts by guessing which one it was.

## 2026-09-24 - a parked vehicle jumping, because two writers disagreed about where it stands

* **Symptom:** at zero speed the vehicle's transform changes slightly and it visibly jumps. Only
  on curves, most of all on one switch, never on straight track - and the position readout does
  not move while it happens.
* **What that combination says on its own:** the position is stable and the attitude is not, and
  the difference is a function of curvature. Only two placements in this code have that property.
* **Cause:** `RailVehicle3D::apply_track_placement()` wrote the body transform **twice**. First
  unconditionally, from `RailVehicleServer::vehicle_get_transform()` - the track sampled under the
  vehicle's centre. Then, when `moved || force_detail_refresh`, again, from a chord it composed
  between the two bogie pivots. On straight track the two agree exactly; on a curve they do not,
  and on a switch they differ most. A parked vehicle therefore showed the centre one every frame
  and the chord one in any frame where something raised `force_detail_refresh` - entering the
  screen, a detail switch, a rebuilt animation binding.
* **Fix:** the composition moved to `RailVehicleServer`, which owns the placement it is made of:
  a vehicle on bogies has its body built from the two pivots, cached against the placement, and
  `RailVehicle3D` takes that one answer and nothing else. The node keeps only what draws - putting
  the bogie nodes where the wheels say they are.
* **Rule:** one piece of state, one writer - and a second writer is not obvious when both look
  correct in isolation. Here the two agreed on every straight track in the game, which is most of
  it, so the disagreement only ever showed where the geometry made it visible.
* **Trap met on the way:** a vehicle built with no mass integrates to **NaN**, and a NaN transform
  never equals itself, so every "did it move" test says yes forever. A fixture without a mass is
  not a vehicle at rest, it is no vehicle at all.

## 2026-09-24 - a .fiz in the project stopped importing, silently

* **Symptom:** `godot-double --headless --path demo --import` prints `Error importing
  'res://tests/fixtures/test_vehicle.fiz'` and rewrites its `.import` with `valid=false`. Nothing
  else says anything, and the tracked `.import` still claimed `type="PackedScene"`.
* **What proved it, in one run:** a probe calling `FizVehicleBuilder.build_model_at()` on the
  fixture and then saving the result by hand. The model built fine (28 properties, 6 components);
  `ResourceSaver.save(model, "...scn")` returned **15**, `ERR_FILE_UNRECOGNIZED`, and the same
  resource saved to `.res` returned 0.
* **Cause:** `.fiz` stopped producing a `PackedScene` when a vehicle became a `VehicleModel`, but
  `FIZImportPlugin._get_save_extension()` still said `"scn"` - and `.scn` is the packed-scene
  extension, which `ResourceSaver` refuses to write a plain `Resource` into.
* **Trap:** Godot does not reimport after `touch` - it compares the source's md5 - so testing an
  importer change means deleting the artifact under `.godot/imported/` first. Two runs looked like
  the fix had not worked when it had.
* **Rule:** an `EditorImportPlugin`'s save extension is part of its contract with `ResourceSaver`.
  When `_get_resource_type()` changes, that extension changes with it, and the only symptom of
  getting it wrong is one line in an import log.

## 2026-09-23 - the cab acted one keypress late, because the dump was cached per step

* **Symptom:** a key in the cab plays its sound at once, but the operation only happens when the
  next key is pressed - so every command appears to lag one keypress behind.
* **Cause:** `RailVehicleServer::vehicle_dump_state()` composes the vehicle's state once per
  physics step and hands the same `Dictionary` to every reader of that step. Its own comment
  stated the premise - "the values cannot change between them, because only a step changes them" -
  and that premise is false: `TrainSystem::send_command()` runs the command synchronously, in the
  middle of a step. A cab widget reports a manipulation and reads the state in the same call
  (`CabinSwitch._on_command_received()` -> `_update_state()`), so it read the values from *before*
  its own command and only caught up when the next command forced a rebuild.
* **Fix:** the cache is keyed on the step **and** on a command counter the vehicle owns.
  `VehicleController::command_executed()` is now the one named operation for "a command has run
  against this vehicle" - it bumps the serial, updates the state and announces it, replacing the
  `update_state()` + `emit_command_received_signal()` pair at the call site.
* **Why not key it on `update_state()` alone:** the step calls that only for a vehicle whose
  physics is active (`RailVehicleServer.cpp`), so a parked or unmanned vehicle would keep a dump
  that nothing ever invalidates.
* **Rule:** a cache keyed on a tick is only correct while the tick is the *only* thing that
  changes what it holds. Write that premise down where the cache lives - and when something
  synchronous can change the same state, the key needs a second half.
* **Rule:** the lag a user reports as "one action late" is a read of a snapshot taken before the
  write, not a slow write. Look for the cache between them before looking at the input path.

## 2026-09-23 - the release re-parsed every scenery because its game dir was "."

* **Symptom:** "scenerie sie nie cachuja, tylko zawsze ida parsing" - in the shipped build every
  scenery and every large include is parsed from scratch on each launch, while the same data
  caches correctly in the editor.
* **First diagnosis, and it was wrong:** the build stamps itself on every build
  (`ADD_CUSTOM_TARGET(build_number ALL)`), and `MaszynaRuntime::check_build_version()` clears
  caches when the stamp moves, so an `upgrade-linux.sh` cycle looked like a guaranteed wipe.
  Reading the code killed it: `clear_cache()` only emits `cache_clear_requested`, and the only
  listeners are `material_manager.gd` and `vehicle_profile_manager.gd` - the scenery cache is not
  connected to it at all.
* **The experiment that pointed the right way:** two headless loads of the same scenery with no
  rebuild between them. The second one did not rewrite its cache entry, so the scenery cache hits
  and nothing about it is broken.
* **What proved it:** loading every `user://cache/scenery_compiled/*.res` and running
  `_is_cache_valid()`'s own checks over it. The entry written by the editor carries
  `src=/home/marcin/Games/MaSzyna/scenery/td.scn` and all 36 dependencies resolve; the five
  entries written by the release carry `src=scenery/baltyk/mod/drogi.scm` and **102 of 102
  dependencies report missing**.
* **Cause, in two halves that only fail together:**
  * `UserSettings::get_maszyna_game_dir()` returns `"."` in an exported build
    (`UserSettings.cpp:119`, there since `d3e7d52`), so `_get_source_path()` records
    `scenery/<file>` and every dependency with it;
  * a bare relative path handed to `FileAccess` resolves against **`res://`**, not against the
    process's working directory - measured: `FileAccess.file_exists("project.godot")` is true and
    `file_exists("README.md")` is false with the repository as the cwd. In an export `res://` is
    the embedded pack, which holds no `scenery/` at all.
  So `_is_cache_valid()` failed on the first dependency of every entry, every time. Parsing still
  worked, because the parser reaches the files by a different route - which is exactly what kept
  the defect invisible.
* **Fix:** the game dir is a real directory in a release too - the one holding the executable
  (`OS::get_executable_path().get_base_dir()`). The editor keeps reading the setting. As a side
  effect the editor and the shipped build now write cache entries under the same key, and the
  game no longer depends on the process's working directory.
* **Rule:** a path that will be handed to `FileAccess` is absolute, or it is silently a `res://`
  path. `"."` for "where the game lives" only works while something else happens to resolve it,
  and it fails in the one place that merely *checks* a file rather than opening it.
* **Rule:** when a cache "does not work", load an entry and run its own validity check over it
  before suspecting whatever invalidates it. Here the invalidation was innocent, the cache hit
  correctly in the editor, and the whole defect was in one field's value.

## 2026-09-24 - the shipped library had no symbols, and the crash was in the parser

* **Symptom:** closing the game during loading segfaults. Two cores, both on a non-main thread,
  both with a first frame that is not code (`#0 0x0` and then a wild address).
* **What cost the most time:** the shipped `libmaszyna.64.so` was **stripped**, so every frame of
  our own library read `?? ()`. Two cores were diagnosed by the *shape* of the stack alone, and
  both guesses were wrong. godot-cpp links with `-s` unless `DEBUG_SYMBOLS` is on, and that is
  `$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>` - a generator expression on **its** target,
  propagated to ours through `TARGET_LINK_LIBRARIES`. Removing `-s` from our own target's
  `LINK_OPTIONS` (which the build already did for `template_debug`) therefore changes nothing for
  a release: the flag was never there. `make release-linux-symbols` builds `RelWithDebInfo` with
  `template_release` instead, which is what actually keeps the symbols.
* **With symbols, one backtrace named it:**
  `MaszynaParser::parse_chunk (maszyna_parser.cpp:270)` -> `Callable::call(...)` -> freed code.
  The parse runs as a `SceneryLoadingTaskQueue` task, i.e. on a worker, and every token is
  dispatched to a GDScript handler through a `Callable`. `callback.is_valid()` is checked and does
  not help: during teardown the object still answers, the script behind it does not.
* **Cause:** nothing stopped the loading queue before the scripts went away. Its destructor does
  drain - drop the pending tasks, join the workers - but it runs when the last reference to a
  `RefCounted` created inside an awaiting coroutine goes, which is *during* that teardown rather
  than before it. The streaming server's planning thread had the same defect and the same shape of
  fix the same day; this is a second worker nobody had drained.
* **Fix:** `SceneryLoadingTaskQueue::drain()` is callable from outside, `SceneryInstancer` keeps
  the queues that are parsing in `_active_queues` and `cancel_loading()` drains them, and
  `maszyna_include.gd::_exit_tree()` calls it before anything is freed - while the scripts are
  still there to be waited for.
* **The fix's own regression, and the lesson in it:** draining a queue means *waiting* for the
  task that is running, and a scenery parse takes seconds. Quitting mid-parse therefore stopped
  crashing and started hanging - the window would not close, and on Windows there is no shell to
  interrupt it from. A stop that waits for the work to finish is not a stop. `MaszynaParser`
  already had an `interrupted` flag for its own budgeting; it now also carries a static
  `cancelled` that the token loop checks, and the GDScript half of a loading task
  (`_count_includes`, which reads every included file) checks the same flag - so the join has
  something short to wait for.
* **And the fix for that had a deadlock in it.** `drain()` drops the queued tasks - but a task in
  this queue may `wait()` for a task it submitted, which the header says outright. Dropping a
  queued task therefore left its waiter spinning in `wait()` for something that would never run,
  the worker was never joined, and `wait_to_finish()` on the main thread blocked for good: the
  window simply never closed. Named from a core taken with `kill -ABRT` on the hung process -
  several threads in `SceneryLoadingTaskQueue::wait` (`:90`) reached from `_run` (`:146`), all on
  the same queue. `wait()` and `is_done()` now give up while the queue is draining.
* **Reading a hang costs nothing, and that is the lesson.** `ptrace_scope=1` blocks attaching a
  debugger to a process that is not a child, but `/proc/<pid>/task/*/wchan` needs no privileges
  and already said "main thread in futex_do_wait, 41 of 62 threads waiting" - and `kill -ABRT`
  turns the hang into a core with a full, symbolised stack. Three guesses were spent before that;
  the measurement took a minute.
* **Not covered by a test, and the attempt is worth recording.** A test that frees a scenery
  mid-parse and asserts the teardown is quick passes *with and without* the fix: headless, the
  parse of `td.scn` is over before the test can interrupt it. A green test that cannot fail is
  worse than none, so it was deleted rather than kept.
* **Rule:** a shipped build keeps its symbol table. The frames worth reading in a crash are the
  extension's own, and without them a core costs hours and still ends in a guess.
* **Rule:** a `Callable` held across a thread boundary is only as valid as the script behind it,
  and `is_valid()` does not tell you that. Whoever owns the thread stops it before the scripts go.
* **Rule:** every worker in the process needs an owner that stops it at teardown. Two were found
  in one day by the same symptom; a destructor is not that owner, because it runs too late.

## 2026-09-24 - the simulation stepped after everything that reads it

* **Symptom:** vehicles judder, worst seen from the external view; a single locomotive does it
  too, so it is not the couplers.
* **Two wrong turns first, both mine.** The frame-dependent `sub_step` looked like the cause, so
  the step was made fixed - which made it *worse*: with a fixed 10 ms step and no interpolation
  the drawn position advances 1 step on some frames and 0 on others, and the 20-step backlog cap
  turns a stalled frame into a lurch. Then the cabin shake looked like it, until the operator said
  the worst of it is the exterior view, which that shake cannot reach. Reverted both.
* **What settled it:** `git log -S` on the old GDScript server. `4ec5490` had already fixed this
  once and its message says the opposite of the first hypothesis - the *fixed tick* was what made
  vehicles judder, and stepping on the rendered frame with a variable delta is what the original
  does (`vehicle_table::update(Deltatime, Iterationcount)`, DynObj.cpp:8181). The same commit
  added `process_priority = -100`, "so the step has to come first - otherwise the vehicles render
  the position of the previous frame".
* **Cause:** the C++ port kept the variable delta and the iteration count, but drove the step from
  `SceneTree`'s `process_frame`, which is emitted **after** every node has been processed. It
  compensated by pushing the new placement onto `RailVehicle3D` at the end of the step - which
  fixes the vehicle's own transform and nothing else. Every other reader that takes a vehicle
  transform in its own `_process` (`ExternalCamera._process()` computes the camera from
  `_view_vehicle.global_transform`) still ran before the step and drew against the previous
  frame's position.
* **Fix:** `RailVehicleStepper`, a node with `process_priority = -100` that calls
  `RailVehicleServer::step_frame()`. The simulation is deterministically through its physics
  before any node is processed, so drawing and state both see the new data in the same frame -
  which is what the priority guaranteed before the port. `process_frame` is left to the servers
  whose work nothing reads back in the same frame.
* **Rule:** `process_frame` is not "the start of the frame", it is the end of one. Anything a node
  reads in its `_process` has to be produced before the `_process` phase, and the only ordering
  Godot gives inside it is `process_priority`.
* **Rule:** when a fix is being reinvented, find the commit that made it the first time
  (`git log -S` on the moved code) and read its message before proposing the opposite. Two of the
  three things tried here had already been decided, with reasons, in `4ec5490`.
* **What the fix left open, and how it was closed.** Taking the frame delta as it comes means a
  stalled frame is integrated with `sub_step = delta / MAX_PHYSICS_ITERATIONS`, which past 0.2 s
  is larger than `PHYSICS_STEP` - the one thing that constant exists to prevent. Dropping the
  excess was the obvious answer and it is wrong here: the scenario's events and multiplayer are
  driven by time, so a simulation running slower than the clock drifts out of both. `step_frame()`
  therefore *owes* what it cannot integrate and the following frames pay it off, and only past
  `maszyna/physics/catch_up_limit` does it take the debt in one step - a visible jump, logged,
  because a jump beats a clock that lies. The debt is reset whenever stepping starts, since time
  that passed while the simulation was stopped is not time it failed to integrate.
* **Trap it exposed:** `test_process_movement_with_invalid_controller_reference_is_noop` passed
  only because the step used to run after GUT's coroutine resumed. Its premise was never true -
  `controller = null` drops a GDScript reference and leaves the object attached to the vehicle. It
  now detaches the controller from the vehicle, which is what "invalid reference" means to the
  server.

## 2026-09-23 - a GDScript subclass silently replaced the native _ready()

* **Symptom:** right after `Cabin3D` moved from GDScript to C++, the camera stopped entering the
  cab at all - not "the interior is missing", but nothing happened. No error, no crash, every
  cabin test green.
* **Cause:** the C++ class implemented `_ready()` as the GDExtension virtual and emitted
  `cabin_ready` from it, which `RailVehicle3D::enter_cabin()` waits on with a one-shot
  connection. `DynamicTrainCabin` (GDScript) defines `_ready()` too - and a script that defines a
  virtual **replaces** the native implementation rather than adding to it. The native `_ready()`
  therefore never ran, the signal was never emitted, and the camera had nothing to wait for.
  `super._ready()` is not a way out either: GDScript refuses it for a native virtual
  ("Cannot call the parent class' virtual function").
* **Fix:** `_notification()` with `NOTIFICATION_READY` / `NOTIFICATION_PROCESS`. A notification is
  delivered to the whole chain - the native class and the script both get it - so a script
  overriding `_ready()` no longer switches the base class off.
* **Rule:** when a C++ class is moved under an existing GDScript subclass, its lifecycle goes in
  `_notification()`, never in the `_ready()`/`_process()` virtuals. Those are overridable, and a
  subclass that already defines one silently takes the base out of the picture.
* **Rule:** the same move is safe for `_notification()` in both directions, which is why
  `VehiclePhysicsNode` and `VehicleController` never showed this - they have no GDScript
  subclass defining the same virtual. The trap needs a subclass to appear.
* **The second half of it, found the same way:** with the lifecycle fixed the camera entered the
  cab and the interior was still missing. `DynamicTrainCabin` also overrode `set_train_id()`, and
  that is where it rebuilt the interior - but `RailVehicle3D` now calls that method **typed**,
  which reaches `Cabin3D::set_train_id()` and never the script's. A script only shadows a native
  method for callers going through `call()`; making the call typed is exactly what removes the
  shadowing.
* **Rule:** a C++ base does not offer a subclass "override this method" unless the method is a
  registered virtual. It announces instead - `Cabin3D` emits `train_id_changed` and the subclass
  reacts - which works the same whether the caller is C++ or GDScript.

## 2026-09-23 - the loco that would not move had nobody in the cab

* **Symptom:** `test_sm42_startup_sequence::test_successful_moving_on` - "Speed should be > 0" -
  red since `87d5f8d`, through weeks of unrelated work. The startup itself was fine: the engine
  reached its revolutions and the brakes released, both asserted two lines above the failure.
* **Cause:** `TMoverParameters::ComputeTotalForce()` decides whether the vehicle is worth
  simulating at all (`Mover.cpp:4485`): `vehicleisactive` is `CabActive != 0 || Vel > 0.0001 ||
  |AccS| > 0.0001 || LastSwitchingTime < 5 || EZT || DMU`, and `switch_physics()` turns the
  integration off when none of them holds. The test built its locomotive with the default
  `cabin_number = 0`, so `VehicleController::initialize_mover()` never called `CabActivisation()`
  - by the original's own rule, an unmanned vehicle stays inactive (`Driver.cpp:2126`). For the
  first five seconds `LastSwitchingTime` kept the physics alive, which is why the early
  assertions passed; the test then waits 5 s + 5 s + 1 s + 2 s, and by the time it looks at the
  speed the Mover had long stopped integrating.
* **Fix:** the test occupies the cab (`cabin_number = 1`), which is what a startup sequence is -
  a driver operating the locomotive.
* **Rule:** a vehicle that is not driven is not simulated, on purpose. Before treating "it does
  not move" as a physics or a wiring bug, check `CabActive`/`PhysicActivation` - the backend
  switches itself off and reports nothing.
* **Rule:** a red test that survives many unrelated commits stops being evidence of the commit
  that turned it red. This one was read as a regression of the physics-server port for weeks; it
  was a test that had never set up a driven vehicle.

## 2026-09-23 - a parked vehicle never had its bogies placed, and four guesses before one print

* **Symptom:** both bogies of a vehicle standing on a curve carried the same tangent
  (`test_rail_vehicle_track_movement::test_bogies_follow_track_tangents`). Nudging the vehicle by
  1 mm fixed it, which made it read as a test artefact rather than a bug.
* **Four wrong hypotheses first**, each argued from reading the code: the config key names, the
  `bogie_rest_global_bases` cache, the order of `_cache_animation_bindings()` against
  `_process_dirty()`, and the duplicated vehicle handle. Two of those were real bugs, fixed in
  the same commit, and neither was this one.
* **What found it:** a `UtilityFunctions::print` of every guard at the top of
  `apply_track_placement()`, plus one inside the bogie loop. The first printed once with
  `force_detail_refresh=1` and then a dozen times with `0`; the second never printed at all. So
  the function returned at `!moved && !force_detail_refresh` on every call after the first, and
  the first had found a pivot spacing of 0.
* **Cause, and the lifecycle that explains it:** `VehiclePhysicsNode::_build()` creates the
  controller, applies the model, attaches the components and then calls `initialize()`, whose
  `initialize_mover()` pushes the wrapper's configuration into the backend before
  `vehicle_changed` is emitted. A component added **after** that - a modder's
  `GenericVehicleComponentNode`, or the wheels a test adds - configures nothing until the next
  tick dirties the Mover. `MoverVehicleWheels` publishes `bogie_pivot_spacing` by reading
  `mover->BDist`, so the one placement the vehicle got saw 0, and `moved` is false forever for a
  vehicle that is standing still.
* **Fix:** `RailVehicle3D` reacts to the controller's `mover_config_changed` signal, which is
  emitted exactly when the configuration reaches the backend, instead of polling a retry flag.
* **Rule:** a per-frame path gated on "something moved" never picks up a value that arrives late.
  Anything derived from configuration is recomputed when the configuration lands - and there is a
  signal for that; do not invent a retry flag beside it.
* **Rule:** after two hypotheses read off the code have failed, stop reading and print. Four
  reasoned guesses cost more than the one `print` that named the branch in a single run.

## 2026-09-23 - every vehicle ran with a bogie pivot spacing of zero

* **Symptom:** `test_rail_vehicle_track_movement::test_bogies_follow_track_tangents` - "bogies
  should follow different tangents on a curved track". Both bogies sat on the same tangent, as if
  the vehicle had no length at all.
* **Cause:** `MoverVehicleWheels::_fill_config_dictionary()` published its keys as **the names of
  the methods that produced them, parentheses and all** - `p_config["get_bogie_pivot_spacing()"]`,
  `p_config["get_track_width()"]`, and nine more across that file and
  `MoverVehicleUniversalController`. Every reader asks for the plain name, so
  `RailVehicle3D.cpp:1106` (`get_config().get("bogie_pivot_spacing", 0.0)`) took the default **for
  every vehicle in the game**, not only in the test: the front and rear bogie were sampled at the
  same point on the track.
* **What proved it:** reading the fill itself after the test failed. A `Dictionary.get(key,
  default)` cannot report a missing key - it returns the default and the caller carries on, so
  there is no error anywhere to grep for. The count is the whole diagnosis: 11 keys, 2 files.
* **Fix:** the keys carry the value's name (`bogie_pivot_spacing`), as every other component's
  fill already did.
* **Rule:** a state or config key is data, not a method name. When a fill is written by
  transcribing accessors - by hand or by a script - grep the result for `["get_` before trusting
  it; the mistake is invisible at the producer and silent at every consumer.
* **Rule:** `Dictionary.get(key, default)` hides a typo forever. Where a key is part of a
  published contract, the test that matters asserts the **key is present**, not merely that the
  value reads sensibly.

## 2026-09-23 - the cab's instrument backlight blinking, once per frame, from the transform

* **Symptom:** in the EP07 cab the desk backlight and the ceiling lamp, switched on, read as
  blinking 0-1-0-1 rather than lit. It looks like z-fighting between the `_on` and `_off`
  submodels, which is what sent the first guesses at the geometry and at double precision.
* **Cheapest instrument first, and it ruled out half the system:** a headless probe printing
  `vehicle_dump_state()`'s `devices_light_enabled`/`roof_light_enabled` every frame in `td.scn` -
  120 frames of `11 11 11 ...`. The state never moves, so nothing on the simulation side is
  involved and the cabin widgets are reading the right value.
* **What proved it:** the same probe with the cab entered (`RailVehicle3D::enter_cabin()`),
  sampling `visible` of every submodel a cabin widget points at. `podswietlenie_on` came out
  `000010000000000000010000000000000010...` - visible for exactly **one frame in fifteen**, and
  `podswietlenie_off` its exact complement. One writer sets it at 10 Hz (`CabinIndicator3D`
  samples every 0.1 s); another clears it every single frame.
* **Cause:** `E3DRenderingServer::instance_set_transform()` ended in `_update_if_built()`, i.e.
  the backend's `update()`, and `E3DNodesBackend::update()` does one thing only - show and hide
  the `_on`/`_off` submodels of every light from `lights_state`. So *moving* a model re-applied
  its whole light state. Harmless while only OPTIMIZED instances pushed a transform
  (`set_notify_transform(instancer == Instancer.OPTIMIZED)`, `2125898`); `9ca6b9f` made the
  notification unconditional so a model's smoke emitter would follow it, and from then on every
  node-instanced model - every cab - re-applied its lights once per frame.
* **Fix:** a transform applies the transform. `E3DInstanceBackend::apply_transform()` is its own
  operation: nothing at all for `E3DNodesBackend` (the generated tree hangs under the attached
  node and moves with it) and just `instance_set_transform` per RID for `E3DOptimizedBackend`,
  which also drops a per-frame re-resolve of the light overrides, the visibility and the layer
  mask for every optimized instance in the scenery.
* **Found on the way:** the czuwak/SHP blinker was broken by the same thing - it was flashing for
  single frames instead of blinking in ~1 s blocks, which nobody had reported as a bug.
* **Rule:** a setter applies what it is named after. Routing every `instance_set_*` through one
  "apply everything the instance knows" call makes the cheapest, most frequent change - a move -
  quietly overwrite state that a different owner set, and the damage is proportional to the frame
  rate rather than to the change.
* **Still open:** the submodels a light switches have **two** managers - the server, through
  `lights_state`, and the cab's MMD widgets, which write `Node3D.visible` directly. They agree
  today only because nothing pushes `lights_state` at a cab after it is built, and the widgets do
  nothing at all under the OPTIMIZED instancer. Recorded in `TODO.md`.

## 2026-09-22 - a teardown abort that is RID allocator corruption, not a double free

* **Symptom:** `test_zzz_ep07_cabin_main_switch` aborts during scenery teardown, in maybe half of
  the runs. Godot's own crash dump shows no extension frames at all, which sends the reader to the
  tail of a backtrace full of unresolved `main+...` offsets.
* **Two instruments that finally said something.** A `print` per group in
  `MaszynaInclude._free_owned_rids()` named the group it dies in - group 5,
  `E3DRenderingServer.instance_free`, 324 instances, and group 6 never runs. Then
  `coredumpctl debug` on the core gave the real stack, which the dump had not:
  `instance_free` -> `E3DOptimizedBackend::clear` -> a RenderingServer call taking a RID -> abort.
  It is **SIGABRT, not SIGSEGV** - the process is not dereferencing null, something is calling
  `abort()`.
* **What the errors say:** `Initializing already initialized RID`, `Attempting to initialize the
  wrong RID`, `Attempting to use an uninitialized RID`, `unimplemented base type encountered in
  renderer scene cull`. That is the RenderingServer's **RID allocator** in an inconsistent state,
  which a plain double free does not produce - it produces "Attempted to free invalid RID".
* **The mechanism, and what correlates with it:** `E3DRenderingServer::_stream_preload()` runs on
  `SceneryStreamingServer`'s **worker thread** and calls the `model_loader` Callable, which is
  GDScript (`e3d_model_manager.gd::load_model`) doing a full `load()` of a resource. Resource
  loading creates renderer resources; the main thread is freeing them at the same moment. Clearing
  `user://cache/rail_vehicle` and `fiz` reproduces it on the first runs and it stops once the cache
  is warm, because a cold cache means far more vehicles being built and therefore far more
  streaming work in flight.
* **Three real hazards found and fixed on the way, none of which was the cause** - worth keeping
  anyway, and worth knowing they are not it:
  * `_free_owned_rids()` cleared each RID list only after its whole loop while the budgeted path
    awaits a frame in the middle, so leaving the tree during that await freed the same RIDs twice;
  * 14 `X::get_instance()->` dereferences had no null check, among them `stream_free()` inside
    `instance_free()` itself and the `EXIT_TREE` pair;
  * `instance_free()` held a `HashMap` iterator across cleanup that re-enters the same server.
* **Half fixed, 2026-09-24.** The worker is now drained before a teardown frees anything:
  `SceneryStreamingServer::drain()` stops the planning thread and joins it, and
  `maszyna_include.gd::_exit_tree()` calls it before `_free_owned_rids()`. The timing is the whole
  point - the server's own destructor already joined the thread, but it runs at module
  de-initialisation, long after the scripts the worker calls into are gone, which is why quitting
  during a load crashed on a worker thread with `#0 0x0`, a jump through a Callable that no longer
  had a script. The reload path (`_clear_content()`) had been stopping streaming since `8d02b43`;
  `_exit_tree()` had not.
* **Still open:** the preload still creates renderer resources on the worker, so the race exists
  whenever a teardown overlaps a *running* stream rather than a shutdown. Parsing on the worker
  and building on the main thread is the remedy; recorded in `TODO.md`.
* **Not reproducible headlessly.** Three attempts - a `SceneTree` script, a scene, a scene with a
  registered camera - all quit mid-load without crashing. A `--script` run has no autoloads, so
  the GDScript `model_loader` is never registered and the worker never enters it at all; and the
  headless renderer does not create the resources the real one does. This one is verified by
  quitting the game during loading, not by a test.
* **Rule:** when a crash is an **abort** rather than a segfault, read the engine's error lines
  before the stack - "already initialized RID" names a corrupted allocator and points at
  concurrency, while "invalid RID" names a double free. They are different bugs and the stack
  looks the same.
* **Rule:** Godot's crash dump is not the stack. When the extension's frames are missing, take the
  core (`coredumpctl debug`) - it resolves the extension's symbols that the in-process dumper does
  not.
* **Rule:** a `Callable` handed to a server that documents "runs on the worker thread" must be read
  all the way down. `load_model` looks like parsing; it is `ResourceLoader.load()`.

## 2026-09-22 - "the C++ port made it 4x slower" was a GPU that never woke up

* **Symptom:** `td.scn` ran at about 30 fps where it had run at roughly 200, right after
  `TrackManager`/`SpatialIndex` moved from GDScript autoloads to C++, and the obvious conclusion
  was that the port was the cause.
* **Cause:** the discrete GPU had not come back from powersave, so the simulator was running on the
  integrated RX 780M. Nothing in the branch was responsible.
* **What pointed at it:** the GPU frame time reported **over 40 ms**. A CPU-side regression in
  GDScript-to-C++ ported logic cannot move the GPU's own frame time - that number alone ruled out
  every code hypothesis, and it was visible from the first measurement.
* **Cost:** two wrong diagnoses (first the minimap/TrackManager, then an `-O0` build overwriting
  an `-O2` one - a real trap, but not this one) and a profiling pass that was planned and then
  cancelled.
* **Rule:** split the frame time into CPU and GPU **before** forming any hypothesis about a
  performance regression. If the GPU time moved, the CPU-side change is not the suspect - check
  which adapter is actually rendering (`--verbose` names it) before touching code.
* **Rule:** a performance number is a measurement of the whole machine, not of the commit. Confirm
  the environment is the same - adapter, power profile, build flags - before the code is.

## 2026-09-22 - a config property and a state key of the same name are not the same value

* **Symptom:** `test_train_battery::test_successful_battery_voltage_drop_after_two_seconds` went
  red - "There should be a battery voltage drop after 2 seconds" - while every other battery test
  stayed green.
* **Cause:** while giving the components typed state properties, `battery_voltage` was taken to be
  one value published twice, so the state dump was pointed at the wrapper's existing
  `battery_voltage` config getter. It is not one value: the authored property is the **nominal**
  voltage (`_do_update_internal_mover` writes it to both `BatteryVoltage` and
  `NominalBatteryVoltage`), and the backend then drains and recharges `BatteryVoltage` at run time
  (`Mover.cpp:946`). Reading the config back reported a battery that never moves.
* **Fix:** `get_live_battery_voltage()` for the state; the authored property keeps its name. Three
  other reuses were checked the same way and do hold - the backend never writes
  `EnginePowerSource.RPowerCable.SteamPressure`, `PowerTrans` or `RAccumulator.RechargeSource`.
* **Rule:** before publishing a state value through an existing config getter, grep the backend for
  an assignment to that field. A value the simulation writes is state; a value only the wrapper
  writes is configuration, and only the second one may be read back from the wrapper's own
  property.

## 2026-09-22 - an uninitialised pointer that only a property read could reach

* **Symptom:** after the vehicle components gained typed state properties, building a vehicle from
  a FIZ crashed with signal 11 inside `PackedScene.pack()`
  (`fiz_train_controller_instancer.gd:218`). The backtrace was pure garbage symbols, and a bisect
  over the converted classes gave inconsistent answers - reverting one class "fixed" it, restoring
  another "broke" it again.
* **What proved it:** `addr2line` on the three `libmaszyna` frames of the crash dump, which the
  garbage tail of the backtrace had hidden:
  `VehicleDoors::get_locked()` -> `VehicleController::get_mover()`, called through
  `MethodBind::bind_call` - i.e. reached by a **property read**, not by the tick.
* **Cause:** `VehicleComponent::train_controller_node` was declared `VehicleController *
  train_controller_node;` with **no initialiser**. It is assigned in `NOTIFICATION_ENTER_TREE`, so
  until a component joined a vehicle the field held whatever was on the stack. Nothing ever read a
  component before it entered the tree, so the defect was unreachable - until the state became
  typed properties and `PackedScene.pack()` started reading them off a freshly constructed,
  never-parented component.
* **Fix:** `= nullptr` on the declaration. The null checks around it were already there and
  correct; they were simply never given a null.
* **Rule:** exposing state as properties makes every getter reachable at times the class was never
  written for - before `_ready()`, before `ENTER_TREE`, during `pack()`, from the inspector. Every
  member those getters touch has to be valid from the constructor, not from the first notification.
* **Rule:** a raw pointer member gets `= nullptr` at its declaration, always. "It is assigned
  before anything reads it" is an assumption about callers, and adding a caller is what breaks it.
* **Trap:** Godot's crash dump prints dozens of resolvable-looking frames from the main binary and
  only a handful from the extension, and the extension's are the unresolved hex ones. Run
  `addr2line -f -C -e <the .so> <offsets>` on those first instead of reading the tail.

## 2026-09-22 - a regex that deleted 588 lines, and the linker that caught it

* **Symptom:** after a scripted removal of five methods from `RailVehicleServer`, the build
  succeeded and the editor refused the extension with
  `undefined symbol: RailVehicleServer::vehicle_move(RID const&, double)`. Every GDScript naming a
  wrapper class then failed to parse, which reads exactly like a broken `global_script_class_cache`
  and sends the search in the wrong direction.
* **Cause:** the deletion used
  `re.search(r"\n(    /\*.*?\*/\n)?    [\w :<>*&]*?Class::name\(.*?\n    \}\n", s, re.S)`.
  The *optional* comment group carries `.*?` under `re.S`, so the engine is free to start the match
  at a newline hundreds of lines earlier, let that group span the whole distance to a comment that
  happens to sit right above the target, and hand back a match containing everything in between.
  `s.replace(match, "")` then deleted 588 of the file's 856 lines - every other method with it.
* **What proved it:** not the compiler. The file still compiled, because a missing definition is
  only an error at link time, and a GDExtension links lazily - the symbol went missing and nothing
  said so until Godot loaded the library. `diff` of `grep -oP "Class::\K\w+"` between `HEAD` and
  the working tree named the casualties in one line.
* **Fix:** the removal walks lines, finds each definition by its own signature line and deletes to
  the matching `    }` at that indentation. Re-checked the same way in the other file the same
  script had touched, which turned out to be intact.
* **Rule:** do not delete a code span with a regex whose optional prefix can match across lines.
  When a script edits source, verify structurally afterwards - compare the list of defined symbols
  against `HEAD`, or the line count - because a deletion that leaves valid syntax has no other
  symptom until link or run time.
* **Rule:** `undefined symbol` from a GDExtension means a *declared and bound* method has no
  definition. Look for a deleted or renamed definition first; the class cache and the import are
  the second question, not the first.

## 2026-09-22 - an unguarded singleton dereference only crashes at teardown

* **Symptom:** `test_zzz_ep07_cabin_main_switch` died with signal 11 while the scenery was being
  freed (`maszyna_include.gd:_free_owned_rids` -> `TrackManager.track_free`), having passed a few
  commits earlier. Nothing in the state work it was bisected against touched teardown.
* **Cause:** `RailVehicle3D` reached the new C++ servers as `TrackManager::get_instance()->...`
  and `RailVehicleServer::get_instance()->...` with no null check. `track_free()` emits
  `tracks_changed`, the vehicle's handler re-applies its start track, and by then the singletons
  can already be unregistered - so the call dereferenced nullptr. `CODE_STYLE.md` says this
  outright ("`Engine::get_singleton()->get_singleton(...)` does not guarantee a valid instance /
  pointer. Always check that the singleton pointer is not `nullptr`"); the guard was simply
  skipped while porting the call sites over from the untyped `->call()` form, which had returned
  a null `Object *` and merely warned.
* **Fix:** every `get_instance()` result in `RailVehicle3D` is taken into a local and checked.
* **Rule:** replacing `Engine::get_singleton()->get_singleton(name)->call("x")` with a typed
  `X::get_instance()->x()` removes the string, not the null. The typed form crashes where the
  untyped one used to print a warning, so the guard becomes *more* necessary, not less.
* **Rule:** a signal emitted from a free/teardown path runs handlers against a half-dismantled
  world. Anything a handler reaches for there has to be checked, whatever it is.

## 2026-09-22 - what porting an autoload to C++ actually costs, and the crash it hides

* **Context:** `TrackManager` (1023 lines) and `SpatialIndex` (55) moved from GDScript autoloads to
  C++ engine singletons, stage 2 of the #184 rework. The port itself was the small part.
* **The bulk of the work is not the port, it is what GDExtension cannot carry over:**
  * **Enums flatten.** `TrackManager.TrackType.TRACK_NORMAL` is valid for a GDScript class and not
    for a native one - it becomes `TrackManager.TRACK_NORMAL`. **370 call sites** across
    `addons/` and `demo/`.
  * **Inner classes have no equivalent.** `TrackManager.EndpointRef` and `BranchNeighbors` became
    `TrackEndpointRef` / `TrackBranchNeighbors`, and a registered class takes **no constructor
    arguments** - every `X.new(a, b)` becomes `X.new()` plus property assignments.
  * **A native class cannot expose a float or RID constant.** `RAIL_HEIGHT`, `SWITCH_MAX_OFFSET`
    and `SWITCH_OFFSET_DELAY` became read-only properties (`TrackManager.rail_height`), and
    `UNDEFINED_TRACK` became plain `RID()` at the call sites.
  * **Typed collections change shape.** `Array[Vector3]` became `PackedVector3Array` and
    `Array[EndpointIndex]` became `PackedInt32Array`, so every annotation *and* every
    `assert_eq(packed, [literal])` had to follow - GUT refuses to compare the two.
* **The crash, and why it is the point:** `test_rail_vehicle_track_movement` aborted with a core
  dump the moment the autoload was gone. `RailVehicle3D::_apply_start_track()` reached the manager
  with `get_tree()->get_root()->get_node_or_null("TrackManager")` and then called
  `->call("track_get_rid_by_name", ...)` on the result. With the autoload node gone that result is
  **nullptr**, and the untyped call dereferences it. Had it been a typed
  `TrackManager::get_instance()->track_get_rid_by_name(...)`, the compiler would have had the
  class and the null check would have been the ordinary one.
* **Rule:** the two prohibitions added in the previous entry are not style - reaching a singleton
  by node path and calling it by string name is exactly what turns "this autoload moved" into a
  segfault in an unrelated test.
* **Trap:** the first headless run after rebuilding the extension re-imports the project and can
  take minutes; the same test then runs in about 3 s. Run `--import` on its own before timing or
  before concluding that anything hangs.

## 2026-09-22 - reading the vehicle state was changing it, in four places

* **Context:** the first stage of the #184 architecture work. The state `Dictionary` is filled by
  `TrainController::_do_fetch_state_from_mover()` and by every `TrainPart`'s own fetch, and those
  fetches had quietly become the place where work happened.
* **What was found, by reading every fetch rather than by a failure:**
  * `TrainController::_consume_coupler_sounds()` **cleared `TCoupling::sounds` on the Mover**
    while filling the dictionary. The coupling events therefore belonged to whoever read the
    state first, and a second reader in the same frame got nothing.
  * `TrainBrake`'s fetch advanced a low-pass filter with `get_process_delta_time()` and stored
    the result. The published `brake_loco_pressure_fall_rate`/`rise_rate` depended on **how often
    the state was read**, which nothing at any call site says.
  * `TrainEngine` emitted `engine_start`/`engine_stop`, and `TrainSecuritySystem`
    `blinking_changed`/`beeping_changed`, from inside the fetch - comparing against "the previous
    value" pulled back out of the dictionary the fetch was filling. The signals fired on a read,
    not on a change.
* **Why it stayed invisible:** `get_state()` is guarded by `state_dirty`, so in practice there was
  exactly one fetch per tick and every one of these looked correct. The guard is what hid them;
  remove it, add a second reader, or skip a frame, and all four change behaviour.
* **Fix:** each moved to the tick that owns it - `_do_process_mover()` for the brake filter and
  for both change detections (against the part's own `previous_*` member), and
  `_handle_mover_update()` for draining the coupler flags. The fetches only read now.
* **Second finding, from the same pass:** the twelve coupler counters were **sound bookkeeping
  living in the vehicle**. They existed solely so a `TriggerMode.CHANGE` sound trigger could see a
  number go up. The vehicle now reports each event once (`coupler_attached` / `coupler_detached`,
  carrying a `CouplingElement`) and `TrainSoundSystem` keeps the counts itself, keyed by vehicle
  RID.
* **Third, found by grepping for the collision rather than by a bug report:** `power_source` was
  written by both `TrainElectricEngine` (the engine's supply) and `TrainLighting` (the lighting's
  supply). Merge order is scene-tree child order, i.e. FIZ section order, so on an electric
  locomotive with lighting whichever merged last won. Lighting now publishes
  `light_power_source`.
* **Rule:** a getter never changes state - see `CODE_STYLE.md`. A value that depends on how often
  it is read is a bug that stays invisible until a second reader appears.
* **Rule:** state that only one layer needs lives in that layer. If the layer were replaced
  wholesale, would the field go with it? Then it does not belong to the layer below.
* **Trap worth knowing:** `test_sm42_startup_sequence.gd::test_successful_moving_on` is red, and
  was red before any of this. Anything touching the physics path has to establish that baseline
  first (stash, rebuild, run) instead of assuming the red came from the change - see `TODO.md`.

## 2026-09-22 - the sound system's per-frame cost was not where the loop was

* **Context:** after the playback tick moved to a worker thread, `TrainSoundSystem._process` was
  the sound system's remaining main-thread cost, and the obvious suspect was its walk over every
  bank of the scenery, every frame.
* **Measured** (headless bench: 300 vehicles, 600 banks, 297 of them in range and updated every
  frame, `Time.get_ticks_usec()` around `_process`): **11.8 ms per frame**, split into
  `controller.state` 5.0, `_update_triggers` 4.8, `_update_brake_sounds` 1.5,
  `_ensure_brake_events` 0.23, running sounds 0.12, `set_parameters` 0.07. The walk itself -
  `_banks.values()`, `is_instance_valid`, `global_position.distance_to` over all 600 - measured
  **0.35 ms**, one thirtieth of it.
* **Where it really went:** `_update_triggers` re-read an untyped descriptor `Dictionary` per
  trigger per tick and converted its fields every time (`String(trigger.get("state_property"))`,
  `StringName(trigger.get("sound_event"))`, ...), and `_update_brake_sounds` walked
  `BrakeSfxEventFactory`'s static tables and called `_primary_source()` per event per tick.
  Those are resolved once now, into `Trigger` and `BrakeEvent` records built where a trigger is
  registered and where the brake events are built.
* **Result: 11.8 ms -> 2.35 ms** for the same workload, of which the remaining bulk is
  `TrainController.state` - a whole state Dictionary rebuilt from the Mover per controller per
  update (~17 us), which is where to look next.
* **Rule:** a loop over a big collection is what the eye finds; what the frame pays is what the
  *body* does. Split the measurement by phase before deciding what to restructure - the walk
  this change was planned around turned out to be 3% of the cost.
* **Trap in the bench, worth knowing in game:** a `RailVehicle3D` with no track computes a
  **NaN** global transform, and `NaN > culling_distance` is false - so such a vehicle is never
  culled and is updated every frame, whatever its distance. It is what made the bench measure
  the in-range path for all 300 vehicles; in game it means a vehicle whose track placement
  failed costs full price forever.

## 2026-09-22 - the sfx playback tick moved off the main thread, and what the numbers showed

* **Context:** the audio crackled, and every playing `SfxPlayer`/`SfxPlayer3D` drove its own
  GDScript `_process` (2-3 per vehicle, hundreds per scenery). The tick now runs on
  `GndSfxServer`'s worker thread and the nodes are proxies.
* **Measured** (headless bench, 200 players with four crossfading automation voices each, main
  thread `Performance.TIME_PROCESS`): **24.7 ms before, 19.7 ms after**, and inside the new one
  wait 12.2 ms, flush+observe 2.2 ms, tick (on the worker) 12.1 ms.
* **Trap - a headless bench understates a worker thread.** With nothing to render, the main
  thread has nothing to overlap with and spends the whole tick inside
  `wait_for_task_completion`, so the total barely moves. What the split shows is the real
  result: 12 ms of GDScript left the frame, and what stays on the main thread is the 2.2 ms
  that copies values into the audio nodes. In game that 12 ms runs against the frame's
  rendering.
* **The other half of it, found by the same measurement:** `modulate()` did the automation
  refresh *and* `_apply_voice_state()` for every voice of the instance synchronously, on the
  caller's thread - while the tick recomputes exactly those gains for every voice anyway
  (`_update_voice`). `TrainSoundSystem` calls `set_parameters` per vehicle per frame, so the
  duplicate was 7.6 ms of the 27.9 ms first measured. `modulate()` now only stores the values
  and raises `automation_refresh_pending`; the clips are looked at by the next tick.
* **Rule:** work that the per-frame tick redoes anyway does not belong in the synchronous API
  path as well. "Apply it now *and* apply it in the tick" is the same doubling as an immediate
  call plus a deferred one.
* **Rule:** the scene tree is not thread safe, global-scope servers are (Godot's
  "Thread-safe APIs"). So the worker writes values (`SfxVoiceSlot`) and the main thread is the
  only code that touches an `AudioStreamPlayer(3D)` - it copies the values in and reads
  `playing` / `get_playback_position()` back for the next tick. No lock: the tick is posted at
  `process_frame` and waited for at the start of the next frame, in `_physics_process` and
  `_process` alike, so the worker only ever runs while the main thread sits inside the engine.
* **Trap:** after running the project against a different checkout of an addon (a `git worktree`
  of the previous commit, to get the "before" number), `.godot/global_script_class_cache.cfg`
  no longer knows the new `class_name`s and every script using them fails to parse. Run
  `--import` before believing that error.

## 2026-09-22 - a vehicle of the previous scenery left in the strip when the search found nothing

* **Symptom:** a search with no results cleared the scenery list, the details and the trainsets,
  but the vehicle strip kept showing the vehicles of the scenery selected before it.
* **What proved it:** running the selector as a scene (`godot --headless res://probe.tscn`, a node
  that instances the screen, types into its search field and prints the state) and reading the log:
  `Invalid type in function 'set_tiles' in base 'PanelContainer (TileGrid)'. The array of argument
  1 (Array) does not have the same element type as the expected typed array argument.`
* **Cause:** the "nothing to show" path called `%TrainsetGrid.set_tiles([])`. A bare `[]` is
  refused by a parameter typed `Array[TileGrid.Tile]` when that type lives in another script, and
  the refusal is a runtime error - the call never runs, so the tiles are never freed and the grid
  never hides. An isolated probe of the same conversion *passed*, because there the element class
  was declared in the calling script; that is what made it look impossible on paper.
* **Fix:** `_show_trainset()` builds `var tiles: Array[TileGrid.Tile] = []` and has one exit that
  hands it over, empty or not. The neighbouring `set_rows([], [])` became
  `set_rows(PackedStringArray(), PackedStringArray())`.
* **Rule:** never hand a bare `[]` or `{}` to a typed collection parameter - declare the typed
  variable and pass that. A refused argument is silent in game: the state simply stays as it was,
  and the only trace is a line in a log nobody is reading.
* **Rule:** probe a screen by **running it as a scene**, not with `--script`. A `SceneTree` script
  starts without autoloads, so every script that names one (`VehicleProfileManager` here) fails to
  compile, and the nodes come up stripped of their scripts - which reads exactly like a broken
  scene and sends the search in the wrong direction.

## 2026-09-21 - smoke emitters spawned at the origin of the world

* **Symptom:** a locomotive whose model carries a `smokesource_*` submodel (sm42, st44, su45) did
  not smoke at all, while `E3DRenderingServer::get_smoke_statistics()` reported the emitter as
  built and the template as parsed (`amount` 175, `lifetime` 3.5 s).
* **Cause:** `_smoke_build()` placed the emitter with `particles_set_emission_transform()` and
  left the `RenderingServer` instance's own transform at identity. Godot's scene cull pushes an
  instance's transform into the emission transform whenever it updates the instance, so the value
  set directly was overwritten with identity and every plume spawned at the world origin.
* **Fix:** the emitter transform goes on the instance (`instance_set_transform()`), which is how a
  `GPUParticles3D` node is driven too, and `particles_set_custom_aabb()` stays in the emitter's
  local space, where the cull transforms it along with the instance.
* **Rule:** a `RenderingServer` particle system is placed through its instance, not through
  `particles_set_emission_transform()`. Configure a server-side effect the way the equivalent node
  configures it - anything the scene cull derives from the instance will be recomputed.

### The whole plume cut off in one frame on a notch change

* **Symptom:** dropping the sm42's main controller by one notch made every particle of the plume
  disappear at once, with no fade, and the smoke came back seconds later. The original never cuts
  smoke off - it stops spawning and lets what is in the air disperse.
* **Cause:** the engine-driven rate was pushed through `particles_set_amount_ratio()`. That is not
  a spawn rate: Godot's particle process deactivates every particle whose index is at or above
  `amount * amount_ratio`, so lowering the ratio kills live particles, and a ratio of 0 - which is
  what the formula gives the moment `Im` or `EnginePower` dips on a notch change - kills all of
  them in the same frame.
* **Fix:** the emitters no longer emit automatically. `particles_set_emitting()` is false and
  `E3DRenderingServer::process_smoke()`, ticked by `SmokeSourceLibrary`, accumulates
  `spawn_rate * intensity * delta` per emitter and calls `particles_emit()` that many times -
  the original's own `m_spawncount` model (`particles.cpp:157-212`). Live particles are never
  touched, so the plume thins and fades on its own.
* **The same bug twice more, in the opacity:** `dizel_fill` was first pushed into
  `ParticleProcessMaterial.color`'s alpha. That multiplies every live particle on every frame, so
  the whole plume stepped down together whenever the Mover floored `dizel_fill` at 0.05
  (`Mover.cpp:5508`). Moving it to `color_initial_ramp` changed nothing: that gradient is read
  every frame too, at a coordinate that is random per particle but fixed for its life, so editing
  the gradient reaches every particle already in the air just the same.
* **Fix:** nothing modulates opacity at runtime at all. The template's own random initial opacity
  is written into `color_initial_ramp` once, when the emitter is built, and never touched again;
  `dizel_fill` is folded into the spawn rate instead (`RailVehicle3D::_update_smoke()`), so a
  notch down means fewer new particles and the plume thins out. That is a deliberate divergence -
  the original scales a particle's opacity at birth (`particles.cpp:330`) and Godot has no
  equivalent channel that does not also reach backwards.
* **Rule:** a `ParticleProcessMaterial` uniform is *not* a spawn-time channel, whatever its name
  suggests - `color`, `color_initial_ramp` and `amount_ratio` all reach the particles already in
  the air. The only things that affect just the particles born from now on are the emission
  itself (how many, and what `particles_emit()` is handed) and constants fixed before the first
  one is born. When a change should not touch what is already flying, it goes into the rate.

### A state key published by one engine part only

* **Symptom:** while chasing the above, a plain `EngineType=DieselEngine` vehicle could never have
  smoked either: its rate came out 0 whatever the throttle.
* **Cause:** `diesel_max_rpm` was added to `TrainDieselElectricEngine`, so on a plain diesel
  `state.get("diesel_max_rpm", 0.0)` fell back to 0, the revolutions deficit went negative and the
  clamp turned it into no smoke.
* **Fix:** the key lives in `TrainDieselEngine` and reads `TMoverParameters::EngineMaxRPM()`,
  which already returns `dizel_nmax * 60` for a diesel and `DElist[MainCtrlPosNo].RPM` for a
  diesel-electric (`Mover.cpp:1099`) - one key for both.
* **Rule:** put a state key on the part that owns the concept, not on the subclass the first
  consumer happened to use, and check whether the Mover already has an accessor that covers every
  subclass.

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

## 2026-09-20 - double slips impassable and painted with the missing-texture checker

* **Symptom:** a train reaching a crossing switch (rozjazd krzyzowy) stops dead and never moves
  again, and the crossing renders a fan of wide flat quads carrying `missing_texture.png` (a
  magenta/black checker, which reads as orange under warm light). Ordinary switches are fine.
* **Proof:** a double slip is not a `track cross` node - `cross` is a road intersection in the
  original (`Track.cpp:419`, `iCategoryFlag = 2`). It is four `track switch` nodes named
  `..._a/_b/_c/_d` (`TTrack::DoubleSlip()`, `Track.cpp:2593`) plus four short `normal` connectors.
  Measuring the distance between the two branch ends of every switch in the data set
  (4 819 switches under `scenery/`) gives a strictly bimodal result: **1 494 of them (31%) are
  0.19-0.21 m apart** - all the `_a/_b/_c/_d` quarters - while ordinary switches sit at 1.5-2.0 m.
  `TrackManager._ENDPOINT_EPSILON` was 0.25 m, so every double slip fell inside it.
* **Cause:** at 0.25 m `_get_or_create_node()` merged a switch's own two branch ends into one
  topology node and `_merge_endpoint_nodes()` then chain-merged transitively (it merges by node
  identity and never re-checks the distance), collapsing all eight endpoints of a double slip into
  a single node. `RailVehiclePhysicsServer._get_motion_connection()` sees many different usable
  targets there, calls the node ambiguous and returns `null`; `_move_vehicle_state()` simply
  `break`s, so the track offset freezes with no error printed. The same collapsed node made
  `rebuild_track_stitches()` build a trackbed stitch to every one of the seven wrong partners -
  the fan of quads. The original's own tolerance is 2 cm per axis (`Equal()`, `Track.cpp:2121`).
* **Second cause (the checker):** the `.scn` sentinel `none` was stored as a material name, so
  `MaterialManager` returned a material whose texture fell back to the placeholder. The original
  keeps a null handle for it (`Track.cpp:491`) and draws no trackbed; the short connectors inside
  a switch group rely on that, because the trackbed material is borrowed from a neighbour
  (`copy_adjacent_trackbed_material()`, `Track.cpp:3326`), a port the wrapper did not have.
* **Fix:** tolerance down to the original's 2 cm, compared per axis like `Equal()`, with the
  endpoint hash given its own cell size; `none` mapped to an empty material name; and
  `copy_adjacent_trackbed_material()` ported, resolved after the topology is built.
* **Cost of the tighter tolerance:** measured over 26 960 track endpoints in `tarniowo`,
  `drawinowo` and `baltyk` - 26 683 are joined to within 2 cm, exactly **2** had their nearest
  partner in the 2-25 cm band, and 275 are genuine line ends. Scenery authors do place endpoints
  exactly; the loose tolerance bought nothing and cost every double slip.
* **Rule:** a geometric tolerance ported from the original must carry the original's value. A
  rounder, "safer" number does not forgive sloppy data - it silently merges geometry that the
  scenery deliberately placed 20 cm apart, and the failure surfaces far away from the constant.
* **Rule:** a movement step that cannot resolve the next track must say so. `break` on a null
  connection turned a topology bug into "the train just stops", which cost a screenshot and a
  full trace to locate.

## 2026-09-21 - the main brake hiss has no interior/exterior distinction to key off

* **Symptom:** asked to make the brake hiss quieter in the cab and louder outside, the obvious
  lever - a `soundproofing` -> GAIN curve per event - turned out to do nothing for the main hiss.
* **Proof:** `pipe_hiss` is built from the `airsound`..`airsound5` labels, and across the whole
  datapack (`dynamic/`) exactly **one** of 1 344 `airsound*` declarations sets `placement:`. The
  rest fall back to `MmdSoundSourceDefinition`'s default `general`, and
  `TrainSoundSystem._soundproofing()` short-circuits `general` to a constant 1.0 regardless of
  where the listener sits. So the whole interior/exterior attenuation model simply does not
  apply to the loudest brake sound there is.
* **Fix:** the pneumatic events (`pipe_hiss`, `local_brake_hiss`, `emergency_brake_hiss`) got
  their own `listener_inside` -> GAIN modulation, baked once by `BrakeSfxEventFactory`; the
  runtime feeds a plain 0/1 from the `_inside_vehicle()` it already computes.
* **Rule:** before reaching for a signal to modulate a sound with, check what the MMD data
  actually declares for that label. A parameter that is a constant for 1 343 of 1 344 sources is
  not a signal, and the placement/soundproofing model only covers labels whose author bothered
  to place them.

## 2026-09-21 - a +38 dB SfxTrack under the cab hiss, and five rounds of guessing instead of one dump

* **Symptom:** the pneumatic hiss when releasing the main brake is deafening in the cab on every
  FV4a vehicle (EU07, EP07, SU45). Changing gains - `brake_volume_factor`, a per-event
  `listener_inside` curve, the releaser's track volume - changed nothing audible, repeatedly.
* **What found it:** looking at the built bank in the editor's Remote tree. `pipe_hiss`, the
  automation on `brake_main_valve_flow` (`airsound2`), has an `SfxTrack` with
  **`volume_db = 38`**. Every other track in the bank sits at or below 0 dB. The outlier is
  visible at a glance; nothing else had to be understood first.
* **Cause:** `_signed_flow_automation()` (`brake_sfx_event_factory.gd:512`) computes
  `maximum_gain = output_scale * (offset + factor * gain_signal_max)` purely as the *divisor*
  that normalises `fade_in_curve`'s points into 0..1, and then also applies the same number as
  `track.volume_db = linear_to_db(maximum_gain)` (`:565`). What the curve just normalised away is
  multiplied straight back in. For su45's `airsound2` (`amplitude_factor` 0.05,
  `amplitude_offset` -0.01, FV4a `input_scale` 800000, `output_scale` 2.0,
  `gain_signal_max` 0.001): `factor` = 40 000, `maximum_gain` = 79.98, `linear_to_db` = **38.06
  dB** - matching the observed value to a tenth. That is a ~80x boost sitting under everything,
  which is why no multiplier further up the chain made any audible difference.
* **Fix:** not applied yet - `track.volume_db` should carry the MMD amplitude
  (`linear_to_db(max(amplitude_factor, 0.001))`, like `_track_for()` does) and `maximum_gain`
  should stay a curve divisor only. Same function also feeds `airsound`, `localbrakesound`,
  `localbrakesound2`, so all of them need re-checking after the change.
* **Rule:** when a sound is wrong, **dump the whole built bank before touching a single
  constant**: a headless tmp script that loads the scene, walks the `SfxPlayer3D` nodes, and
  prints every event's name plus each clip's `track.volume_db`. An anomaly like +38 dB among
  0 dB tracks is obvious in one listing. Five rounds of "change a multiplier, ask the operator to
  relaunch and listen" produced nothing, because a constant further up the chain cannot be
  evaluated by ear while an 80x boost sits below it.
* **Rule:** a gain that was derived as a normalisation divisor must never also be applied as a
  gain. If a value appears both in a curve's denominator and in a `volume_db`, that is the bug.

## 2026-09-21 - distant buildings cut out of the fogged sky, whatever the fog distance

* **Symptom:** at fog 100%, rain 100% and a fog distance of 330 m the skyline still shows: every
  distant building, tree and hill is a flat silhouette against the sky instead of dissolving into
  the fog.
* **What proved it:** reading the screenshot's pixels. The sky is *exactly* `(149, 113, 95)` over
  its whole area - it takes the fog colour in full - while every distant object saturates at
  `(170, 133, 113)`, brighter than the fog itself and identical from one object to the next. A
  fogged object can only come out brighter than the fog when its fog amount is above 1.
* **Cause:** the node's `fog_density` was applied twice. `apply_visual_configuration()` scaled
  Skydome's own `day/night_fog_density` by `fog_density / FOG_REFERENCE_DENSITY` *and* handed the
  same `fog_density` to `weather.storm_fog_intensity`; Skydome adds the two
  (`Skydome.gd:1265`, clamped at 1.5) into `Environment.fog_density`. At the slider's 100% that is
  1.03-1.13, and Godot's depth fog does not clamp `fog_amount = pow(fog_z, curve) * fog_density` -
  a fully fogged pixel becomes `1.1 * fog colour - 0.1 * its own colour`, so it is brighter than
  the sky and still carries its own silhouette.
* **Second half of it:** the sky's fog share is `fog_sky_affect` alone, while geometry at
  `fog_distance` takes `fog_density`. The wrapper computed `fog_sky_affect` from
  `fog_sky_height / fog_distance` and lerped it to 1.0 with the rain, never looking at the
  density - so a light fog under a downpour put a fully fogged sky behind barely fogged terrain,
  the same seam the other way round.
* **Fix:** the day/night density stays Skydome's own haze (0.005/0.02) and the storm boost carries
  only the rest (`fog_density - base_density`), so the sum is the wanted opacity and never passes
  1.0; `sky_affect` is multiplied by that opacity.
* **Rule:** an opacity that is summed from two sources has to be summed where it is *set*, not
  where it is used - and a value the engine does not clamp (`fog_density` over 1.0) turns a
  blend into an extrapolation, which is why the artefact looked like a lighting bug and not like
  too much fog.
* **Rule:** the sky and the geometry in front of it are fogged by two different shaders with two
  different parameters (`fog_sky_affect` vs `fog_density`). They only agree when the sky's share
  carries the depth fog's opacity; every horizon seam starts here.

## 2026-09-21 - the whole scenery unlit, day and night, since the OPTIMIZED instancer

* **Symptom:** no street lamp and no lit window anywhere in a scenery is ever lit, at any hour.
  `stary_jawor_noc.scn` starts at 21:12 and the town is pitch black.
* **What proved it:** the data declares the lights plainly - of the 975 files with a model-node
  `lights` block, the modes used across the whole data set are `ls_Dark` 3180 times, `ls_Off`
  1797, `ls_On` 733, `ls_Home` 607 and `ls_Blink` 15. `stary_jawor_noc` alone places 1001
  light-bearing models, 1430 light groups and 452 declared `FREE_SPOTLIGHT` submodels. None of it
  reached the renderer.
* **Cause, in three independent places:**
  * `e3d_parser.cpp` hides every `light_on*` submodel, so only `lights_state` can show it;
  * `lights_state` lived on the **node** (`e3d_model_instance.gd`), and since `2125898` scenery
    models are RIDs with no node at all - nothing could set it. It worked in `demo_3d` only
    because that scene places `E3DModelInstance` nodes;
  * `maszyna_node_model_importer.gd` parsed `lights`/`lightcolors` and threw them away
    (`obj.lights` commented out since `923b293`), and `MaszynaModelData` had no field for them.
* **Fix:** the light state moved into `E3DRenderingServer` - declared modes per instance,
  resolved against a time of day and a light level pushed by `MaszynaEnvironmentNode`, with the
  node left as a proxy. Real lights (spot/omni) are RIDs owned by the server and streamed through
  `SceneryStreamingServer` with a range of their own, far shorter than the model's.
* **Rule:** state that an instancer is meant to honour belongs to the server, not to the node that
  happens to create the instance. The moment a second instancer appeared without nodes, every
  feature parked on the node silently stopped existing - and silently, because a light that is
  merely never switched on looks exactly like a light that was never implemented.
* **Rule:** identifying what a model contains is not the instancer's job either. Both backends
  were walking the tree to pair `light_onNN` with `light_offNN`; that walk is now
  `E3DLightFactory::discover()` and the backends only render what it lists.

### Godot's spot cone stops at 90 degrees, the data's does not

* `Light3D::PARAM_SPOT_ANGLE` is capped just under 90. Of the 871 `FREE_SPOTLIGHT` submodels in
  the data set exactly 6 are wider - `elektryczne/lampa_parkowa01` at 117 degrees (38 of them in
  `stary_jawor_noc`), `nastawnie/nastawnia_laziska_huta_lh1` at 150, and four more nastawnie.
  Those become omni lights; a spot would have silently rendered a wrong cone.
* **Rule:** before mapping an engine parameter one to one, check the range of the values the data
  actually holds. Six outliers in 871 are invisible in a spot check and obvious in a histogram.

### A street lamp that lights nothing

* Nine models named `latarnia*` carry a light but **no** `FREE_SPOTLIGHT` submodel at all - in the
  original they never lit the scene either, `TP_FREESPOTLIGHT` only draws a glare billboard
  (`opengl33renderer.cpp:4646`). They do model where the light goes: a halo billboard at the lamp
  head carrying the only non-identity matrix in the model, and a quad on the ground spanning the
  lit patch. Both are found by their material (`elektryczne/poswiata`, `elektryczne/light1|2`),
  never by name - `latarnial_str` calls its halos `pos11/pos22/pos33` while the other eight call
  them `plane02/plane04/plane06`, and the pool is `placek` in eight of them and `plane01` in the
  ninth.
* The halo also carries the lamp's colour, so nothing has to be invented: mercury blue
  `(0.61, 0.59, 1.0)` for `betdziur`, sodium orange `(1.0, 0.66, 0.18)` for `lbc`/`str`, warm
  white `(0.90, 0.84, 0.64)` for `drew`/`hs`.
* **Rule:** a model that declares no light may still say exactly where its light falls. Read the
  geometry the author drew for the glow before adding a tuning constant.

### The lit patch says how wide the cone is, not where the light ends

* **Symptom:** the synthesized street lamps were there but barely visible - a huge, dim pool with
  no lamp at its centre, and you had to walk right up to one to see anything.
* **Three separate causes, all found by measuring the nine `latarnia*` models rather than by
  looking at the screen again:**
  * **Two heads, one light.** The four `latarniay_*` models are two-armed and carry a halo at each
    end (z of -0.76 and +0.76 on `latarniay_str`, -0.99/+0.99 on `latarniay_betdziur`). The quirk
    took the first halo it found, so half of every double lamp was unlit and the one light it did
    make sat off to one side. 26 of the 124 quirk lamps in `stary_jawor_noc` are of this kind.
  * **The cone was measured along the wrong axis.** The lit patch is 15.0 m across in **every one
    of the nine models**, single- and double-armed alike, while its other axis is stretched to
    cover the arms - 15.1 m with one, 18.0-22.0 m with two. Taking `max(x, z)` therefore widened
    the cone of exactly the double lamps that already had the wrong number of lights. Only the
    across axis describes a single head.
  * **The range was the patch edge.** The patch marks where the light is still meant to be
    *visible*, so using it as `LIGHT_PARAM_RANGE` - where the light dies - left the whole pool in
    the dimmest part of the falloff. The street lamps in this data set that do declare a
    spotlight put the range at 40 m (`elektryczne/lampa_parkowa01`, mounted at 4.9 m) or 80 m
    (`linia053/lamp-y`, `lamp-5`, `lamp-i`).
* **Rule:** when geometry stands in for a light, separate what it actually measures from what it
  merely suggests. The patch's width is data; its edge is not a falloff radius.
* **Rule:** a constant that is identical across every model in a family (15.0 m here) is the one
  the author meant; a value that varies with the model's shape is describing something else.

### A RenderingServer light is not a Light3D - it inherits none of the node's defaults

* **Symptom:** switching shadows on for the scenery lights striped the whole station square with
  regular bands radiating from the lamp - shadow acne, not a cone.
* **What proved it:** the project already sets `maszyna/rendering/lights_shadow_reverse_cull_face`
  to `false` in `demo/project.godot`, so the obvious suspect was ruled out on paper - and removing
  the `light_set_reverse_cull_face_mode()` call changed nothing, while putting it back (passing
  that same `false`) fixed it. Setting it *explicitly* was the fix, which means the light had
  started with it on.
* **Cause:** `SpotLight3D`/`OmniLight3D` set their parameters in their own constructors.
  `RenderingServer::spot_light_create()` hands back a light carrying the server's defaults
  instead, and those are not the same - reverse cull face is on, and the shadow biases differ
  from the 0.03 (spot) / 0.1 (omni) and normal bias 1.0 a node would use.
* **Fix:** every shadow parameter the scenery lights rely on is now set explicitly right after the
  light is created, next to the colour, range and energy.
* **Rule:** when a feature is ported from a node to a RenderingServer RID, assume **nothing**
  carries over. Read the node's constructor and set each parameter it sets; a default that
  happens to match is luck, and the ones that do not match surface as a rendering artefact far
  from the code that caused it.
* **Trap:** a project setting being read does not mean its value is being applied. Here the
  setting said `false`, the code read `false`, and the light was still culling in reverse -
  because nobody had ever written that `false` into the light.

## 2026-09-21 - "the release runs old GDScript" - the release was never unpacked into the game dir

* **Symptom:** after `make release-linux` and unpacking, `./reloaded` in the game directory ran the
  track code from before the last fix, and clearing the caches changed nothing.
* **What proved it:** file identity, not reasoning about the export. The `.so` inside the zip is
  byte-identical to `demo/bin/libmaszyna/linux/libmaszyna.64.so`, the embedded pck carries the
  `build_number.txt` of that same compile - so the export is fresh. But
  `/mnt/ArchiwumX/Games/MaSzyna/reloaded` had mtime 23:11 and md5 `5aa4a837...`, while the freshly
  built one was `4fa78d7a...`, and the repo root held untracked `reloaded`, `libmaszyna.64.so`
  and both zips.
* **Cause:** `upgrade-linux.sh` / `upgrade-windows.sh` started with
  `cd <repo> && make ... && cp bin/linux/<zip> ./ && unzip -o <zip>`. After the `cd`, `./` is the
  repo, so every upgrade unpacked the new build **into the repo** and the game directory kept
  running whatever had been unpacked there last.
* **Fix:** the scripts build with `make -C "$REPO" release-linux` (no `cd`) and unpack with
  `unzip -o "$REPO/bin/linux/<zip>" -d "$GAME"`, where `$GAME` is the directory holding the script.
* **Rule:** when a build "has no effect", first prove that the binary being run is the binary that
  was built - mtime and md5 of the file on disk, against the artifact in `bin/`. Cache, export and
  packing are the second question, not the first.
* **Rule:** a shell one-liner that both `cd`s and uses a relative destination has two working
  directories in it. Name the destination absolutely.

## 2026-09-21 - no fog in the exported release, perfect fog in the editor

* **Symptom:** the same scenery at the same time of day: in `godot-double demo/` the fog is a clean
  gradient, in the exported release there is none at all at a fog distance of 80 m and a total
  white-out at 4160 m. "As if Skydome were not there - moving the clouds slider only makes milk."
* **Ruled out first, in this order:** the release binary was fresh (`reloaded` and
  `libmaszyna.64.so` in the game directory carried the timestamp of the zip in `bin/linux`); the
  export preset excludes only `addons/gut`, `examples` and `tests`; and the symlinked addons
  (`gnd_skydome`, `gnd_weather`, `gnd_sfx`, `libmaszyna` are symlinks into `vendor/`) **are**
  exported with their content - the pck holds `Skydome.gdc`, `WeatherNode.gdc` and the shaders.
  Godot follows the symlinks.
* **Two dead instruments on the way:** `grep -c` on the exported binary counts *lines*, and a
  binary has almost none, so every count it gave was meaningless; and `FileAccess.file_exists()`
  on a `.gd` inside a pck is always false, because GDScript is stored there as `.gdc` beside a
  `.gd.remap`. Inspect a pck by loading it with `ProjectSettings.load_resource_pack()` in a
  throwaway project and walking it with `DirAccess`.
* **Cause:** `Script.get_property_default_value()` returns **null for every property** of a
  GDScript compiled into an exported pck. `SkydomeSettings.get_value()` uses exactly that as its
  fallback, and in a release the `gnd_skydome/*` project settings do not exist at all - only the
  addon's own EditorPlugin ever registers them. So every look value the wrapper read came back
  null and `float(null)` is 0.0: fog densities, fog distance begins and volumetric lengths all
  became zero, while the editor, where the settings do exist, looked right.
* **Isolated with a control:** `maszyna_model_data.gd` - `extends Resource` with two plain
  `@export` vars and no dependencies - returns null for its defaults from the pck as well, so it
  is the compiled script, not Skydome's dependencies.
* **Fix:** `GndSkydomeMaszynaEnvironment._skydome_value()` reads the project setting and falls back
  to the live value on the Skydome node, which carries the real defaults as its member
  initialisers. Cached on the first read, because five of the six call sites write the same
  property back scaled and would otherwise compound it. The vendored addon is untouched.
* **Rule:** a default that only exists in a script's source does not survive the export. Anything
  read through `get_property_default_value()` is null in a release; fall back to a live object's
  own value instead.
* **Rule:** a project setting registered by an EditorPlugin does not exist in an exported build
  unless it was written into `project.godot`. `add_custom_project_setting()` deliberately keeps
  values equal to the default out of that file, so those are exactly the ones that vanish.
* **Rule:** test the invariant, not the intermediate. `test_maszyna_environment_node.gd` asserted
  `night_vol_fog_density` on its own and stayed green through a change that multiplied the optical
  depth by 4.6; it now asserts density times length, which is what the fog actually looks like.
