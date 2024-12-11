# AGENTS

Planning and architecture:

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

## Mover Integration Rules

- Do not patch `src/maszyna/McZapkie/Mover.cpp` by changing the physics or coupler-force formulas unless the same change is first justified against `main`.
- When behavior diverges, assume the bug is in the wrapper/integration layer first:
  `TrackManager`, `LegacyRailVehicle`, `TrainSet3D`, track offsets, neighbour mapping, or coupler state exported into the mover.
- If `Mover` must be touched, prefer refactoring or adapting data structures to match the original expectations, not changing the underlying equations.
- Before changing `Mover` semantics, compare the relevant fragment with `main` and treat `main` as the source of truth for calculations.

## Track / Offset Semantics

- Keep one consistent offset convention across the wrapper:
  higher `track_offset` is ahead, lower `track_offset` is behind.
- In `TrainSet3D`, the first vehicle child is the lead vehicle at `start_track_offset`; trailing vehicles must be placed by subtracting spacing.
- Editor preview and runtime placement must use the same offset sign and same authored-order semantics.

## Coupler / Neighbour Debugging

- Do not guess coupler-side bugs from motion alone. First inspect runtime state.
- Use exported debug state to compare:
  `track_offset`, `track_id`, `connected_number`, `front_neighbour_*`, and `rear_neighbour_*`.
- When debugging side interpretation, distinguish two layers:
  explicit mover coupling in `Couplers[*]` vs virtual same-track neighbours in `Neighbours[*]`.
- Add or extend debug dumps in wrapper state before changing side logic, so the issue can be verified from the existing debug panel.

## Safe Change Strategy

- Prefer small, isolated fixes in wrapper code over broad changes across placement, neighbour lookup, and mover internals at once.
- After each behavioral fix, rebuild and verify with the three-vehicle scenario:
  `SM42`, `Wagon1`, `Wagon2`, including init, forward motion, collision, and decouple.
