Every rule marked "see `CODE_STYLE.md`" has its reasoning and examples there; the rule itself
binds as written here. Unless stated otherwise, a rule applies to GDScript and C++ alike.

Planning and architecture:

* REQUIRED: **separation of concerns, enforced, not aspired to.** A layer owns one kind of thing
  and knows nothing of the layers above it. State that only one layer needs lives in that layer.
  The test: if this layer were replaced wholesale, would the field go with it? - see `CODE_STYLE.md`
* REQUIRED: the backend a layer happens to be implemented on (e.g. the vendored Mover) **never
  appears in its public interface** - not in a method name, a parameter or a returned type

Code generation:

* Keep code clean and do minimal code changes
* Follow DRY and KISS principles
* use english comments (if needed)
* a vehicle is held by its `RailVehicleServer` RID and commanded with
  `RailVehicleServer.vehicle_send_command(vehicle_rid, ...)`; its scenery name (`train_id`) may be
  empty or repeated and is only for finding it (`vehicle_get_rid_by_name`). Call a
  `VehicleController` directly only where the composition already holds it (e.g. `VehicleComponent`s)
* for unclear/critical sections, or logic ported from the original engine instead of wrapped from Mover,
  leave a short comment pointing to the original source (e.g. `Train.cpp:8516`, `DynObj.cpp:1812`)
* GDSCRIPT: avoid type interference, use explicit type declaration
* GDSCRIPT: do not use `!=` in `if` conditions, use `not ... == ...`
* GDSCRIPT: do not use is_empty(), when "if not x / if x" is possible (i.e. empty strings, empty arrays)
* GDSCRIPT: do not update node state directly in setters; use `_dirty`, `_process`, and `_process_dirty`
* GDSCRIPT: do not add helper wrappers for simple signal connect/disconnect logic; connect signals directly in place
* GDSCRIPT: do not wrap method callbacks in `Callable(...)` when direct signal method connection is sufficient
* GDSCRIPT: do not add `is_connected()` guard clutter; one direct `connect` and one matching direct `disconnect`
* GDSCRIPT: a signal of a node that stands in a `.tscn`/`.scn` is connected **in that scene**, in
  its `[connection]` list; `connect()` in code only for nodes the script creates at runtime
* GDSCRIPT: do not add singleton existence guards like `Engine.has_singleton(...)` unless operator
  explicitly asks, and do not replace singleton/global access with `/root/...` lookups
* GDSCRIPT: never pass a bare `[]` or `{}` to a parameter typed `Array[T]` / `Dictionary[K, V]` -
  declare the typed variable and pass it (the call is silently refused, see `FINDINGS.md`)
* do not use set/get/has_meta for accessing/saving/loading node state
* a reusable component names no path outside itself: own files preloaded relative to it, the
  user's assets as `@export` slots; no absolute `res://` - see `CODE_STYLE.md`
* no long node paths in code (`get_node("A/B/C")`, `$A/B/C`, `../..`): `%Name` inside the scene,
  the scene root's signals and methods outside it. No layout-only wrapper containers; node names
  say what the node is - see `CODE_STYLE.md`
* keep guards minimal; no guard bloat or defensive condition chains when one condition is enough
* DRY and KISS, concretely: a private function with one call site is not a helper; never do the
  same action twice to be safe; no forwarding wrappers - see `CODE_STYLE.md`
* state has an owner and changes through a named operation of that owner, one writer per field,
  even inside one script - see `CODE_STYLE.md`
* exclusive state (one focused section, one open window, one selected row) has exactly one manager
  that grants and takes it; others **ask** by signal - see `CODE_STYLE.md`
* a case every receiver branches on is two signals, not a parameter (`navigate_left`,
  `navigate_right`); a choice that travels is an **enum**, never a bare number or a nameless
  `bool` - see `CODE_STYLE.md`
* names come from the vocabulary of the data and of the original engine (`trainset`, not a synonym)
* PROHIBITED: **never create an `ensure_*` API**, under any name - state is initialised where it
  is created and set where it changes - see `CODE_STYLE.md`
* PROHIBITED: **no magic numbers.** A non-self-evident literal gets a named constant; a ported
  value keeps the original's value and a source reference (`Track.cpp:35`). Exempt: 0, 1, -1, 2
  as themselves and literal positional indices - see `CODE_STYLE.md`
* PROHIBITED: **a getter never changes state** - no filter ticks, flag consumption, signals,
  writes elsewhere or lazy building in any `get_*`/property getter/`_get()` - see `CODE_STYLE.md`
* PROHIBITED: **never reach a known class through `Object::call("method_name")`**, nor a singleton
  by name or `get_tree()->get_root()->get_node_or_null(name)`; a string call only where the class
  cannot be known at build time, with a comment saying so - see `CODE_STYLE.md`
* PROHIBITED: **a public API takes and returns RIDs, Variants and `Callable`s - never raw
  pointers** (`ObjectID` for an object). Pointers stay inside one class - see `CODE_STYLE.md`
* PROHIBITED: **never work around a missing or mistimed event** - no retry, re-request, poll,
  "next frame" flag, deferred call or second attempt. Fix the order, or give the owner an event
  that says the value has landed - see `CODE_STYLE.md`
* PROHIBITED: **never wire anything up in a hot path** - no `connect`, `get_node`, path resolution
  or subscription in `_process`/`_physics_process`/a tick, not even behind `_dirty`; wire once in
  `_enter_tree()`, `_ready()` or an owner's init - see `CODE_STYLE.md`
* recurring work, in order of preference: C++ (a singleton on `SceneTree`'s `process_frame`), a
  `Timer` (not one per instance of something numerous), `_process` with a delta accumulator
* `_process` is a last resort in C++ as much as in GDScript - prefer event-driven code. What lands
  there is minimal: no loops, allocations or lookups, nothing while idle; an unavoidable loop is
  bounded by a budget or the nearest N - see `CODE_STYLE.md`

General guidelines:

* IMPORTANT: do only what operator want, do not assume anything by yourself!
* IMPORTANT: do not expand requested scope, API, or stored state unless operator explicitly asks for it
* if you are not sure, ask operator for decision
* whatever is left out of a task (not ported, skipped, deferred) goes to `TODO.md`, not only to the
  session report, so it isn't forgotten
* every significant finding (a root cause that took a measurement to find, a trap in the data or in
  the engine, a wrong assumption that cost time) goes to `docs/findings-archive.md` - symptom, what
  proved the cause, the fix and the rule - and its rule, one line, to the index in `FINDINGS.md`;
  not only to the session report

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

* use the vendored `gnd-sfx` addon, never a bare `AudioStreamPlayer`; bank-building convention in
  `CODE_STYLE.md`
* before changing any sound constant, dump the built bank first - every event with its clips'
  `track.volume_db`, `unit_size` and `max_distance` - and look for the value that stands out.
  Never test a hypothesis by changing a number and asking the operator to relaunch and listen

Checks:

* compile c++ plugin and check result
* run Godot in headless mode outside sandbox, look for parse errors
* REQUIRED: **short timeouts, never block the conversation on a run.** Pick the ceiling from what
  the command should take: `--check-only` 5 s, a probe 15 s, `--import` 45 s (run it on its own
  after a rebuild), one GUT script 60 s, an incremental `make compile-debug` 180 s - the tool's
  own timeout to match. A batch of test scripts or a probe goes to the background; do not sit in
  the turn waiting for it. A probe silent for 15 s is hung: kill it, do not wait. Never a
  windowed Godot run
* TESTS: never write a test that reads the game directory (`scenery/`, `dynamic/`, `textures/`) -
  CI has none. Everything a test needs is a fixture in `demo/tests/fixtures/` or
  `demo/tests/materials/`. A throwaway diagnostic script does not belong in `demo/tests/` either.
* TESTS: never run the whole test suite. Before a commit run only the test scripts you wrote or
  modified, one script at a time: `-gdir=res://tests/ -gselect=<script name>` (`-gtest=` does
  not filter here and runs everything)
* do not run tests or headless Godot after every edit - only before a commit, or when operator asks
* TESTS: **redirect a headless run to a file and read the file** - do not pipe it through
  `grep | head`: `head` closes the pipe, the run dies of SIGPIPE, and a pass reads as a hang.
* TESTS: a GDScript that fails to **parse** is not reported as failing - GUT ignores it, finds no
  match for `-gselect` and runs the whole directory until the timeout, so a syntax error looks
  like a hang. Parse-check first:
  `godot-double --headless --path demo --check-only -s res://tests/<file>.gd` (`--import` does
  not catch type errors).

Before every commit:

* REQUIRED: **review the diff being committed against `CODE_STYLE.md` and the rules above,
  before committing it** - `git diff --staged`, line by line. Look for: state written from outside
  its owner, a getter with a side effect, a magic number, a `->call("name")` where the class is
  known, a raw pointer or an `ensure_*` in a public API, an unguarded singleton dereference, a bare
  `[]`/`{}` handed to a typed collection, a private helper with one call site, work added to
  `_process`, and a name that does not come from the data or the original engine. Fix what the
  review finds in the same commit.

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
