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
* GDSCRIPT: do not wrap method callbacks in `Callable(...)` when direct signal method connection is sufficient
* GDSCRIPT: do not add singleton existence guards like `Engine.has_singleton(...)` around normal project singleton usage unless operator explicitly asks for that behavior
* GDSCRIPT: do not replace normal singleton/global access with `/root/...` lookups as a workaround
* GDSCRIPT: do not add `is_connected()` guard clutter for signal lifecycle issues; keep one direct `connect` and one matching direct `disconnect`
* keep guards minimal; do not generate guard bloat or defensive condition chains when one necessary condition is enough
* do not useset/get/has_meta for accessing/saving/loading node state
* GDSCRIPT: do not use is_empty(), when "if not x / if x" is possible (i.e. empty strings, empty arrays)

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
