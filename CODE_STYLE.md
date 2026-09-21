## Code style
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
1. Do not add singleton existence guards like `Engine.has_singleton(...)` around normal project singleton usage unless explicitly requested
2. Do not replace normal singleton/global access with `/root/...` lookups as a workaround
3. Do not add `is_connected()` guard clutter for signal lifecycle issues; keep one direct `connect` and one matching direct `disconnect`
4. Do not update node state directly in setters; use `_dirty`, `_process`, and `_process_dirty`
5. In `if` conditions, do not use `!=`; use `not ... == ...`
6. Send train commands through the high-level `TrainSystem.send_command(train_id, ...)` API. Access a
   `TrainController` directly only where the composition already holds it (e.g. `TrainPart`s)
7. **GDScript is interpreted and `_process` is not free.** Anything recurring is written in this
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

### Per-frame work

Applies to C++ and GDScript alike - a loop in a native `_process` scales with the collection just
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
