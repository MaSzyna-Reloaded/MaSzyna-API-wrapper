## Code style
Unless a section says otherwise, every rule here applies to GDScript and C++ alike.

> [!IMPORTANT]  
> Exported/All classes used in Godot do live in the `godot` namespace   
> Your linter must be compatible with code style defined in `.clang-format`
### Naming
1. Function naming: `snake_case`
2. Variable naming: `snake_case`
3. Class naming: `PascalCase`
4. Parameter naming: `snake_case` with `p_` as an prefix
### Indentation and braces
1. Braces at the end of line (K & R style)
2. As less nesting as possible
3. Indentation: 4 spaces
4. Indentation inside class privacy declarations
```cpp
//Example code matching this style
void set_door_open(bool p_state) {
    if (condition) {
        door_state = p_state;
        return;
    }
    //Do something else
    return;
}
```
```gdscript
func _process(delta):
    return delta * 2
```
### GDScript
The short GDScript rules (singleton guards, `/root/...`, `is_connected()`, setters and `_dirty`,
`not ... == ...`, `TrainSystem.send_command`) are listed in `AGENTS.md`.

1. **A signal of a scene node is connected in the scene.** If the node stands in the `.tscn`, its
   signal goes into the scene's `[connection]` list - not into `_ready()`. The wiring then sits
   where the node does, the editor keeps it correct when the node is renamed or moved, and it
   exists before any script runs. `connect()` in code is for nodes the script instantiates itself
   (a row built in a loop, a player added by hand).
2. **A long node path in code is an antipattern.** `get_node("A/B/C/D")`, `$A/B/C` and worst of
   all a `../..` that climbs out of the node's own scene: the node is then pinned to a layout it
   does not own, and moving one container breaks it. Inside its own scene a node is reached by its
   unique name (`%Name`); anything outside comes in through the scene root's own signals and
   methods.

   A scene `[connection]` is a different matter and is *not* an offender: it stores a path from
   the scene root, and the `../../..` the editor's node dock shows is only how the editor draws
   where the receiver sits relative to the emitter. Nothing to clean there.

   What is worth cleaning is the tree itself: a container that wraps a single child earns nothing
   and lengthens every path through it, and a name has to say what the node is - `VehiclesScroll`,
   not `ScrollContainer`; `SceneryPanel`, not `ListPanel` when four lists share the screen.
3. **GDScript is interpreted and `_process` is not free.** Anything recurring is written in this
   order of preference:
   1. **C++** - a singleton connects itself to `SceneTree`'s `process_frame` and does the work
      natively (`SceneryStreamingServer::_process_streaming()`,
      `E3DRenderingServer::_process_smoke()`). No script runs per frame at all.
   2. **A `Timer`** - the engine fires the callback, so nothing is interpreted between ticks. Use
      it whenever the work is periodic and the node carrying it is a single one (an autoload, a
      system, a screen). Not when it would be one `Timer` per instance of something there are many
      of - that trades interpretation for nodes.
   3. **`_process` with a delta accumulator** - only when the work genuinely has to look at every
      frame and cannot move to C++.
   A bare `_process` that runs every frame to do a handful of calls is the thing to avoid: the
   interpreter costs more than the calls. Whichever of the three it ends up being, it is still
   bound by "Per-frame work" below.

### A case that every receiver branches on is not a parameter

**First ask what the receiver does with the value.** When every listener opens by branching on it,
the branch *is* the signal - emit one per case and let each listener connect to the one it cares
about. That removes the parameter, the branch in every receiver, and the dead half of each handler:

```gdscript
# not this - one signal carrying a side, and an "if" at the top of every listener
signal navigate_out(edge: Edge)

# this - the screen connects only what means something to it
signal navigate_left
signal navigate_right
```

**What does travel is an enum, never a number.** A mode that is passed on, stored or compared
carries its own type; a number standing for a case is semantic rubbish - the call site cannot be
read, nothing checks the value, and the reader has to open the emitter to learn what it meant:

```gdscript
# not this
func set_detail(level: int) -> void:   # 0? 2? the caller reads like arithmetic

# this
enum Detail { LOW, HIGH, OPTIMIZED }
func set_detail(level: Detail) -> void:
```

An `int` parameter is for something counted or offset - a row step, a size, an index. A direction,
a side, a mode or a state is never one. The same goes for a `bool` that names nothing at the call
site (`build(true)`): make it an enum or split the function.

Names themselves come from the vocabulary of the data and of the original engine: a `.scn` declares
`trainset`, so the code says `trainset` - not `consist`, not any other synonym the wrapper invents.

### A component names no path outside itself

What a reusable component preloads decides whether it can be moved or reused at all:

```gdscript
# not this - the component knows where it lives and who uses it
const MARKER: Shader = preload("res://ui/selection_marker.gdshader")
const UI_SOUNDS: SfxBank = preload("res://startup/ui_sounds.tres")

# this - its own asset relative to itself, the user's asset as a slot the user fills
const MARKER: Shader = preload("selection_marker.gdshader")
@export var sounds: SfxBank = null
```

A relative `preload` survives the component being renamed, moved or lifted into another project; an
absolute `res://` does not. And whatever belongs to the user rather than to the component - a sound
bank, a theme, a texture, a scene to spawn - is an `@export` its scene fills in, the same rule the
addon follows towards `demo/`.

### Exclusive state has one manager, and every state has an owner

**Exclusive state** is state only one thing may hold at a time: the focused section of a screen,
the open modal, the selected row, the active camera. It needs one manager, and the manager is the
only code that hands it out:

```gdscript
# not this - the section lights itself up, and nobody dims the one that had the focus
[connection signal="navigate_down" from="…/Vehicles" to="…/Actions" method="grab_section_focus"]

# this - the section asks, the screen decides and dims the rest
[connection signal="navigate_down" from="…/Vehicles" to="." method="focus_actions"]
```

A component that takes exclusive state for itself is not obviously wrong at the call site, and the
failure shows up as two of them holding it at once - two lit sections, two open windows.

**Every piece of state has an owner**, and it changes through a named operation of that owner
rather than by an assignment made from somewhere else:

```gdscript
# not this, somewhere in the middle of another method
_previous_section = Section.TRAINSETS

# this
reset_focus_history()
```

The name is the reason the state changed, written down once where the field lives. This holds
inside a single script as well as across objects: one writer per field, and the writing has a name.

### Do not multiply entities (DRY, KISS)

* **A private function with one call site is not a helper.** Its body belongs at that call site.
  Splitting it out hides the order of what happens and buys nothing back.
* **Never do the same thing twice to be safe.** An immediate call plus a deferred one, a direct
  call plus the same work through a signal, a condition checked in the caller and again inside the
  callee - each pair means the author did not know which one was correct. Work that out and keep
  one.
* **A wrapper that only forwards is noise.** So is a variable that is read once, a parameter that
  is always passed the same value, and state that is derivable from state already kept.

Doubling up does not make a doubtful fix more likely to work; it makes the next reader carry the
doubt as well, and it hides which of the two paths the behaviour actually comes from.

### Never create an `ensure_*` API

Not `_ensure_built()`, not `_ensure_viewport()`, not
`_ensure_sections()`, and not the same idea under a friendlier name.

A function like that re-checks and re-derives, on every call, state the code already knew at the one
moment it changed. Its cost is whatever it happens to walk, its call site says nothing about what it
changes, and the moment the state is actually set is nowhere to be found. **State is initialised
where it is created and set where it changes - once, explicitly.**

The engine's own are no pattern to copy. `ScrollContainer.ensure_control_visible()` is the example
that got this written down; where it exists, compute the value and set the property:

```gdscript
# not this
scroll.ensure_control_visible(item)

# this
scroll.scroll_vertical = int(item.position.y + item.size.y - scroll.size.y)
```

It has a failure mode on top of the cost: it reads state that a change made in the same frame has
just invalidated - a `visible` toggle, a queued re-sort - and then silently does nothing.

### Separation of concerns, enforced

This is the rule the rest of this file exists to protect.

A layer owns one kind of thing. A simulation backend owns physical quantities; a sound system owns
what is audible; a rendering server owns what is drawn; a screen owns what is on it. **State that
only one layer ever needs lives in that layer**, and the question that settles an argument is:
*if this layer were replaced wholesale, would this field go with it?*

```cpp
// not this - twelve counters on the vehicle, existing only so a sound trigger can see a change
int coupler_sound_counts[ 12 ] = {};

// this - the vehicle reports the event once, and whoever cares counts it
emit_signal( detaching ? coupler_detached_signal : coupler_attached_signal, element );
```

The failure is not untidiness. It is that the vehicle can no longer be tested without the sound
model in mind, the sound model cannot be replaced without touching the vehicle, and the field is
maintained by people who have no idea what it is for.

**The backend never appears in a public interface** - not in a method name, not in a parameter,
not in a returned type:

```cpp
// not this - "mover" is the vendored backend, and it is nobody's business out here
Dictionary get_mover_state();
void update_mover();

// this - a vehicle has state and config, and operations that change them
Dictionary get_state();
void apply_config();
```

### No magic numbers

A literal that is not self-evident from the expression around
it gets a named constant: a threshold, a limit, an index base, a conversion factor, a count, a
delay, a bitmask.

```gdscript
# not this
_coupler_events[vehicle_rid][element + 6] += 1

# this - the same code, saying why
const COUPLER_DETACH_OFFSET:int = 6
_coupler_events[vehicle_rid][COUPLER_DETACH_OFFSET + element] += 1
```

The name is where the next reader learns what the value means, and the one place it changes. Two
call sites sharing a literal by coincidence are a bug waiting for one of them to be tuned.

A value ported from the original engine carries **the original's value and a reference to where it
came from**, never a rounder number that looks safer:

```gdscript
## Original engine: Track.cpp:35 fMaxOffset
const SWITCH_MAX_OFFSET: float = 0.1
```

`FINDINGS.md` (2026-09-20) is what this rule is made of: a 0.25 m tolerance that "looked safe"
where the original used 0.02 m silently merged every double slip in the data set.

What needs no name: 0, 1, -1 and 2 used as themselves, and an index that is literally the position
being addressed.

### A getter never changes state

A `get_*`, a property getter, a `_get()` - anything a *reader*
calls - returns a value and does nothing else. It does not advance a filter, consume a flag, emit
a signal, write to another object, or build what it returns on the way out.

The failure mode is what makes this worth a rule of its own: a value computed inside a getter
depends on **how often it is read**, and nothing at the call site says so. One reader looks
correct. The bug appears when a second reader is added, or when a frame skips the read - far from
the getter, and looking like anything but a getter.

```cpp
// not this - the filter advances once per read, so the value depends on the number of readers
void TrainBrake::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
    local_brake_pressure_previous += (p_mover->LocBrakePress - local_brake_pressure_previous)
                                     * get_process_delta_time() * 5.0;
    p_state["brake_loco_pressure"] = local_brake_pressure_previous;
}

// this - the filter advances once per tick, with the tick's own delta; the getter returns it
void TrainBrake::_do_process_mover(TMoverParameters *p_mover, const double p_delta) {
    local_brake_pressure += (p_mover->LocBrakePress - local_brake_pressure) * p_delta * 5.0;
}
```

The same rule kills three more idioms seen in this codebase: consuming a flag on the Mover while
filling a state dictionary (the flag is then eaten by whoever happened to read first), emitting a
change signal from inside a fetch (the signal fires on read, not on change), and comparing against
"the previous value" pulled back out of the dictionary the fetch is filling. Change detection
belongs in the tick, against the owner's own member.

### Call a method, do not name it

When the class is known, include its header and call the
method. `Object::call("name")` gives up every check the compiler would have made - the name, the
argument count, the types - and a typo or a rename returns `null` at run time with nothing
printed. It is the same class of silent failure as a bare `[]` passed to an `Array[T]` parameter
(see `FINDINGS.md`, 2026-09-22): the call simply does not happen, and the state stays as it was.

```cpp
// not this - the header is already included two lines up, and the enum is used typed
electric_engine->call("set_pantograph_wire_voltage", TrainElectricEngine::PANTOGRAPH_FIRST, voltage);

// this
electric_engine->set_pantograph_wire_voltage(TrainElectricEngine::PANTOGRAPH_FIRST, voltage);
```

A singleton is reached the same way - a typed `static X *get_instance()` and typed methods, the
shape `TrainSystem`, `SceneryStreamingServer` and `MaszynaRuntime` already have. Not a name looked
up on an `Object`, and not `get_tree()->get_root()->get_node_or_null(name)` standing in for one.

A string call is allowed only where the class genuinely cannot be known at build time - a GDScript
node that a C++ node merely hosts - and the call site says so in a comment.

### No pointers in a public API

Applies to C++ above all, and to servers first. A public method takes and returns `RID`s,
`Variant`s and `Callable`s. A handle is a `RID`, an object is an `ObjectID`, a callback is a
`Callable`.

```cpp
// not this - the server now depends on a lifetime it does not own
RID controller_create(TrainController *p_controller);

// this - Godot's own servers are the reference
// (PhysicsServer3D::body_attach_object_instance_id)
RID  vehicle_create();
void vehicle_attach_object_instance_id(const RID &p_vehicle, uint64_t p_id);
```

A pointer that crosses a public boundary makes every caller responsible for a lifetime it did not
create, and the resulting dangle surfaces far from the code that caused it. Pointers stay inside
one class.

### Never work around a missing event

A value that is not there yet is an ordering defect, and the
things that look like a fix are all the same mistake:

```cpp
// not this - the placement could not finish, so it asks to be run again
if (pivot_spacing <= 0.0) {
    force_detail_refresh = true;   // "try again next frame"
    return;
}

// this - the owner announces that the configuration reached the backend, and the work happens
// there, once
controller->connect(VehicleController::mover_config_changed_signal,
                    callable_mp(this, &RailVehicle3D::_on_vehicle_config_changed));
```

The same applies to a deferred call added beside a direct one, a second `_ready()`-time retry, a
counter that gives up after N frames, and a `_process` that keeps checking whether something has
appeared. Each of them works often enough to survive review and leaves the real defect - the
operation that published its result before it had one, or never published it at all - in place.

Two questions settle it. *What produces this value, and has that operation finished?* If it has
not, the observer is being run too early: move it behind the event, or make the producing
operation complete before anything can observe it. *Is there an event for it?* If there is none,
add one at the owner - a signal that means "this has landed", not "this is about to happen" -
rather than polling for its effect.

### Wiring is not per-frame work

Resolving a path, finding a node, connecting a signal,
subscribing to anything: that happens **once**, where the node comes into being - `_enter_tree()`,
`_ready()`, or an init the owner calls. Never in `_process`/`_physics_process`, and a `_dirty`
flag around it does not make it acceptable - the flag only hides that the wiring is being
re-decided on a frame boundary.

```cpp
// not this - the subscription lives in the per-frame path
void Node::_process(double delta) {
    if (dirty) {
        vehicle = get_node_or_null(vehicle_path);
        vehicle->connect(changed_signal, callable_mp(this, &Node::_on_changed));
    }
}

// this - wired on entering the tree, and the frame does only frame work
void Node::_enter_tree() {
    vehicle = get_node_or_null(vehicle_path);
    vehicle->connect(changed_signal, callable_mp(this, &Node::_on_changed));
}
```

There is a second failure beyond the cost: a node that switches its processing off until the
thing it depends on exists can never subscribe to it, because the code that would subscribe is
the code that is not running. That deadlock is what this rule exists to prevent.

### Per-frame work

A loop in a native `_process` scales with the collection just
as badly, it only takes more objects to show.

**Running per frame is a last resort.** Before writing one, ask whether the work can be
event-driven instead: a signal, a setter, a `_dirty` flag consumed on the next change rather than
polled. If it truly has to run every frame:

* **No loops.** A per-frame loop makes the frame cost scale with the number of things iterated.
  Keep the path flat: mirror what the loop would have looked up onto the object that needs it, at
  the moment it changes, and let the frame do arithmetic on that alone (as
  `E3DRenderingServer::SmokeObject` carries its own transform and visibility instead of looking
  its instance up).
* No allocations, no `get_node()`, no string work, no `find`/`has` over a collection, no singleton
  or `ProjectSettings` lookups - resolve all of it once and cache it.
* Nothing at all while the work is idle: turn the processing off (`set_process(false)`, or
  disconnect from `process_frame`) when there is nothing to do, and back on when there is.
* If a per-frame loop is genuinely unavoidable, **bound it** - a fixed budget per frame, or the
  nearest N, the way `SceneryStreamingServer` spends a few milliseconds per frame and leaves the
  rest for the next one, or `E3DRenderingServer::_process_smoke()` visits at most
  `MAX_SMOKE_SOURCES_PER_FRAME` emitters and carries on round-robin.

## Classes
1. Explicit privacy declarations
```hpp
//Example of explicit privacy modifiers and indentation inside them
class Example {
    public:
        int variable
        int variable2

    protected:
        godot::String "aaa";

    public:
        void _bind_methods();
};
```
### Declarations
1. Enums - explicit
2. Namespaces - explicit
3. Classes - explicit
```cpp
namespace godot {
    class Example {
        public:
            TrainDoor::Controls controls = TrainDoor::Controls::CONTROLS_PASSENGER;
    }
}
```

### Conversions
Always use `static_cast<type>`, don't use C-style cast

### Logging
For dev logging, use and only Godot's built-in methods.  
For in-game logging, use `GameLog` but be aware that it'll only post log messages to the Dev console or anything else connected to it's `log_updated` signal. It won't print logs to the Godot's console

### Sound
This project has a sound system - the vendored `gnd-sfx` addon (`SfxBank` / `SfxEvent` /
`SfxPlayer`, `SfxPlayer3D` for positional sound). Use it. A bare `AudioStreamPlayer` with a
`preload`ed stream is not the way to add a sound, not even a single UI click.

How a bank is built:

1. One `SfxBank` resource per screen or subsystem, saved next to the assets it uses
   (e.g. `demo/startup/ui_sounds.tres`).
2. **Events are named after what happened, not after the sample or the widget**:
   `load_scenery`, `back_button`, `list_item_click`, `list_item_hover`, `apply_skin`. Code says
   `_ui_sounds.play(&"list_item_click")` and never learns which file that is - swapping the
   sample, or pointing several events at one sample, is then the bank's business alone and
   touches no code.
3. A plain one-shot needs no automation: an `SfxEvent` with `name` and one `SfxClip` in its own
   `clips` is enough. Automations are for sound driven by a continuous parameter.
4. `SfxPlayer` for non-positional sound (UI, music), `SfxPlayer3D` for anything in the world.
5. One bank may be shared by several scenes; each scene owns its own player and plays its own
   gestures, rather than reaching into another scene's player.
6. Gain belongs in the bank - in the event's own track `volume_db` or its curves. Do not add a
   global multiplier or a Project Setting on top of correctly calibrated per-event data, and
   never apply a value that a curve already normalised against (see `FINDINGS.md`, 2026-09-21).

Before changing any sound constant, dump the built bank first - every event, every clip's
`track.volume_db`, `unit_size` and `max_distance` - and look for the value that stands out. An
anomaly is visible in one listing; guessing at multipliers is not.

### Tests
Tests use only the public interface of the tested classes - no calls to private methods
(`_name()`) and no reads/writes of private members (`_name`). If a test needs private access,
treat it as a sign that the class API should be redesigned (e.g. expose a public query, split
the logic into a separate class) instead of reaching into internals.

## C++ notes

### Singletons C++

* Never inherit from RefCounted (use plain `Object` as a base)
* Register and de-register signletons in a proper order (check dependencies)
* De-registering sequence should follow the schema:
  - call `Engine::unregister_singleton()` with checking `Engine::has_singleton()`
  - call `memdelete()` if pointer is not nullptr, then assign nullptr to the pointer
  - double check order of deallocation singletons
* `Engine::get_singleton()->get_singleton("<name>")` and/or `CustomSingleton::get_instance()` does not guarantee
  a valid instance / pointer. Always check that the singleton pointer is not `nullptr`.

### Pointers, Refs and RefCounted objects

* Prefer Refs instead of raw pointers, if applicable
* Do not mix `Ref` with `memnew()` / `memdelete()`
* Instantiate Refs in the heap:
  ```
  SomeRefCountedObject obj;
  obj.instantiate();
  ```
* Avoid circular dependencies between Refs - they may cause infinite lifecycle and memory leaks
* Use raw pointers to avoid circular dependencies

### E3DModel / E3DSubModel

* when public API of `E3DModel` or `E3DSubModel` changes,
  the `E3DModel.FORMAT_VERSION` must be updated to invalidate the E3D cache automatically

### Godot properties

* Property names exposed to Godot must use canonical `snake_case` without slashes.
* A property's setter and getter must be named `set_<property_name>` and `get_<property_name>`; custom accessor names
  are not allowed.
* Inspector grouping paths may contain slashes and must be passed only through the optional grouping argument of the
  `BIND_PROPERTY_*` macros. Grouping paths are not part of the public property name.
* Collapse overlapping grouping segments in public names, for example `power/power_source` becomes `power_source`,
  not `power_power_source`.

### MAKE_* macros

* MAKE_* macros / macros.hpp are deprecated. Use straight and readable declarations.  

### Random generation

* **Do not use C/C++ native random generation (`std::rand`, `random`, etc.). Always use Godot's built-in functions such as `godot::UtilityFunctions::randf_range`, `godot::UtilityFunctions::randi`, etc.**

## Addon architecture rules

* use sub-plugins https://docs.godotengine.org/en/stable/tutorials/plugins/editor/making_plugins.html#using-sub-plugins
* place editor plugins in `addons/libmaszyna/editor` directory
* register all types and singletons in main plugin: `addons/libmaszyna/libmaszyna.gd`
* register / unregister custom types and singletons in `_enable_plugin()` and `_disable_plugin()` methods,
  see: https://docs.godotengine.org/en/stable/tutorials/plugins/editor/making_plugins.html#registering-autoloads-singletons-in-plugins
* avoid leaking instances at runtime/editor exit
