Planning and architectue:

* apply separation of concerns

Code generation:

* Keep code clean and do minimal code changes
* Follow DRY and KISS principles
* use english comments (if needed)
* GDSCRIPT: avoid type interference, use explicit type declaration

General guidelines:

* IMPORTANT: do only what operator want, do not assume anything by yourself!
* IMPORTANT: do not expand requested scope, API, or stored state unless operator explicitly asks for it
* if you are not sure, ask operator for decision

Custom nodes and Godot Editor:

* place editor related code in addons/libmaszyna/editor
* use libmaszyna.gd just for bootstraping and proxying to editor plugins
* make sure C++ singletons never inherit from RefCounted

Documentation:

* after changes update Godot documentation in `doc_classes`

Build:

* if C++ code changes, use cmake to build c++ extension (check Makefile and compile-debug target)
* if GDSCRIPT code changes, check errors with godot (out of sandbox)

Checks:

* compile c++ plugin and check result
* run Godot in headless mode outside sandbox, look for parse errors
