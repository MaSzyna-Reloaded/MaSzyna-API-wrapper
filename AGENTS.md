Planning and architectue:

* REQUIRED: **separation of concerns, enforced, not aspired to.** A layer owns one kind of thing
  and knows nothing of the layers above it. A simulation backend does not hold sound bookkeeping;
  a physics server does not know what a gauge shows; a rendering server does not decide game
  rules; a UI node does not reach into a vehicle's internals. When a piece of state is only ever
  needed by one layer, it lives **in that layer** - the counters a sound system compares against
  belong to the sound system, not to the vehicle that reports the event. The test is a question:
  if this layer were replaced wholesale, would the field go with it? Then it belongs there.
  Mixing them is not a shortcut that costs style points; it is what makes a layer impossible to
  replace or test on its own.
* REQUIRED: the backend a layer happens to be implemented on **never appears in its public
  interface** - not in a method name, not in a parameter, not in a returned type. The vehicle has
  state and config and the operations that change them; that its physics happens to be the
  vendored Mover is an implementation detail with no business being named outside it.

Code generation:

* Keep code clean and do minimal code changes
* Follow DRY and KISS principles
* use english comments (if needed)
* send train commands through the high-level `TrainSystem.send_command(train_id, ...)` API; call a
  `TrainController` directly only where the composition already holds it (e.g. `TrainPart`s)
* for unclear/critical sections, or logic ported from the original engine instead of wrapped from Mover,
  leave a short comment pointing to the original source (e.g. `Train.cpp:8516`, `DynObj.cpp:1812`)
* GDSCRIPT: avoid type interference, use explicit type declaration
* GDSCRIPT: do not use `!=` in `if` conditions, use `not ... == ...` instead
* GDSCRIPT: do not update node state directly in setters; use `_dirty`, `_process`, and `_process_dirty`
* GDSCRIPT: do not add helper wrappers for simple signal connect/disconnect logic; connect signals directly in place
* GDSCRIPT: a signal of a node that stands in a `.tscn`/`.scn` is connected **in that scene**, in
  its `[connection]` list, never in a script. `connect()` in code is only for nodes the script
  creates itself at runtime
* a reusable component names no path outside itself. Its own files are preloaded relative to it
  (`preload("selection_marker.gdshader")`), so it survives being renamed, moved or lifted into
  another project; everything that belongs to whoever uses it (a sound bank, a theme, a texture, a
  scene to spawn) is an `@export` slot the user fills. An absolute `res://` inside a component
  nails it to one project layout and is the antipattern
* a long node path in code is an antipattern: `get_node("A/B/C/D")`, `$A/B/C`, and above all a
  `../..` that climbs out of the node's own scene. Reach a node of the same scene by its unique
  name (`%Name`); what is outside the scene comes in through the scene root's own signals and
  methods, never by walking up to it
* a scene tree that is deep only because of layout is an antipattern too - a container that wraps
  a single child earns nothing. Node names say what the node is: `VehiclesScroll`, not
  `ScrollContainer`; `SceneryPanel`, not `ListPanel` when four lists share the screen
* GDSCRIPT: do not wrap method callbacks in `Callable(...)` when direct signal method connection is sufficient
* GDSCRIPT: do not add singleton existence guards like `Engine.has_singleton(...)` around normal project singleton usage unless operator explicitly asks for that behavior
* GDSCRIPT: do not replace normal singleton/global access with `/root/...` lookups as a workaround
* GDSCRIPT: do not add `is_connected()` guard clutter for signal lifecycle issues; keep one direct `connect` and one matching direct `disconnect`
* keep guards minimal; do not generate guard bloat or defensive condition chains when one necessary condition is enough
* DRY and KISS, concretely: do not multiply entities. A private function called from exactly one
  place is not a helper - put its body there. Do not perform the same action twice to be safe (an
  immediate call and a deferred one, a guard in the caller repeated inside the callee, a wrapper
  that only forwards): work out which one is correct and keep that one alone
* state has an owner, and it changes through a named operation of that owner - not by writing its
  field from somewhere else. `_previous_section = Section.TRAINSETS` in some unrelated method says
  nothing; `reset_focus_history()` says why it happens. This holds inside a single script too: one
  writer per field, and the writing has a name
* state that is exclusive - one focused section, one open window, one selected row - has exactly
  one manager that grants it and takes it away. Whoever wants it **asks** (a signal), and the
  manager decides; a component that takes it for itself leaves two of them holding it, and that
  bug is invisible until two of them are lit at once
* do not useset/get/has_meta for accessing/saving/loading node state
* GDSCRIPT: do not use is_empty(), when "if not x / if x" is possible (i.e. empty strings, empty arrays)
* GDSCRIPT: never pass a bare `[]` or `{}` to a parameter typed `Array[T]` / `Dictionary[K, V]` -
  declare the typed variable and pass it. Across scripts the bare literal is refused at runtime,
  the call never happens, and the state silently stays as it was (see `FINDINGS.md`, 2026-09-22)
* a case that every receiver branches on is not a parameter - it is two signals (or two methods).
  One signal plus an `if` at the top of every listener multiplies branches for nothing; emit
  `navigate_left` and `navigate_right`, not `navigate_out(side)`
* a choice that does travel carries an **enum**, never a bare number. `navigate_out(-1)` for left
  is semantic rubbish: the caller cannot read it, nothing checks it, and the next reader has to
  find the emitter to learn what -1 means. An `int` is for something counted or offset (a row
  step, a size, an index), never for a direction, a side, a mode or a state
* names come from the vocabulary of the data and of the original engine - a `.scn` declares
  `trainset`, so the code says trainset, not a synonym invented in the wrapper
* PROHIBITED, in GDSCRIPT and in C++ alike: **never create an `ensure_*` API** - no
  `_ensure_built()`, `_ensure_viewport()`, `_ensure_sections()`, nor the same idea under a friendlier
  name. State is initialised where it is created and set where it changes, once and explicitly; it is
  not re-checked and re-derived on every call by a function that "ensures" it - see `CODE_STYLE.md`
* PROHIBITED, in GDSCRIPT and in C++ alike: **no magic numbers.** A literal that is not
  self-evident from the expression it sits in gets a named constant - a threshold, a limit, an
  index base, a conversion factor, a count, a delay, a bitmask. `+ 6` is a magic number;
  `COUPLER_DETACH_OFFSET + element` is the same code that says why. The name is the place the
  next reader learns what the value means, and the single place it changes. A value ported from
  the original engine carries the original's value **and** a reference to where it came from
  (`Track.cpp:35`), never a rounder number that looks safer - see `FINDINGS.md`. The exceptions
  are the ones that need no name: 0, 1, -1 and 2 used as themselves, and array indices that are
  literally the position being addressed - see `CODE_STYLE.md`
* PROHIBITED, in GDSCRIPT and in C++ alike: **a getter never changes state.** A `get_*`, a
  property getter, a `_get()` and anything else a reader calls must be free of side effects - it
  does not tick a filter with `get_process_delta_time()`, does not consume a flag on the Mover,
  does not emit a signal, does not write to another object, does not lazily build what it returns.
  Whatever the value needs happens where the state changes, in a named operation of its owner; the
  getter only returns it. A value that depends on **how often** it is read is a bug that stays
  invisible until a second reader appears - see `CODE_STYLE.md`
* PROHIBITED, in GDSCRIPT and in C++ alike: **never reach a known class through
  `Object::call("method_name")`.** Include the header and call the method - the compiler then
  checks the name, the arity and the argument types, and a rename becomes a build error instead of
  a runtime no-op returning `null`. A singleton is reached the same way: a server exposes a typed
  `static X *get_instance()` and typed methods, never a name looked up on an `Object`, and never
  `get_tree()->get_root()->get_node_or_null(name)` standing in for one. A string call is allowed
  only where the class genuinely cannot be known at build time (a GDScript node that a C++ node
  merely hosts), and the call site says so in a comment - see `CODE_STYLE.md`
* PROHIBITED, in GDSCRIPT and in C++ alike: **a public API takes and returns RIDs, Variants and
  `Callable`s - never raw pointers.** This holds for servers above all: a handle is a `RID`, an
  object is an `ObjectID`, a callback is a `Callable`. Godot's own servers are the reference -
  `PhysicsServer3D::body_attach_object_instance_id(RID, ObjectID)`, never a `Node *`. A pointer
  that crosses a public boundary makes the caller responsible for a lifetime it does not own, and
  the resulting dangle surfaces far from the code that caused it. Pointers stay inside one class -
  see `CODE_STYLE.md`
* PROHIBITED, in GDSCRIPT and in C++ alike: **never work around a missing or mistimed event.**
  When a value is not there yet, the answer is never a retry, a re-request, a poll, a "try again
  next frame" flag, a deferred call or a second attempt. Those hide a broken order of operations
  and they keep working just well enough that nobody finds the real defect. Fix it where it is:
  make the operation that produces the value complete before anything observes it, or give the
  owner an event that says the value has landed and react to that. A flag whose name means
  "do it again" is the smell; if one is being added, the ordering is wrong
* PROHIBITED, in GDSCRIPT and in C++ alike: **never wire anything up in a hot path.** A
  `connect`, a `get_node`, a path resolution, a subscription - none of it belongs in `_process`,
  `_physics_process` or a per-frame tick, not even behind a `_dirty` flag. Wiring happens once,
  where the node comes into being: `_enter_tree()`, `_ready()`, or an explicit init called by the
  owner. A subscription made from `_process` also cannot be waited for - a node that stops
  processing until the thing it subscribes to exists would never subscribe at all
* GDSCRIPT: interpretation costs. For anything recurring prefer, in this order: C++ (a singleton on
  `SceneTree`'s `process_frame`), then a `Timer` (unless it would be one per instance of something
  numerous), then `_process` with a delta accumulator. Never a bare per-frame `_process` doing a
  handful of calls - see `CODE_STYLE.md`
* putting work in `_process` is a last resort, in C++ exactly as much as in GDScript - prefer
  event-driven code.
  What does land there must be minimal and optimal: no loops, no allocations, no lookups
  (`get_node`, singletons, `ProjectSettings`, searches over collections), and no processing at all
  while there is nothing to do. An unavoidable per-frame loop must be bounded by a budget or by the
  nearest N. See `CODE_STYLE.md`

General guidelines:

* IMPORTANT: do only what operator want, do not assume anything by yourself!
* IMPORTANT: do not expand requested scope, API, or stored state unless operator explicitly asks for it
* if you are not sure, ask operator for decision
* whatever is left out of a task (not ported, skipped, deferred) goes to `TODO.md`, not only to the
  session report, so it isn't forgotten
* every significant finding (a root cause that took a measurement to find, a trap in the data or in
  the engine, a wrong assumption that cost time) goes to `FINDINGS.md` - symptom, what proved the
  cause, the fix and the rule it leaves behind - not only to the session report

Custom nodes and Godot Editor:

* place editor related code in addons/libmaszyna/editor
* assets of demo scenes (sounds, sfx banks, textures, materials, ...) belong in `demo/`, never in
  `addons/libmaszyna` - the addon exposes a property/slot the demo scene fills
* use libmaszyna.gd just for bootstrapping and proxying to editor plugins
* make sure C++ singletons never inherit from RefCounted

Documentation:

* GDSCRIPT documentation belongs in code comments
* do not update `doc_classes` XML for GDSCRIPT changes

Build:

* if C++ code changes, use cmake to build c++ extension (check Makefile and compile-debug target)
* if GDSCRIPT code changes, check errors with godot (out of sandbox)

Sound:

* this project has a sound system (the vendored `gnd-sfx` addon) - use it, never a bare
  `AudioStreamPlayer`. `CODE_STYLE.md` has the bank-building convention: one `SfxBank` per
  screen/subsystem, events named after the gesture and not after the sample, simple one-shots
  without automations, gain kept in the bank
* before changing any sound constant, dump the built bank first - every event with its clips'
  `track.volume_db`, `unit_size` and `max_distance` - and look for the value that stands out.
  Never test a hypothesis by changing a number and asking the operator to relaunch and listen

Checks:

* compile c++ plugin and check result
* run Godot in headless mode outside sandbox, look for parse errors
* TESTS: never write a test that reads the game directory (a scenery from `scenery/`, a vehicle
  from `dynamic/`, a texture from `textures/`). CI has no game directory, so such a test is dead
  there. Everything a test needs is a fixture in `demo/tests/fixtures/` or `demo/tests/materials/`.
  The same goes for a throwaway diagnostic script: it does not belong in `demo/tests/`.
* TESTS: never run the whole test suite. Before a commit run only the test scripts you wrote or
  modified, one script at a time: `-gdir=res://tests/ -gselect=<script name>` (`-gtest=` does
  not filter here and runs everything)
* do not run tests or headless Godot after every edit - only before a commit, or when operator asks
* TESTS: **redirect a headless run to a file and read the file** - do not pipe it through
  `grep | head`. `head` closes the pipe, the run dies of SIGPIPE part-way, and the result reads as
  a hang or a timeout when the test actually passed. The same output in a file says "Passing Tests
  2" plainly.
* TESTS: a GDScript that fails to **parse** is not reported as a failing test - GUT prints
  "Ignoring script ... because it does not extend GutTest", finds no match for `-gselect`, and
  then runs the whole directory until the timeout. So a syntax error looks exactly like a hanging
  test. Parse-check first, cheaply:
  `godot-double --headless --path demo --check-only -s res://tests/<file>.gd`. `--import` does not
  catch it: a type error only surfaces once the script's dependencies resolve.

Before every commit:

* REQUIRED: **review the diff being committed against `CODE_STYLE.md` and the rules above,
  before committing it** - `git diff --staged`, line by line, not the intention behind it and not
  after the operator points at something. Look for: state written from outside its owner, a getter with a side effect, a magic number,
  a `->call("name")` where the class is known, a raw pointer or an `ensure_*` in a public API, an
  unguarded singleton dereference, a bare `[]`/`{}` handed to a typed collection, a private helper
  with one call site, work added to `_process`, and a name that does not come from the data or the
  original engine. Fix what the review finds in the same commit.

Commit style:

* Commit messages must be written in English.
* If the change has a GitHub Issue, prefix the first line with its number in the existing `(#NUMBER) Message` format.
* The first line must name the subject and scope of the change briefly and unambiguously. Avoid generic verbs when they
  do not identify what the commit changes. A title may name an area followed by a short clarification, for example
  `Wire Devices Manager - web interface`.
* After a blank line, add 2-3 `*` bullet points as a mini changelog. State specifically what was added, changed, or
  fixed. Do not replace concrete changes with benefits or generic claims.
* Do not add a file list; the diff already provides it.
* Every commit message line, including the title and bullets, must be no longer than 80 characters. Wrap at a natural
  boundary and indent bullet continuations by two spaces.
* Do not add AI attribution, session links, or tool metadata to commit or pull request footers unless a maintainer
  explicitly requests it.
