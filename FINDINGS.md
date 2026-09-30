# Findings

These are rules left behind by root causes that already cost someone a measurement. Each rule
names the entry in `docs/findings-archive.md` that has the symptom, the proof and the fix; code
comments cite those entries by date and title. Check the matching area before diagnosing
anything. Open work belongs in `TODO.md`.

## Diagnosing
* Measure the data before reading the code, and after two failed hypotheses read off the code,
  stop reading and print. *(09-24 pantograph lost the wire; 09-23 four guesses before one print)*
* Split frame time into CPU and GPU before any performance hypothesis, and confirm the adapter,
  power profile and build flags. *(09-22 GPU that never woke up)*
* Measure by phase before restructuring: the visible loop is rarely the cost. *(09-22 sound
  system's per-frame cost)*
* When a build or a code change "has no effect", prove that the binary running is the one built
  (mtime, md5), then read the cache file (`strings`, `.hash`) before any other hypothesis.
  *(09-21 release never unpacked; 09-20 low-poly interior)*
* When a cache "does not work", load an entry and run its own validity check before suspecting
  invalidation. *(09-23 game dir ".")*
* "One action late" means a read of a snapshot taken before the write. Look for the cache
  between them. *(09-23 cab one keypress late)*
* An **abort**: read the engine's error lines first. "Already initialized RID" means concurrency,
  "invalid RID" means a double free. Use `coredumpctl debug`, not Godot's dump, and
  `addr2line -f -C -e <.so>` on the extension's hex frames. *(09-22 RID allocator; 09-22
  uninitialised pointer)*
* A hang costs nothing to read: `/proc/<pid>/task/*/wchan`, and `kill -ABRT` for a symbolised
  core. A shipped build keeps its symbol table. *(09-24 shipped library had no symbols)*
* When a fix is being reinvented, `git log -S` the moved code and read the commit that made it
  first. *(09-24 simulation stepped after readers)*
* Many copies of one sound: the defect is phase, not level. Look for a start offset first.
  *(09-24 trainset ringing)*
* Prove a fix to a value by printing it where it is used, not where it is set. A later line
  can overwrite it. *(09-24 trainset ringing, the fix that did not work)*
* Before changing a sound constant, dump the whole built bank (`track.volume_db` of every clip).
  *(09-21 +38 dB SfxTrack)*
* A red test that survives many unrelated commits is no evidence of the commit that turned it red.
  *(09-23 loco with nobody in the cab)*

## Porting the original engine
* Port the whole `LoadFIZ_*` / loader function: derived counts, container sizes, fallbacks, and
  the state it sets at the end. A struct default is what a vehicle **without** that section gets.
  *(09-24 NaN forces; 09-24 spring brake)*
* One FIZ key can feed several Mover fields. Grep every `extract_value(..., "Key", ...)`.
  *(09-24 line breaker opened)*
* A value from a scenery token carries the unit the loader gives it right after parsing. Read the
  lines after `>>`. *(09-24 wire 100x too resistive)*
* A Mover method nothing in `Mover.cpp` calls is driven from DynObj.cpp/Train.cpp. Grep for its
  callers. *(09-24 induction motor never pulled)*
* The vendored engine assumed its own `stdafx.h`. Check overload-sensitive calls (`abs`, `min`,
  `max`) when something numeric just stops. *(09-24 C's abs())*
* Before porting a field that looks like data, read what the backend does with its **name**
  (`pantstate`). *(09-24 what a "load" is)*
* A ported tolerance carries the original's value, never a rounder "safer" one. A tolerance that
  papers over data becomes load-bearing once something trusts the structure. *(09-20 double
  slips; 09-24 pantograph)*
* A component's default is what a vehicle **without** the FIZ key gets - check it against the
  original's default, not against a plausible value. *(09-28 `permit_list` [0, 0, 0])*
* A ported formula carries the original's frame with it: when our loft or basis maps an axis the
  other way (profile x to the left, not `RenderLoft`'s right), every angle in that plane flips.
  Compare world coordinates of both sides, not the formulas. *(09-28 cant reversed)*
* Mirroring a loft's cross vector flips its winding: with culling off the face shows lit from
  below. A joint between two meshes takes each side's section from the code that built that mesh.
  *(09-28 switch trackbed dark, ballast wings)*
* Geometry the original does not have is a workaround until measured: a stitch lifted 1 cm
  over both beds fought them in depth, and closed gaps of 2-5 cm. *(09-28 trackbed stitches
  flickered)*
* Before caching or dropping a call as redundant, open the callee and the original line cited
  above it. *(09-20 coupled wagons drifting)*
* A tuning factor with no counterpart in the original scales the data's errors with the data.
  *(09-20 blotchy ground)*
* A vehicle that is not driven is not simulated (`CabActive`/`PhysicActivation`). *(09-23 loco
  with nobody in the cab)*
* A parser of the original's data mirrors its tolerance: skip what is not known, and let every
  section header close the open table. *(09-25 E186 cab half built)*
* A workaround in a `Mover*` call is a sign that a FIZ key is not ported yet. Before keeping one,
  read the key's default in `LoadFIZ_*`. *(09-25 pantographs raised only with the master valve forced)*
* A Mover field every engine type computes (EngineVoltage) is published by the part they all
  have, not by the first subclass that needed it. *(09-29 the E186 screen showed no line voltage)*
* Grep every `extract_value(..., "Key")` of LoadFIZ_Cntrl against the parser before calling the
  section ported. *(09-29 MainInitTime was never loaded)*
* A `Cntrl.` key belongs to the vehicle, not to one engine type. Check that it reaches the Mover
  for every `EngineType` that uses it. *(09-25 SU46 would not release its train)*
* Every `OnCommand_*` works without its gauge. A control only in `MmdSemanticCatalog` is dead in a
  cab that does not model it. *(09-20 E186 Ctrl+J)*
* A pantograph at 0 V has three causes (no wire in reach, dead wire, no contact). Report them
  separately. *(09-24 pantograph)*
* A wire chain that shares no end with a powered one is fed across the overlap by the section-ends
  pass (Traction.cpp:858). Check the span's chain and its feed before the contact. *(09-29 td.scn's
  second track dead)*
* Survey the data (a histogram) before mapping a parameter or trusting a "supported" list, and
  read the geometry drawn for a glow before adding a tuning constant. *(09-21 spot cone, street
  lamp; 09-20 missing shaders)*
* When porting a contract consumed by scripts nobody here maintains, run the real scripts against
  it. *(09-24 Python cab screens)*
* Check sun-altitude thresholds against a winter day. *(09-20 orange fog)*
* A section is parsed for every `EngineType` that has it, and a parser never reaches its engine
  node by a cast to one engine class - a failed cast drops the section without a word. Check each
  `EngineType` against its `LoadFIZ_*` case and its `readMPT*()`. *(09-27 SA134 without a gearbox)*
* A property's default is the Mover's default, not a value that looks sensible. *(09-27 SA134
  without a gearbox)*
* A loader's derived fields are part of the port: the fields it sets from the ones it read
  (`Imin = IminLo`). *(09-27 automatic start without thresholds)*
* A loop that steps a control until it gets somewhere ends on "did not move", never on "is not
  there yet" - the vehicle may refuse the step. *(09-27 the driver's update hung on a refused
  controller)*
* "Zero" of a master controller is its no-power position (`MainCtrlPowerPos()`), not position 0:
  below it a universal controller brakes. *(09-27 the SM42 stood braked)*
* A field goes where the original's loader reads it, not where its first consumer is (MCPN is
  every vehicle's, not the engine's). *(09-27 a control car had no controller)*
* A FIZ section is applied whatever the order the file gives it in: `Cntrl.` may follow
  `Engine:` (EN57 keeps it in the brake include). *(09-27 EN57 without a master controller)*
* The game data is written for Windows and `cParser`: a quoted text is one token without its
  quotes, and a file name matches letter case aside. *(09-28 timetable screens without their
  background)*
* The player takes a vehicle only once the scenery is loaded: a cab activated before the
  trainset is coupled reaches no other car (`SendCtrlToNext`). *(09-28 ED72 motor cars dead)*
* A vehicle may have several components of one type (two couplers): capture and keep them all,
  never one per type. *(09-28 ED72 tore apart on the first pull)*
* A coupled controller (`CoupledCtrl`) goes on into the field shunt: the cab's range and position
  are main + shunt (Train.cpp:985, 9410). *(09-28 ED72 stuck at 36-43 km/h)*
* A property default is the original's default for the absent key (`TCoupling::PowerFlag` is
  24V|110V, not 0) - a wrong one breaks every vehicle that omits the key. *(09-28 ED72 dead)*

* A shadow's normal bias is texel x `shadow_normal_bias` per cascade: compute it against the
  thinnest caster before tuning; two shadowed directional lights halve the atlas. *(09-27 thin
  station objects lost their sun shadows)*
* A gap the vertices do not show is a cut-out: check the texture's alpha where the UVs land. The
  trackbed's texture length is its material's `size:`, not the track's tex_length. *(09-28 the
  trackbed hung over the terrain)*
* A ground-level surface lit at a grazing angle shadows itself: take it out of the casters rather
  than raise the light's bias, and render the spot without shadows before blaming geometry.
  *(09-28 a gap between the trackbed and the terrain)*
* A RenderingServer light starts with the server's defaults, not a node's: diff the whole
  `Light3D` constructor against `_light_initialize()` - `shadow_blur` 0 zeroes a spot's depth
  bias. *(09-28 street lamps shadowed their own pool)*
* A blurry texture: compare its DDS size with the limit it loaded under before blaming mipmaps -
  the cab has its own limit, as in the original. *(09-29 blurry cab gauges)*

* A scenery model does not hide its `*_on` submodels - only a vehicle does (Model3d.cpp:2221);
  dump the model's tree before calling a light missing. *(09-29 shunting signal dwarfs never lit)*

* "The AI can drive it, the player cannot": log the AI's vehicle commands and replay them on the
  player's path - what the AI sends and the cab cannot is the gap. *(09-28 ST45 FuelStart)*

## State, ownership, events
* An action that needs two things is spent only when both exist: a call with an invalid handle is
  ignored silently, and the flag that said "still to do" is gone. *(09-30 the vehicles stood off
  their tracks in the editor)*
* A geometric value nobody publishes reads as zero, not as missing, and zero makes two things
  identical - grep for assignments to an exported property before trusting it is filled, and treat
  "both halves report the same number" as the signature. *(09-27 both pantographs at the origin;
  09-23 bogie pivot spacing of zero)*
* A value composed from two inputs that land in either order is published by **every** event that
  changes an input; one event plus a consumed dirty flag loses the race silently. *(09-27 both
  pantographs at the origin)*
* Removing an exported property needs a grep for its **reads**, not only its assignments. *(09-27
  both pantographs at the origin)*
* A command the original sends along the couplers (`SendCtrlToNext`) is sent only once the
  trainset is coupled: couple at endtrainset, after the vehicles stand on their tracks, then give
  the driver its orders. A cab switched on before that leaves the unit's other cabs inactive.
  *(09-27 EN57 vented its pipe from the rear cab)*
* Configuration never sets what the original switches at run time: the alerter is enabled by the
  cab's activation, not by a component's `enabled`. *(09-27 EN57 vented its pipe from the rear
  cab)*
* Every timer of the simulated train, the cab's relays included, runs on the simulation clock
  (`SimulationServer.simulation_advanced`), never on the frame. *(09-27 the cab's relays ran on real
  time)*
* One piece of state, one writer. Two writers that both look correct disagree only where the
  geometry shows it. *(09-24 parked vehicle jumping)*
* A getter never changes state. A value that depends on how often it is read stays invisible until
  a second reader appears. *(09-22 reading state changed it)*
* A cache keyed on a tick is correct only while the tick is its only writer. Write that premise
  down. *(09-23 cab one keypress late)*
* A config property and a state key of the same name are different when the backend writes the
  field. Grep for the assignment. *(09-22 config vs state)*
* A state/config key is data, not a method name (grep for `["get_`). `Dictionary.get(key,
  default)` hides typos, so test that the key is present. *(09-23 pivot spacing zero)*
* Config-derived values are recomputed on the config event (`VehicleServer.vehicle_config_changed`),
  never by a retry flag or a "moved" gate. *(09-23 bogies never placed)*
* A setter applies what it is named after, not "everything the instance knows". *(09-23 cab
  backlight blinking)*
* Put a state key on the part that owns the concept. *(09-21 diesel_max_rpm)*
* A feature parked on a node vanishes when an instancer without nodes appears, so the owning
  server subscribes to the events itself. *(09-24 switch blades; 09-21 scenery unlit)*
* Readiness describes built content for a specific camera revision, not an empty queue.
  *(09-20 streaming from menu camera)*
* A vehicle is configured once, before anything can see it: a controller given after it entered the
  tree restarts it and clears every coupling made meanwhile. *(09-30 couplings undone by the second
  configuration)*
* A trainset placed again in another order lets go of its old pairs first, or it closes into a ring
  and every walk along it never ends. *(09-30 reordering a trainset hung the editor)*
* A cab (the original's TTrain) is at work only for a driven vehicle: its logic is attached on
  `DriverSystem.vehicle_driven_changed`, never to every vehicle. *(09-29 every vehicle's cab ran
  each step)*

## Godot / GDExtension
* Memory that grows with a flat object count and no leak reported at exit is a referenced
  container: diff two jemalloc heap dumps before reading code, and suspect the binding's value
  types too. *(09-27 every rebuilt state dump stayed in memory)*
* A node in the scene tree keeps no pointer to another object: nodes as `ObjectID`, the vehicle by
  RID. *(09-30 "Edit FIZ" aborted the editor)*
* A `RefCounted` crosses a binding as `Ref<>`: a raw `T*` returned to GDScript takes a reference
  away and frees it. *(09-30 a raw pointer returned to GDScript freed the vehicle)*
* A `Resource`'s setter stores data; joining, registering and connecting happen where the object
  becomes live - loading a resource calls every setter. Its server handle is `_get_rid()`, never a
  `get_rid()` of its own. *(09-30 a stored vehicle description came up half a vehicle)*
* A builder hands the servers what it built by the node's handle, never through the node's exported
  properties - a script subclass of a native node saves them with the scene. *(09-30 a builder's
  paths saved into the scene)*
* A native node a script may subclass does its lifecycle work in `_notification()`: a script's
  `_enter_tree()`/`_ready()` replace the extension's. *(09-30 a script subclass shadows the native
  lifecycle)*
* Code resumed by `await` of a signal runs inside the emission: it cannot free the emitter ("Object
  is locked"). *(09-30 a test ran on the stack of the signal it awaited)*
* A node taken out of the tree and put back ("Edit FIZ") runs `_enter_tree()`/`_exit_tree()` again
  but `_ready()` once: subscribe where you unsubscribe. *(09-30 "Edit FIZ" disconnected what was
  never connected)*
* `SceneTree.process_frame` is emitted **before** every node's `_process` (measured, Godot 4.7.2):
  a C++ singleton on it steps ahead of every reader, no node needed. A node added to the root from
  an `_enter_tree()` of the main scene fails (`add_child()`, root busy). *(09-30 the simulation
  clock never ticked; corrects 09-24)*
* The Mover measures couplers from "the last refresh plus ten times the movement since": refresh
  locations and neighbours every physics sub-step, never once a frame, or the result depends on
  the frame rate. *(09-27 couplers stiffened by a long frame)*
* What the AI remembers of the tracks belongs to one way of driving: a turn or a takeover from a
  player starts it afresh. *(09-27 the AI stood at a clear signal)*
* A point of the route is passed when the train has driven up to it (distance counter), never
  because it is no longer read - a thrown switch changes the route standing. *(09-29 a thrown
  switch held the train at a clear signal)*
* A signal a train ignores (a Tm at stop) is ignored behind it as much as ahead; an `if ... else if`
  of the original stays exclusive in the port. *(09-29 a train held by a Tm at stop it had passed)*
* A control showing the vehicle's state never acts on it (a setter that acts, `button_pressed`
  that emits `toggled`); the driver never sends a command the vehicle does not have. *(09-29 a cab
  built on a running train lowered its pantograph)*
* A `.scn` key may repeat (`event2` twice): check the original's loader for a list before keeping
  a key in a Dictionary. *(09-29 a goods train left past its exit signal)*
* An event the original clears once passed (a signal at proceed, `Point.Clear()`) is cleared in the
  port - kept, it holds on what it shows later. *(09-29 a signal closing behind the train braked
  it hard)*
* The vehicle's supply voltage is held through a loss of up to 0.2 s (NoVoltTime), and the driver's
  readiness is tested against the consist on every update (a line breaker tripped while driving).
  *(09-29 EP07 rolled out of Markowo without power)*
* `lerpf()` does not return its end exactly, `std::lerp` does: a port whose result is compared
  exactly takes the end as it is. *(09-29 the driving aid flickered)*
* A model rebuilt is a view change: nothing the simulation reads lives in the node that draws it
  (a pantograph's raise), and a rebuild resets nothing. *(09-29 a model rebuilt dropped the
  vehicle's voltage)*
* A cab control is its own submodel only - a mesh under it is another control's or nobody's, as
  the original's `control_mapper::find` (`Train.cpp:64`). *(09-29 the E186 screen's OP1/OP2
  turned its page off; 09-30 its op12 still did)*
* Only an opaque submodel hides a cab control from the mouse - a translucent one is not in the
  original's pick pass (`opengl33renderer.cpp:1208`). *(09-30 E186's spring brake release could
  not be clicked through its glass cap)*
* A control whose value lives only in the cab (`CabinState`) shows that value when it is built; a
  cab rebuilt resets nothing. *(09-29 a rebuilt cab showed the E186 screen's page button off)*
* Who drives a vehicle is kept by the vehicle, not by its driver: a player may take the cab before
  the driver exists. *(09-27 the AI drove the cab the player started in)*
* Whoever attaches something shared per vehicle takes away only what it attached - another owner
  may have replaced it meanwhile. *(09-27 the AI stood still in the cab the player left)*
* Simulated time has one clock, `SimulationServer`'s: read `simulation_get_time()` or take
  `simulation_advanced(seconds)`, never a `delta * simulation_speed` of your own. *(09-27 three
  clocks)*
* A C++ class under an existing GDScript subclass keeps its lifecycle in `_notification()`, never
  in `_ready()`/`_process()`. A script shadows a native method only for `call()` callers.
  *(09-23 subclass replaced _ready())*
* Exposed properties make getters reachable before `ENTER_TREE` and during `pack()`. Raw pointer
  members are `= nullptr` at declaration. *(09-22 uninitialised pointer)*
* A typed `X::get_instance()` removes the string, not the null. Guard it, especially in handlers
  run from teardown. *(09-22 unguarded singleton)*
* Porting an autoload: enums flatten, no inner classes, no constructor arguments, no float/RID
  constants, packed arrays. *(09-22 porting an autoload)*
* Never pass a bare `[]`/`{}` to a typed collection parameter. The call silently never happens.
  *(09-22 vehicle strip)*
* Probe a screen by running it as a scene. `--script` has no autoloads. *(09-22 vehicle strip;
  09-22 RID allocator)*
* A RenderingServer RID inherits none of its node's defaults. Set every parameter the node's
  constructor sets, and place particles through the instance transform. *(09-21 RenderingServer
  light; 09-21 smoke at origin)*
* `color`, `color_initial_ramp` and `amount_ratio` reach particles already in the air. Only the
  emission itself affects new ones. *(09-21 plume cut off)*
* An emitter whose rate its owner drives spawns nothing until the owner has set it - a default
  rate spawns before the first tick, and emits on a culled system burst when it comes into view.
  *(09-28 puff from a diesel that is off)*
* A valid RID does not mean its `Node3D` is in the tree. *(09-20 E3D node leaving tree)*
* The sky and the geometry are fogged by different parameters. An opacity summed from two sources
  is summed where it is set, and unclamped values extrapolate. *(09-21 fogged sky)*
* An `EditorImportPlugin`'s save extension changes with `_get_resource_type()`. Godot reimports
  on md5, not mtime. *(09-24 .fiz stopped importing)*
* `global_script_class_cache.cfg` is updated only by an editor scan. Run `--import`. *(09-20
  new class_name; 09-22 sfx tick)*
* A node-typed property written into a `.tscn` by hand needs `node_paths=PackedStringArray(...)`
  in the node's header; without it the `NodePath` is not resolved, the property stays empty and
  the next save from the editor drops it. *(09-26 semaphore lost its model)*
* A new catalogue in the same locale re-translates nothing: `set_locale()` of an unchanged locale
  and `add_translation()`/`remove_translation()` send no `NOTIFICATION_TRANSLATION_CHANGED`.
  The code that swaps it notifies the main loop. *(09-25 catalogue swapped, UI unchanged)*

* The Mover's train brake handle has three positions and only `BrakeLevelSet()` moves them
  together, comparing with `fBrakeCtrlPos`: a second setup leaves `BrakeCtrlPosR` at lap. A pipe
  that will not charge - read `dpMainValve` first. *(09-26 FV4a handle left at lap)*

## Threads and teardown
* What points into another owner's memory asks for that owner by `ObjectID`, not by the
  singleton's name - at teardown the name goes first. *(09-30 the Mover server freed the Movers)*
* Every worker needs an owner that stops it before the scripts go. A destructor runs too late. A
  stop must not wait for the whole job, and a drain must not drop tasks someone waits on.
  *(09-24 no symbols / parser; 09-22 RID allocator)*
* A `Callable` across a thread is only as valid as its script, and `is_valid()` does not tell you.
  Read a worker `Callable` all the way down. *(09-24 parser; 09-22 RID allocator)*
* The scene tree is not thread safe; global-scope servers are. Work the tick redoes anyway does
  not also belong in the synchronous API. *(09-22 sfx tick off main thread)*

## Build, release, export
* A game or editor still running the old library writes cache entries under a version the new
  scripts bumped - rebuild with Godot closed, or bump the version again. *(09-28 ED72 cache)*
* glibc is only forward compatible. Check the highest `GLIBC_` of every shipped binary, and build
  on an old sysroot. *(09-24 Linux release glibc)*
* A path handed to `FileAccess` is absolute, or it is silently `res://`. *(09-23 game dir ".")*
* A script-source default does not survive export (`get_property_default_value()` is null), and
  EditorPlugin settings do not exist in a release. *(09-21 no fog in release)*
* A one-liner that `cd`s and uses a relative destination has two working directories.
  *(09-21 release never unpacked)*
* Code whose output is cached on disk bumps its cache tag (`structure-vN`, `CACHE_VERSION`,
  `FIZ_PARSER_FORMAT_VERSION`) in the same commit, and every `ResourceCache` is wired into "Clear
  cache". *(09-20 low-poly interior; 09-20 E186 main tank)*
* Never delete a code span with a regex whose optional prefix can match across lines, and verify
  scripted edits structurally. `undefined symbol` means a deleted definition. *(09-22 regex that
  deleted 588 lines)*
* Never apply clang-tidy `--fix` per file: a rename lands in the declaration, not in its uses
  elsewhere. `style-fix` only formats, headers are self-contained, and style-check binds the same
  double API as the build. *(09-29 style-fix broke the build)*
* The export reconverts a scene only when its own file changes; a scene instancing another,
  changed one (`[editable]` above all) ships its old diff of it. A release is exported from an
  empty `demo/.godot/exported`. *(09-29 vehicle card missing in release)*

## Tests
* A test is checked against a build without the fix, and one that cannot fail is deleted.
  *(09-24 line breaker opened; 09-24 parser)*
* Test the invariant, not the intermediate. *(09-21 no fog in release)*
* The headless dummy renderer keeps no texture data. A vehicle without mass or a track is NaN, and
  NaN never compares equal or culls. *(09-24 Python screens; 09-24 parked vehicle; 09-22 sound
  cost)*
* The headless dummy renderer's mesh storage is not thread safe: meshes created on the streaming
  worker and on the main thread at once corrupt the heap, and the crash shows later, at teardown.
  *(09-26 headless test crashes at teardown)*
* A test that drives a scenery vehicle by commands takes it from its driver and activates a cab:
  `IncMainCtrl()` refuses every step while no cab is active. *(09-28 the EP07 trip test never
  moved)*
* A test that passes alone and fails in the suite inherits a singleton's state from an earlier
  script: a test that changes a server's state (`SimulationServer.simulation_speed`, a Project
  Setting) restores it in `after_each`. Reproduce with `-gpre_run_script` setting that state.
  *(09-30 EP07 tests driven at x20)*

* Hold an `E3DModel` in a variable for as long as its submodels are used: freeing it clears
  every submodel (`E3DModel::clear()`), so `load_model(...).get_node(...)` gives a mesh-less
  submodel. *(09-29 submodels without meshes)*

## Sound
* A method bound with different arguments is still one connection: when the key changes
  (a vehicle's RID), disconnect it first. *(09-28 coupler events under a handle nothing read)*
* A cab control sounds through the cab's bank as an event placed at its submodel, never through
  its own `AudioStream` player. *(09-25 cab clicks cut each other off)*
* A gain derived as a normalisation divisor is never also applied as a gain. *(09-21 +38 dB)*
* Check what the MMD declares (`placement:`) before modulating with a parameter. *(09-21 brake
  hiss)*
* Stopping a player resets what its triggers remember about playing. A sound that is due
  out of earshot resumes past its opening bookend (sound.cpp:360). Skip a clip by not starting
  it: an Ogg playback asked to start at its end starts at 0. *(09-28 engine silent after the
  camera came back)*
* A vehicle lamp's hotspot-to-falloff band is 1 deg wide (sm42 fspot 21.5-22.5), so anything
  ramped over it switches on and off; the glare fades over its own band. *(09-28 glare blinking
  with the viewing angle)*
* `LastStationLatency` (driver `latency`) is the departure less the arrival: positive is early,
  the delay shown is its negative. *(09-29 an early freight train shown 7 min late)*
* An extension class named like an engine class (`CameraServer`) builds fine and is refused at
  run time: check the name against Godot's classes, and the `--import` log for "already
  registered". *(09-29 an extension class named like an engine class never registered)*
* When the API godot-cpp is generated from changes (`extension_api.json`, precision), rebuild
  godot-cpp from clean objects in every build dir, and check the library for undefined `godot::`
  symbols (`nm -D -C --undefined-only`). *(09-29 debug library would not load)*
* The scenery parser reads cp1250 (as the original's files are written); a byte past ASCII taken
  as a signed char became U+FFFD and every W4 of a Polish-named station (`Krzyżowa#...`) matched
  no timetable. A W4's station is cut at `#` and made plain ASCII (Event.cpp:715-719). Look for
  W4 through every include level (`ip/pkp/w4n.inc <name>`). *(09-30 Krzyżowa 2 timetable never
  moved)*
* The Mover's `DistCounter` (`total_distance`) grows only in its own movement, not with
  `vehicle_move()`/`trainset_move()`; a way driven that a test must see is measured along the
  driver's route table. *(09-30 station shown never caught up in a test)*
