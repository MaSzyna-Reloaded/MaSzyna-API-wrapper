Planning and architectue:

* apply separation of concerns

Code generation:

* Keep code clean and do minimal code changes
* Follow DRY and KISS principles
* use english comments (if needed)
* GDSCRIPT: avoid type interference, use explicit type declaration
* GDSCRIPT: do not use `!=` in `if` conditions, use `not ... == ...` instead
* GDSCRIPT: do not update node state directly in setters; use `_dirty`, `_process`, and `_process_dirty`
* GDSCRIPT: do not add helper wrappers for simple signal connect/disconnect logic; connect signals directly in place
* GDSCRIPT: do not add private helper methods that only wrap one simple condition, loop, or single call; inline that logic at the call site unless it removes real duplication or names a non-trivial concept
* GDSCRIPT: do not wrap method callbacks in `Callable(...)` when direct signal method connection is sufficient
* GDSCRIPT: do not add singleton existence guards like `Engine.has_singleton(...)` around normal project singleton usage unless operator explicitly asks for that behavior
* GDSCRIPT: do not replace normal singleton/global access with `/root/...` lookups as a workaround
* GDSCRIPT: if a script declares `class_name`, use that class name directly for construction and type declarations; do not create `SomethingScript = preload("...")` constants just to instantiate that class
* GDSCRIPT: do not add `is_connected()` guard clutter for signal lifecycle issues; keep one direct `connect` and one matching direct `disconnect`
* C++: get project singletons through `Engine::get_singleton()->get_singleton("<name>")` or existing `get_instance()` methods; do not fetch them through `SceneTree`, `/root`, or autoload node lookup
* keep guards minimal; do not generate guard bloat or defensive condition chains when one necessary condition is enough
* do not useset/get/has_meta for accessing/saving/loading node state

General guidelines:

* IMPORTANT: do only what operator want, do not assume anything by yourself!
* IMPORTANT: do not expand requested scope, API, or stored state unless operator explicitly asks for it
* if you are not sure, ask operator for decision

Custom nodes and Godot Editor:

* place editor related code in addons/libmaszyna/editor
* use libmaszyna.gd just for bootstrapping and proxying to editor plugins
* make sure C++ singletons never inherit from RefCounted

Documentation:

* GDSCRIPT documentation belongs in code comments
* do not update `doc_classes` XML for GDSCRIPT changes

Build:

* if C++ code changes, use cmake to build c++ extension (check Makefile and compile-debug target)
* if GDSCRIPT code changes, check errors with godot (out of sandbox)

Checks:

* compile c++ plugin and check result
* run Godot in headless mode outside sandbox, look for parse errors
* when operator asks to run one GUT test case/file, do not run the full suite
* run one GUT test file directly with `godot --path demo --headless -s addons/gut/gut_cmdln.gd -gconfig= -gtest=res://tests/test_file.gd -gexit`
* when selecting one GUT test file through a directory, use `-gprefix` with the file prefix, for example `-gdir=res://tests -gprefix=test_track_manager_topology`
* run one GUT test method with `-gunit_test_name=test_method_name` in addition to the single file selection
