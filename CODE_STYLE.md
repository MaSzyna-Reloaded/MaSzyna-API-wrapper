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
### Classes
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


### MAKE_* macros

* MAKE_* macros / macros.hpp are deprecated. Use straight and readable declarations.  

## Addon architecture rules

* use sub-plugins https://docs.godotengine.org/en/stable/tutorials/plugins/editor/making_plugins.html#using-sub-plugins
* place editor plugins in `addons/libmaszyna/editor` directory
* register all types and singletons in main plugin: `addons/libmaszyna/libmaszyna.gd`
* register / unregister custom types and singletons in `_enable_plugin()` and `_disable_plugin()` methods,
  see: https://docs.godotengine.org/en/stable/tutorials/plugins/editor/making_plugins.html#registering-autoloads-singletons-in-plugins
* avoid leaking instances at runtime/editor exit
