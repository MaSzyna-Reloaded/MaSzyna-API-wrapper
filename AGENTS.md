Planning and architectue:

* apply separation of concerns

Code generation:

* Keep code clean and do minimal code changes
* Follow DRY and KISS principles
* use english comments (if needed)
* GDSCRIPT: avoid type interference, use explicit type declaration
* follow CODE_STYLE.md rules!

General guidelines:

* IMPORTANT: do only what operator want, do not assume anything by yourself!
* if you are not sure, ask operator for decision
* When operator is requesting mistakes, tell him directly and double ask for confirmation

Custom nodes and Godot Editor:

* place editor related code in addons/libmaszyna/editor
* use libmaszyna.gd just for bootstraping and proxying to editor plugins

Documentation:

* after changes update Godot documentation in `doc_classes`

Build:

* use cmake to build c++ extension (check Makefile and compile-debug target)

Checks:

* compile c++ plugin and check result
* run Godot in headless mode outside sandbox, look for parse errors
